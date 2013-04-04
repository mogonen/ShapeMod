#ifndef __SHAPE_H__
#define __SHAPE_H__

#include <list>
#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include "Matrix.h"
#include "Auxiliary.h"

class Shape;
#define ShapePtr Shape*

enum{POLYLINE_CURVE, POINT_CURVE, CURVE, CURVED_SHAPE, MESH_SHAPE, SUBDIV_SHAPE, FLAT_SHAPE, BEZIER_SHAPE, LUMO_SHAPE, MESH};
//enum{DEFAULT, SELECTED, OUTLINE, SOLID, OUTLINE_SOLID};
enum Command{
	CLICK,
	CTRL_CLICK,
	DRAG,
	INSERT,
	DELETE,
	SELECT,
	UNDO,
	BUILD,
	REBUILD,
	REVERT,
	BLEND_MESH_NORMALS,
	BUILD_NO_SPINE
};

//enum{ UNSELECTED, SELECTED, HIDDEN};

enum{SELECTED, DRAGGABLE, SELECTABLE, NO_DELETE, SELECT_PARENT, HIDDEN, HIDDEN_AS_CHILD, LIGHT, HEAVY};

class Shape{

	void (*_shapeChangedHandler)(void*);
	void * _param;
	Vec3 _t0;
	list<ShapePtr> _childs;
	ShapePtr _parent;

	void _drawAll(){
		glPushMatrix();
			glTranslatef(_t0.x, _t0.y, _t0.z);


			if (!is(HIDDEN))
				draw(is(SELECTED));
			glColor3f(0,0,0);
			for(list<ShapePtr>::iterator it = _childs.begin(); it!= _childs.end(); it++){
				if (!(*it)) continue;
				if (!(*it)->is(HIDDEN_AS_CHILD))
					(*it)->_drawAll();
			}
		glPopMatrix();
	};

   void _draw(int mode){
		glPushMatrix();
			glTranslatef(_t0.x, _t0.y, _t0.z);
			if (!is(HIDDEN))
				draw(mode);
		glPopMatrix();
	};

	unsigned int _flags;
	int _id;
	/*bool _selected;
	bool _draggable;
	bool _selectable;
	*/
	static int _COUNT;

protected:
	virtual void onClick(const Vec3& p){};

public:

	Shape(){_shapeChangedHandler=0; _parent = 0; _flags = 0; set(DRAGGABLE); set(SELECTABLE); _id = _COUNT++;};
	virtual ~Shape(){
		unparent();
		for(list<ShapePtr>::iterator it = _childs.begin(); it!= _childs.end(); it++)
			(*it)->_parent = 0;
	};

	virtual bool isFlat(){return true;};
	virtual bool isOn(const Vec3& p)=0;
	
	virtual void draw(int mode=0)=0;

	//send generic command to the shape
	virtual void exec(int = 0, void* p =0){};

	virtual void shapeChanged(int type = 0){
		if (_shapeChangedHandler)
			_shapeChangedHandler(_param);
		else 
			onShapeChanged(type);
	};

	virtual void onShapeChanged(int type = 0){};

	void setShapeChangedHandler(void (*handler)(void*), void* param){_shapeChangedHandler = handler; _param = param;};
	virtual int type()=0;

	virtual void drag(Vec3 p){
		if (is(DRAGGABLE))
			_t0 = _t0 + p; 
	};

	const Vec3& T(){return _t0;};

	Vec3 gT(){
		if (!_parent)
			return _t0;
		return _t0 + _parent->gT();
	};

	virtual ShapePtr select(const Vec3& p){
		Vec3 p0(p);
		p0 = p0 - _t0;
		ShapePtr selected = 0;

		if ( is(SELECTABLE) && isOn(p0) )
			selected = this;

		if (!_childs.empty()){
			for(list<ShapePtr>::iterator it = _childs.begin(); it!= _childs.end(); it++){

				ShapePtr sp = (*it)->select(p0);
				if (sp){
					selected = sp;
					/*if (sp->is(SELECT_PARENT) && sp->isChild())
						selected = sp->_parent;*/
				}
			}
		}

		return selected;
	};

	void click(const Vec3& p){
		Vec3 p0(p);
		p0 = p0 - _t0;
		onClick(p0);
	}

	virtual void onActivate(){};
	virtual void onDeactivate(){};

	void drawAll(){
		if (_parent){
			Vec3 pt = _parent->gT();
			glPushMatrix();
				glTranslatef(pt.x, pt.y, pt.z);
				_drawAll();
			glPopMatrix();
		}else 
			_drawAll();
	};

	void drawSelf(int mode){
		if (_parent){
			Vec3 pt = _parent->gT();
			glPushMatrix();
				glTranslatef(pt.x, pt.y, pt.z);
				_draw(mode);
			glPopMatrix();
		}else 
			_draw(mode);
	};

	void resetT(){_t0.set();};
	virtual void frezeT(){};

	void unparent(){
		if (_parent) 
			_parent->_childs.remove(this);
		_parent = 0;
	};

	bool isChild(){return _parent!=0;};
	bool adopt(ShapePtr s){
		if (s->_parent)
			return false;
		_childs.push_back(s);
		s->_parent = this;
		return true;
	};

	//flag operations
	bool is(unsigned int bit){return _flags & (1 << bit);};
	void set(unsigned int bit){_flags |= (1 << bit);};
	void unset(unsigned int bit){_flags &= ~(1 << bit);};
	int id(){return _id;};

	//save&load
	virtual int load(std::ifstream&){return -1;};
	virtual int save(std::ofstream&){return -1;};

};


#endif
