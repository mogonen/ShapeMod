#ifndef __MESHSHAPE_H__	
#define __MESHSHAPE_H__


#include "CMesh.h"
#include "ControlPoint.h"
#include "Color.h"

using namespace std;
#define RAD 0.075
typedef Vertex** Verts;
typedef Vertex*** OutlineMap;

struct VertexStore{
	Vec3 * v3;
	Vec4 * v4;
	VertexStore(int s){
		v3 = new Vec3[s];
		v4 = new Vec4[s];
	};
};

class MeshShape:public Shape{

	Vertex* last_v;
	void insertSegment(const Vec3&);
	Vertex** getOutlineVerts(const Vec3&, const Vec3&, Mesh*, double, bool c = true);

protected:

	bool _isreversed;
	bool _shapechanged;
	CMesh * _control;
	void drawBorder();
	void getHits(list<ArrCurve*>, const Vec2&, const Vec2&, double&, double&);
	void flatShade();
	void genControlPoints();
	Vec3 tri[3];

public:

	MeshShape(){
		_control = new CMesh();
		_isreversed = false; 
		_shapechanged = false;
	}

	//shape methods
	virtual void draw(int i=0);
	bool isOn(const Vec3& p){return false;};
	int  type(){return MESH_SHAPE;};
	void onClick(const Vec3& p);
	void onActivate(){last_v = 0;};
	void exec(int cmd = 0 , void* p = 0);

	virtual void buildControlMesh();
	void makeOutline();
	void makeOutline(Vertex**, Vertex*, Vertex*);
	void makeOutline2(); // hexagon
	void makeOutline2(Vertex**, Vertex*, Vertex*, OutlineMap&, int);
	void makeOutline3();
	void makeOutline4();

	virtual int save(std::ofstream&);
	virtual int load(std::ifstream&);
	
};

#endif