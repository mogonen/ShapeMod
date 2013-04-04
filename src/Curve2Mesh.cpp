#include <map>
#include "Curve2Mesh.h"

Eye * Eye::_eye;

Eye * Eye::get(){
	if (!_eye){
		_eye = new Eye();
		_eye->set(ON_P, ON_N, ON_U);
	}
	return _eye;
};

void Eye::setCam(Camera * cam){
	Eye::get()->P.set(cam->pos());
	Eye::get()->N.set((AIM - cam->Pos).normalize());
	Eye::get()->U.set(cam->Up);
};

C2M::C2M(){
	_mesh = 0;
	ac = 0;
}

void C2M::draw(){

	if (DRAW_AUX)
	for (list<RingPtr>::iterator it = _rings.begin(); it != _rings.end(); it++)
		(*it)->drawAll();

	glColor3f(0.9, 0.05, 0.05);
	glLineWidth(2.0);
	for (list<Curve*>::iterator it = _spines.begin(); it != _spines.end(); it++)
		(*it)->draw();

	if (_mesh) //DRAW_AUX &&
		_mesh->draw();

	glColor3f(0.1, 0.1, 0.99);
	if (ac)
		ac->draw();

}

void C2M::updateCurves(){
	_curves.clear();
	/*for (list<StrokePtr>::iterator it = _sman->getStrokes()->begin(); it != _sman->getStrokes()->end(); it++){
		ArrCurve * ac = new ArrCurve((*it)->toArr(), (*it)->size(), true);
		if ((*it)->isClosed())
			ac->close();
		_curves.push_back(ac);
	}*/
}

void C2M::buildRings(Curve* sp){
	if (!sp)
		return; 
	ac = new ArrCurve(sp->toArr(), sp->size(), true);
	RingPtr pre = 0;
	for(int i=0; i<ac->size(); i++){

		Vec3 o = ac->getP(i);
		Vec3 no = (o - Eye::get()->P).normalize();
		Vec3 tan = ac->getT(i);
		Vec3 up = (no%tan).normalize();

		double z = (o - Eye::get()->P).norm();
		Vec3 pmid = Eye::get()->P + no*( PZ / (no*Eye::get()->N) );

		ac->setP(i, pmid);
		double t0 = getHit(pmid, up);
		double t1 = getHit(pmid, -up);
		//cout<<"..........................................."<<endl;
		if (t0<0 || t1<0)
			continue;

		Vec3 p0 = pmid + up*t0;
		Vec3 p1 = pmid - up*t1;

		o = (p0+p1)*0.5;
		z = (o - Eye::get()->P).norm();
		//o = Eye::get()->P + ((p0+p1)*0.5 - Eye::get()->P ).normalize()*z;
		no = (o - Eye::get()->P).normalize();

		Vec3 n0 = (p0 - Eye::get()->P).normalize();
		Vec3 n1 = (p1 - Eye::get()->P).normalize();
		double a0 = z * (n0*no);
		double a1 = z * (n1*no);
		double a = (a0+a1)*0.5;

		//recompute center to be exactly in the middle of p0 and p1 
		RingPtr rng  = new Ring(o, Eye::get()->P + a*n0, Eye::get()->P + a*n1);
		//rng->reorient(tan);
		if (pre)
			rng->setPrevNext(pre,0);
		else
			_rings.push_back(rng);
		pre = rng;
	}
}

void C2M::buildRings(){

	_rings.clear();
	updateCurves();
	for (list<Curve*>::iterator it = _spines.begin(); it != _spines.end(); it++)
		buildRings(*it);
}

double C2M::getHit(Vec3 p3, Vec3 n3){

	Vec3 p(p3);
	Vec3 n((n3 - Eye::get()->N*(n3*Eye::get()->N) ).normalize());
	
	double tmin = 999999999999;
	bool hit = false;
	Vec3 hitp;
	for(list<ArrCurve*>::iterator it =_curves.begin(); it!=_curves.end(); it++){

		ArrCurve*  ac = (*it);
		Vec3 p0(ac->getP(0));

		for(int i=1; i<ac->size();i++){
	
			Vec3 p1(ac->getP(i));
			Vec3 n1 = (p1 - p0);

			double len1 = n1.norm();
			double t = getIntersectionDist(p, n, p1, n1.normalize() );

			if (t>0 && t<tmin && ( p + t*n - p1 ).norm()<len1 ){
				tmin = t;
				hit = true;
			}
			p0.set(p1);
		}
	}

	if (hit) 
		return tmin;
	return -1;
}

void C2M::buildMesh(RingPtr ring, double len, int orgsize){

	int pre_size = 0;
	Vertex** pre_verts = 0;
	int rot = 0;
	while(ring){

		int size = (orgsize)?orgsize:ring->getSegmentNum(len);

		rot=rot + (pre_size?(pre_size-size):0);
		Vec3 * pos = ring->getVertexPos(size, rot);

		Vertex ** verts = new Vertex*[size];
		for(int i=0; i<size; i++)
			verts[i] = _mesh->addVertex(pos[i]);

		if (pre_verts){
			for(int i=0; i < min(size, pre_size); i++ )
				_mesh->addQuad(pre_verts[i], pre_verts[(i+1)%pre_size], verts[(i+1)%size], verts[i]);	

			if (pre_size - size == 1){
				//cout<<"1"<<endl;
				_mesh->addTriangle( pre_verts[pre_size-2], pre_verts[pre_size-1], verts[size-1]);
			}else if (pre_size - size == -1){
				//cout<<"-1"<<endl;
				_mesh->addTriangle( pre_verts[0], verts[size-1], verts[0]);
			}
			/*else if (pre_size - size == 2 ){
				cout<<"2"<<endl;
				_mesh->addQuad(pre_verts[pre_size-2], pre_verts[pre_size-1], pre_verts[0], verts[0]);
			}else if (pre_size - size == -2 ){
				cout<<"-2"<<endl;
				_mesh->addQuad(pre_verts[0], verts[0], verts[size-1], verts[size-2]);
			}*/
		}

		pre_verts = verts;
		pre_size = size;
		ring = ring->next();
	}
}

void C2M::buildMesh(RingPtr ring, int size){

	Vertex** pre_verts = 0;
	Vertex** root_verts = 0;
	Vec3* prex = new Vec3();
	bool closed = ring->isClosed();
	while(ring){
		
		Vec3 * pos = ring->getVertexPos(size, prex);
		Vertex ** verts = new Vertex*[size];
		for(int i=0; i<size; i++)
			verts[i] = _mesh->addVertex(pos[i]);

		if (pre_verts)
			for(int i=0; i<size; i++){
				int i1 = ((i+1)%size);
				_mesh->addQuad(pre_verts[i], pre_verts[i1], verts[i1], verts[i]);
			}
		else 
			root_verts = verts;
		pre_verts = verts;
		ring = ring->next();
	}

	if (closed){
		for(int i=0; i<size; i++){
			int i1 = ((i+1)%size);
			_mesh->addQuad(pre_verts[i], pre_verts[i1], root_verts[i1], root_verts[i]);
		}
	}
}

void C2M::buildMesh(RingPtr ring, double len, int &size){

}

void C2M::buildMesh(int s){
	if (_mesh)
		delete _mesh;
	_mesh = new Mesh();
	for(list<RingPtr>::iterator it = _rings.begin(); it != _rings.end(); it++){
		buildMesh(*it, s);
	}
}

void C2M::saveMesh(char * fname){
	if (_mesh)
		_mesh->exportOBJ(fname);
}


Ring::Ring(CorrPtr cp){
	_cp = cp;
	_p1 = cp->P();
	_p2 = cp->corr()->P();
	_p0 = (_p1 + _p2)*0.5;
	_rad = (_p1 - _p0).norm();
	_n0 = (_p2 - _p1).normalize()%(_p1-Eye::get()->P).normalize();
	_prev = 0;
	_next = 0;
	_orgrad =_rad;
	_org = _p0;
	_isclosed = false;
}

Ring::Ring(Vec3 p0, Vec3 p1, Vec3 p2){

	double d = (p1-p0).normalize() * (p1).normalize();
	double d2 = (p2-p0).normalize() * (p2).normalize();
	
	_p0 = p0;
	_p1 = p1;
	_p2 = p2;
	_rad = (_p1 - _p0).norm();
	//cout<<"ringing!:"<<_rad<<endl;
	_n0 = (_p2 - _p1).normalize()%(_p0 - Eye::get()->P).normalize();
	_prev = 0;
	_next = 0;
	//p1.print(); p2.print();
	_orgrad =_rad;
	_org = _p0;
	_isclosed = false;

}

Ring::Ring(Vec3 p0, Vec3 n0, double rad){
	_p0 = p0;
	_n0 = n0;
	//_n0.print();n0.print();cout<<"--"<<endl;
	_p1 = _p0 + ((_p0 - Eye::get()->P)%n0).normalize()*rad;
	_rad = rad; 
	_prev = 0;
	_next = 0;
	/*_orgrad =_rad;
	_org = _p0;
	reorient(n0);*/
	_isclosed = false;
}

void Ring::setRadius(double r){
	_rad = r;
	_p1 = (_p1 - Eye::get()->P).normalize(); 		
	_p2 = (_p2 - Eye::get()->P).normalize();
		
	_p0 = (_p1+_p2).normalize();
		
	double ca = _p0*_p1;
	double z = r / sqrt(1-ca*ca);
	double a = sqrt(z*z - _rad*_rad);

	_p0 = _p0*z;
	_p1 = _p1*a;
	_p2 = _p2*a;

	_orgrad =_rad;
	_org = _p0;
}

void Ring::reorient(Vec3 n){		
	double d = (_p1 - _p0)*n;
	_p0  = n*d + _p0;
	_rad = sqrt(_rad*_rad - d*d);
	_n0  = n.normalize();
}

void Ring::setPrevNext(RingPtr p, RingPtr n){
	_prev = p;
	_next = n;
	if (_prev)
		_prev->_next = this;
	if (_next)
		_next->_prev = this;
}

void Ring::_draw(){
	glColor3f(0.5,0.5,0.5);
	double inc = 2.0*PI / RSIZE;
	double a = 0;
	glLineWidth(2.0);
	glBegin(GL_LINE_LOOP);
	for(int i=0; i<RSIZE; i++){
		glVertex3f(_rad*cos(a), _rad*sin(a), 0);
		a=a+inc;
	}
	glEnd();
}

void Ring::draw(){
	Vec3 nn(0,0,1);
	Vec3 ax = (_n0%nn).normalize();
	double rot = acos(_n0*(-nn))/PI*180.0;
	//ax.print();	cout<<"rot:"<<rot<<endl;
	glLineWidth(2.0);
	glColor3f(0,0,1);
	/*glBegin(GL_LINES);
		glVertex3f(_p1.x, _p1.y, _p1.z);
		glVertex3f(_p0.x, _p0.y, _p0.z);
		glVertex3f(_p0.x, _p0.y, _p0.z);
		glVertex3f(_p2.x, _p2.y, _p2.z);
	glEnd();

	glColor3f(0,1,1);
	glBegin(GL_LINES);
		glColor3f(0,0,1);

		Vec3 p0 = Eye::get()->P;
		glVertex3f(p0.x, p0.y, p0.z);
		glVertex3f(_p1.x, _p1.y, _p1.z);

		glVertex3f(p0.x, p0.y, p0.z);
		glVertex3f(_p2.x, _p2.y, _p2.z);

	glEnd();

		glVertex3f(_p1.x, _p1.y, _p1.z);
		glVertex3f(_p2.x, _p2.y, _p2.z);
	glEnd();
*/

	glPushMatrix();
		glTranslatef(_p0.x, _p0.y, _p0.z);
		glRotatef(rot, ax.x, ax.y, ax.z);
		draw();
	glPopMatrix();

	glBegin(GL_POINTS);
		glVertex3f(_p0.x, _p0.y, _p0.z);
	glEnd();

	/*if (DRAW_AUX){
		glEnable(GL_LIGHTING);
		glPushMatrix();
			glTranslatef(_org.x, _org.y, _org.z);
			glutSolidSphere(_orgrad-0.0002, 18, 18);
		glPopMatrix();
		glDisable(GL_LIGHTING);
	}*/
}

void Ring::drawAll(){
	glColor3f(0.7,0.65,0.65);
	for(RingPtr rng = this; rng; rng = rng->_next)
		rng->draw();
}

void RingNode::addRing(RingPtr r){
	_rings.push_back(r); 
}

double Ring::getNiceRadius(){
	return ((_p1-_p2)%(Eye::get()->P*2.0-_p1-_p2).normalize()).norm()/2.0;
}

Vec3 * Ring::getVertexPos(int num, int rot){
	Vec3 ny = (_p1-_p0).normalize();
	Vec3 nx = -(_n0%ny).normalize();
	ny = (_n0%nx).normalize();
	Vec3 * ps = new Vec3[num];
	double inc = 2.0*PI / num;
	double a =- rot*(inc/2.0);
	for(int i=0; i<num; i++){
		Vec3 p = _p0 + nx*_rad*cos(a)+ ny*_rad*sin(a);
		ps[i] = p;
		a=a+inc;
	}
	return ps;
}

Vec3 * Ring::getVertexPos(int num, Vec3 * prey, int rot){

	if (!prey->norm())
		prey->set( Eye::get()->N );

	Vec3 nx = (*prey % _n0 ).normalize();
	Vec3 ny = (_n0%nx).normalize();
	prey->set(ny);
	
	//ny = (_n0%nx).normalize();
	/*double dt = (*prex)*nx;
	cout<<dt<<endl;
	if ( dt && dt < 0.8){
		cout<<"- ";
		nx = -nx;
	}else
		cout<<"+ ";
	prex->set(nx);*/

	Vec3 * ps = new Vec3[num];
	double inc = 2.0*PI / num;
	double a =- rot*(inc/2.0);
;
	for(int i=0; i<num; i++){
		Vec3 p = _p0 + nx*_rad*cos(a)+ ny*_rad*sin(a);
		ps[i] = p;
		
		a=a+inc;
	}
	return ps;
}

double Ring::getSegmentLength(int num){
	double a = _rad / cos(PI/num);
	return sqrt(_rad*_rad - a*a);
}

int Ring::getSegmentNum(double len){
	return (int)(PI/asin( len/(2*_rad) ));
}


Vec3 * Ring::getSilhouettePoints(Vec3 p){
	Vec3 * pp = new Vec3[2];
	Vec3 v =  _p0-p;
	double a = v.norm();
	double l = sqrt( a*a - _rad*_rad);
	double h = _rad * l/ a;
	double b = sqrt(l*l - h*h);

	Vec3 po = p+b*v.normalize();
	Vec3 nn = v.normalize()%_n0;
	pp[0] = po + nn*h;
	pp[1] = po - nn*h;
	return pp;
}

list<Curve*> C2M::getProjectionCurves(Camera * cam){

	list<Curve*> curves;
	_eye.set(cam->pos());
	for (list<RingPtr>::iterator it = _rings.begin(); it != _rings.end(); it++){
		Ring * rng = (*it);
		Vec3 * c1 = new Vec3[50];
		Vec3 * c2 = new Vec3[50];
		int ix=0;
		while(rng){
			Vec3 * ps = rng->getSilhouettePoints(_eye);
			//now project them, lazy projection 
			c1[ix] = ps[0];
			c2[ix] = ps[1];
			ix++;
			rng = rng->next();
		}

		ArrCurve * ac1 = new ArrCurve(c1, ix);
		ArrCurve * ac2 = new ArrCurve(c2, ix);
		ac2->reverse();
		curves.push_back(ac1);
		curves.push_back(ac2);
	}
	return curves;
}

void updateVertsByCurve(void* param){
	map<Vertex**, CorrPtr> * vmap = (map<Vertex**, CorrPtr>*)param;
	map<Vertex**, CorrPtr>::iterator it;   
	for( it = vmap->begin(); it != vmap->end(); it++ ){


		Vertex* * v = it->first;
		CorrPtr cp = it->second;
		
		/*it->first->setP(it->second->P());
		it->first->setC(it->second->getN());*/
		v[0]->setP(cp->P());
		v[0]->setC(cp->getN());

		if(!cp->corr())
			continue;

		v[5]->setP(cp->corr()->P());
		v[5]->setC(cp->corr()->getN());
		
		Vec3 n = v[5]->P() - v[0]->P();

		v[2]->setP(v[0]->P() + n*0.45);
		v[3]->setP(v[0]->P() + n*0.55);
		v[2]->setC(Vec3(0,0,1));
		v[3]->setC(Vec3(0,0,1));

	    v[1]->setP(v[0]->P()+n*0.225);
		v[4]->setP(v[0]->P()+n*0.775);

		v[1]->setC( ((v[0]->C() + v[2]->C())*0.5).normalize() );
		v[4]->setC( ((v[3]->C() + v[5]->C())*0.5).normalize() );

	}
 //((F2M*)param)->blendMesh();
}

Mesh* F2M::buildMesh(){

	if (!_cp)
		return 0;

	if (_mesh)
		delete _mesh;
	_mesh = new Mesh();
	_vnum = 6;

	CorrPtr cp0 = _cp->tipCorr();
	if(!cp0)
		return 0;
	Vertex* * v = createVerts(cp0->P(), cp0->corr()->P());
	v[0]->setC(-cp0->N());
	v[5]->setC(-cp0->corr()->N());
	_vertmap[v] = cp0;

	cp0 = cp0->nextCorr();
	buildMesh(cp0, v);
	map<Vertex**, CorrPtr> * vmap = new map<Vertex**, CorrPtr>();
	vmap->swap(_vertmap);
	cp0->curve()->setShapeChangedHandler(&updateVertsByCurve, (void*)vmap);
	return _mesh;
}

void F2M::buildMesh(CorrPtr cp0, Vertex* * v0s){
	if (!cp0)
		return;
	CorrPtr cp00 = cp0;
	Vec3 n001(0,0,1);
	list<CorrPtr> que;
	//CorrPtr cp0 = que.front();
	//que.pop_front();

	bool exit = false;
	while(cp0 && !exit){
		if (cp0->isTip())
			exit = 1;
			//*/
		Vertex* * v1s = createVerts(cp0->P(), cp0->corr()->P());

		v1s[0]->setC(-cp0->N());
		v1s[5]->setC(-cp0->corr()->N());
		
		v1s[1]->setC( ((v1s[0]->C() + v1s[2]->C())*0.5).normalize() );
		v1s[4]->setC( ((v1s[3]->C() + v1s[5]->C())*0.5).normalize() );

		if (exit){
			Vec3 nz(0,0,1);
			v1s[2]->setC(v1s[0]->P() - v1s[2]->P());
			v1s[3]->setC(v1s[1]->P() - v1s[3]->P());
		}
		//_mesh->addQuad(vfirst, v0, v1, vlast);
		for(int i=0; i < _vnum-1; i++)
			_mesh->addQuad(v1s[i], v1s[i+1], v0s[i+1], v0s[i]);
			
		if (cp0->isSplit()){

			//we just hit a joint
			list<Vertex*> verts;
			Vertex* * vduz= new Vertex*[_vnum];
			for(int i=0; i<_vnum; i++)
				vduz[i] = v1s[i];
			_vertmap[vduz] = cp0;

			for(int i=0; i<_vnum/2; i++){
				Vertex* tmp = v1s[i];
				v1s[i] = v1s[_vnum-i-1];
				v1s[_vnum-i-1] = tmp;
			}

			Vec3 pmid(0,0,0);
			int count = 0;
			for(CorrPtr cpi = cp0->nextCorr(); cpi && cpi!=cp0 && cpi!=cp0->corr(); cpi = cpi->corr()->nextCorr()){
				pmid = pmid + cpi->P() + cpi->corr()->P();
				count++;
			}

			pmid = pmid + cp0->P()+ cp0->corr()->P();
			pmid = pmid / ( (count+1)*2.0 );
			Vertex* * vs0 = v1s;

			for(CorrPtr cpi = cp0->nextCorr(); cpi && cpi!=cp0 && cpi!=cp0->corr(); cpi = cpi->corr()->nextCorr()){

				Vertex* * vs = createVerts(cpi->P(), cpi->corr()->P());
				_vertmap[vs] = cpi;
				vs[0]->setC(-cpi->N());
				vs[5]->setC(-cpi->corr()->N());
				vs[1]->setC( ((vs[0]->C() + vs[2]->C())*0.5).normalize() );
				vs[4]->setC( ((vs[3]->C() + vs[5]->C())*0.5).normalize() );
				//_vertmap[vs[0]] = cpi;
				//_vertmap[vs[5]] = cpi->corr();

				buildMesh(cpi->nextCorr(), vs);
				/*for(int i=1; i<_vnum-1; i++)
					verts.push_back(vs[i]);
					*/
				if (vs0){
					CPPtr cpmid = cpi->go(cpi->find(cpi->prevCorr()) / 2);
					Vec3 p0 = cpmid->P();
					Vec3 p1 = p0 + (pmid - p0)*0.75; //(vs[_vnum/2-1]->P() + vs0[_vnum/2]->P())*0.5;
					Vertex* * vs01 = createMidVerts(p0, p1);
					vs01[0]->setC(-cpmid->N());
					vs01[1]->setC( ((vs01[0]->C() + vs01[2]->C())*0.5).normalize() );
					_vertmap[vs01] = (CorrPtr)cpmid;
					int i = 0;
					for( ; i<_vnum/2-1; i++){
						_mesh->addQuad(vs[i], vs[i+1], vs01[i+1], vs01[i]);
						_mesh->addQuad(vs01[i], vs01[i+1], vs0[_vnum-i-2], vs0[_vnum-i-1]);
					}
					i--;
					verts.push_back(vs0[_vnum-i-2]);
					verts.push_back(vs01[i+1]);
					verts.push_back(vs[i+1]);
				}
				vs0 = vs;
			}

			CPPtr cpmid = cp0->corr()->go(cp0->corr()->find(cp0->corr()->prevCorr()) / 2);
			Vec3 p0 = cpmid->P();
			Vec3 p1 = p0 + (pmid - p0)*0.75;//(v1s[_vnum/2-1]->P() + vs0[_vnum/2]->P())*0.5;
			Vertex* * vs01 = createMidVerts(p0, p1);
			_vertmap[vs01] = (CorrPtr)cpmid;
			vs01[0]->setC(-cpmid->N());
			vs01[1]->setC( ((vs01[0]->C() + vs01[2]->C())*0.5).normalize() );

			int i = 0;
			for( ; i<_vnum/2-1; i++){
				_mesh->addQuad(v1s[i], v1s[i+1], vs01[i+1], vs01[i]);
				_mesh->addQuad(vs01[i], vs01[i+1], vs0[_vnum-i-2], vs0[_vnum-i-1]);
			}
			i--;
			verts.push_back(vs0[_vnum-i-2]);
			verts.push_back(vs01[i+1]);
			verts.push_back(v1s[i+1]);
	
			/*int i = 0;
			for( ; i<_vnum/2-1; i++)
				_mesh->addQuad(v1s[i], v1s[i+1], vs0[_vnum-i-2], vs0[_vnum-i-1]);
			i--;
			verts.push_back(vs0[_vnum-i-2]);
			verts.push_back(v1s[i+1]);

			/*FacePtr f = new Face();
			_mesh->addFace(f);
			for (list<Vertex*>::iterator it = verts.begin(); it != verts.end(); it++)
				f->addNextVertex(*it);
		*/
			//_mesh->addQuad(v1s[0], v1s[1], vs0[4], vs0[5]);
			
			//connect the middle 
			Vec3 p;
			for (list<Vertex*>::iterator it = verts.begin(); it != verts.end(); it++)
				p = p+(*it)->P();
			p = p / verts.size();
			p.print();
			Vertex* midv = Vertex::create(_mesh);
			midv->setP(p);
			Vec3 n;
			//midv->setC(Vec3(0,0,0.1));
			Vertex* v0 = 0;
			int ii =0;
			for (list<Vertex*>::iterator it = verts.begin(); it != verts.end(); it++){
				Vertex* v1 = (*it);
				if (v0){
					FacePtr f = new Face(4);
					/*f->addNextVertex(midv);
					f->addNextVertex(v0);
					f->addNextVertex(v1*/
					_mesh->addFace(f);
				}
				//n = n + v1->C();
				v0 = v1;
				ii++;
				//if (ii==3) break;
			}
			midv->setC(n/verts.size());
			midv->setC(Vec3(0,0,1));
			FacePtr f = new Face(4);
			/*f->addNextVertex(midv);
			f->addNextVertex(verts.back());
			f->addNextVertex(verts.front());*/
			_mesh->addFace(f);//*/
			break;
		}else 
			_vertmap[v1s] = cp0;
		v0s = v1s;
		cp0 = cp0->nextCorr();
	};
	if (!cp0 || !cp0->isTip())
		return;
	cp0 = cp0->corr();
	double a = cp0->find(cp0->corr())*STEP /(_vnum-1);
	CPPtr it = cp0;
	//reposition verticies
	for(int i = 1; i<_vnum-1; i++){
		v0s[i]->setP(cp0->go(a*i));
		Vec3 t = (it->go(a*i+STEP) - it->go(a*i-STEP)).normalize();
		v0s[i]->setC( -(Eye::get()->N%t).normalize() );
		//it = it->next();
	}

	if (cp00->isTip()){
		cp00 = cp00->corr();
		double a = cp00->find(cp00->corr())*STEP /(_vnum-1);
		//reposition verticies
		for(int i = 1; i<_vnum-1; i++){
			v0s[i]->setP(cp00->go(a*i));
			Vec3 t = (it->go(a*i+STEP) - it->go(a*i-STEP)).normalize();
			v0s[i]->setC( -(Eye::get()->N%t).normalize() );
			//it = it->next();
		}	
	}

	/*
	Vertex** vs = new Vertex*[_vnum-2];
	for(int i=0; i<_vnum-2; i++)
		vs[i] = new Vertex();
	vs[0]->setP(cp0->P());
	vs[3]->setP(cp0->corr()->P());

	Vec3 vn = (vs[3]->P() - vs[0]->P());
	vs[1]->setP(vs[0]->P()+vn*0.25);
	vs[2]->setP(vs[0]->P()+vn*0.75);
	vs[0]->setC(Vec3(0,0,1));
	vs[3]->setC(Vec3(0,0,1));

	vs[1]->setC(-cp0->N());
	vs[2]->setC(-cp0->corr()->N());

	_mesh->addQuad(vs[0], vs[1], v0s[1], v0s[0]);
	_mesh->addQuad(vs[1], vs[2], v0s[3], v0s[2]);
	_mesh->addQuad(vs[2], vs[3], v0s[5], v0s[4]);

	_mesh->addTriangle(vs[1], v0s[2],v0s[1]);
	_mesh->addTriangle(vs[2], v0s[4],v0s[3]);
	//*/
}

Vertex* * F2M::createVerts(Vec3 v0, Vec3 v1){
	Vertex** vs = new Vertex*[_vnum];
	for(int i=0; i<_vnum; i++)
		vs[i] = Vertex::create(_mesh);

	vs[0]->setP(v0);
	vs[5]->setP(v1);

	Vec3 vn = (vs[5]->P() - vs[0]->P());

	vs[2]->setP(vs[0]->P() + vn*0.45);
	vs[3]->setP(vs[0]->P() + vn*0.55);
	vs[2]->setC(Vec3(0,0,1));
	vs[3]->setC(Vec3(0,0,1));

    vs[1]->setP(vs[0]->P()+vn*0.225);
	vs[4]->setP(vs[0]->P()+vn*0.775);

	return vs;
}

Vertex* * F2M::createMidVerts(Vec3 v0, Vec3 v1){

	int n = _vnum/2;
	Vertex** vs = new Vertex*[n];
	for(int i=0; i<n; i++)
		vs[i] =Vertex::create(_mesh);

	vs[0]->setP(v0);
	vs[2]->setP(v1);
	vs[1]->setP((v0+v1)*0.5);
	vs[2]->setC(Vec3(0,0,1));

	return vs;
}

//*/

/*
Vertex* * F2M::createVerts(Vec3 v0, Vec3 v1){
	Vertex** vs = new Vertex*[_vnum];
	for(int i=0; i<_vnum; i++)
		vs[i] = new Vertex();

	vs[0]->setP(v0);
	vs[3]->setP(v1);

	Vec3 vn = (vs[3]->P() - vs[0]->P());
	//vs[1]->setP(vs[0]->P()+vn*0.075);
	//vs[4]->setP(vs[0]->P()+vn*0.925);
	//vs[0]->setC(Vec3(0,0,1));
	//vs[5]->setC(Vec3(0,0,1));

    vs[1]->setP(vs[0]->P()+vn*0.45);
	vs[2]->setP(vs[0]->P()+vn*0.55);
	vs[1]->setC(Vec3(0,0,1));
	vs[2]->setC(Vec3(0,0,1));

	return vs;
}
//*/

void F2M::setVertexNormals(Vertex* v0, Vertex* v1){

	/*Vec3 pmid = (v0->P() + v1->P())/2.0;

	Vec3 n0 = ((pmid -Eye::get()->P).normalize()%(v0->P() - pmid).normalize()).normalize();
	n0 = (n0%(pmid -Eye::get()->P).normalize()).normalize();

	//n0.set(0, 1, 1);
	//n0 = n0.normalize();
	v0->setC(n0);

	Vec3 n1 = ((pmid -Eye::get()->P).normalize()%(v1->P() - pmid).normalize() ).normalize();
	n1 = (n1%(pmid -Eye::get()->P).normalize()).normalize();

    //n1.set(0, -1, 1);
	//n1 = n1.normalize();
	v1->setC(n1);
	*/
}

void F2M::blendMesh(){
	if (_mesh)
		_mesh->blendVertexColors();
}