#ifndef __SKELETON_H__
#define __SKELETON_H__

#include "Curve2Mesh.h"

#define JntP Joint*
#define SpnP Spine*

class Joint;
class Spine;


class Joint{
	list<SpnP> _spines;
public:
	void addSpine(SpnP s){_spines.push_back(s);};
};

class Spine{

	JntP _j0;
	JntP _j1;
public:
	Spine(JntP j0, JntP j1){_j0 = j0; _j1=j1;};
};

class Skeleton{

	list<SpnP> _spines;
	list<JntP> _joints;

public:
	JntP addJoint(){
		JntP jp = new Joint(); 
		_joints.push_back(jp);
	};

	SpnP addSpine(JntP j0, JntP j1){
		SpnP sp = new Spine(j0, j1);
	};

};

#endif