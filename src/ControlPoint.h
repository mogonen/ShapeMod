#ifndef __CONTROLPOINT_H__
#define __CONTROLPOINT_H__
#include "Curve.h"
class ControlPoint;

//typedef map<Curve*, vector<ControlPoint*>*> ControlMap;

class ControlPoint{


protected:
	ControlPoint(ShapePtr sp, Vec3 * p);

	ShapePtr _shape;
	bool _selected;
	bool _undraggable;
	double _rad;
	Vec3* _p;

	static vector<ControlPoint*> _controls;
	//static ControlMap _controlsmap;
	static ControlPoint* _active;

	void * _store;

	static int _selected_count;

	virtual void _drag(const Vec3& p);

public:
	
	ControlPoint(){_undraggable = false;};

	ShapePtr shape(){return _shape;};

	virtual void draw();
	virtual void clicked(){};
	void store(void* p){_store = p;};
	void* retrieve(){return _store;};
	void select(){_selected = true;_selected_count++;};

	static void drag(const Vec3& p);
	static int handleMouseAll(int, Vec3);
	static ControlPoint* activate(ShapePtr, const Vec3& p);
	static void deactivate(){
		if (_active && _active->_shape)
			_active->_shape->shapeChanged(HEAVY);
		_active = 0;
	};
	static void dragSelected(const Vec3& p);

	static ControlPoint* create(ShapePtr sp, Vec3 * p);
	static void add(ControlPoint * cp);
	//ControlPoint* create(ShapePtr sp, Curve* c, Vec3 p, int typ=0);
	static Vec3 (*toView)(Vec3 p);

	static void drawAll();
	static void draw(ShapePtr);
	static void deselect();
	static int selectedCount(){return _selected_count;};

	static void clear();
	static void remove(ShapePtr);
	static ControlPoint* active(){return _active;};

};

class TangentPoint:public ControlPoint{

	TangentPoint* _tp0, *_tp1, *_tp2;
	bool _isparent, _iscont;

protected:
	void _drag(const Vec3& p);

	TangentPoint(ShapePtr sp, Vec3 * p, TangentPoint* t):ControlPoint(sp, p){
		_tp0 = _tp1 = _tp2 = 0;
		if (t){
			_isparent = false;
			t->set(this);
		}else
			_isparent = true;
		_iscont   = false;
	}

public:

	void set(TangentPoint* t, int i=-1);

	void draw();

	static TangentPoint* create(ShapePtr sp, Vec3 * p, TangentPoint* t=0);

};


#endif