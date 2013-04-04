#ifndef _H_EYE
#define _H_EYE

#include "Vector.h"
#include "Camera.h"

static const double PZ  = 1.0;
static const double FOV = 60.0;

static const Vec3 ON_P(0,0,0);
static const Vec3 ON_N(0,0,-1);
static const Vec3 ON_U(0,1,0);

static const Vec3 AIM(0,0,-1);

static const Vec3 TOP_P(0, 1.5, -1);
static const Vec3 TOP_U(0, 0, -1);
static const Vec3 SOL_P(-1.5, 0, -1);
static const Vec3 SOL_U(0,1,0);

class Eye{

	static Eye * _eye;
	
public:
	Vec3 P, N, U;

	void set(Vec3 p, Vec3 n, Vec3 u){
		P.set(p); N.set(n); U.set(u);
	};

	static void setCam(Camera * cam);
	static Eye * get();
};

#endif