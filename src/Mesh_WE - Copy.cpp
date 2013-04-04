#include <iostream> 
#include <fstream>
#include <map>
#include "Mesh.h"
#include "Eye.h"

bool Mesh::DRAW_MESH = true;
bool Mesh::DRAW_WIREFRAME = false;
int Mesh::DRAWING_MODE = Mesh::NORMALMAP;

void Mesh::addFace(FacePtr f){
	_faces.push_back(f);
}

FacePtr Mesh::addQuad(VertexPtr v0, VertexPtr v1, VertexPtr v2, VertexPtr v3){
	FacePtr f = new Face(4);
	_faces.push_back(f);
	f->addNextVertex(v0);
	f->addNextVertex(v1);
	f->addNextVertex(v2);
	f->addNextVertex(v3);
	return f;
}

FacePtr Mesh::addQuad(EdgePtr e0, EdgePtr e1, EdgePtr e2, EdgePtr e3){
	FacePtr f = new Face(4);
	f->edge0 = e0;
	e0->setNext(e1, f);
	e1->setNext(e2, f);
	e2->setNext(e3, f);
	e3->setNext(e0, f);
	_faces.push_back(f);
	return f;
}

FacePtr Mesh::addTriangle(VertexPtr v0, VertexPtr v1, VertexPtr v2){
	FacePtr f = new Face(3);
	_faces.push_back(f);
	f->addNextVertex(v0);
	f->addNextVertex(v1);
	f->addNextVertex(v2);
	return f;
}

VertexPtr Mesh::addVertex(Vec3* p){
	VertexPtr v = ::Vertex::create(this);
	v->setP(p);
	_verts.push_back(v);
	return v;
}

VertexPtr Mesh::addVertex(const Vec3& p){
	VertexPtr v = ::Vertex::create(this);
	v->setP(p);
	_verts.push_back(v);
	return v;
}

VertexPtr Mesh::addVertex(VertexPtr v){
	_verts.push_back(v);
	return v;
}
/*
	bool Vertex::insert(Edge* e0, Edge* e) {

	  if (e->v0 != this && e->v1 != this) 
		return false;

	  if (_rot.size() <= 0) {
		_rot[e] = e;
		_rrot[e] = e;
		return true;
	  };

	  if (_rot.find(e0) == _rot.end() || _rot.find(e) != _rot.end()) 
		return false;
	  
	  Edge* e1 = _rot[e0];
	  _rot[e] = e1;
	  _rot[e0] = e;
	  _rrot[e1] = e;
	  _rrot[e] = e0;
	  return true;
	};

	void Vertex::remove(Edge* e) {
	  Edge* e0 = _rrot[e];
	  Edge* e1 = _rot[e];
	  if (e0 != e && e1 != e) {
		_rot[e0] = e1;
		_rrot[e1] = e0;
	  }
	  _rot.erase(e); _rrot.erase(e);
	};

	Edge* Vertex::next(Edge* e_u) {
	  if (_rot.find(e_u) == _rot.end()) return NULL;
	  return _rot[e_u];
	};

	Edge* Vertex::prev(Edge* e_u) {
	  if (_rrot.find(e_u) == _rrot.end()) return NULL;
	  return _rrot[e_u];
	};

*/

EdgePtr Mesh::addEdge(VertexPtr v0, VertexPtr v1){
	EdgePtr e = new Edge(v0, v1);
	_verts.push_back(v0);
	_verts.push_back(v1);
	_edges.push_back(e);
	return e;
}

void Mesh::addEdge(EdgePtr e){
	_verts.push_back(e->v0);
	_verts.push_back(e->v1);
	_edges.push_back(e);
}

/*
EdgePtr Mesh::insertEdge(VertexPtr v0, VertexPtr v1, EdgePtr){
	EdgePtr e = new Edge(v0, v1);
	_verts.push_back(v0);
	_verts.push_back(v1);
	_edges.push_back(e);
	return e;
}
*/



void Mesh::drawEdges(){
	for(list<EdgePtr>::iterator it = _edges.begin(); it!=_edges.end(); it++){
		Vec3 p0 = (*it)->v0->getP();
		Vec3 p1 = (*it)->v1->getP();
		glBegin(GL_LINES);
			glVertex3f(p0.x, p0.y, p0.z);	
			glVertex3f(p1.x, p1.y, p1.z);
		glEnd();
	}
}

void Mesh::draw(int i){
	if (!DRAW_MESH)
		return;

	if (_edges.size()&& !DRAW_WIREFRAME)
		drawEdges();

	/*for(list<VertexPtr>::iterator itv = _verts.begin(); itv!=_verts.end(); itv++)
		(*itv)->draw();
	*/
	
	for(list<FacePtr>::iterator itf = _faces.begin(); itf!=_faces.end(); itf++){

		FacePtr f = (*itf);

		glColor3f(0, 0, 0);

		if (DRAW_WIREFRAME){
			glLineWidth(1.0);
			glColor3f(0.0f,0.0f,0.0f);
			glBegin(GL_LINE_LOOP);
			for(list<VertexPtr>::iterator itv = f->verts.begin(); itv!=f->verts.end(); itv++){
				Vec3 p = (*itv)->getP();
				glVertex3f(p.x, p.y, p.z);
			}
			glEnd();
			/*glPointSize(.0);
			glBegin(GL_POINTS);
			for(list<VertexPtr>::iterator itv = f->verts.begin(); itv!=f->verts.end(); itv++){
				Vec3 p = (*itv)->getP();
				glVertex3f(p.x, p.y, p.z);
			}
			glEnd();*/
		}

		//glEnable(GL_LIGHTING);
		glColor3f(0.5, 0.2, 0.2);
		glBegin(GL_POLYGON);
			for(list<VertexPtr>::iterator itv = f->verts.begin(); itv!=f->verts.end(); itv++){
				if (!(*itv))
					continue;
				Vec3 p = (*itv)->getP();
				Vec4 n = (*itv)->getCol();
				//glNormal3f(n.x, n.y, n.z);
				if (DRAWING_MODE == NORMALMAP)
					//glColor4f( n.x , n.y  , n.z, n.w );
					glColor4f((n.x+1)/2.0 , (n.y+1)/2.0  , n.z, n.w );
				if  (DRAWING_MODE == ALPHAMAP)
					glColor4f(0, 0, 1 , n.z*n.w );
				else if (DRAWING_MODE == SOLID)
					glColor3f(1.0, 1.0, 1.0);
				   //glColor3f(1, 0, 0  );
				glVertex3f(p.x, p.y, p.z);
			}
		glEnd();
		//glDisable(GL_LIGHTING);
	}
}

EdgePtr * Vertex::sortEdges(EdgePtr e0){

	EdgePtr * es = new EdgePtr[edges.size()-1];
	Vec3 x = (e0->other(this)->getP() - getP()).normalize();
	Vec3 y = (x%Vec3(0,0,1)).normalize();
	int sz = 0; 
	for(list<EdgePtr>::iterator it = edges.begin(); it != edges.end(); it++){
		if (*it != e0)
			es[sz++] = (*it);
	}
	
	double * deg = new double[sz];
	for(int i = 0; i<sz; i++ ){
		Vec3 n((es[i]->other(this)->getP() - getP()).normalize());
		deg[i] = (n * x);
		if (n*y<0)
			deg[i] = -2 - deg[i];
	}

	//now sort:
	for(int i = 0; i<sz; i++)
		for(int j=i+1; j<sz; j++)
			if (deg[i] < deg[j]){
				//swap
				double d = deg[j];
				deg[j] = deg[i];
				deg[i] = d;
				EdgePtr e = es[j];
				es[j] = es[i];
				es[i] = e;
			}
			
	return es;
}


int Mesh::exportOBJ(char * fname){

  ofstream out(fname); 
  if(!out) { 
    cout << "Cannot open file.\n"; 
    return -1; 
  } 

  map<VertexPtr, int> vtable;

  out<<"#"<<fname<<endl;
  out<<"#vertices"<<endl;
  int ind = 1;
  for(list<VertexPtr>::iterator itv = _verts.begin(); itv != _verts.end(); itv++){
	  VertexPtr v = (*itv);
	  out<<"v "<< v->getP().x <<" "<< v->getP().y <<" "<< v->getP().z << endl;
	  vtable[v]=ind++;
  }

  out<<"#faces"<<endl;
  for(list<FacePtr>::iterator itf = _faces.begin(); itf!=_faces.end(); itf++){
	  out<<"f ";
	  for(list<VertexPtr>::iterator itv = (*itf)->verts.begin(); itv != (*itf)->verts.end(); itv++)
		  out<<vtable[(*itv)]<<" ";
	  out<<endl;
  }

  out.close();
  cout <<" mesh saved to "<<fname; 
  return 0;
}



void Mesh::onClick(const Vec3& p){
	 ControlPoint::select(this, p);
};

void Mesh::exec(int cmd, void* param){

	if (cmd == 2){
		makeOutline();
		return;
	}

	if (cmd == 3){
		//subdivCC();
		CatmullClark();
		return;
	}

	Vec3 p = *((Vec3*)param);
	ControlPoint* cp = ControlPoint::select(this, p);

	if (cp && cmd == 0)
		last_v = 0;

	VertexPtr v = 0;
	if (cp)
		v = (VertexPtr)cp->retrieve();
	else{
		v = Vertex::create(this);
		Vec3 * p0 = new Vec3(p);
		v->setP(p0);

		cp = ControlPoint::create(this, p0);
		cp->store(v);
	}
	if (last_v){
		addEdge(last_v, v);
	}
	last_v = v;
}


void Mesh::makeOutline(){

	makeOutline(0, 0, _edges.front());
	cout<<"faces:"<<_faces.size()<<endl;

	for(list<FacePtr>::iterator itf = _faces.begin(); itf!=_faces.end(); itf++)
		(*itf)->updateVertsByEdges();

	cout<<"done";
};


void Mesh::makeOutline(EdgePtr pe0, EdgePtr pe1, EdgePtr ee){
	double rad = 0.15;

	bool end = false;
	VertexPtr v  = ee->v1;

	if (!pe0){
		Vec3 tan = ( ee->v1->getP() - ee->v0->getP()).normalize();
		Vec3 n = (Vec3(0,0,1) % tan).normalize();

		Vec3 *p0 = new Vec3( ee->v0->getP() + n*rad );
		Vec3 *p1 = new Vec3( ee->v0->getP() - n*rad );

		VertexPtr new_v0 = this->addVertex(p0);
		VertexPtr new_v1 = this->addVertex(p1);
		ControlPoint::create(this, p0);
		ControlPoint::create(this, p1);
		
		pe0 = this->addEdge(ee->v0, new_v0);
		pe1 = this->addEdge(ee->v0, new_v1);
	}

	while(v->edges.size() <= 2 && !end){

		EdgePtr ee1 = 0;
		for(list<EdgePtr>::iterator it = v->edges.begin(); it!=v->edges.end();it++)
			if ( (*it)!= ee )
				ee1 = (*it);
	
		VertexPtr v1 =0;
		if (!ee1){
			end = true;
			v1 = v;
		}else
			v1 = ee1->other(v);

		Vec3 tan = (v1->getP() - ee->other(v)->getP()).normalize();
		Vec3 n = (Vec3(0,0,1) % tan).normalize();

		Vec3 *p0 = new Vec3( v->getP() + n*rad );
		Vec3 *p1 = new Vec3( v->getP() - n*rad );

		VertexPtr new_v0 = this->addVertex(p0);
		VertexPtr new_v1 = this->addVertex(p1);
		ControlPoint::create(this, p0);
		ControlPoint::create(this, p1);
		EdgePtr e0 = this->addEdge(v, new_v0);
		EdgePtr e1 = this->addEdge(v, new_v1);

		new_v0->setCol(Vec3(0.5,0.15, 0.8));
		new_v1->setCol(Vec3(0.5,0.15, 0.8));

		//introduce a quad face!
		if (pe0){
			EdgePtr side0 = this->addEdge(pe0->v1, new_v0);
			EdgePtr side1 = this->addEdge(new_v1, pe1->v1);

			FacePtr f0 = this->addQuad(ee, pe0, side0, e0);
			this->addQuad(ee, e1, side1, pe1);
		}

		pe0 = e0;
		pe1 = e1;

		v = v1;
		ee = ee1;		
	}
	
	if (!end && v->edges.size()>2){
		// this is a joint!
		EdgePtr * branches = v->sortEdges(ee);
		int sz = v->edges.size() - 1;
		Vec3 p0 = ee->other(v)->getP();
		Vec3 p00 = p0;

		EdgePtr e0 = 0;
		EdgePtr e00 = 0;
		for(int i=0; i<sz+1; i++){
			Vec3 tan = (i<sz)? (branches[i]->other(v)->getP() - p0).normalize() : (p00-p0).normalize();
			Vec3 n = (Vec3(0,0,1) % tan).normalize();
			Vec3 * p = new Vec3(v->getP() + n*rad);
			VertexPtr o = this->addVertex(p);
			EdgePtr e = this->addEdge(v, o);
			o->setCol(Vec3(0.5,0.15, 0.8));
			ControlPoint::create(this, p);
			if (e0)
				makeOutline(e0, e, branches[i-1]);
			 else
				e00 = e;

			e0 = e;
			p0 = branches[i%sz]->other(v)->getP();
		}

		EdgePtr side0 = this->addEdge(pe0->v1, e00->v1);
		EdgePtr side1 = this->addEdge(e0->v1, pe1->v1);

		this->addQuad(ee, pe0, side0, e00);
		this->addQuad(ee, e0, side1, pe1);
	}
}

void Mesh::subdivCC(){

}

void Mesh::CatmullClark(){
	list<EdgePtr> oldedges = _edges;
	for(list<EdgePtr>::iterator it = oldedges.begin(); it!=oldedges.end();it++){
		VertexPtr v = Vertex::create(this);
		EdgePtr e = (*it)->split(v, 0.5, this);
		e->flag.store(v);
	}

	list<FacePtr> newfaces;
	for(list<FacePtr>::iterator it = _faces.begin(); it!=_faces.end(); it++){
		VertexPtr vmid = Vertex::create(this);
		vmid->setP((*it)->mid());
		EdgePtr efirst = (*it)->edge0;
		EdgePtr e = efirst->next(*it);
		while(e && e!=efirst){
			Vertex* v = e->v0; //(Vertex*)e->flag.retrieve();
			if (v)
				//this->addEdge(vmid, v);
			e = e->next(*it);
		}
	}
	//_faces.swap(newfaces);
}