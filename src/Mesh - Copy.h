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
	void blendVertexColors();

	VertexPtr addVertex(const Vec3& p);
	VertexPtr addVertex(VertexPtr v);
	VertexPtr addVertex(Vec3* v);

	EdgePtr addEdge(VertexPtr v0, VertexPtr v1);
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
};

class Vertex{

	Vec4 _col;
	Vertex(){};

private:

	map<Edge*, Edge*> _rot;
	map<Edge*, Edge*> _rrot;

public:

	Flag flag;
	Vec3 * p0;
	list<FacePtr> faces;
	list<EdgePtr> edges;

	~Vertex(){delete p0;};

	void setP(const Vec3& p){p0 = new Vec3(p);};
	void setP(Vec3 * p){p0 = p;};
	const Vec3& getP(){return *p0;};

	void draw();

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

	bool insert(Edge* e0, Edge* e);
	void remove(Edge* e);
	Edge* next(Edge* e_u);
	Edge* Vertex::prev(Edge* e_u);
	void Vertex::clearRot(){
	  _rot.clear(); _rrot.clear();
	};
};

class Edge{
		
	Edge** _neighbors;
	Face** _f;
public:

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
	
	EdgePtr split(VertexPtr v, double t = 0){

		if (t > 0)
			v->setP(v0->getP()*t + v1->getP()*(1-t));

		v1->edges.remove(this);
		EdgePtr e = new Edge(v, v1);
		e->_f[0] = _f[0];
		e->_f[1] = _f[1];
		v1 = v;
		v->edges.push_back(this);
		//v->insert(this, e);
		if (_neighbors){
			setNext(e);
			e->setNext(this, 1);
		}
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

	void setNext(Edge* e, int dir = 0){
		_neighbors[dir*2+1] = e;
		e->_neighbors[dir*2] = this;
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

	Edge* next(FacePtr f){
		if (_f[0] == f)
			return _neighbors[1];
		else if (_f[1] == f)
			return _neighbors[3];
		return 0;
	};
};

class Face{
	
	int _size;
	Vec3 _mid;

public:

	list<VertexPtr> verts;
	list<EdgePtr> edges;

	Face(int s){_size = s;};
	int size(){return _size;};

	void addNextVertex(VertexPtr v){verts.push_back(v); v->faces.push_back(this);};
	void addEdge(EdgePtr e){
		edges.push_back(e);
		_mid = ( _mid *(edges.size()-1) + e->v0->getP() ) / edges.size(); 
	};

	void setEdges(EdgePtr e0, EdgePtr e1, EdgePtr e2, EdgePtr e3){

		edges.clear();
		edges.push_back(e0);
		edges.push_back(e1);
		edges.push_back(e2);
		edges.push_back(e3);

		//update rotation
		/*e0->corner(e1)->insert(e0, e1);
		e1->corner(e2)->insert(e1, e2);
		e2->corner(e3)->insert(e2, e3);
		e3->corner(e0)->insert(e3, e0);*/

		updateVertsByEdges();
	};

	void setEdgeDir(int d0, int d1, int d2, int d3){
		if (edges.size()!=4)
			return;
		int d[4] = {d0, d1, d2, d3};
		setEdgeDir(d);
	};

	void setEdgeDir(int * d){
		Edge* e0 = 0;
		int i = 0;
		for(list<Edge*>::iterator it = edges.begin(); it != edges.end(); it++){
			Edge* e = (*it);
			e->setF(this, d[i]);
			if (e0)
				e0->setNext(e, d[i-1]);
			e0 = e;
			i++;
		}
		e0->setNext(edges.front(), d[0]);
	}

	void updateVertsByEdges(){

		verts.clear();
		EdgePtr e0 = edges.back();
		_mid.set(0,0,0);
		int dir  = 0;
		int dir0 = 0;
		int dir00 = 0;
		for(list<EdgePtr>::iterator it = edges.begin(); it!=edges.end(); it++ ){
			EdgePtr e = (*it);
			if ( (e->v0 != e0->v0) && (e->v0 != e0->v1) ){
				verts.push_back(e->v0);
				_mid = _mid + e->v0->getP();
				dir = 1;
			}else{
				verts.push_back(e->v1);
				_mid = _mid + e->v1->getP();
				dir = 0;
			}
			e->setF(this, dir);
			if (e0)
				e0->setNext(e, dir0);
			else
				dir00 = dir;
			e0  = e;
			dir0 = dir;
		}
		e0->setNext(edges.front(), dir00);
		_mid = _mid / verts.size();

	};
	
	Vec3 mid(){return _mid;};
};

#endif