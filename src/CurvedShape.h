#ifndef __CURVEDSHAPE_H__
#define __CURVEDSHAPE_H__

#include <list>
#include "Shape.h"
#include "Curve.h"
#include "Canvas.h"
#include "Curve2Mesh.h"

class CurvedShape;
struct Isopam;

class CurvedShape:public Shape{

	enum{NO_SPINE, SPINE};

	list<Curve*> _border;
	Curve* _spine;
	Curve* _border0;

	Mesh* _mesh;

	double getHit(Vec3 p, Vec3 n, CurvePos&, Vec3& phit);
	double getHit(Vec2 p, Vec2 n, CurvePos&);

	void buildShapeBySpine(Curve*, int caps = 0);
	void buildShapeNoSpine();
	void buildShapeBySpine();
	void buildBorderBySpine(Curve*,  int caps = 0);
	void buildRadialShape(Curve* sp, int caps, int vnum);

	Vertex* * createVerts(int num);
	float _flatness;
	float _n_z;

public:
	CurvedShape():Shape(){_mesh = 0; _border0 = 0;  _spine = 0; _flatness = FLATNESS; _n_z = N_Z; };// set(SELECT_PARENT);};
	~CurvedShape();

	void draw(int mode);
	bool isOn(const Vec3& p);
	void exec(int command = 0, void* param = 0);
	int type(){return CURVED_SHAPE;};

	void revert();
	void flip();
	void updateIsopams();
	float flatness(){return _flatness;};
	float nz(){return _n_z;};
	//void drag(Vec3);

	void drawBorderAux();
	void onClick(const Vec3& p);

	list<Isopam> isopams;

	static float RADIUS;
	static float FLATNESS;
	static int CAPS;
	static int VNUM;
	static int CAPS_ON;
	static float SPINE_STEP;
	static float FADEIN;
	static float FEATHER;

	//static Vec3 TO_N;
	static float N_Z;
};

struct Isopam{
	CurvePos cp0, cp1;
	double flatness;
	bool hasmid;
	bool twopointed;
	int vnum;
	Vertex** verts;

	double alpha0, alpha1;
	CurvedShape* cshape;

	Isopam(const CurvePos& pcp0 , const CurvePos& pcp1, CurvedShape* cs, bool hm = true){
		cp0 = pcp0;
		cp1 = pcp1;
		flatness   = -1;
		twopointed = true;
		hasmid = hm;
		alpha0 = 1.0 - CurvedShape::FEATHER;
		alpha1 = 1.0;
		cshape = cs;
	};

	Isopam(const CurvePos& pcp0, CurvedShape* cs, bool tp = false){
		cp0 = pcp0;
		flatness = -1;
		hasmid = false;
		twopointed = tp;
		alpha0 = 1.0 - CurvedShape::FEATHER;
		alpha1 = 1.0;
		cshape = cs;
	};

	void setVerts(Vertex** v, int n){
		verts = v;
		vnum = n;
		updateVerts();
	}

	void updateVerts();
	void updateVertsNoP();
};
#endif