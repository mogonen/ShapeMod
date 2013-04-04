#include "MeshShape.h"

void MeshShape::draw(int m){
	if (_control)
		drawBorder();
}

void MeshShape::drawBorder(){
	glColor3f(1.0, 0, 0);
	glLineWidth(2.0);
	glBegin(GL_LINES);
	list<Vertex*> verts = _control->verts();
	for(list<Vertex*>::iterator itv = verts.begin(); itv != verts.end(); itv++){
		Vertex* v = *itv;
		if ((*itv)->next ){
			Vec3 p0 = (*itv)->P();
			Vec3 p1 = (*itv)->next->P();
			glVertex3f(p0.x, p0.y, p0.z);
			glVertex3f(p1.x, p1.y, p1.z);
		}
	}
	glEnd();
	glColor3f(0,0,0);
	_control->draw(Mesh::WIRE_OVERRIDE);
	/*glBegin(GL_LINE_LOOP);
		glVertex3f(tri[0].x, tri[0].y, tri[0].z);
		glVertex3f(tri[1].x, tri[1].y, tri[1].z);
		glVertex3f(tri[2].x, tri[2].y, tri[2].z);
	glEnd();*/
}

void MeshShape::onClick(const Vec3& p){
	 ControlPoint::activate(this, p);
};

void MeshShape::exec(int cmd, void* param){

	if(cmd == Command::UNDO){ //simple UNDO
		if (last_v){
			Vertex* tmp = 0;
			if (last_v->val())
				tmp = last_v->to.front();
			_control->removeVertex(last_v);
			last_v = tmp;
		}
		return;
	}

	if (!param && cmd != Command::CLICK)
		return;

	if (_control->sizeF()){
		insertSegment( *((Vec3*)param) );
		return;
	}


	ControlPoint* cp = 0;
	Vec3 p = *((Vec3*)param);
	cp = ControlPoint::activate(this, p);

	if (_control->sizeF()){
		if (!cp)
			insertSegment(p);
		return;
	}

	if (cp)
		last_v = 0;

	Vertex* v = 0;
	if (cp){
		v = (Vertex*)cp->retrieve();
		//if (last_v) v->to.push_back(last_v);
	}else{
		v = Vertex::create(_control);
		Vec3 * p0 = new Vec3(p);
		v->setP(p0);

		cp = ControlPoint::create(this, p0);
		cp->store(v);
	}

	if (last_v){//double link
		last_v->to.push_back(v);
		v->to.push_back(last_v); 
	}

	last_v = v;
}

void MeshShape::buildControlMesh(){
	if (_control->sizeF())
		return;
	ControlPoint::remove(this);
	makeOutline4();
	_control->extractBorder();
}

void MeshShape::makeOutline(){
	makeOutline(0, 0, _control->firstV());
};

void MeshShape::makeOutline(Vertex** p_ov, Vertex* v0, Vertex* v){

	bool end = false;
	if (!v0)
		v0 = v;
	while(v->to.size()<2 && !end){

		Vertex* v1 = v; 
		if (v->to.size())
			v1 = v->otherLink(v0);
		else
			end = true;

		Vec3 tan = (v1->P() - v0->P()).normalize();
		Vertex** new_v = this->getOutlineVerts(v->P(), tan, _control, RAD);

		//introduce a quad face!
		if (p_ov){

			_control->addQuad(v, v0, p_ov[0], new_v[0]);
			_control->addQuad(v, new_v[1], p_ov[1], v0);

			p_ov[0]->setNext(new_v[0]);
			new_v[1]->setNext(p_ov[1]);
		}else{
			new_v[1]->setNext(v0);
			v0->setNext(new_v[0]);
		}

		p_ov = new_v;                  
		v0 = v;
		v = v1;		
	}

	if (end){
		p_ov[0]->setNext(v);
		v->setNext(p_ov[1]);
	}

	if (!end && v->to.size()>1){
		// this is a joint!
		Vertex* * branches = v->sortTo(v0);
		int sz = v->to.size();
		Vec3 p0 = v0->P();
		Vec3 p00 = p0;

		Vertex* o0 = 0;
		Vertex* o00 = 0;
		for(int i=0; i<sz+1; i++){
			Vec3 tan = (i<sz)? (branches[i]->P() - p0).normalize() : (p00-p0).normalize();
			Vec3 n = (Vec3(0,0,1) % tan).normalize();
			Vec3 * p = new Vec3(v->P() + n*RAD);
			Vertex* o = _control->addVertex(p);
			ControlPoint::create(this, p);
			if (o0){
				Vertex* oo[] = {o0,o};
				makeOutline(oo, v, branches[i-1]);
			}else
				o00 = o;
			o0 = o;
			p0 = branches[i%sz]->P();
		}

		_control->addQuad(v, v0, p_ov[0], o00);
		_control->addQuad(v, o0, p_ov[1], v0);

		p_ov[0]->setNext(o00);
		o0->setNext(p_ov[1]);
	}
}

void MeshShape::makeOutline2(){
	int osz = _control->size()*_control->size();
	OutlineMap omap = new Vertex**[osz];
	for(int i = 0; i < osz; i++)
		omap[i] = 0x00;
	Mesh* spine = _control->deepCopy();
	delete _control;
	_control = new CMesh();
	makeOutline2(0, 0, spine->firstV(), omap, _control->size());
	delete spine;
	delete omap;
};

void MeshShape::makeOutline2(Vertex** p_ov, Vertex* v0, Vertex* v, OutlineMap& omap, int osz){

	bool end = false;
	if (v0){
		int eid = v0->edgeUid(v);
		if (omap[eid]){
			Vertex** ov = omap[eid];
			_control->addQuad(p_ov[0], ov[1] , ov[0], p_ov[1]);
			return;
		}
	}else{
		if (v->to.size()<=2){ // if not a joint
			v0 = v;
			Vertex* v0_2 = (v->to.size()==2)?v->to.back():v;
			v = v->to.front();
			Vec3 tan = (v->P() - v0_2->P()).normalize();
			p_ov = this->getOutlineVerts(v0->P(), tan, _control, RAD);
			if (v0_2 != v0)
				omap[v0_2->edgeUid(v0)] = p_ov;
		}
	}

	//this builds an WEdge between joint verticies
	while( v->to.size() < 3 && !end ){ // while vert is not joint or end

		int eid = v0->edgeUid(v);
		if (omap[eid]){
			Vertex** ov = omap[eid];
			_control->addQuad(p_ov[0], ov[0] , ov[1], p_ov[1]);
			break;
		}

		Vertex* v1 = v;
		if (v->to.size()>1)
			v1 = v->otherLink(v0); //so, will always go forward
		else
			end = true;

		//introduce a quad face!
		Vec3 tan = (v1->P() - v0->P()).normalize();
		Vertex* * new_v = this->getOutlineVerts(v->P(), tan, _control, RAD);
		_control->addQuad(p_ov[0], new_v[0], new_v[1], p_ov[1]);

		p_ov = new_v;                  
		v0 = v;
		v = v1;
	}

	if (v->to.size()>2 ){ //joint vertex condition
		omap[ v0->edgeUid(v) ] = p_ov;
		if (v->flag.is(Flag::VISITED))
			return;

		v->flag.set(Flag::VISITED);
		Vertex** branches = v->sortTo(v0);
		int sz = v->to.size();
		Face* fmid = new Face(sz*2);
		_control->addFace(fmid);
		for(int i=0; i<sz; i++){
			Vec3 tan = (branches[i]->P() - v->P()).normalize();
			Vec3 p = v->P() + tan*RAD;
			Vertex** vs = this->getOutlineVerts(p, tan, _control, RAD);
			if (i)
				makeOutline2(vs, v, branches[i], omap, osz);
			fmid->verts[i*2] = vs[0];
			fmid->verts[i*2+1] = vs[1];
		}
		_control->addQuad(p_ov[0], fmid->verts[1], fmid->verts[0], p_ov[1]);
		fmid->update();
	}
}

Vertex** MeshShape::getOutlineVerts(const Vec3& o, const Vec3& tan, Mesh*m, double rad, bool control){
	Vertex**v = new Vertex*[2];
	Vec3 n = (Vec3(0,0,1) % tan).normalize();
	Vec3 * p0 = new Vec3( o + n*rad );
	Vec3 * p1 = new Vec3( o - n*rad );

	v[0] = m->addVertex(p0);
	v[1] = m->addVertex(p1);
	if (control){
		ControlPoint::create(this, p0);
		ControlPoint::create(this, p1);
	}
	return v;
}

void MeshShape::makeOutline3(){
	int osz = _control->size() * _control->size();
	OutlineMap omap = new Vertex**[osz];
	for(int i = 0; i < osz; i++)
		omap[i] = 0x00;

	Mesh* omesh = new Mesh();
	list<Vertex*> verts = _control->verts();

	for(list<Vertex*>::iterator it = verts.begin(); it!= verts.end();it++){
		//three type verts: Joint (val:3) / Bridge (val:2) / End (val:1)
		Vertex* v = (*it);
		if (v->val()>2){ // a joint
			Vertex** branches = v->sortTo(0);
			int sz = v->to.size();
			Face* fmid = new Face(sz*2);
			omesh->addFace(fmid);
			for(int i = 0; i < sz; i++){
				Vec3 tan = (branches[i]->P() - v->P()).normalize();
				Vec3 p = v->P() + tan*RAD;
				Vertex** vs = this->getOutlineVerts(p, tan, omesh, RAD);
				fmid->verts[i*2] = vs[0];
				fmid->verts[i*2+1] = vs[1];
				
				if (Vertex** vs1 = omap[branches[i]->edgeId(v)])
					omesh->addQuad(vs[0], vs1[1], vs1[0], vs[1]);
				else
					omap[v->edgeId(branches[i])] = vs;
			}
			fmid->update();

		}else{ //a bridge or an end vertex
			Vertex* v0 = ( v->val() == 2 )? v->to.back() : v;
			Vertex* v1 = v->to.front();
			Vec3 tan = (v1->P() - v0->P()).normalize();
			Vertex** vs = this->getOutlineVerts(v->P(), tan, omesh, RAD);
			int e = 0;
			if (Vertex** vs1 = omap[v1->edgeId(v)])
				omesh->addQuad(vs[0], vs1[1], vs1[0], vs[1]);
			else
				omap[v->edgeId(v1)] = vs;
			if (v0!=v){ //inverse order in neg tangent direction

				if (Vertex** vs1 = omap[v0->edgeId(v)]){
					omesh->addQuad(vs[1], vs1[1], vs1[0], vs[0]);
				}else{
					//Vertex* vs_i[2] = {vs[1], vs[0]}; // this causes error! figure it out!!!!
					Vertex** vs_i = new Vertex*[2]; 
					vs_i[0] = vs[1]; vs_i[1] = vs[0];
					omap[v->edgeId(v0)] = vs_i;
				}
			}
		}
	}

	delete _control;
	_control = new CMesh(omesh);
}

void MeshShape::makeOutline4(){

	int osz = _control->size() * _control->size();
	OutlineMap omap = new Vertex**[osz];
	for(int i = 0; i < osz; i++)
		omap[i] = 0x00;

	Mesh* omesh = new Mesh();
	list<Vertex*> verts = _control->verts();

	for(list<Vertex*>::iterator it = verts.begin(); it!= verts.end();it++){
		//three type verts: Joint (val:3) / Bridge (val:2) / End (val:1)
		Vertex* v = (*it);
		if (v->val()>2){ // a joint
			Vertex** branches = v->sortTo(0);
			int sz = v->to.size();
			Face* fmid = new Face(sz);
			omesh->addFace(fmid);

			for(int i = 0; i < sz; i++){
				Vec3 n = (branches[i]->P() - v->P()).normalize()*0.5 + (branches[(i-1+sz)%sz]->P() - v->P()).normalize()*0.5;
				Vec3 * p = new Vec3( v->P() + n*RAD*2 );
				Vertex* vn = omesh->addVertex(p);
				fmid->verts[i] = vn;
				//ControlPoint::create(this, p);				
			}

			fmid->update();
			//now update omesh
			for(int i = 0; i< sz; i++ ){
				Vertex* v0 = fmid->verts[i];
				Vertex* v1 = fmid->verts[(i+1)%sz];
				if (Vertex** vs1 = omap[branches[i]->edgeId(v)])
					omesh->addQuad(v0, vs1[1], vs1[0], v1);
				else{
					Vertex** vs = new Vertex*[2];
					vs[0] = v0; vs[1] = v1;
					omap[v->edgeId(branches[i])] = vs;
				}
			}

		}else{ //a bridge or an end vertex
			Vertex* v0 = ( v->val() == 2 )? v->to.back() : v;
			Vertex* v1 = v->to.front();
			Vec3 tan = (v1->P() - v0->P()).normalize();
			Vertex** vs = this->getOutlineVerts(v->P(), tan, omesh, RAD);
			int e = 0;
			if (Vertex** vs1 = omap[v1->edgeId(v)])
				omesh->addQuad(vs[0], vs1[1], vs1[0], vs[1]);
			else
				omap[v->edgeId(v1)] = vs;

			if (v0!=v){ //inverse order in neg tangent direction

				if (Vertex** vs1 = omap[v0->edgeId(v)]){
					omesh->addQuad(vs[1], vs1[1], vs1[0], vs[0]);
				}else{
					//Vertex* vs_i[2] = {vs[1], vs[0]}; // this causes error! figure it out!!!!
					Vertex** vs_i = new Vertex*[2]; 
					vs_i[0] = vs[1]; vs_i[1] = vs[0];
					omap[v->edgeId(v0)] = vs_i;
				}
			}
		}
	}

	delete _control;
	/*omesh->extractBorder();
	subdivCC(omesh);*/
	_control = new CMesh(omesh);
}


void MeshShape::genControlPoints(){
	list<Vertex*> verts = _control->verts();
	for(list<Vertex*>::iterator it = verts.begin(); it!= verts.end(); it++){
			ControlPoint::create(this, (*it)->p0);
	}
}

int MeshShape::save(std::ofstream& outf){
	return _control->exportOBJ(outf);
}

int MeshShape::load(std::ifstream& inf){
	int err = _control->load(inf);
	 //generate control points
	list<Vertex*> verts = _control->verts();
	for(list<Vertex*>::iterator it = verts.begin(); it!= verts.end();it++)
		ControlPoint::create(this, (*it)->p0);

	if (_control->sizeF())
		_control->extractBorder();

	return err;
}

void MeshShape::insertSegment(const Vec3& p){

	CEdge* e = _control->pickEdge(p);
	if(!e)
		return;

	Corner* c0 = e->split();
	Corner* c1 = c0->vNext();
	CFace* endf = (c1)?c1->f:0;
	ControlPoint::create(this, c0->v->p0);

	while(c0 && c0->f!=endf){
		Corner* c01 = c0->next->next->e->split(0.5,c0);
		Corner* c0n = c01->vNext();
		CEdge::insert(c0, c01);
		ControlPoint::create(this, c01->v->p0);
		c0 = c0n;
	}

	if (c0 && c0->f == endf)
		CEdge::insert(c0, c0->next->next->next);
	else while(c1){
		Corner* c11 = c1->next->next->e->split(0.5, c1);
		Corner* c1n = c11->vNext();
		CEdge::insert(c1, c11);
		ControlPoint::create(this, c11->v->p0);
		c1 = c1n;
	}
}


/*
void MeshShape::insertSegment(const Vec3& p){

	if (!_control->sizeF())
		return;

	list<Face*> faces = _control->faces();
	Face* f = 0;
	for(list<FacePtr>::iterator it = faces.begin(); it!= faces.end();it++){
		if ((*it)->isIn(p)){
			f = *it;
			break;
		}
	}

	if (!f)
		return;

	int i0 = 0;
	for(; i0<f->size; i0++)
		if (f->v(i0)->next == f->v(i0+1))
			break;
	
	Vertex* v0 = _control->addVertex( f->v(i0)->P()*0.5 + f->v(i0+1)->P()*0.5 );
	Vertex* v1 = _control->addVertex( f->v(i0+2)->P()*0.5 + f->v(i0+3)->P()*0.5 );
	_control->addQuad(v0, f->v(i0+1) , f->v(i0+2), v1);
	f->v(i0)->setNext(v0);
	v0->setNext(f->v(i0+1));

	f->v(i0+2)->setNext(v1);
	v1->setNext(f->v(i0+3));

	f->verts[(i0+1)%4] = v0;
	f->verts[(i0+2)%4] = v1;

	ControlPoint::create(this, v0->p0);
	ControlPoint::create(this, v1->p0);
}

void MeshShape::getHits(list<ArrCurve*> border, const Vec2& p, const Vec2& n, double& tmin0, double& tmin1){
	
	tmin0 = 999999999999;
	tmin1 = 999999999999;
	bool hit0 = false;
	bool hit1 = false;

	for(list<ArrCurve*>::iterator it =border.begin(); it!=border.end(); it++){

		int size = (*it)->size();
		Vec3 * pts = (*it)->toArr(); 
		//lazy projection
		Vec2 p0(pts[0]);
		for(int i=1; i<size;i++){
	
			Vec2 p1(pts[i]);
			Vec2 n1 = (p1 - p0);

			double len = n1.norm();
			double t0 = getIntersectionDist(p, n, p0, n1.normalize());
			double t1 = getIntersectionDist(p, -n, p0, n1.normalize());

			double len0 = ( p + t0*n - p0 ).norm();
			double len1 = ( p - t1*n - p0 ).norm();
			if (t0>0 && t0<tmin0 && len0 < len ){
				tmin0 = t0;
				hit0 = true;
			}

			if (t1>0 && t1<tmin1 && len1 < len ){
				tmin1 = t1;
				hit1 = true;
			}

			p0.set(p1);
		}
	}

	tmin0 = (hit0)?tmin0:-1;
	tmin1 = (hit1)?tmin1:-1;
}

void MeshShape::flatShade(){

	list<Face*> faces = _control->faces();
 
	for(list<FacePtr>::iterator it = faces.begin(); it!= faces.end();it++){
		Face* f = *it;
		if (f->size!=4)
			continue;
		f->update();
		Vec2 pAB[2];
		for(int i=0;i<2;i++){
			pAB[i] = getIntersection(f->v(i)->P() ,(f->v(i+1)->P() - f->v(i)->P()).normalize(),  f->v(i+3)->P(), (f->v(i+2)->P() - f->v(i+3)->P()).normalize());
			tri[i].set(pAB[i].x, pAB[i].y, -1.0);
		}
		double k = pAB[0]*pAB[1];
		double x = (k/pAB[0].y - k/pAB[1].y) / ( pAB[0].x / pAB[0].y - pAB[1].x / pAB[1].y);
		double y = (k/pAB[0].x - k/pAB[1].x) / ( pAB[0].y / pAB[0].x - pAB[1].y / pAB[1].x);

		//pAB[0].print(); pAB[1].print();
		Vec2 pAB2(x, y);
		tri[2].set(x, y, -1.0);
		//cout<<k<<" p0*p2="<<pAB[0]*pAB2<<" p1*p2="<<pAB[1]*pAB2<<endl;

		Vec3 n(x,y,-1);
		n = -n.normalize();
		//cout<<k<<"  "<<x<<","<<y<<endl;n.print();
		f->setC(Vec4(n));
		f->setN(n);
	}
}

RGBA* MeshShape::pickFromQuad(const Vec3& p0, const Vec3& u, const Vec3& v, int x, int y, BitMap *bmap){

	RGBA min(1.0,1.0,1.0), max, mean;
	int n = 0;
	for(int i = 0 ; i < x; i++){
		for(int j = 0 ; j < y; j++){
			Vec3 p = p0 + u*(i*1.0/x) + v*(j*1.0/y);
			int * pp = toScreen(p);
			Byte* col = bmap->get(pp[0], pp[1]);
			RGBA rgba = toRGBA(col);

			if(rgba.norm() < min.norm())
				min.set(rgba);

			if(rgba.norm() > max.norm())
				max.set(rgba);
			mean= mean+rgba;
			n++;
		}
	}
	mean = mean / n;
	RGBA res[3] = {min, max, mean};
	return res;
}

void MeshShape::flatShade(){

	list<Face*> faces = _control->faces();
 
	for(list<FacePtr>::iterator it = faces.begin(); it!= faces.end();it++){
		Face* f = *it;
		if (f->size!=4)
			continue;
		f->update();
		Vec3 n0, n1;
		for(int i=0;i<2;i++){
			Vec3 pAB = getIntersection(f->v(i)->P() ,(f->v(i+1)->P() - f->v(i)->P()).normalize(),  f->v(i+3)->P(), (f->v(i+2)->P() - f->v(i+3)->P()).normalize());
			double c = sqrt( 1.0 / (pAB.x*pAB.x + pAB.y*pAB.y + 1) );
			if (i == 0)
				n0.set(pAB.x*c, pAB.y*c, c);
			else
				n1.set(pAB.x*c, pAB.y*c, c);
			//ControlPoint::create(this, new Vec3(pAB));
		}
		Vec4 n = (n0.normalize() % n1.normalize());
		f->setC(n);
		f->setN(Vec3(n.x, n.y, n.z));
	}
}

/*
Shape* MeshShape::load(ifstream inf){

}
*/

	/*
	//_border = new Vertex*[_control->size()*2];
	_bordersize = 1;
	for(Vertex* v = _border[0]->next; v&&v!=_border[0]; v = v->next)
		_border[_bordersize++] = v;
	_bordersize0 = _bordersize;
	_border0 = new Vertex*[_bordersize];
	for(int i = 0; i < _bordersize; i++ )
		_border0[i] = _border[i];*/


	/*Vec3* border_p = new Vec3[_subdiv->size()];
	Vertex** newborder = new Vertex*[_bordersize*2];
	for(int i=0; i<_bordersize; i++){
		Vertex* v  = _border[i];
		Vertex* v1 = _border[(i+1)%_bordersize];
		border_p[v->id()].set( (_border[(i-1+_bordersize)%_bordersize]->P() + v1->P() + v->P()*6)/8.0 );
		int ei = (v->id()<v1->id())?( v->id() + v->id()*vs):(v1->id() + v->id()*vs);
		Vec3 pmid = (v->P() + v1->P())*0.5; 
		evs[ei] = _subdiv->addVertex(pmid);
		newborder[i*2] = v;
		newborder[i*2+1] = evs[ei];
	}
	_border = newborder;
	_bordersize*=2; */

	/*
	glBegin(GL_LINE_LOOP);
	for(int i=0; i<_bordersize; i++){
		Vec3 p  = _border[i]->P();
		glVertex3f(p.x, p.y, p.z);
	}
	glEnd();
	
	
	

void MeshShape::makeOutline2(Vertex** p_ov, Vertex* v0, Vertex* v, OutlineMap& omap){

	bool end = false;
	if (!v0)
		v0 = v;

	while((v0 == v)||(v->to.size()<2 && !end)){

		if (v->hasLink(v0) ){
			OutlineMap::iterator it = omap.find(v); 
			if ( it!=omap.end() ){
				Vertex** ov = it->second;
				_control->addQuad(p_ov[0], ov[1] , ov[0], p_ov[1]);
				p_ov[0]->setNext(ov[1]);
				ov[0]->setNext(p_ov[1]);
			}else
				omap[v0] = p_ov;
			return;
		}

		Vertex* v1 = v;
		if (v->to.size())
			v1 = v->otherLink(v0                     );
		else
			end = true;

		Vec3 tan = (v1->P() - v0->P()).normalize();
		Vertex* * new_v = this->getOutlineVerts(v->P(), tan, _control, RAD);
		//introduce a quad face!
		if (p_ov){
			_control->addQuad(p_ov[0], new_v[0], new_v[1], p_ov[1]);
			p_ov[0]->setNext(new_v[0]);
			new_v[1]->setNext(p_ov[1]);
		}else{
			//_border_v0 = new_v[0];
			//_control->border_v0 = new_v[0];
			//new_v[1]->setNext(new_v[0]);
			omap[v] = new_v;
		}

		p_ov = new_v;                  
		v0 = v;
		v = v1;
	}

	if ( v->hasLink(v0) ){ //looping back to the joint vert
		OutlineMap::iterator it = omap.find(v); 
		if ( it!=omap.end() ){
			Vertex** ov = it->second;
			_control->addQuad(p_ov[1], ov[1] , ov[0], p_ov[0]);
			//p_ov[0]->setNext(ov[1]);
			//ov[0]->setNext(p_ov[1]);
		}else
			omap[v0] = p_ov;
	}else if (end){
		p_ov[0]->setNext(p_ov[1]);
	}else if (v->to.size()>1){
		// this is a joint!
		Vertex* * branches = v->sortTo(v0);
		int sz = v->to.size();
		Face* fmid = new Face((sz+1)*2);
		_control->addFace(fmid);
		int vi = 0;
		Vertex** vs0 = 0;
		Vertex** vs00 = 0;
		for(int i=0; i<sz+1; i++){
			Vec3 tan = (i<sz)? (branches[i]->P() - v->P()).normalize() : (v0->P() - v->P()).normalize();
			Vec3 p = v->P() + tan*RAD;
			Vertex** vs = this->getOutlineVerts(p, tan, _control, RAD);

			if (i<sz)
				makeOutline2(vs, v, branches[i], omap);

			fmid->verts[vi++] = vs[0];
			fmid->verts[vi++] = vs[1];

			if (vs0)
				vs0[1]->setNext(vs[0]);
			else
				vs00 = vs;
			vs0 = vs;
		}
		fmid->update();
		vs0[1]->setNext(vs00[0]);
		p_ov[0]->setNext(vs0[1]);
		vs0[0]->setNext(p_ov[1]);
		_control->addQuad(p_ov[0], vs0[1] , vs0[0], p_ov[1]);
	}
}

static int* (*toScreen)(const Vec3& p);
void MeshShape::colorize(Mesh* m, BitMap* bmap, int ci){

	list<Vertex*> verts = _subdiv->verts();
	for(list<Vertex*>::iterator it = verts.begin(); it!= verts.end();it++){
		Vertex* v = (*it);
		int * pp = toScreen(v->P());
		Byte* col = bmap->get(pp[0], pp[1]);
		RGBA rgba = toRGBA(col);
		rgba.w = 1.0;
		v->setC(rgba);
	}

	/*
	list<Face*> faces = _subdiv->faces();
	for(list<FacePtr>::iterator itf = faces.begin(); itf!= faces.end(); itf++){
		Face* f = (*itf);
		Vertex* v0 = 0;
		Vec3 p01_0;
		for(int i = 0; i < f->size+1; i++){
			Vec3 p0 = f->v(i)->P();
			Vec3 p1 = f->v(i+1)->P();
			Vec3 p01 = (p0 + p1)*0.5;
			if (v0){
				RGBA* cols = pickFromQuad(f->P(), (p01_0 - f->P()), (p01 - f->P()), 10, 10, bmap);
				v0->v4_0.set(v0->v4_0 + cols[0]);
				v0->v4_1.set(v0->v4_1 + cols[1]);
				v0->v4_2.set(v0->v4_2 + cols[2]);
				v0->flag.inc();
			}
			v0 = f->v(i);
			p01_0.set(p01);
		}
	}

	for(list<Vertex*>::iterator it = verts.begin(); it!= verts.end();it++){
		Vertex* v = (*it);
		int n = v->flag.getC();
		if (ci == 0)
			v->setC(v->v4_0);
		else if (ci == 1)
			v->setC(v->v4_1);
		else
			v->setC(v->v4_2);

		v->flag.reset();
		v->resetStorage();
	}
}	
*/