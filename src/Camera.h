//
// Camera.h
// OpenGL Camera Class
// 
// Christopher Root, 2006
//
//

#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <GL/glut.h>
#include "Matrix.h"

#define INACTIVE 0
#define TRANSLATE 1
#define ROTATE 2
#define ZOOM 3

class Camera {

	private:
	// all variables starting with 'Default' hold the initial camera values
	// these values are used if the camera is reset to its initial state
	Vec3   DefaultPos;
	Vec3   DefaultAim;
	Vec3   DefaultUp;
	double DefaultNear;

	double DefaultAzim;
	double DefaultElev;

	double CurrentAzim;
	double CurrentElev;

	void Initialize();

public:
	Vec3 Pos;
	Vec3 Aim; 
	Vec3 Up;

	float NearPlane;
	float FarPlane;
	float Fov;

	// constructors

	// default constructor
	Camera();

	// constructor setting up camera orientation
	// P is position in 3D, A is the aim coordinate in 3D, U is the up vector
	Camera(Vec3 P, Vec3 A, Vec3 U);

	// constructor setting up camera orientation and view volume
	// P is position in 3D, A is aim coordinate in 3D, U is up vector
	// Near is near clipping plane, Far is far clipping plane, 
	// ViewAngle is field of view angle in degrees
	Camera(Vec3 P, Vec3 A, Vec3 U, float Near, float Far, float ViewAngle);

	// sets the clipping planes of the view volume
	void SetClippingPlanes(float Near, float Far);

	// sets the FOV, ViewAngle should be in degrees
	void SetFOV(float ViewAngle);	

	// set routines for Pos, Aim, and Up vector
	void SetPos(Vec3 P);
	void SetAim(Vec3 A);
	void SetUp(Vec3 U);
	void setAll(Vec3 P, Vec3 A, Vec3 U);

	// reset the camera to its initial position
	void Reset();

	// focus camera to some input aim position
	void SetCenterOfFocus(Vec3 NewAim);

	// function to use the camera as the opengl camera
	// W and H are the width and height of the window
	void PerspectiveDisplay(int W, int H);
	void OrthoDisplay(int W, int H);

	// function that handles mouse events
	Vec3 HandleMouseEvent(int button, int state, int x, int y);

	// function that handles mouse movements
	Vec3 HandleMouseMotion(int x, int y);
	
	Vec3 toWorld(int, int);
	Vec3 toView(const Vec3& );
	int* toScreen(const Vec3&);
	Vec3 center();

	const Camera& operator=(const Camera& cam);
	Matrix getProj();

	bool isInactive();
	Vec3 pos(){return Pos;};
	Vec3 norm(){return (Aim-Pos).normalize();};
	Vec3 up(){return Up;};

};

#endif

