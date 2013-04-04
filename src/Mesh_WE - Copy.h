#ifndef _MESH_H__	
#define __MESH_H__

#include <list>
#include <vector>
#include <set>
#include <GL/glut.h>
#include "Vector.h"
#include "Shape.h"
#include "ControlPoint.h"

using namespace std;

#define EdgePtr Edge*
#define VertexPtr Vertex*
#define MeshPtr Mesh*
#define FacePtr Face*

class Mesh;
class Edge;
class Vertex;
class Face;

class Mesh:public Shape{

	list<VertexPtr>	_verts;
	list<FacePtr>	_faces;
	list<EdgePtr>	_edges;

	VertexPtr last_v;

public:

	static bool DRAW_WIREFRAME;
	static int  DRAWING_MODE;
	static bool DRAW_MESH;
	enum{NORMALMAP, ALPHAMAP, SOLID, WIREFRAME, SOLID_WIREFRAME};


	VertexPtr addVertex(const Vec3& p);
	VertexPtr addVertex(VertexPtr v);
	VertexPtr addVertex(Vec3* v);

	EdgePtr addEdge(VertexPtr v0, VertexPtr v1);
	void addEdge(Edge* e);
	void addFace(FacePtr);

	FacePtr addQuad(VertexPtr, VertexPtr, VertexPtr, VertexPtr);
	FacePtr addTriangle(VertexPtr, VertexPtr, VertexPtr);

	FacePtr addQuad(EdgePtr, EdgePtr, EdgePtr, EdgePtr);
	FacePtr addTriangle(EdgePtr, EdgePtr, EdgePtr);

	int exportOBJ(char * fname);
	int size(){return _verts.size();};

	void drawEdges();

	//shape methods
	void draw(int i=0);
	bool isOn(const Vec3& p){return false;};
	virtual int type(){return MESH;};
	void onClick(const Vec3& p);
	void onActivate(){last_v = 0;};

	void exec(int cmd = 0 , void* p = 0);

	void makeOutline();
	void makeOutline(EdgePtr, EdgePtr, EdgePtr);

	void subdivCC();
	void dooSabin();
	void CatmullClark();
		
	void blendVertexColors(){};
};

class Vertex{

	Vec4 _col;
	Vertex(){};
/*
private:

	map<Edge*, Edge*> _rot;
	map<Edge*, Edge*> _rrot;
*/
public:

	Flag flag;
	Vec3 * p0;
	list<FacePtr> faces;
	list<EdgePtr> edges;

	~Vertex(){delete p0;};

	void setP(const Vec3& p){p0 = new Vec3(p);};
	void setP(Vec3 * p){p0 = p;};
	const Vec3& getP(){return *p0;};

	void draw(){};

	void setCol(const Vec4& c){_col = c;};
	void setCol(const Vec3& c){setCol(c, 1.0);};
	void setCol(const Vec3& c, double w){
		_col.x = c.x;
		_col.y = c.y;
		_col.z = c.z;
		_col.w = w;
	};


	Vec4 getCol(){return _col;};
	float * getCoords(){return (float*)p0;};

	static VertexPtr create(MeshPtr m){
		VertexPtr v = new Vertex();
		return v;
	};

	EdgePtr * sortEdges(EdgePtr);

/*	bool insert(Edge* e0, Edge* e);
	void remove(Edge* e);
	Edge* next(Edge* e_u);
	Edge* Vertex::prev(Edge* e_u);
	void Vertex::clearRot(){
	  _rot.clear(); _rrot.clear();
	};*/
};

class Edge{
		
	Edge** _neighbors;
	Face** _f;
public:
	enum {LEFT, RIGHT, NODIR};

	Flag flag;

	VertexPtr v0; 
	VertexPtr v1;

	Edge(VertexPtr pv0, VertexPtr pv1){
		v0 = pv0;
		v1 = pv1;
		v0->edges.push_back(this);
		v1->edges.push_back(this);
		_f = new Face*[2];
		_f[0] = _f[1] = 0;
		_neighbors = new Edge*[4];
	};

	VertexPtr other(VertexPtr v){return (v==v0)?v1:v0;};
	FacePtr   other(FacePtr f){return (f==_f[0])?_f[1]:_f[0];};
	VertexPtr other(EdgePtr e){
		if (v0 == e->v0 || v0 == e->v1)
			return v1;
		else 
			return v0;
	};

	VertexPtr corner(EdgePtr e){
		if (v0 == e->v0 || v0 == e->v1)
			return v0;
		else if (v1 == e->v0 || v1 == e->v1)
			return v1;
		else 
			return 0;
	};

	FacePtr same(Edge* e){
		if (_f[0] == e->_f[0] || _f[0] == e->_f[1])
			return _f[0];
		else if (_f[1] == e->_f[0] || _f[1] == e->_f[1])
			return _f[1];
		return 0;
	}

	int dir(EdgePtr e){
		if (v0 == e->v0 || v0 == e->v1)
			return LEFT;
		else if (v1 == e->v0 || v1 == e->v1)
			return RIGHT;
		else 
			return NODIR;
	};
	
	EdgePtr split(VertexPtr v, double t = 0.5, Mesh* m=0){
		if (t > 0)
			v->setP(v0->getP()*t + v1->getP()*(1-t));

		//v1->edges.remove(this);
		EdgePtr e = new Edge(v, v1);
		v1 = v;
		e->_f[0] = _f[0];
		e->_f[1] = _f[1];

		//v->edges.push_back(this);
		//v->insert(this, e);
		if (_neighbors){
			setNext(e);
			e->setNext(this, 1);
		}
		if (m)
			m->addEdge(e);
		return e;
	};

	void setNeighbors(Edge* e0, Edge *e1, int dir=0){
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

	void setNext(Edge* e, int dir){
		_neighbors[dir*2+1] = e;
		e->_neighbors[dir*2] = this;
	};

	int setNext(Edge* e, Face* f=0){
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

	int setPrev(Edge* e, Face* f=0){
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

	Edge* next(int dir=0){
		return _neighbors[dir*2+1];
	};

	Edge* prev(int dir=0){
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

	Edge* next(Face* f){
		if (_f[0] == f)
			return _neighbors[1];
		else if (_f[1] == f)
			return _neighbors[3];
		return 0;
	};

	Edge* prev(Face* f){
		if (_f[0] == f)
			return _neighbors[0];
		else if (_f[1] == f)
			return _neighbors[2];
		return 0;
	};
};

class Face{
	
	int _size;
	Vec3 _mid;

public:

	list<VertexPtr> verts;
	EdgePtr edge0;

	Face(int s){_size = s;};
	int size(){return _size;};

	void addNextVertex(VertexPtr v){verts.push_back(v); v->faces.push_back(this);};

	void updateVertsByEdges(){
		verts.clear();
		_mid.set(0,0,0);
		EdgePtr e = edge0;
		EdgePtr e0 = edge0->prev(this);
		while(e && e0){
			if ((e->v0 != e0->v0) && (e->v0 != e0->v1) ){
				verts.push_back(e->v0);
				_mid = _mid + e->v0->getP();
			}else{
				verts.push_back(e->v1);
				_mid = _mid + e->v1->getP();
			}
			e0  = e;
			e = e->next(this);
			if (e == edge0)
				break;
		}
		_mid = _mid / verts.size();
	};
	
	Vec3 mid(){return _mid;};
};

#endif