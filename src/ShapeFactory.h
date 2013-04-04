#ifndef __SHAPEFACTORY_H__	
#define __SHAPEFACTORY_H__

#include "MeshShape.h"
#include "CurvedShape.h"
#include "SubdivShape.h"
#include "BezierShape.h"
#include "FlatShape.h"

//CurvedShape * cp;

class ShapeFactory{

public:
	static Shape* newShape(int type){

		switch(type){

			case CURVED_SHAPE:
				return 0;// new CurvedShape();
			break;

			case MESH_SHAPE:
				return new MeshShape();
			break;

			case SUBDIV_SHAPE:
				return new SubdivShape();
			break;

			case FLAT_SHAPE:
				return new FlatShape();
			break;

			case BEZIER_SHAPE:
				return new BezierShape();
			break;

		}
		return 0;
	};

	static Shape* loadShape(std::ifstream& inf, int type){
		Shape * shape = ShapeFactory::newShape(type);
		shape->load(inf);
		return shape;
	};

};

#endif