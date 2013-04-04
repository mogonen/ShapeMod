#ifndef __LUMOSHAPE_H__	
#define __LUMOSHAPE_H__

#include <GL/glew.h>
#include "Canvas.h"
#include "Color.h"

class LumoShape:public Shape{

	DoubleMap* _pmap, *_vmap;
	BitMap* _bmap;
	GLuint _tmap;

	Vec2 _quad;

	void renderToTex();
	void render();
	list<Curve*> _curves;

public:

	LumoShape();
	void draw(int i=0);
	bool isOn(const Vec3& p){return false;};
	int  type(){return LUMO_SHAPE;};
	void onClick(const Vec3& p){};
	void onActivate(){};
	void onShapeChanged(int type=0){};
	void exec(int cmd = 0 , void* p = 0);

};

#endif