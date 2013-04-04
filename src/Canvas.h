#ifndef __LAYER_H__
#define __LAYER_H__

#include <list>
#include <hash_set>
#include <GL/glut.h>

#include "Matrix.h"
#include <algorithm>
#include "Shape.h"
#include "Stroke.h"
#include "Curve.h"
#include "ControlPoint.h"
#include "ShapeFactory.h"


#define ShapePtr Shape*
#define UNITZ 0.01

struct MOUSE{
	enum{DRAGGED, DOWN, UP, SELECT};
};

class Canvas{

	list<ShapePtr> _shapes;
	list<Curve*> _curves;
	list<Curve*> _selected;

	list<Vec3*> _layers;

	ShapePtr _active;

	Vec3 _mp0;
	int _mstate;


	static Canvas * _canvas;

	StrokeManager * strokeman;

public:

	Canvas(){_shapes.clear();_active =0; strokeman = new StrokeManager();};
	void insert(ShapePtr sp){
		_shapes.push_back(sp);
		if (sp->type() <= CURVE)
				_curves.push_back((Curve*)sp);
		
		activate(sp); 
		cout<<"new shape:"<<sp<<endl;
	};

	void remove(ShapePtr sp){_shapes.remove(sp);
		_shapes.remove(sp);
		ControlPoint::remove(sp);
		if (sp->type()<=CURVE)
			_curves.remove((Curve*)sp);
		delete sp;
	};

	void sendToFront(ShapePtr sp){
		_shapes.remove(sp);
		_shapes.push_front(sp);
	};

	void sendToBack(ShapePtr sp){
		_shapes.remove(sp);
		_shapes.push_back(sp);
	};

	void removeActive(){
		if (!_active || _active->is(NO_DELETE))
			return;
		remove(_active);
		_active = 0;
	};

	void deselect(){
		_selected.clear();
		for(list<ShapePtr>::iterator it = _shapes.begin(); it!=_shapes.end(); it++)
			(*it)->unset(SELECTED);
		ControlPoint::deselect();
	};

	void deselectAll(){
		deactivate();
		deselect();
	};

	void activate(ShapePtr sp){
		if (!sp)
			return;
		_active = sp;
		_active->onActivate();
	};

	void deactivate(){
		if (!_active)
			return;
		_active->onDeactivate();
		_active = 0;
	};

	void selectPrev(){
		if (!_shapes.size())
			return;
		if (_shapes.size() == 1){
			activate(_shapes.front());
			return;
		}
		std::list<ShapePtr>::iterator it = std::find(_shapes.begin(), _shapes.end(), _active);
		if (it == _shapes.begin())
			it = _shapes.end();
		it--;
		activate(*it);
	};

	void selectNext(){
		if (!_shapes.size())
			return;
		if (_shapes.size() == 1){
			activate(_shapes.front());
			return;
		}
		std::list<ShapePtr>::iterator it = std::find(_shapes.begin(), _shapes.end(), _active);
		if (it != _shapes.end())
			it++;
		
		if (it == _shapes.end())
			it = _shapes.begin();
	
		activate(*it);
	};

	void activeUp(){
		if (!_active)
			return;

		if (ControlPoint::selectedCount()){
			ControlPoint::dragSelected(Vec3(0,0, UNITZ));
			return;
		}

		std::list<ShapePtr>::iterator it = std::find(_shapes.begin(), _shapes.end(), _active);
		std::list<ShapePtr>::iterator itn = it;
		itn++;
		if ( itn == _shapes.end())
			return; 
		ShapePtr tmp = *it;
		(*it) = *itn;
		(*itn) = tmp;
	}

	void activeDown(){
		if (!_active)
			return;

		if (ControlPoint::selectedCount()){
			ControlPoint::dragSelected(Vec3(0,0,-UNITZ));
			return;
		}

		std::list<ShapePtr>::iterator it = std::find(_shapes.begin(), _shapes.end(), _active);
		std::list<ShapePtr>::iterator itp = it;
		if ( itp == _shapes.begin())
			return; 
		itp--;
		ShapePtr tmp = *it;
		(*it) = *itp;
		(*itp) = tmp;
	};

	void clear(){
		for(list<ShapePtr>::iterator it = _shapes.begin(); it!=_shapes.end(); it++)
			delete *it;
		_shapes.clear();
		_curves.clear();
		deselect();
	};

	void draw();
	//void drawCurves();
	//Screwed Ad hoc code -bad!
	void drawCurvedShapes(bool);

	ShapePtr active(){return _active;};
	bool handleMouse(int state, const Vec3& p); 

	list<Curve*> selectedCurves(){
		return _selected;
	};

	Curve* activeCurve(){
		if (_active->type() <= CURVE)
			return (Curve*)_active;
		return 0;
	}

	void flushSelectedCurves(){
		for(list<Curve*>::iterator it = _selected.begin(); it!=_selected.end(); it++){
			_shapes.remove(*it);
			_curves.remove(*it);
		}
	};

	void flushActiveCurve(){
		if (_active->type() > CURVE)
			return;
		_shapes.remove(_active);
		_curves.remove((Curve*)_active);
	}

	void assignLayerDepths(){
		double z = 1.0; 
		for(list<Vec3*>::iterator it = _layers.begin(); it!=_layers.end(); it++){
			(*it)->z = z;
			z = z + 1.0; 
		}
	}


	int saveTo(const char * fname);
	int loadFrom(const char * fname);

	//statics
	static bool ISOLATE;
	static int TOOL;
	enum{MESH_TOOL, SPLINE_TOOL, STROKE_TOOL};

	enum{BORDER, SPINE};

	static int MODE;
	enum {SKETCH, EDIT, SHAPEMAP, OUTLINE, ALPHA};

	static Canvas* getCanvas(){return _canvas;};

};

#endif

/*
class BBox2D{
	Vec2 _p0, _p1;
public:
	BBox2D(){
		double max = 9999999;
		_p0.set(max,max);
		_p1.set(-max, -max);
	}
	BBox2D(Vec2 p0, Vec2 p1){
		_p0 = p0;
		_p1 = p1;
	};

	void update(Vec2 p){

		if (_p0.x > p.x)
			_p0.x = p.x;

		if (_p0.y > p.y)
			_p0.y = p.y;

		if (_p1.x < p.x)
			_p1.x = p.x;

		if (_p1.y < p.y)
			_p1.y = p.y;
	};

	bool isIn(Vec2 p){
		if ((_p0.x < p.x ) && (_p0.y < p.y) && (_p1.x > p.x) && (_p1.y < p.y))
			return true;
		return false;
	};
};

	void activeUp(){
		if (!_active)
			return;

		if (ControlPoint::selectedCount()){
			ControlPoint::dragSelected(Vec3(0,0,0.01));
			return;
		}

		std::list<ShapePtr>::iterator it = std::find(_shapes.begin(), _shapes.end(), _active);
		std::list<ShapePtr>::iterator itn = it;
		itn++;
		if ( itn == _shapes.end())
			return; 
		ShapePtr tmp = *it;
		(*it) = *itn;
		(*itn) = tmp;
	}

	void activeDown(){
		if (!_active)
			return;

		if (ControlPoint::selectedCount()){
			ControlPoint::dragSelected(Vec3(0,0,-0.01));
			return;
		}

		std::list<ShapePtr>::iterator it = std::find(_shapes.begin(), _shapes.end(), _active);
		std::list<ShapePtr>::iterator itp = it;
		if ( itp == _shapes.begin())
			return; 
		itp--;
		ShapePtr tmp = *it;
		(*it) = *itp;
		(*itp) = tmp;
	};

*/