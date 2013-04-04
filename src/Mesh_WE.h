#ifndef _WEMESH_H__	
#define __WEMESH_H__
#include "Mesh.h"

using namespace std;

#define WEdgePtr WEdge*

namespace WingedEdge{
class WEdge;
class WEMesh;


class WEdge{
		
	WEdge** _neighbors;
	Face** _f;
public:
	enum {LEFT, RIGHT, NODIR};

	Flag flag;

	Vertex* v0; 
	Vertex* v1;

	WEdge(Vertex* pv0, Vertex* pv1){
		v0 = pv0;
		v1 = pv1;
		_f = new Face*[2];
		_f[0] = _f[1] = 0;
		_neighbors = new WEdge*[4];
	};

	Vertex* other(Vertex* v){return (v==v0)?v1:v0;};
	FacePtr   other(FacePtr f){return (f==_f[0])?_f[1]:_f[0];};
	Vertex* other(WEdgePtr e){
		if (v0 == e->v0 || v0 == e->v1)
			return v1;
		else 
			return v0;
	};

	Vertex* corner(WEdgePtr e){
		if (v0 == e->v0 || v0 == e->v1)
			return v0;
		else if (v1 == e->v0 || v1 == e->v1)
			return v1;
		else 
			return 0;
	};

	FacePtr same(WEdge* e){
		if (_f[0] == e->_f[0] || _f[0] == e->_f[1])
			return _f[0];
		else if (_f[1] == e->_f[0] || _f[1] == e->_f[1])
			return _f[1];
		return 0;
	}

	int dir(WEdgePtr e){
		if (v0 == e->v0 || v0 == e->v1)
			return LEFT;
		else if (v1 == e->v0 || v1 == e->v1)
			return RIGHT;
		else 
			return NODIR;
	};
	
	WEdgePtr split(Vertex* v, double t, WEMesh* m=0);

	void setNeighbors(WEdge* e0, WEdge *e1, int dir=0){
		if (e0){ //prev
			_neighbors[dir*2] = e0;
			e0->_neighbors[dir*2+1] = this;
		}
		if (e1){
			_neighbors[dir*2+1] = e1;
			e1->_neighbors[dir*2] = this;
		}
	};

	void setF(Face* f, int dir = 0){
		_f[dir] = f;
	};

	void setNext(WEdge* e, int dir){
		_neighbors[dir*2+1] = e;
		e->_neighbors[dir*2] = this;
	};

	int setNext(WEdge* e, Face* f=0){
		int dir = NODIR;
		if (v0 == e->v0 || v0 == e->v1)
			dir = LEFT;
		if (v1 == e->v0 || v1 == e->v1)
			dir = RIGHT;
		_neighbors[dir*2+1]  = e;
		if (f)
			_f[dir] = f;
		e->setPrev(this, f);
		return dir;
	};

	int setPrev(WEdge* e, Face* f=0){
		int dir = NODIR;
		if (v0 == e->v0 || v0 == e->v1)
			dir = RIGHT;
		if (v1 == e->v0 || v1 == e->v1)
			dir = LEFT;
		_neighbors[dir*2]  = e;
		if (f)
			_f[dir] = f;
		return dir;
	};

	WEdge* next(int dir=0){
		return _neighbors[dir*2+1];
	};

	WEdge* prev(int dir=0){
		return _neighbors[dir*2];
	};

	Face* f(int dir=0){return _f[dir];};

	Vertex* v(int i, int dir = 0){
		if (dir)
			return (i)?v0:v1;
		return (i)?v1:v0;
	};

	Vertex* v(int i, Face* f){
		if (_f[0] == f)
			return (i)?v0:v1;
		else if (_f[1] == f)
			return (i)?v1:v0;
		return 0;
	};

	WEdge* next(Face* f){
		if (_f[0] == f)
			return _neighbors[1];
		else if (_f[1] == f)
			return _neighbors[3];
		return 0;
	};

	WEdge* prev(Face* f){
		if (_f[0] == f)
			return _neighbors[0];
		else if (_f[1] == f)
			return _neighbors[2];
		return 0;
	};
};

class WEMesh:public Mesh{

	list<WEdgePtr>	_WEdges;

public:

	WEdgePtr addWEdge(Vertex* v0, Vertex* v1);
	void addWEdge(WEdge* e);
	void drawWEdges();
	FacePtr WEMesh::addQuad(WEdgePtr e0, WEdgePtr e1, WEdgePtr e2, WEdgePtr e3);

	void updateVertsByWEdges(Face* f){
			delete f->verts;
			f->verts = new Vertex*[f->size];
			WEdgePtr e = 0;//WEdge0;
			WEdgePtr e0 = e->prev(f);
			int i=0;
			while(e && e0){
				if ((e->v0 != e0->v0) && (e->v0 != e0->v1) ){
					f->verts[i] = e->v0;
				}else{
					f->verts[i] = (e->v1);
				}
				e0  = e;
				e = e->next(f);
				/*if (e == WEdge0)
					break;*/
				i++;
			}
		};

};

};

#endif