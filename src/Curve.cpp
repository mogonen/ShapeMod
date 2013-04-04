#include <stdlib.h>
#include <GL/glut.h>
#include "Vector.h"
#include "Curve.h"
#include "ControlPoint.h"

Curve::Curve(Vec3 * pts, int size, bool closed){
	for(int i =0; i<size; i++)
		_pts.push_back(new Vec3(pts[i]));
	_isclosed = closed;
}

Curve::~Curve(){
	ControlPoint::remove(this);
}

Vec3 * Curve::toArr(int len){
	if(!len)
		len = size();
	Vec3 * arr = new Vec3[len];
	for(int i=0; i<len; i++)
		arr[i] = getP(i*1.0/(len-1.0));
	return arr;
}

Vec3 Curve::getT(double t){
	double e = 0.025;
	double t1 = (t+e)>1.0?1.0:(t+e);
	double t0 = (t-e)<0?0:(t-e);
	if (_isclosed){
		t1 = (t+e)>1.0?(t+e-1.0):(t+e);
		t0 = (t-e)<0 ? (t-e+1.0) : (t-e);
	}
	return (getP(t1) - getP(t0)).normalize();
}

double Curve::length(){
	double len=0;
	int SAMPLES = size();
	Vec3 p_pre;
	for(int i=0; i<SAMPLES; i++){
		Vec3 p = getP(i/(SAMPLES-1.0));
		if(i>0)
			len+=(p-p_pre).norm();
		p_pre.set(p);
	}
	return len;
}

void Curve::draw(int){
	//if (_draw_samples == -1)
		_draw_samples = size()*10;
	glBegin(GL_LINE_STRIP);
	for(int i=0; i < _draw_samples; i++){
		Vec3 p = getP(i*1.0/(_draw_samples-1));
		glVertex3f(p.x, p.y, p.z);
	}
	glEnd();
}

bool Curve::isOn(const Vec3& p){

	int sz = size()*10;
	for(int i=0; i<sz; i++){
		Vec3 p0 = getP(i/(sz-1.0));
		if ((p0 - p).norm() < PICK)
			return true;
	}
	return false;
}

bool Curve::isIn(const Vec3& p) {
  // works with lazy projection
  bool  oddNodes = false;
  Vec3 p1 = getP(0.0);
  Vec3 p0 = getP(0.99);
  for (double t = 0; t < 1; t+=0.1) {
    if ( (p1.y < p.y && p0.y >= p.y) ||  (p0.y < p.y && p1.y >= p.y)) {
      if  ( p1.x + (p.y-p1.y) / ( p0.y - p1.y)*( p0.x - p1.x )< p.x ) 
        oddNodes=!oddNodes; 
	}
    p0.set(p1);
	p1 = getP(t+0.1);
  }
  return oddNodes; 
}

void Curve::append(Curve* c, bool reversed, bool tip){
	vector<Vec3*>::iterator end = _pts.end();
	if (tip)
		end = _pts.begin();
	if (reversed)
		_pts.insert(end, c->_pts.rend(), c->_pts.rbegin());
	else
		_pts.insert(end, c->_pts.begin(), c->_pts.end());
}

void Curve::remove(Vec3 * p){
	//_pts.erase(std::remove(_pts.begin(), _pts.end(), p), _pts.end()); 
}

void Curve::insert(Vec3 * p, int i){
	if (i<0)
		_pts.push_back(p);
	else
		_pts.insert(_pts.begin()+i, p);
}

int Curve::getI(Vec3*p){
	int i =0;
	for(vector<Vec3*>::const_iterator it = _pts.begin(); it!=_pts.end(); it++){
		if (*it == p)
			return i;
		i++;
	}
}

double Curve::pickT(const Vec3& p, double rad){
	int s = samples();
	Vec3 p0 = getP(0.0);
	for(int i=1; i < s; i++){
		double t = i/(s-1.0);
		Vec3 p1 = getP(t);
		Vec3 p0p1 = (p1 - p0);
		double p0p1_len = p0p1.norm();
		p0p1 = p0p1 / p0p1_len; //cheaper normalizing
		double pd = (p-p0)*p0p1;
		Vec3 pp = p0 + p0p1*pd; // p projected on |p0p1|
		double d = (p - pp).norm();
		if (d < rad)
			return t +  (pd / p0p1_len) * (1/(s-1.0));		
		p0 = p;
	}
	return -1;
}

int Curve::pick(const Vec3& p, double rad, bool ends){
	Vec3 p0 = P(0);

	if( ends && (p-p0).norm() < rad )
		return 0;

	int s = size();
	for(int i=1; i < s; i++){
		Vec3 p1 = P(i);
		Vec3 p0p1 = (p1 - p0);
		double p0p1_len = p0p1.norm();
		p0p1 = p0p1 / p0p1_len;
		double pp_len = (p-p0)*p0p1;
		if (pp_len>0 && pp_len < p0p1_len){
			double d = (p - (p0 + p0p1*pp_len) ).norm();
			if (d < rad)
				return (i-1);	
		}
		p0 = p1;
	}

	if ( ends && (p-p0).norm()<rad )
		return s-1;

	return -1;
}

bool Curve::close(double d){
	if (!d || (P(0) - P(size()-1)).norm() < d)
		_isclosed = true;
	return _isclosed;
}


void CurvePoint::setPrevNext(CPPtr p, CPPtr n){
	_prev  = p;
	_next = n;
	if (_prev)
		_prev->_next = this;
	if (_next)
		_next->_prev = this;
}

CPPtr CurvePoint::go(int d){
	int inc = d>0?1:(d<0?-1:0);
	CPPtr cp = this;
	for(int i=0; i!=d; i+=inc){
		if (!cp)
			return cp;
		if (inc>0)
			cp = cp->next();
		else
			cp = cp->prev();
	}
	return cp;
}

CPPtr CurvePoint::last(){
	CPPtr last = this;
	for(;last->next();last = last->next())
		if (last->next() == this)
			break;
	return last;
}

CPPtr CurvePoint::first(){
	CPPtr first = this;
	for(; first->prev(); first = first->prev())
		if (first->prev() == this)
			break;
	return first;
}

int CurvePoint::find(CPPtr cp){

	CPPtr it_n = this;
	CPPtr it_p = this;
	int i_n = 0;
	int i_p = 0;

	while(it_n || it_p){

		if (it_n == cp)
			return i_n;

		if (it_p == cp)
			return i_p;

		if (it_n){
			it_n = it_n->_next;
			i_n++;
		}
		if (it_p){
			it_p = it_p->prev();
			i_p--;
		}
	}
	return 0;
}

Vec3 CurvePoint::go(double dist){
	if (dist*dist < 0.00001)
		return _p0;
	int dir =  (dist > 0 )?1:-1;
	CPPtr it0 = this;
	CPPtr it1 = this->go(dir);
	while(it0 && it1 && dist){
		Vec3 n = (it1->_p0 - it0->_p0);
		double d = n.norm();
		if ( d < dist)
			dist-=d;
		else{
			return it0->_p0 + n * (dist/d);
		}
		it0  = it1;
		it1 = it1->go(dir);
	}
	return it0->_p0;
}

void CurvePoint::updateNT(){
	_t0 = ((_next?_next->P():_p0) - (_prev?_prev->P():_p0)).normalize();
	//_n0 = (Eye::get()->N%_t0).normalize();
}

void CurvePoint::drawAll(){
glColor3f(0.0f,1.0f,0.0f);
	glPointSize(4.0f);
	glBegin(GL_POINTS);
	for(CPPtr cp = this; cp; cp = cp->next())	
		glVertex3f(cp->_p0.x, cp->_p0.y, cp->_p0.z);
	glEnd();
	glBegin(GL_LINE_STRIP);
	for(CPPtr cp = this; cp; cp = cp->next())	
		glVertex3f(cp->_p0.x, cp->_p0.y, cp->_p0.z);
	glEnd();

/*	glBegin(GL_LINES);
	for(CPPtr cp = this; cp; cp = cp->next()){			
		Vec3 p1 = cp->_p0 + cp->_n0*0.02;
		glVertex3f(cp->_p0.x, cp->_p0.y, cp->_p0.z);
		glVertex3f(p1.x, p1.y, p1.z);
	}
	glEnd();*/
	
}

bool CurvePoint::isIn(Vec3 p) {
  // int      i, j=polySides-1 ;
  bool  oddNodes = false;
  CPPtr cpi = this->first();
  CPPtr cpj = this->last();
  CPPtr first = cpi;
  CPPtr last = cpj;

  for (; cpi && cpi!=last; cpi = cpi->next()) {
    if ( (cpi->_p0.y < p.y && cpj->_p0.y >= p.y) ||  (cpj->_p0.y < p.y && cpi->_p0.y >= p.y)) {
      if  ( cpi->_p0.x + (p.y-cpi->_p0.y) / ( cpj->_p0.y - cpi->_p0.y)*( cpj->_p0.x - cpi->_p0.x )< p.x ) {
        oddNodes=!oddNodes; 
	  }
	}
    cpj = cpi; 
  }
  return oddNodes; 
}

BSpline::BSpline(Vec3 *pts, int size, bool closed):Curve(pts, size, closed){
	for(int i=0; i<size; i++)
		ControlPoint::create(this, _pts[i]);
}

Vec3 BSpline::getP(double T){
	int sz = _pts.size();
	Vec3 p;
	int cid = (int)(T * sz);
	double t = T * sz - cid;
	double b[3] = {((1-t)*(1-t)*0.5), (-t*t+t+0.5), (t*t*0.5)};
	for(int j=0; j < 3; j++){
		int pi = cid+j-1;
		if (!_isclosed)
			pi = (pi < 0)?0:((pi>=sz)?(sz-1):pi);
		else
			pi = (pi+sz)%sz;
		p = p + (*_pts[pi])*b[j];
	}
	return p;
}

void BSpline::exec(int cmd, void*p){
	insert((Vec3*)p);
	ControlPoint::create(this, (Vec3*)p);
	( (Vec3*)p)->print();
}

ArrCurve::ArrCurve(Vec3 * p, int s, bool resampleit){

	if (resampleit)
		_pts = ArrCurve::resample(p, s,0);
	else 
		_pts = p;
	_size = s;
	_len = 0;
	length();
	_draw_aux = false;
	_isclosed = false;
}

ArrCurve::ArrCurve(ArrCurve& ac){
	_size = ac.size();
	_pts = new Vec3[_size];
	for(int i = 0; i<_size; i++ )
		_pts[i] = ac._pts[i];
	_len = 0;
	length();
	_draw_aux = false;
	_isclosed = ac.isClosed();
}

ArrCurve::ArrCurve(double* hs, int size, Vec3 pos, Vec3 nx, Vec3 ny, double width, double height){

	_draw_aux = false;
	double min =  9999999; 
	double max = -9999999;
	for(int i=0; i<size; i++){
		if (hs[i] < min)
			min = hs[i];
		if (hs[i] > max)
			max = hs[i];
	}

	_pts = new Vec3[size];
	for(int i=0; i<size; i++)
		_pts[i] = pos + ny*( (hs[i]/(max-min))*height) + nx*(((i*1.0)/size)*width);

	_size  = size;
}

Vec3 ArrCurve::getP(double t){
	t = (t>1)?1.0:((t<0)?0:t);
	double tt =  (t*(_size-1));
	int i = (int)tt;
	double ttt = tt - i;
	return _pts[i]*(1-ttt) + _pts[i+1]*ttt;
}

Vec3 ArrCurve::getP(int i){

	if (_isclosed)
		i = (i+_size)%_size;
	else
		i = (i>=_size)?(_size-1):((i<0)?0:i);

	return _pts[i];
}

Vec3 ArrCurve::getT(int t){
	int t1 = (t+1>=_size)?(_size-1):t+1;
	int t0 = (t-1<0)?0:(t-1);
	if (_isclosed){
		t1 = (t+1+_size)%_size;
		t0 = (t-1+_size)%_size;
	}
	return (_pts[t1] - _pts[t0]).normalize();
}

double ArrCurve::length(){
	double len = 0;
	for(int i=1; i<_size; i++)
		len+=(_pts[i]-_pts[i-1]).norm();
	_len = len;
	return len;
}

void ArrCurve::draw(){
	if (_draw_aux)
		drawGLAux();
	else{
		glBegin(GL_LINE_STRIP);
		for(int i=0; i<_size; i++)
			glVertex3f(_pts[i].x, _pts[i].y, _pts[i].z);
		if (_isclosed)
			glVertex3f(_pts[0].x, _pts[0].y, _pts[0].z);
		glEnd();
	}
}

void ArrCurve::drawGLAux(){

	glColor3f(1.0, 0, 0);
	glPointSize(3.0);
	glBegin(GL_POINTS);
	for(int i=0; i<_size; i++)
		glVertex3f(_pts[i].x, _pts[i].y, _pts[i].z);
	glEnd();

	glColor3f(0, 0, 0);
	glBegin(GL_LINE_STRIP);
	for(int i=0; i<_size; i++)
		glVertex3f(_pts[i].x, _pts[i].y, _pts[i].z);
	glEnd();

	glColor3f(0, 0, 1.0);
	glBegin(GL_LINES);
	for(int i=0; i<_size; i++){
		Vec3 t = getT(i);
		Vec3 p1 = _pts[i] + t*0.02;
		//_pts[i].print();p1.print();cout<<endl;
		glVertex3f(_pts[i].x, _pts[i].y, _pts[i].z);
		glVertex3f(p1.x, p1.y, p1.z);
	}
	glEnd();

}

void ArrCurve::resample(int size){
	if (size<=0)
		size = _size;
	_len = length();
	_step = _len/(size-1.0);

	Vec3 * newp = ArrCurve::resample(_pts, _size);
	delete [] _pts;
	_pts = newp;
	_size = size;
	length();
}

void ArrCurve::resample(double step){
	int newsize = 0;
	Vec3 * newp = ArrCurve::resample(_pts, _size, step, newsize);
	delete [] _pts;
	_pts = newp;
	_size = newsize;
	length();
}

Vec3* ArrCurve::resample(Vec3 * p, int size, int newsize){
	return ArrCurve::resample(p, size, 0, (!newsize)?size:newsize );
}

Vec3* ArrCurve::resample(Vec3 * p, int size, double step, int& newsize){

	double len = ArrCurve::length(p, size);
	if (!step)
		step = len/(newsize-1.0);
	else 
		newsize = (int)(len/step)+1;
	

	Vec3 * newp = new Vec3[newsize];
	int j = 1;
	double d_tot = 0;
	Vec3 pre(p[0]);
	newp[0].set(pre);
	for(int i = 1; i < newsize; i++){
		double d = 0;
		while(d_tot<step && j<size){
			d = (p[j]-pre).norm();
			d_tot+=d;
			if (d_tot<step){
				pre.set(p[j]);
				j++;
			}
		}
		double t = (d_tot - step) / d;
		newp[i]  = p[j]*(1-t) + pre*t;
		pre.set(newp[i]);
		d_tot = 0;
	}
	cout<<"step:"<<step<<" ns:"<<newsize<<endl;
	newp[newsize-1]  = p[size-1];
	return newp;
}

Vec3* ArrCurve::resample(Curve* c, double step, int& newsize){
	double len = c->length();
	int size = c->size();
	if (!step)
		step = len/(newsize-1.0);
	else 
		newsize = (int)(len/step)+1;

	Vec3 * newp = new Vec3[newsize];
	int j = 1;
	double d_tot = 0;
	Vec3 pre(c->getP(0));
	Vec3 p;
	newp[0].set(pre);
	for(int i = 1; i < newsize; i++){
		double d = 0;
		while(d_tot<step && j<size){
			p.set(c->getP(j*1.0 / size));
			d = (p-pre).norm();
			d_tot+=d;
			if (d_tot<step){
				pre.set(p);
				j++;
			}
		}
		double t = (d_tot - step) / d;
		newp[i]  = p*(1-t) + pre*t;
		pre.set(newp[i]);
		d_tot = 0;
	}
	newp[newsize-1]  = c->getP(1.0);
	return newp;
}

double ArrCurve::length(Vec3 *p, int size){
	double len = 0;
	for(int i=1; i<size; i++){
		len+=(p[i]-p[i-1]).norm();
	}return len;
}

void ArrCurve::movingAverage(int num, bool norm){

	int end = (_isclosed)?_size+1:_size-1;
	for(int j=0; j<num; j++){
		Vec3 p0(_pts[0]); 
		Vec3 p1(_pts[1]);
		
		for(int i = 1; i<end; i++){
			int ind = i%_size;
			Vec3 p2(_pts[(i+1)%_size]);
			if (norm)
				_pts[ind] = p1.normalize()*((p0 + p1 + p2 ).norm()/3.0);
			else
				_pts[ind] = (p0 + p1 + p2 ) / 3.0; 
			p0 = p1;
			p1 = p2;
		}
	}
}

void ArrCurve::reverse(){
	for(int i =0; i<_size/2; i++){
		Vec3 tmp = _pts[i];
		_pts[i] = _pts[_size-i-1];
		_pts[_size-i-1] = tmp; 
	}
}

void ArrCurve::append(Vec3 * p, int s){

	Vec3 * newp = new Vec3[_size + s];
	for(int i=0; i<_size; i++)
		newp[i] = _pts[i];

	for(int i=0; i<s; i++)
		newp[i+_size] = p[i];
	delete [] _pts;
	_pts = newp;
	_size+= s;
}

double ArrCurveUtils::findMaxH(int i0, int i1, int& maxi){

	Vec3 p0 = _ac->getP(i0);
	Vec3 n = (_ac->getP(i1) - p0);
	double len = n.norm();
	n = n.normalize();

	double maxh = 0;
	for(int i = i0+1; i < i1; i++){
		Vec3 n1 = (_ac->getP(i) - p0);
		double hip = n1.norm();
		double a = n1*n;
		double h = sqrt(hip*hip - a*a);
		if (maxh < h){
			maxh = h;
			maxi = i;
		}
	}
	//cout<<endl<<"mnaxh:"<<maxh<<endl;
	return maxh;
}

int * ArrCurveUtils::findMinMax(int step, int& mmi){

	int i1 = _ac->size()-step;
	maxhs = new double[i1];
	int * maxis = new int[i1];

	for(int i = 0; i < i1; i++)
		maxhs[i] = findMaxH(i, i+step, maxis[i]);

	double  * maxhs_f = new double[i1];
	for(int k=0;k<3; k++){
		for(int i=1; i < i1-1; i++)
			maxhs_f[i] = (maxhs[i-1]+maxhs[i+1]+maxhs[i+1])/3.0;
		maxhs = maxhs_f;
	}

	/*int * maxmins = new int[20];
	mmi = 0;
	for(int i = 1; i < i1-1; i++)
		if ( maxhs[i] > maxhs[i-1] && maxhs[i] > maxhs[i-1]  )
			maxmins[mmi++] = maxis[i];
	
	int * newmaxmins = new int[mmi];
	for(int i = 0; i < mmi; i++)
		newmaxmins[i] = maxmins[i];
		*/

	mmi = 1;
	double hmax=0;
	int * newmaxmins = new int[1];
	for(int i=0; i<i1-1; i++)
		if (hmax < maxhs[i]){
			hmax = maxhs[i];
			newmaxmins[0] = maxis[i];
		}

	return newmaxmins;
}

ArrCurve* ArrCurveUtils::getHCurve(int step, Vec3 pos, int &locmax){
	int i1 = _ac->size()-step;
	maxhs = new double[i1];
	int * maxis = new int[i1];

	for(int i = 0; i < i1; i++)
		maxhs[i] = findMaxH(i, i+step, maxis[i]);

	double  * maxhs_f = new double[i1];
	for(int k=0; k<10; k++ ){
		for(int i=1; i < i1; i++)
			maxhs_f[i] = (maxhs[i-1] + maxhs[i] + maxhs[i+1])/3.0;
	
		maxhs_f[0] = maxhs[0];
		maxhs_f[i1-1] = maxhs[i1-1];
		maxhs = maxhs_f;
	}
	locmax = 0;
	for(int i=1; i<i1-1; i++)
		if ( (maxhs[i] > maxhs[i-1]) && (maxhs[i] > maxhs[i+1]) )
			locmax++;

	cout<<"local maxes:"<<locmax<<endl;

	return new ArrCurve(maxhs, i1, pos, Vec3(1,0,0), Vec3(0,1,0), 0.5, 0.2);
}

void Knot::solve(double r){

	Vec3 up(0.0, 0.0, 1.0);
	double eps = 0.0001;
	_rad = r;
	double diam = _rad*2;
	int istop = 1;
	int precoll = 0;
	int lowest = 5;
	resample(diam/2 + eps);

	int * colls = new int[_size];
	for(int i=0; i < _size; i++)
		colls[i] = 0;
	Vec3 ax(0,0,1);
	Vec3 * newp = new Vec3[_size];
	for(int i = 0; i < _size; i++)
		newp[i] = _pts[i];
	
	bool crossed = false;

	for(int i = lowest; i < _size; i++){

		Vec3 dir = (_pts[i]-_pts[i-2]);
		double len  = dir.norm();
		Vec3 ncross = dir%ax;
		dir = dir.normalize();

		for(int j=lowest; j < _size; j++){

			if (abs(i-j) < lowest)
				continue;
			if (!crossed){
				/*Vec3 dir2 = (_pts[j] - _pts[j-2]);
				double len2 = dir2.norm();
				dir2 = dir2.normalize();
				double d = getIntersectionDist(Vec2(dir),Vec2(_pts[i-2]), Vec2(dir2), Vec2(_pts[j-2]));
				double d2 = getIntersectionDist(Vec2(dir2),Vec2(_pts[j-2]), Vec2(dir), Vec2(_pts[i-2]));

				if ( (d>=0 && d<=len) && (d2>=0 && d2<=len2) )
					crossed = true;
					*/
				Vec3 v0 = _pts[j-2] - _pts[i-1];
				Vec3 v1 = _pts[j] - _pts[i-1];
				double d0 = v0*dir;  
				double d1 = v1 * dir;

				if ( ((d0 < len) && (d1 < len) && (d0 >0 ) && (d1 > 0)) && ((v0*ncross)*(v1*ncross)<0) )
					crossed = true;
			}

			Vec3 v =  (newp[i] - newp[j]);
			double d = v.norm();
			if (d + eps < diam ){
				newp[i]  = newp[i] + up*(sqrt(diam*diam - d*d)*istop);
				colls[i] = j;
				colls[j] = i;
			}

		}
		bool collision_end = precoll && (abs(colls[i] - precoll) > lowest);
		if (collision_end){
			//cout<<"corss:"<<crossed<<endl;
			istop = istop*-1;
			crossed = false;
		}

		precoll = colls[i];
	}

	for(int i=0; i<_size; i++)
		cout<<colls[i]<<" ";

	delete [] _pts;
	_pts = newp;
	resample(diam/2);

	cout<<endl;
	this->movingAverage(4);
}

PointCurve::PointCurve(Vec3 p){
	_p0 = p; 
	ControlPoint::create(this, &_p0);
};

void PointCurve::draw(){
	glPointSize(4.0);
	glBegin(GL_POINTS);
	glVertex3f(_p0.x, _p0.y, _p0.z);
	glEnd();
}


//n-total degree, i-current points, t-time
int factorial(int i){
	int x = 1;
	while(i>0)
		x*=i--;
	return x;
}	

double berstein(int n, int i, double t){
	return (factorial(n)/factorial(n-i)/factorial(i))*pow(1-t,n-i)*pow(t,i);
}
	
Vec3 Bezier::getP(double t){
	int i;
	Vec3 p(0,0,0);
	for(i=0; i < _deg+1; i++){
		double b = berstein(_deg, i, t);
		p = p + b*(*_pts[i]);
	}
	return p;
}

Vec3 Patch::P(double t, double s){
	Vec3 p = _f0->getP(t)*s + _f1->getP(t)*(1-s);
	p = p + _g0->getP(s)*(1.0 - t) + _g1->getP(s)*t;
	p = p - _f0->getP(0)*s*(1.0-t) - _f1->getP(0)*(1-s)*(1.0-t) - _f0->getP(1.0)*s*t - _f1->getP(1.0)*(1-s)*t;
	return p;
}

//slow
Vec3 Patch::N(double t, double s){
	Vec3 p = _f0->getN(t)*s - _f1->getN(t)*(1-s);
	p = p + _g0->getN(s)*(1.0 - t) - _g1->getN(s)*t;
	p = p - _f0->getN(0)*s*(1.0-t) + _f1->getN(0)*(1-s)*(1.0-t) - _f0->getN(1.0)*s*t + _f1->getN(1.0)*(1-s)*t;
	return p;
}


void Patch::draw(int w, int h){

	//paralel to s
	for(int i=0; i < w; i++){
		double t = 1.0/(w-1.0)*i;
		glBegin(GL_LINE_STRIP);
			for(int j =0; j< h; j++){
				double s  = 1.0/(h-1.0)*j;
				Vec3 p = P(t, s);
				glVertex3f(p.x, p.y, p.z);
			}
		glEnd();
	}
	//paralel to t
	for(int j=0; j < h; j++){
		double s  = 1.0/(h-1.0)*j;
		glBegin(GL_LINE_STRIP);
			for(int i =0; i< w; i++){
				double t = 1.0/(w-1.0)*i;
				Vec3 p = P(t, s);
				glVertex3f(p.x, p.y, p.z);
			}
		glEnd();
	}
}

int Patch::ind(int i, int j, int w){
	return i + j*w;
}
//super slow
void Patch::fill(int w, int h){

	int sz = w*h;

	Vec3* ps = new Vec3[sz];
	Vec3* ns = new Vec3[sz];

	for(int j=0; j < h; j++){
		double s  = 1.0/(h-1.0)*j;
		for(int i =0; i< w; i++){
			double t = 1.0/(w-1.0)*i;
			ps[ind(i,j, w)] = P(t, s);
			ns[ind(i,j, w)] = N(t, s);
		}
	}

	for(int j=0; j < h-1; j++){
		for(int i = 0; i< w-1; i++){
	
			Vec3 p[4];
			p[0] = ps[ind(i, j, w)];
			p[1] = ps[ind(i+1, j, w)];
			p[2] = ps[ind(i+1, j+1, w)];
			p[3] = ps[ind(i, j+1, w)];

			Vec3 n[4];
			n[0] = ns[ind(i, j, w)];
			n[1] = ns[ind(i+1, j, w)];
			n[2] = ns[ind(i+1, j+1, w)];
			n[3] = ns[ind(i, j+1, w)];
			
			glBegin(GL_POLYGON);
			for(int k=0; k<4; k++){
				glColor3f((n[k].x+1)/2, (n[k].y+1)/2, (n[k].z+1)/2 );
				glVertex3f(p[k].x, p[k].y, p[k].z);
			}
			glEnd();
		}
	}


}
/*
Vec3 Patch::P(double t, double s){
	Vec3 p = _f0->getP(t)*s + _f1->getP(t)*(1-s);
	p = p + _g0->getP(s)*(1.0 - t) + _g1->getP(s)*t;
	p = p - _f0->getP(0)*s*(1.0-t) - _f1->getP(0)*(1-s)*(1.0-t) - _f0->getP(1.0)*s*t - _f1->getP(1.0)*(1-s)*t;
	return p;
}*/


Vec3 DPatch::P(int i, int j){
	double t = (i*1.0 / _Ni);
	double s = (j*1.0 / _Ni);
	Vec3 p = _p(i, 0)*(1.0-s) + _p(i, _Ni)*s;
	p = p + _p(0, j)*(1.0 - t) + _p(_Ni, j)*t;
	p = p - _p(0,0)*(1-s)*(1.0-t) - _p(_Ni, 0)*(1-s)*t - _p(_Ni, _Ni)*s*t - _p(0, _Ni)*s*(1-t);
	return p;
}

Vec3 DPatch::N(int i, int j){
	double t = (i*1.0 / _Ni);
	double s = (j*1.0 / _Ni);
	Vec3 p = _n(i, 0)*(1.0-s) + _n(i, _Ni)*s;
	p = p + _n(0, j)*(1.0 - t) + _n(_Ni, j)*t;
	p = p - _n(0,0)*(1-s)*(1.0-t) - _n(_Ni, 0)*(1-s)*t - _n(_Ni, _Ni)*s*t - _n(0, _Ni)*s*(1-t);
	return p;
}


void DPatch::update(){

	double T  = 1.0 / _Ni;
	for(int i=0; i<_N; i++)
		_ps[ind(i, 0)] = _fs[0]->getP(i*T);

	for(int i=0; i<_N; i++)
		_ps[ind(_Ni, i)] = _fs[1]->getP(i*T); 

	for(int i=0; i<_N; i++)
		_ps[ind(_Ni-i, _Ni)] = _fs[2]->getP(i*T); 

	for(int i=0; i<_N; i++)
		_ps[ind(0, _Ni-i)] = _fs[3]->getP(i*T);
		
	for(int j=1; j<_Ni;j++)
		for(int i=1; i<_Ni; i++)
			_ps[ind(i,j)] = P(i, j);

	//now normals
	for(int i=1; i<_Ni; i++)
		_ns[ind(i, 0)] = -(_p(i+1,0) - _p(i-1,0)).normalize() % Vec3(0,0,1);

	for(int i=1; i<_Ni; i++)
		_ns[ind(_Ni, i)] = -(_p(_Ni, i+1) - _p(_Ni, i-1)).normalize() % Vec3(0,0,1);

	for(int i=1; i<_Ni; i++)
		_ns[ind(i, _Ni)] = -(_p(i-1, _Ni) - _p(i+1, _Ni)).normalize() % Vec3(0,0,1);

	for(int i=1; i<_Ni; i++)
		_ns[ind(0, i)] = -(_p(0, i-1) - _p(0, i+1)).normalize() % Vec3(0,0,1);

	_ns[ind(0, 0)] = -(_p(1, 0) - _p(0,1)).normalize() % Vec3(0,0,1);
	_ns[ind(_Ni, 0)] = -(_p(_Ni, 1) - _p(_Ni-1,0)).normalize() % Vec3(0,0,1);
	_ns[ind(_Ni, _Ni)] = -(_p(_Ni-1, _Ni) - _p(_Ni, _Ni-1)).normalize() % Vec3(0,0,1);
	_ns[ind(0, _Ni)] = -(_p(0, _Ni-1) - _p(1, _Ni)).normalize() % Vec3(0,0,1);

	for(int j=1; j<_Ni;j++)
		for(int i=1; i<_Ni; i++){
			 Vec3 n = N(i, j);
			 //n.z = sqrt(1.0 -n.x*n.x -n.y*n.y);
			_ns[ind(i,j)] = n;
		}
}

void DPatch::fill(){

	update();

	for(int j=0; j < _Ni; j++){
		for(int i = 0; i< _Ni; i++){
	
			Vec3 p[4];
			p[0] = _ps[ind(i, j)];
			p[1] = _ps[ind(i+1, j)];
			p[2] = _ps[ind(i+1, j+1)];
			p[3] = _ps[ind(i, j+1)];
		
			Vec3 n[4];
			n[0] = _ns[ind(i, j)];
			n[1] = _ns[ind(i+1, j)];
			n[2] = _ns[ind(i+1, j+1)];
			n[3] = _ns[ind(i, j+1)];
	
			glBegin(GL_POLYGON);
			for(int k=0; k<4; k++){
				glColor3f((n[k].x+1)/2, (n[k].y+1)/2, n[k].z );
				glVertex3f(p[k].x, p[k].y, p[k].z);
			}
			glEnd();
		}
	}
}