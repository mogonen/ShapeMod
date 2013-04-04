#include <iostream> 
#include <fstream>
#include <map>
#include "Mesh_WE.h"

using namespace WingedEdge;

FacePtr WEMesh::addQuad(WEdgePtr e0, WEdgePtr e1, WEdgePtr e2, WEdgePtr e3){
	FacePtr f = new Face(4);
	f->data = e0;
	e0->setNext(e1, f);
	e1->setNext(e2, f);
	e2->setNext(e3, f);
	e3->setNext(e0, f);
	_faces.push_back(f);
	return f;
}

/*
	bool Vertex::insert(WEdge* e0, WEdge* e) {

	  if (e->v0 != this && e->v1 != this) 
		return false;

	  if (_rot.size() <= 0) {
		_rot[e] = e;
		_rrot[e] = e;
		return true;
	  };

	  if (_rot.find(e0) == _rot.end() || _rot.find(e) != _rot.end()) 
		return false;
	  
	  WEdge* e1 = _rot[e0];
	  _rot[e] = e1;
	  _rot[e0] = e;
	  _rrot[e1] = e;
	  _rrot[e] = e0;
	  return true;
	};

	void Vertex::remove(WEdge* e) {
	  WEdge* e0 = _rrot[e];
	  WEdge* e1 = _rot[e];
	  if (e0 != e && e1 != e) {
		_rot[e0] = e1;
		_rrot[e1] = e0;
	  }
	  _rot.erase(e); _rrot.erase(e);
	};

	WEdge* Vertex::next(WEdge* e_u) {
	  if (_rot.find(e_u) == _rot.end()) return NULL;
	  return _rot[e_u];
	};

	WEdge* Vertex::prev(WEdge* e_u) {
	  if (_rrot.find(e_u) == _rrot.end()) return NULL;
	  return _rrot[e_u];
	};

*/

WEdgePtr WEMesh::addWEdge(Vertex* v0, Vertex* v1){
	WEdgePtr e = new WEdge(v0, v1);
	_verts.push_back(v0);
	_verts.push_back(v1);
	_WEdges.push_back(e);
	return e;
}

void WEMesh::addWEdge(WEdgePtr e){
	_verts.push_back(e->v0);
	_verts.push_back(e->v1);
	_WEdges.push_back(e);
}

/*
WEdgePtr Mesh::insertWEdge(Vertex* v0, Vertex* v1, WEdgePtr){
	WEdgePtr e = new WEdge(v0, v1);
	_verts.push_back(v0);
	_verts.push_back(v1);
	_WEdges.push_back(e);
	return e;
}
*/

void WEMesh::drawWEdges(){
	for(list<WEdgePtr>::iterator it = _WEdges.begin(); it!=_WEdges.end(); it++){
		Vec3 p0 = (*it)->v0->P();
		Vec3 p1 = (*it)->v1->P();
		glBegin(GL_LINES);
			glVertex3f(p0.x, p0.y, p0.z);	
			glVertex3f(p1.x, p1.y, p1.z);
		glEnd();
	}
}


WEdgePtr WEdge::split(Vertex* v, double t = 0.5, WEMesh* m){
		if (t > 0)
			v->setP(v0->P()*t + v1->P()*(1-t));

		WEdgePtr e = new WEdge(v, v1);
		v1 = v;
		e->_f[0] = _f[0];
		e->_f[1] = _f[1];

		if (_neighbors){
			setNext(e);
			e->setNext(this, 1);
		}
		if (m)
			m->addWEdge(e);
		return e;
	};

/*
WEdgePtr * Vertex::sortWEdges(WEdgePtr e0){

	WEdgePtr * es = new WEdgePtr[WEdges.size()-1];
	Vec3 x = (e0->other(this)->P() - P()).normalize();
	Vec3 y = (x%Vec3(0,0,1)).normalize();
	int sz = 0; 
	for(list<WEdgePtr>::iterator it = WEdges.begin(); it != WEdges.end(); it++){
		if (*it != e0)
			es[sz++] = (*it);
	}
	
	double * deg = new double[sz];
	for(int i = 0; i<sz; i++ ){
		Vec3 n((es[i]->other(this)->P() - P()).normalize());
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
				WEdgePtr e = es[j];
				es[j] = es[i];
				es[i] = e;
			}
			
	return es;
}

*/