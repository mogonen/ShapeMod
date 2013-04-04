#ifndef __MESH_H__	
#define __MESH_H__

#include <list>
#include <vector>
#include <set>
#include <GL/glut.h>
#include <string>
#include "Vector.h"
#include "Shape.h"
#include "ControlPoint.h"
#include "Auxiliary.h"

using namespace std;

#define FacePtr Face*

class Mesh;
class Vertex;
class Face;
struct Neighbor;

class Mesh:public Shape{

protected:
	list<Vertex*>	_verts;
	list<FacePtr>	_faces;

	int _voff;

public:


	Mesh(){border_v0 = 0; _voff =0;};
	~Mesh();

	int addVertex(Vertex* v);
	Vertex* addVertex(const Vec3& p);
	Vertex* addVertex(Vec3* v);

	void removeVertex(Vertex*); // ????

	void addFace(FacePtr);
	Face* splitFace(FacePtr, int, int);

	FacePtr addQuad(Vertex*, Vertex*, Vertex*, Vertex*);
	FacePtr addTriangle(Vertex*, Vertex*, Vertex*);

	int exportOBJ(char * fname);
	int exportOBJ(std::ofstream&);
	int loadOBJ(std::ifstream&);
	int size(){return _verts.size();};
	int sizeF(){return _faces.size();};

	void drawLinks();
	void drawBorder();

	virtual void draw(int i=0);
	bool isOn(const Vec3& p){return false;};
	int type(){return MESH;};
	void blendVertexColors(){};

	Mesh * refCopy(){
		Mesh * m = new Mesh();
		m->_verts = _verts;
		m->_faces = _faces;
		return m;
	};

	Mesh * deepCopy();

	Vertex* firstV(){return _verts.front();};
	int vOff(){return _voff;};

	list<Vertex*> verts(){return _verts;};
	list<Face*> faces(){return _faces;};
	void resetFaces(){_faces.clear();};
	void updateFaces();

	Vertex* border_v0;
	Vertex* extractBorder();

	int save(std::ofstream&);
	int load(std::ifstream&);

	void initNeighbors();

	//fix it!
	enum{NORMALMAP, ALPHAMAP, SOLID, WIREFRAME, SOLID_WIREFRAME, SHADED};
	static const unsigned char WIRE_OVERRIDE = 8;
	static const unsigned char SOLID_OVERRIDE = 16;
	static const unsigned char FACECOLOR_OVERRIDE = 32;
	static const unsigned char FLATSHADE_OVERRIDE = 64;
	static int  DRAWING_MODE;
	static bool DRAW_WIREFRAME, DRAW_MESH;
};

class Vertex{

	Vertex(){
		p0 = 0;
		next = 0;
		prev = 0;
	};

	unsigned int _id;
	Mesh * _mesh;

public:

	Flag flag;
	Vec3 * p0;
	RGBA c0;

	list<Vertex*> to;

	~Vertex(){delete p0;};

	void setP(const Vec3& p){ 
		if (p0) 
			p0->set(p);
		else
			p0 = new Vec3(p);
	};

	void setP(Vec3 * p){ p0 = p; };
	const Vec3& P(){ return *p0; };
	const RGBA& C(){ return c0; };
	void setC(const Vec4& c){c0= c;};
	void setC(const Vec3& c){setC(c, 1.0);};
	void setC(const Vec3& c, double w){
		c0.x = c.x;
		c0.y = c.y;
		c0.z = c.z;
		c0.w = w;
	};

	int id(){return _id;};
	Mesh* mesh(){return _mesh;};
	void setMesh(Mesh* m){_mesh = m;};
	int edgeId(Vertex * v, int sz = 0){
		sz = (sz)?sz:_mesh->size();
		return _id + v->_id*sz;
	};

	int edgeUid(Vertex * v, int sz = 0){
		sz = (sz)?sz:_mesh->size();
		return ( _id < v->_id )? ( _id + v->_id*sz ) : ( _id*sz + v->_id);
	};

	float * getCoords(){return (float*)p0;};

	void draw(){};

	int val(){return to.size();};

	Vertex* * sortTo(Vertex*);
	static Vertex* create(Mesh* m){
		Vertex* v = new Vertex();
		v->_id = m->addVertex(v);
		v->_mesh = m;
		return v;
	};

	bool hasLink(Vertex* from){
		if (!to.size())
			return false;
		for(list<Vertex*>::iterator it = to.begin(); it != to.end(); it++)
			if (*it == from)
				return true;
		return false;
	};

	Vertex* otherLink(Vertex* v){
		if (to.front() == v)
			return to.back();
		else
			return to.front();
	}

	//only for border verts
	Vertex* prev;
	Vertex* next;
	void setNext(Vertex* n){
		next = n;
		n->prev = this;
	}

	bool isBorder(){return prev && next;}

	//some storage
	Vec3 v3_0, v3_1, v3_2;
	Vec4 v4_0, v4_1, v4_2;
	void resetStorage(){
		v3_0.set(); v3_1.set(); v3_2.set();
		v4_0.set(); v4_1.set(); v4_1.set();
	}

};


class Face{
	
protected:
	Vec3 _p0;
	Vec3 _n0;
	Vec4 _c0;
public:

	int size;
	Vertex** verts; //strickly cockwise
	Neighbor** neighbors; 

	Face(int s=0){size = s; verts = new Vertex*[s]; data = 0; neighbors = 0;};

	Vec3 P(){return _p0;};
	Vec3 N(){return _n0;};
	RGBA C(){return _c0;};
	void setC(const Vec4& c){_c0.set(c);};
	void setN(const Vec3& n){_n0.set(n);};

	virtual void update(){
		_p0.set();
		_c0.set();
		for(int i=0; i<size; i++){
			_p0 = _p0 + verts[i]->P();
			_c0 = _c0 + verts[i]->C();
		}
		_p0 = _p0 / size;
		_c0 = _c0 / size;
	};

	Vertex* v(int i){return verts[i%size];};
	int edgeId(int i){return verts[i%size]->id() + verts[(i+1)%size]->id()*verts[0]->mesh()->size();}
	int edgeUid(int i){
		int i0 = verts[i%size]->id();
		int i1 = verts[(i+1)%size]->id();
		int ms = verts[0]->mesh()->size();
		return (i0<i1)?(i0+i1*ms):(i1+i0*ms);
	}

	void * data;
	int pick(const Vec3& p, double rad);
	bool isIn(const Vec3& p);

	void initNeighbors(){
		neighbors = new Neighbor*[size];
		for(int i=0; i<size; i++)
			neighbors[i] = 0;
	}
};

struct Neighbor{
	Face* f;
	int vi;

	Neighbor(Face* lf, int li){
		f = lf;
		vi = li;
	}
};

//incase an Edge is needed
struct Edge{

	Vertex* v0;
	Vertex* v1;

	Face*	f0;
	Face*   f1;

	Edge(Vertex* lv0, Vertex* lv1, Face* f){
		f0 = f; f1 = 0;
		v0 = lv0; 
		v1 = lv1;
	};

	bool pick(const Vec3& p, double rad);

	static list<Edge*> createEdges(Mesh*);
	static Edge** createEdgeMatrix(Mesh*, bool uns = true);
};

#endif