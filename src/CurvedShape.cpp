#include "CurvedShape.h"

float CurvedShape::RADIUS = 0.05;
float CurvedShape::FLATNESS = 0.05;
float CurvedShape::SPINE_STEP = 0.015;
float CurvedShape::FADEIN = 0.25;
float CurvedShape::FEATHER = 0;
float CurvedShape::N_Z = 1.0;

int   CurvedShape::CAPS = 12;
int   CurvedShape::VNUM = 5;
int   CurvedShape::CAPS_ON = 1;

CurvedShape::~CurvedShape(){
	if (_mesh)
		delete _mesh;

	for(list<Curve*>::iterator it =_border.begin(); it!=_border.end(); it++)
		if (*it)
			delete *it;
	
	if(_spine)
		delete _spine;
	/*
	for(list<Curve*>::iterator it =_border.begin(); it!=_border.end(); it++)
		Canvas::getCanvas()->remove(*it);
	Canvas::getCanvas()->remove(_spine);
	*/
}

void CurvedShape::draw(int mode){

	if (_mesh)
		_mesh->draw();

	if (mode){
		if (mode == 1)
			glColor3f(0,0,0);
		for(list<Curve*>::iterator it =_border.begin(); it!=_border.end(); it++){
			(*it)->draw(mode);
			if (Canvas::MODE== Canvas::EDIT)
				ControlPoint::draw(*it);
		}
		if (_spine && mode == 2){
			_spine->draw(mode);
			if (Canvas::MODE== Canvas::EDIT)
				ControlPoint::draw(_spine);
		}
	}
}

void CurvedShape::exec(int cmd, void* param){

	if (cmd == Command::BLEND_MESH_NORMALS && _mesh){
		_mesh->blendVertexColors();
		return;
	}

	if (_border.empty()){
		_border.swap(Canvas::getCanvas()->selectedCurves());
		Canvas::getCanvas()->flushSelectedCurves();
		Canvas::getCanvas()->deselect();
	}

	if(!_spine){
		_spine = Canvas::getCanvas()->activeCurve();
		Canvas::getCanvas()->flushActiveCurve();
	}

	if (!_spine)
		return;

	/*adopt(_spine);
	_spine->set(NO_DELETE);
	_spine->unset(SELECTABLE);
	_spine->unset(DRAGGABLE);
	_spine->set(HIDDEN_AS_CHILD);*/

	if (_mesh){
		isopams.clear();
		delete _mesh;
	}

	_mesh = new Mesh();
	_flatness = FLATNESS;
	_n_z = N_Z;
	
	if (cmd == Command::BUILD_NO_SPINE || (_spine->isClosed() && _spine->type()!=POINT_CURVE) ){
		buildShapeNoSpine();
	}else{
		if (!_border.size()){
			cout<<"building border..."<<endl;
			buildBorderBySpine(_spine, CAPS/2 * CAPS_ON);
			//Canvas::getCanvas()->insert(_border.front());
		}
		cout<<"building shape..."<<endl;
		buildShapeBySpine(_spine, CAPS*CAPS_ON);
		_border0 = _border.front();

		//_border0->adopt(this);
		/*adopt(_border0);
		//_border0->unset(SELECTABLE);
		_border0->set(NO_DELETE);
		_border0->unset(DRAGGABLE);
		//_spine->set(HIDDEN_AS_CHILD);*/
	}
	cout<<"done."<<endl;
	//_isomap = new map<Vertex**, Isopam*>();
	//buildShapeNoSpine();
}

bool CurvedShape::isOn(const Vec3& p){
	if (_border0 && _border0->isIn(p))
		return true;
	else
		return false;
}

void CurvedShape::onClick(const Vec3& p){
	for(list<Curve*>::iterator it =_border.begin(); it!=_border.end(); it++)
		ControlPoint::activate(*it, p);
	if (_spine)
		ControlPoint::activate(_spine, p);
}

void CurvedShape::revert(){
	for(list<Curve*>::iterator it =_border.begin(); it!=_border.end(); it++)
			(*it)->reverse();
	updateIsopams();
}

void CurvedShape::flip(){
	for(list<Curve*>::iterator it =_border.begin(); it!=_border.end(); it++)
			(*it)->flipNormals();
	updateIsopams();
}


double CurvedShape::getHit(Vec3 p, Vec3 n, CurvePos& cp,  Vec3& phit){
	
	double tmin = 999999999999;
	bool hit = false;
	Vec3 hitp;
	for(list<Curve*>::iterator it =_border.begin(); it!=_border.end(); it++){

		double step = 0.0025;
		int size = (*it)->length() / step;
		Vec3 * pts = (*it)->toArr(size); // ArrCurve::resample(*it, 0.01, size);
		Vec3 p0(pts[0]);

		step = 1.0/(size-1);
		double ct = 0;
		for(int i=1; i<size;i++){
	
			Vec3 p1(pts[i]);
			Vec3 n1 = (p1 - p0);

			double len = n1.norm();
			double t = getIntersectionDist(p, n, p0, n1.normalize());
			double len0 = ( p + t*n - p0 ).norm();
			if (t>0 && t<tmin && len0 < len ){
				tmin = t;
				hit = true;
				cp.c = *it;
				cp.t = ct + step * (len0/len);
				phit = p + t*n ;
			}
			p0.set(p1);
			ct+=step;
		}
		delete [] pts;
	}

	if (hit) 
		return tmin;
	return -1;
}

double CurvedShape::getHit(Vec2 p, Vec2 n, CurvePos& cp){
	
	double tmin = 999999999999;
	bool hit = false;
	for(list<Curve*>::iterator it =_border.begin(); it!=_border.end(); it++){

		double step = 0.0025;
		int size = (*it)->length() / step;
		Vec3 * pts = (*it)->toArr(size); // ArrCurve::resample(*it, 0.01, size);
		//lazy projection
		Vec2 p0(pts[0]);

		step = 1.0/(size-1);
		double ct = 0;
		for(int i=1; i<size;i++){
	
			Vec2 p1(pts[i]);
			Vec2 n1 = (p1 - p0);

			double len = n1.norm();
			double t = getIntersectionDist(p, n, p0, n1.normalize());
			double len0 = ( p + t*n - p0 ).norm();
			if (t>0 && t<tmin && len0 < len ){
				tmin = t;
				hit = true;
				cp.c = *it;
				cp.t = ct + step * (len0/len);
			}
			p0.set(p1);
			ct+=step;
		}
		delete [] pts;
	}

	if (hit) 
		return tmin;
	return -1;
}


void CurvedShape::buildShapeNoSpine(){
	C2S c2s;
	list<Curve*> curves;
	curves.push_back(_spine);
	c2s.resampleCurves(curves);
	c2s.buildCorrespondances();

	c2s.filter(6);
	c2s.growCaps();
	
	F2M f2m;
	f2m.setCP(c2s.getCorrs().front());

	_mesh = f2m.buildMesh();
}

void CurvedShape::buildShapeBySpine(){
}

void CurvedShape::updateIsopams(){
	list<Isopam>::iterator it;   
	for( it = isopams.begin(); it != isopams.end(); it++)
			(*it).updateVerts();
}

void borderChanged(void*param){
	if (param)
		((CurvedShape*)param)->updateIsopams();
}

void CurvedShape::buildShapeBySpine(Curve* sp, int caps){
	if (!sp)
		return; 

	if (sp->type() == POINT_CURVE){
		//cout<<"c0"<<endl;
		buildRadialShape(sp, caps*4, VNUM);
		return;
	}

	int sz = sp->length() / SPINE_STEP;
	ArrCurve ac(sp->toArr(sz), sz, true);
	Vertex* * last = 0;
	Vertex* * first = 0; 
	int vnum = VNUM;
	for(int i=0; i<ac.size(); i++){
		
		Vec3 o = ac.getP(i);
		//Vec3 no = (o - Eye::get()->P).normalize();
		Vec3 tan = ac.getT(i);
		Vec2 up = (Vec3(0,0,1)%tan).normalize();
		//Vec3 pmid = Eye::get()->P + no*( PZ / (no*Eye::get()->N) );
		Vec2 pmid = o;

		ac.setP(i, pmid);
		CurvePos cp0, cp1;
		Vec3 ph0, ph1;
		double t0 = getHit(pmid, up, cp0);
		double t1 = getHit(pmid, -up, cp1);

		if (t0<0 || t1<0)
			continue;

		/*Vec3 p0 = pmid + up*t0;
		Vec3 p1 = pmid - up*t1;*/
		Vertex* * verts = createVerts(vnum);

		Isopam iso = Isopam(cp0, cp1, this);
		iso.setVerts(verts, vnum);
		isopams.push_back(iso);
		if (last){
			for(int i=0 ; i < vnum-1; i++)
				_mesh->addQuad(verts[i], verts[i+1], last[i+1], last[i]);
		}else{
			first = verts;
		}
		last = verts;
	}

	for(list<Curve*>::iterator it =_border.begin(); it!=_border.end(); it++)
		(*it)->setShapeChangedHandler(borderChanged, (void*)this);

	//radial caps:
	if (!caps)
		return;
	int vncap = vnum/2 + (vnum%2);
	int cap0 = 0;
	int cap1 = 0;
	double inc = PI / (caps+1);
		Vec3 n1y = (ac.getP(sz-1) - ac.getP(sz-5)).normalize();
		Vec3 n1x = (Vec3(0,0,1)%n1y).normalize();

		Vec3 n0y = (ac.getP(4) - ac.getP(0)).normalize();
		Vec3 n0x = (Vec3(0,0,1)%n0y).normalize();

		Vertex* *pre0 = first;
		Vertex* *pre1 = last;
		Vertex* pivot_vert0 = first[vncap-1];
		Vertex* pivot_vert1 = last[vncap-1];
		for(int i=0; i<caps;i++){
			double deg = (i+1)*inc;
			Vec3 n1 = n1x*cos(deg) + n1y*sin(deg);
			Vec3 n0 = n0x*cos(PI+deg) + n0y*sin(PI+deg);
			CurvePos cp0, cp1;
			//Vec3 ph0, ph1;
			double t0 = getHit(ac.getP(0), n0, cp0);
			double t1 = getHit(ac.getP(sz-1), n1, cp1);

			if (t0>0) {
				Vertex** v0s = createVerts(vncap);
				v0s[vncap-1] = pivot_vert0;
				Isopam iso0 = Isopam(cp0, this);						
				iso0.setVerts(v0s, vncap);
				isopams.push_back(iso0);
				for(int j=0; j<vncap-2; j++){
					if (i==0){
						_mesh->addQuad(v0s[j], v0s[j+1], pre0[vnum-j-2], pre0[vnum-j-1]);
						_mesh->addTriangle(v0s[j+1],pre0[vnum-j-2], pivot_vert0);
					}else{
						_mesh->addQuad(v0s[j], v0s[j+1], pre0[j+1], pre0[j]);
						_mesh->addTriangle(v0s[j+1], pre0[j+1], pivot_vert0);
					}
				}
				pre0 = v0s;
				cap0++;
			}

			if (t1>0){
				Vertex** v1s = createVerts(vncap);
				v1s[vncap-1] = pivot_vert1;
				Isopam iso1 = Isopam(cp1, this);
				iso1.setVerts(v1s, vncap);
				isopams.push_back(iso1);
				for(int j=0; j<vncap-2; j++){
					_mesh->addQuad(v1s[j], v1s[j+1], pre1[j+1], pre1[j]);
					_mesh->addTriangle(v1s[j+1], pre1[j+1],pivot_vert1);
				}
				pre1 = v1s;
				cap1++;
			}
			
		}
		for(int j = 0; j < vncap-2; j++){
			if (cap0){
				_mesh->addQuad(first[j], first[j+1], pre0[j+1], pre0[j]);
				_mesh->addTriangle(first[j+1], pre0[j+1],pivot_vert0);
			}
			if (cap1){
				_mesh->addQuad(last[vnum-j-1], last[vnum-j-2], pre1[j+1], pre1[j]);
				_mesh->addTriangle(last[vnum-j-2], pre1[j+1], pivot_vert1);
			}
		}

		//add feather
		if (!cap0){
			int i = 0;
			int num = sz*FADEIN;
			for(list<Isopam>::iterator it = isopams.begin(); i < num && it!=isopams.end(); it++){
				(*it).alpha0 = (i*1.0)/num;
				(*it).alpha1 = (*it).alpha0 ;//+ (num*0.025);
				i++;
			}
		}

		if (!cap1){
			int i = 0;
			int num = sz*FADEIN;
			for(list<Isopam>::iterator it = isopams.end(); i < num && it!=isopams.end(); it--){
				(*it).alpha0 = (i*1.0)/num;
				(*it).alpha1 = (*it).alpha0 + (num*0.025);
				i++;
			}
		}

		updateIsopams();
}

void CurvedShape::buildRadialShape(Curve* sp, int caps, int vnum){
	if (!sp || sp->type() != POINT_CURVE)
		return; 
	cout<<"-1"<<endl;
	Vertex* pivot_vert = Vertex::create(_mesh);
	pivot_vert->setP(sp->getP(0));
	pivot_vert->setC(RGB(0,0,_n_z));
	CurvePos cp_piv(sp,0);
	cout<<"-2"<<endl;

	Vertex* * pre = 0;
	Vertex* * first = 0;
	for(list<Curve*>::iterator it =_border.begin(); it!=_border.end(); it++){
		Curve* c = (*it);
		//double step = c->length() / (caps-1);
		for(int i=0; i<caps;i++){
			CurvePos cp;
			cp.t = i*1.0/(caps-1);
			cp.c = c;
			Vertex* * verts = createVerts(vnum);
			verts[vnum-1] = pivot_vert;
			Isopam iso(cp, cp_piv, this, false);
			iso.setVerts(verts, vnum);
			isopams.push_back(iso);
			if (pre){
				for(int j=0; j<vnum-2; j++)
					_mesh->addQuad(verts[j], verts[j+1], pre[j+1], pre[j]);
				_mesh->addTriangle(verts[vnum-2], pre[vnum-2], pivot_vert);
			} else
				first = verts;
			pre = verts;
		}
		if (pre){
			for(int j=0; j<vnum-2; j++)
				_mesh->addQuad(first[j], first[j+1], pre[j+1], pre[j]);
			_mesh->addTriangle(first[vnum-2], pre[vnum-2], pivot_vert);
		}
		(*it)->setShapeChangedHandler(borderChanged, (void*)this);
	}
	sp->setShapeChangedHandler(borderChanged, (void*)this);
}

void CurvedShape::buildBorderBySpine(Curve* sp, int caps){
	if (!sp)
		return; 
	int sz = StrokeManager::CONTROL_POINTS;
	ArrCurve ac(sp->toArr(sz), sz, true);

	Vec3 *p0s, *p1s;
	if (!caps){
		p0s = new Vec3[sz];
		p1s = new Vec3[sz];
	}else{
		p0s = new Vec3[(sz+caps)*2];
		p1s = &p0s[sz+caps];
	}
	for(int i=0; i<ac.size(); i++){
		
		Vec3 o = ac.getP(i);
		//Vec3 no = (o - Eye::get()->P).normalize();
		Vec3 tan = ac.getT(i);
		Vec3 up = (Vec3(0,0,1)%tan).normalize();
		Vec3 pmid = o; //Eye::get()->P + no*( PZ / (no*Eye::get()->N) );

		p0s[i] = pmid + up*RADIUS;
		p1s[sz-1-i] = pmid - up*RADIUS;

		//to avoid degenerous cases
		if (i==0){
			p0s[i] = p0s[i] - tan*0.0025;
			p1s[sz-1-i] = p1s[sz-1-i] - tan*0.0025;
		}else if (i==ac.size()-1){
			p0s[i] = p0s[i] + tan*0.0025;
			p1s[0] = p1s[0] + tan*0.0025;
		}
	}
	if (!caps){
		Curve* c0 = new BSpline(p0s, sz, false);
		Curve* c1 = new BSpline(p1s, sz, false);
		BSpline* b0 = ((BSpline*)c0);
		_border.push_back(c0);
		_border.push_back(c1);
	}else{
		double inc = PI / (caps+1);
		Vec3 n1y = (ac.getP(sz-1) - ac.getP(sz-5)).normalize();
		Vec3 n1x = (Vec3(0,0,1)%n1y).normalize();

		Vec3 n0y = (ac.getP(4) - ac.getP(0)).normalize();
		Vec3 n0x = (Vec3(0,0,1)%n0y).normalize();

		for(int i=0; i<caps;i++){
			double deg = (i+1)*inc;
			p0s[sz+i] = ac.getP(sz-1) + n1x*RADIUS*cos(deg) + n1y*RADIUS*sin(deg);
			p0s[sz*2+caps+i] = ac.getP(0) + n0x*RADIUS*cos(PI+deg) + n0y*RADIUS*sin(PI+deg);
		}
		Curve* c = new BSpline(p0s, (sz+caps)*2, true);
		_border.push_back(c);
	}
}

Vertex* * CurvedShape::createVerts(int num){
	Vertex* * v = new Vertex*[num];
	for(int i=0; i<num; i++)
		v[i] = Vertex::create(_mesh);
	return v;
}

/*
void CurvedShape::buildMeshWithSpineParametrically(
Vertex* * pre = 0;
	double step = 0.05;
	for(double t=0; t<1+step; t+=step ){

		CurvePos cp0, cp1;
		cp0.t = t;
		cp0.c = c0;
		cp1.t = 1.0-t;
		cp1.c = c1;

		Isopam * iso = new Isopam(cp0, cp1, 0.05, false);

		Vertex* * verts = createVerts(6);
		isomap[verts] = iso;
		setupVerts(verts, 6, iso);
		if (pre){
			for(int i=0 ; i < 5; i++)
				_mesh->addQuad(verts[i], verts[i+1], pre[i+1], pre[i]);
		}
		pre = verts;
	}
)
*/

void Isopam::updateVerts(){

	if (!hasmid){
		updateVertsNoP();
		return;
	}

	float flat = (flatness>0)? flatness : cshape->flatness();

	Vec4 to(0, 0, cshape->nz(), alpha1);

	verts[0]->setP(cp0.P());
	verts[0]->setC(cp0.N(), alpha0);

	verts[vnum-1]->setP(cp1.P());
	verts[vnum-1]->setC(cp1.N(), alpha0);
	Vec3 n = verts[vnum-1]->P() - verts[0]->P();
	
	if (vnum%2){ // odd vnumber
		verts[vnum/2]->setP(verts[0]->P() + n*0.5);
		verts[vnum/2]->setC(to);
	} else {
		verts[vnum/2-1]->setP(verts[0]->P() + n*(0.5 - flat));
		verts[vnum/2-1]->setC(to);

		verts[vnum/2]->setP(verts[0]->P() + n*(0.5 + flat));
		verts[vnum/2]->setC(to);
	}
	
	//Vec3 n = verts[vnum-1]->P() - verts[0]->P();
	//now other vertices
	int vnum2 = (vnum/2) + (vnum%2) - 2;
	double s = (0.5 - flat) / (vnum2+1);
	for(int i = 1; i < vnum2+1; i++){
		double t = i*s;
		verts[i]->setP(verts[0]->P() + n*t);
		verts[vnum-1-i]->setP(verts[vnum-1]->P() - n*t);

		verts[i]->setC(verts[0]->C()*t*2 + to*(1-t*2));
		verts[vnum-1-i]->setC(verts[vnum-1]->C()*t*2 + to*(1-t*2));
	}
}

void Isopam::updateVertsNoP(){

	float flat = (flatness>0)? flatness : cshape->flatness();

	verts[0]->setP(cp0.P());
	verts[0]->setC(cp0.N(), alpha0);
	if (twopointed){
		verts[vnum-1]->setP(cp1.P());
		verts[vnum-1]->setC(Vec3(0,0,1), alpha1);
	}
	Vec3 n = verts[vnum-1]->P() - verts[0]->P();
	double s = (0.5 - flat) / (vnum-2);
	if (twopointed)
		s = (1.0 - flat) / (vnum-2);
	for(int i = 1; i < vnum-1; i++){
		double t = i*s;
		verts[i]->setP(verts[0]->P() + n*t);
		verts[i]->setC(verts[0]->C()*(1-flat-t) + verts[vnum-1]->C()*(t+flat));
	}
}

void CurvedShape::drawBorderAux(){
	/*list<Isopam>::iterator it; 
	cout<<"size::::"<<isopams.size()<<endl;
	for( it = isopams.begin(); it != isopams.end(); it++){
		CurvePos cp0 = (*it).cp0;
		CurvePos cp1 = (*it).cp1;
		cp0.draw();
		if ((*it).twopointed)
			cp1.draw();
	}*/
	int ndir = _border.front()->normaldir();
	int sz = _border.front()->size() / 2;
	ArrCurve ac(_border.front()->toArr(sz), sz, true);
	for(int i=0; i < ac.size()-1; i++){
		Vec3 p0 = ac.getP(i);
		Vec3 n0 = ((ac.getT(i) % Eye::get()->N ).normalize()*ndir);
		Vec3 p1 = p0 + n0*0.035;

		glBegin(GL_LINES);
			//glColor3f(n0.x, n0.y, n0.z);
			glColor3f((n0.x+1)/2.0 , (n0.y+1)/2.0, n0.z);
			glVertex3f(p0.x, p0.y, p0.z);
			glVertex3f(p1.x, p1.y, p1.z);
		glEnd();

	}

	glBegin(GL_LINE_LOOP);
	for(int i=0; i < ac.size()-1; i++){
		Vec3 n0 = ((ac.getT(i) % Eye::get()->N ).normalize()*ndir);
		//glColor3f(n0.x, n0.y, n0.z);
		glColor3f((n0.x+1)/2.0 , (n0.y+1)/2.0  , n0.z);
		Vec3 p0(ac.getP(i));
		glVertex3f(p0.x, p0.y, p0.z);
	}
	glEnd();
}

/*
void CurvedShape::drag(Vec3 p){
	Shape::drag(p);
}*/

/*
	_c2s->resampleCurves(_border);
	_c2s->buildCorrespondances();
	_c2s->filter(3);
	_c2s->growCaps();
	((F2M*)_c2m)->setCP(_c2s->getCorrs().front());
	((F2M*)_c2m)->buildMesh();
	((F2M*)_c2m)->blendMesh();
*/