#include "CMesh.h"

Corner*  CEdge::split(double t, Corner* c){

	bool haslink = c0->v->next == c0->next->v; 
	Vec3 p = c0->v->P()*t + c0->next->v->P()*(1-t);
	Vertex* v = c0->v->mesh()->addVertex(p);
	CEdge* enew = create((CMesh*)c0->v->mesh());

	Corner* c0new = new Corner(v, c0->f);
	enew->set(c0new);
	Corner* c0n = c0->next;
	c0->setNext(c0new);
	c0new->setNext(c0n);
	enew->c0 = c0new;

	Corner* c1new =0;
	if (c1){
		c1new = new Corner(v, c1->f);
		enew->set(c1, 1);
		Corner* c1n = c1->next;
		c1->setNext(c1new);
		c1new->setNext(c1n);
		set(c1new, 1);
	}

	//border linking
	if (haslink){
		Vertex* vn = c0->v->next;
		c0->v->setNext(v);
		v->setNext(vn);
	}

	return (c && c->f != c0->f)?c1new :c0new;
}


Corner* Corner::vNext(){
	Corner* c1 = e->other(this);
	return (c1)?c1->next:0;	
};

Corner* Corner::vPrev(){
	return prev->e->other(prev);
};

CEdge * CEdge::insert(Corner* ic0, Corner* ic1){

	Corner* c0n_0 = ic0->next;
	Corner* c1n_0 = ic1->next;

	CFace* f0 = ic0->f;
	CFace* f1 = new CFace();

	f1->c0 = ic1;
	ic1->f = f1;
	ic0->v->mesh()->addFace(f1);

	Corner* c0n = new Corner(ic1->v, f0);
	Corner* c1n = new Corner(ic0->v, f1);

	ic0->setNext(c0n);
	ic1->setNext(c1n);
	c0n->setNext(c1n_0);
	c1n->setNext(c0n_0);

	//check edges
	if (ic0->e->c0 == ic0)
		ic0->e->set(c1n);
	else
		ic0->e->set(c1n,1);

	if (ic1->e->c0 == ic1)
		ic1->e->set(c0n);
	else
		ic1->e->set(c0n,1);


	for(Corner* c = ic1->next; c!=ic1; c = c->next)
		c->f = f1;

	CEdge* e = CEdge::create( (CMesh*) ic0->v->mesh() );
	e->set(ic0);
	e->set(ic1, 1);
	ic0->f->c0 = ic0;

	return e;
}

CEdge * CEdge::create(CMesh* m){                                                                                              
	CEdge* e = new CEdge();
	m->add(e);
	return e;
};

CMesh::CMesh(Mesh*m){

	list<Face*> faces = m->faces();
	_verts = m->verts();
	for(list<Vertex*>::iterator it = _verts.begin(); it != _verts.end(); it++)
		(*it)->setMesh(this);

	int vs = m->size();
	int es = vs*vs;
	CEdge** ematrix = new CEdge*[es];
	for(int i=0; i<es; i++)
		ematrix[i] = 0;

	for(list<Face*>::const_iterator it = faces.cbegin(); it != faces.cend(); it++){

		Face* f = *it;
		CFace* cf = new CFace();
		_faces.push_back((Face*)cf);
		Corner* c0 = 0;

		for(int i = 0; i < f->size; i++ ){

			Corner* c = new Corner(f->v(i), cf);
			int euid = f->edgeUid(i);
			if ((unsigned int)ematrix[euid]){
				ematrix[euid]->set(c,1);
			}else{
				CEdge* e = CEdge::create(this);
				e->set(c);
				ematrix[euid] = e;
			}

			if (c0)
				c0->setNext(c);
			else
				cf->c0 = c;
			c0 = c;
		}

		c0->setNext(cf->c0);
		cf->update();
	}

	delete ematrix;

	//border face
	/*CFace* bf = new CFace();
	for(list<CEdge*>::const_iterator it = _edges.cbegin(); it != _edges.cend(); it++){
		CEdge * e = *it;
		if (e->c1)
			continue;
		e->c1 = new Corner(e->c0->next->v, bf);
	}*/
}

bool CEdge::pick(const Vec3& p, double rad){
	Vec3 p0 = c0->v->P();
	Vec3 p1 = c0->next->v->P();
	Vec3 p0p1 = (p1 - p0);
	double p0p1_len = p0p1.norm();
	p0p1 = p0p1 / p0p1_len;
	double pp_len = (p-p0)*p0p1;
	if (pp_len>0 && pp_len < p0p1_len){
		double d = (p - (p0 + p0p1*pp_len) ).norm();
		if (d < rad)
			return true;	
	}
	return false;
}

CEdge* CMesh::pickEdge(const Vec3& p){
	CEdge* e = 0;
	for(list<CEdge*>::iterator it = _edges.begin(); it != _edges.end(); it++)
		if ( (*it)->pick(p, 0.015) )
			return *it;
	return e;
}

void CMesh::draw(int m){
	//Mesh::draw(m);

	if (!_faces.size()){ // this should be just a graph 
		drawLinks();
		return;
	}

	for(list<Face*>::const_iterator it = _faces.cbegin(); it != _faces.cend(); it++){
		CFace* f = (CFace*)(*it);
		glBegin(GL_LINE_LOOP);
		int i = 0;
		for(Corner* c = f->c0; c!=f->c0->prev; c = c->next){
			Vec3 p = c->v->P();
			glVertex3f(p.x, p.y, p.z);
			i++;
		}
		//glColor3f(0, 0.25*i ,0);
		Vec3 p = f->c0->prev->v->P();
		Corner* c = f->c0->prev;
		//p = p + (c->next->v->P() - p).normalize()*0.01 + (c->prev->v->P() - p).normalize()*0.01;
		glVertex3f(p.x, p.y, p.z);
		glEnd();
	}

	/*glPointSize(8.0);
	glBegin(GL_POINTS);
	for(list<CEdge*>::const_iterator it = _edges.cbegin(); it != _edges.cend(); it++){
		glColor3f(0, 0 , 1);
		Corner* c = (*it)->c0;
		Vec3 p = c->v->P();
		p = p + (c->next->v->P() - p).normalize()*0.05 + (c->prev->v->P() - p).normalize()*0.02;
		glVertex3f(p.x, p.y, p.z);

		glColor3f(0, 0 , 0.35);
		c = (*it)->c1;
		if (c){
			p = c->v->P();
			p = p + (c->next->v->P() - p).normalize()*0.05 + (c->prev->v->P() - p).normalize()*0.02;
			glVertex3f(p.x, p.y, p.z);
		}
	}
		glEnd();*/
}

/*void CMesh::insertIsopam(CEdge* e, double t){
	Corner* c = e->split(t);
}*/