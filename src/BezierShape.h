#ifndef __BEZIERSHAPE_H__	
#define __BEZIERSHAPE_H__

#include "MeshShape.h"

struct BPatch;

struct VertexNormal{

	Vec3* v0, *v1;
	list<Vec3*> nlist;
	Vec3** quad;
	bool isborder;


	VertexNormal(bool ib){
		v0 = v1 = 0;
		isborder = ib;
		if (ib)
			quad = 0;
		else
			quad = new Vec3*[4];
	}

	void update();
	void draw();
};

class BezierShape:public MeshShape{

	list<Curve*> _curves;
	list<BPatch*> _patches;
	void buildCurves();
	void buildCurves2();

	map<Vec3*, TangentPoint*> tpmap;
	map<Vertex*, VertexNormal*> _vtmap;
	Bezier* edgeBezier(Vec3* p0, Vec3* p1);

	void updateBPatch();

public:

	void draw(int m);
	void exec(int cmd = 0 , void* p = 0);
	static bool MODE;
};


struct BPatch{
	//discreete bilinear patch

	Curve** fs;
	Vec3* ps;
	Vec3* ns;

	unsigned char seams;

	static int N, Ni, NN;
	static double T;
	static void setN(int n){
		N = n;
		Ni = N-1;
		NN = n*n;
		T = 1.0/Ni;
	}

	BPatch(){
		ps= new Vec3[NN];
		ns= new Vec3[NN];
		seams = 0x00;
	};

	BPatch(Curve** c){
		fs = c;
		seams = 0x00;
		ps= new Vec3[NN];
		ns= new Vec3[NN];
	};

	void setCurves(Curve** c){fs = c;}

	static inline int ind(int i, int j){return i + j*N;};
	inline Vec3 p(int i, int j){return ps[i + j*N];};
	inline Vec3 n(int i, int j){return ns[i + j*N];};

	void writeCurves();
	void computeNormals();
	void computeNormals2();
	void initCornerNormals();

	void update(bool nz=false);
	void draw();

	Vec3* cornerNormalPtr(int i);
	Vec3* preCornerPtr(int);
	Vec3* postCornerPtr(int);
	Vec3* cornerQuadDiagonalPtr(int);

	bool isSeam(int i){return seams & (1<<i);}
	void setSeam(int i){seams = seams | (1<<i);}

private:
	Vec3 interpolate(int, int, Vec3*);
	void interpolateNormalsAccrossEdge(int ei);
};


#endif