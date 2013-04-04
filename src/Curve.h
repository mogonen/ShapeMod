#ifndef __CURVE_H__
#define __CURVE_H__

#include <map>
#include <vector>
#include <list>
#include <stdio.h>
#include "Vector.h"
#include "Matrix.h"
#include "Eye.h"
#include "Shape.h"

using namespace std;
class Curve;
#define CPPtr CurvePoint* 
#define PICK 0.015

class Curve:public Shape{

protected:

	vector<Vec3*> _pts;
	bool _isclosed;
	int _draw_samples;
	int _normal_dir;

public:

	Curve(){_isclosed = false; _normal_dir = 1.0;};
	Curve(Vec3 *, int, bool closed = false);
	~Curve();

	virtual Vec3 getP(double t)=0;
	virtual Vec3 getT(double t);
	virtual Vec3 getN(double t){
		return ( getT(t)%Vec3(0,0,1) ).normalize()*normaldir();
	};

	virtual double length();
	virtual int size(){return _pts.size();};
	virtual int samples(){return size()*4;};
	virtual Vec3 * toArr(int len=0);
	virtual void draw(int m=0);
	virtual int type(){return CURVE;};

	virtual void remove(Vec3 * p);
	virtual void insert(Vec3 * p, int i=-1);
	virtual void append(Curve*, bool reverse=false, bool tip = false);
	virtual void reverse(){};

	virtual int getI(Vec3 * p);
	virtual const Vec3& P(int i){return *_pts[i];};
	bool isFirst(Vec3 * p){return p == _pts.front();};
	bool isLast(Vec3 * p){return p == _pts.back();};
	double pickT(const Vec3&, double);
	int pick(const Vec3&, double, bool ends = false);

	bool close(double d = 0);
	bool isClosed(){return _isclosed;};

	bool isOn(const Vec3& p);
	bool isIn(const Vec3& p);
	void flipNormals(){_normal_dir*=-1;};
	int normaldir(){return _normal_dir;};
	
};

struct CurvePos{
	Curve* c;
	double	t;
	CurvePos(){};
	CurvePos(Curve* cc, double tt){ c = cc; t= tt;}
	Vec3 P(){return c->getP(t);};
    Vec3 T(){return c->getT(t);};
	Vec3 N(){return (c->getT(t) % Eye::get()->N ).normalize()*c->normaldir();};

	void draw(){
		Vec3 n = N();
		Vec3 p0 = P();
		Vec3 p1 = p0 + n*0.02;
		glBegin(GL_LINES);
			glColor3f(n.x, n.y, n.z);
			glVertex3f(p0.x, p0.y, p0.z);
			glVertex3f(p1.x, p1.y, p1.z);
		glEnd();
	};

};

class CurvePoint{

protected:
	Curve* _c;
	CPPtr _next;
	CPPtr _prev;
	Vec3 _p0, _t0, _n0;
	double _t;

	void updateNT();

public:

	CPPtr next(){return _next;};
	CPPtr prev(){return _prev;};
	CPPtr last();
	CPPtr first();

	CPPtr go(int);
	Vec3 go(double);
	int find(CPPtr);

	void setPrevNext(CPPtr p, CPPtr n);
	Vec3 P(){return _p0;};
	Vec3 T(){return _t0;};
	Vec3 N(){return _n0;};

	Curve* curve(){return _c;};

	//recomputed point
	Vec3 getP(){return _c->getP(_t);};
	Vec3 getN(){return ( _c->getT(_t) % Eye::get()->N ).normalize()*_c->normaldir();};

	void drawAll();
	bool isIn(Vec3 p);
};

struct CP{
	Vec3 P, T, N;
};

//a curve of array points
class ArrCurve:public Curve{

protected:
	Vec3 * _pts;
	int _size;
	double _len;
	double _step;
	bool _draw_aux;

public:
	ArrCurve(Vec3*, int size, bool resampleit = false);
	ArrCurve(ArrCurve&);
	ArrCurve(double *, int, Vec3 p, Vec3 nx, Vec3 ny, double w, double h);

	Vec3 getP(double t);
	Vec3 getP(int);
	Vec3 getT(int);
	void setP(int i, Vec3 p){_pts[i].set(p);};

	Vec3 first(){return _pts[0];};
	Vec3 last(){return _pts[_size-1];};

	int size(){return _size;};
	double length();
	Vec3* toArr(int l=0){return _pts;};

	void draw();
	void drawGLAux();

	void set(Vec3 * p, int s){delete [] _pts; _pts = p; _size = s;};
	void append(Vec3 * p, int s);

	void movingAverage(int num, bool norm=false);
	void resample(int size=0);
	void resample(double step);
	void reverse();

	//equadistance resampling
	static Vec3* resample(Curve* c, double step, int& newsize);
	static Vec3* resample(Vec3 * p, int size, double step, int& newsize);
	static Vec3* resample(Vec3 * p, int size, int outsize = 0);
	static double length(Vec3 * p, int size);
};

class ArrCurveUtils{

	ArrCurve* _ac;

public:
	double * maxhs;
	ArrCurveUtils(ArrCurve* ac){_ac = ac; };
	int * findMinMax(int step, int& mmi);
	double findMaxH(int i, int step, int& maxi);

	ArrCurve* getHCurve(int step, Vec3 p, int&);
};

class CorrCurve:public Curve{
	CP** _pts;
	int * _corr;
	CorrCurve** _cc;
	int _size;
public:
	CP* operator[](int i){i = (i<0)?0:((i>=_size)?_size-1:i); return _pts[i];};
	CP* corr(int i){if (i<0 || i>=_size) return 0; return _cc[i]->_pts[i];};
};

//Quadric BSPline
class BSpline:public Curve{
public:
	BSpline():Curve(){};
	BSpline(Vec3 *, int, bool closed = false);
	Vec3 getP(double t);
	void exec(int, void*p=0);
};

class Bezier:public Curve{

	int _deg;
	
public:
	Bezier(int deg):Curve(){_deg = deg;};
	Vec3 getP(double t);
};

class Knot:public ArrCurve{ 

	double _rad;

public:
	Knot(ArrCurve* ac):ArrCurve(ac->toArr(), ac->size()){};
	void solve(double rad);
};

class PolyLine:public ArrCurve{
};

class PointCurve:public Curve{
	Vec3 _p0;
public:
	PointCurve(Vec3 p);

	Vec3 getP(double t){return _p0;};

	void draw();

	int size(){return 1;};
	int type(){return POINT_CURVE;};
};

class Patch{

	Curve* _f0, * _f1, *_g0, *_g1;

	int ind(int, int, int);

public:

	Patch(Curve* f0, Curve* f1, Curve* g0, Curve* g1){
		_f0 = f0; _f1 = f1; _g0 = g0; _g1 = g1;
	};

	virtual Vec3 P(double, double);
	virtual Vec3 N(double, double);

	void draw(int w, int h);
	void fill(int w, int h);

};

class DPatch{ //Discrete Kunz Patch

	Curve** _fs;
	Vec3* _ps;
	Vec3* _ns;

	int _N, _Ni, _NN;

	inline int ind(int i, int j){return i + j*_N;};
	inline Vec3 _p(int i, int j){return _ps[i + j*_N];};
	inline Vec3 _n(int i, int j){return _ns[i + j*_N];};

public:

	DPatch(Curve** fs, int n){
		_fs = fs;
		_N = n;
		_Ni = _N-1;
		_NN = n*n;
		_ps = new Vec3[_NN];
		_ns = new Vec3[_NN];
	};

	Vec3 P(int, int);
	Vec3 N(int, int);

	void update();

	void draw();
	void fill();

};



#endif