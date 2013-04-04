#ifndef __FLATSHAPE_H__
#define __FLATSHAPE_H__

#include "MeshShape.h"

class FlatShape:public MeshShape{

	void flatShade();

public:
	FlatShape():MeshShape(){};

	void draw(int i=0);
	void exec(int cmd=0, void* param=0);
	void onShapeChanged(int type);

	int load(std::ifstream& inf);
};


#endif