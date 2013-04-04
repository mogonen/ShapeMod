#ifndef _MESH_H__	
#define __MESH_H__

#include <list>
#include <vector>
#include <set>
#include <GL/glut.h>
#include "Vector.h"
#include "Shape.h"
#include "ControlPoint.h"
#include "Auxiliary.h"


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


	//aux methods
	void makeOutline();
	void makeOutline(EdgePtr, EdgePtr, EdgePtr);
	void subdivCC();
	void blendVertexColors(){};
};

class Vertex{

	Vec4 _col;
	Vertex(){};

public:

	Flag flag;
	Vec3 * p0;
	list<FacePtr> faces;
	list<EdgePtr> edges;

	~Vertex(){delete p0;};

	void setP(const Vec3& p){ p0 = new Vec3(p);};
	void setP(Vec3 * p){ p0 = p; };
	const Vec3& getP(){ return *p0; };
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

	void draw(){};

	EdgePtr * sortEdges(EdgePtr);

	static VertexPtr create(MeshPtr m){
		VertexPtr v = new Vertex();
		return v;
	};
};

class Edge{
		
public:

	Flag flag;

	VertexPtr v0; 
	VertexPtr v1;

	Edge(VertexPtr pv0, VertexPtr pv1){
		v0 = pv0;
		v1 = pv1;
		v0->edges.push_back(this);
		v1->edges.push_back(this);
	};

	VertexPtr other(VertexPtr v){return (v==v0)?v1:v0;};
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
		v1 = v;
		v->edges.push_back(this);
		return e;
	};
};

class Face{
	
	Vec3 _mid;

public:

	list<VertexPtr> verts;
	list<EdgePtr> edges;

	Face(int s=0){};
	Vec3 mid(){return _mid;};

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
		updateVertsByEdges();
	};
	
	VertexPtr updateVertsByEdges(){

		verts.clear();
		EdgePtr e0 = edges.back();
		_mid.set(0,0,0);
		for(list<EdgePtr>::iterator it = edges.begin(); it!=edges.end(); it++ ){
			EdgePtr e = (*it);
			if ( (e->v0 != e0->v0) && (e->v0 != e0->v1) ){
				verts.push_back(e->v0);
				_mid = _mid + e->v0->getP();
			}else{
				verts.push_back(e->v1);
				_mid = _mid + e->v1->getP();
			}
			e0  = e;
		}
		_mid = _mid / verts.size();
	};
	

	void updateVertsByEdges(){
	}

};

#endif