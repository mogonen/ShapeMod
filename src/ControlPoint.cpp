#include "ControlPoint.h"

vector<ControlPoint*> ControlPoint::_controls;
//ControlMap ControlPoint::_controlsmap;
ControlPoint* ControlPoint::_active = 0;
int ControlPoint::_selected_count = 0;

ControlPoint:: ControlPoint(ShapePtr sp, Vec3 * p){ 
	_shape = sp; _p = p; 
	_selected = false;
	_undraggable = false;
	_rad = 0.015;
};

void ControlPoint::_drag(const Vec3& p){
	_p->set(*_p + p );
	_shape->shapeChanged(LIGHT);
}

void ControlPoint::drag(const Vec3& p){
	//Vec3 pre = toView(*_p);
	if (_active)
		_active->_drag(p);
};

int ControlPoint::handleMouseAll(int state, Vec3 p){

	_active = 0;
	for (vector<ControlPoint*>::iterator it = ControlPoint::_controls.begin(); it != ControlPoint::_controls.end(); it++){
		//Vec3 pp = toView(*((*it)->_p));
		Vec3 pp = *(*it)->_p;
		double rad = (p - pp).norm();
		if (rad < PICK && state==1)
			_active = (*it);
	}
	return (_active!=0);
};

ControlPoint* ControlPoint::activate(ShapePtr sp, const Vec3& p){
	ControlPoint* selected = 0;
	for (vector<ControlPoint*>::iterator it = ControlPoint::_controls.begin(); it != ControlPoint::_controls.end(); it++){
		if ( (*it)->_shape != sp )
			continue;
		//Vec3 pp = toView(*((*it)->_p));
		Vec3 pp = *((*it)->_p);
		double rad = (Vec2(p) - Vec2(pp) ).norm();
		if (rad < PICK ){
			_active = (*it);
			selected = _active;
		}
	}	
	return selected;
};

ControlPoint* ControlPoint::create(ShapePtr sp, Vec3 *p){
	ControlPoint* cp = new ControlPoint(sp, p);
	ControlPoint::_controls.push_back(cp);
	return cp;
};
/*
ControlPoint* ControlPoint::create(ShapePtr sp, Vec3 p, int type){
	ControlPoint* cp = new ControlPoint(sp, new Vec3(p), type);
	ControlPoint::_controls.push_back(cp);
	return cp;
};
*/
void ControlPoint::add(ControlPoint * cp){
	ControlPoint::_controls.push_back(cp);
	/*ControlMap::iterator it = ControlPoint::_controlsmap.find(cp->_curve);
	if (it==ControlPoint::_controlsmap.end()){
		vector<ControlPoint*>* list = new  vector<ControlPoint*>();
		list->push_back(cp);
		ControlPoint::_controlsmap[cp->_curve] = list;
	}else{
		it->second->push_back(cp);
	}*/
};

void ControlPoint::draw(){
	glBegin(GL_POINTS);
	/*if (_draggable){
			glColor3f(1.0, 0.5, 0.5);
		} else {
			glColor3f(0.1, 0.1, 0);
		}*/
	if (_undraggable)
			glColor3f(0.5, 0.5, 0.15);
	else if (_selected)
		glColor3f(0.95, 0.95, 0.15);
	else
		glColor3f(0.95, 0.95, 0.75);

	if (_active == this)
		glColor3f(0.95, 0.95, 0.95);

	glVertex3f(_p->x, _p->y, _p->z);
	glEnd();
};

void ControlPoint::drawAll(){
	
	glColor3f(1.0, 0.7, 0);
	glPointSize(4.0);
	//glBegin(GL_POINTS);
		for (vector<ControlPoint*>::iterator it = ControlPoint::_controls.begin(); it != ControlPoint::_controls.end(); it++)
			(*it)->draw();
	//glEnd();
};

void ControlPoint::draw(ShapePtr sp){

	glBegin(GL_POINTS);
	for (vector<ControlPoint*>::iterator it = ControlPoint::_controls.begin(); it != ControlPoint::_controls.end(); it++)
			if ((*it)->_shape == sp)
				(*it)->draw();
	glEnd();

	/*ControlMap::iterator itm = ControlPoint::_controlsmap.find(c);
	if (itm==ControlPoint::_controlsmap.end())
		return;
	vector<ControlPoint*> * cplist = itm->second;
	/*for (vector<ControlPoint*>::iterator it = cplist->begin(); it != cplist->end(); it++){
		if (*it)
			(*it)->draw();
	}*/

};

void ControlPoint::clear(){
	_controls.clear();
};

void ControlPoint::remove(ShapePtr sp){
	vector<ControlPoint*>::iterator it = ControlPoint::_controls.begin(); 		
	while(it != ControlPoint::_controls.end()){
		if ((*it)->_shape == sp){
			ControlPoint * cp = (*it);
			it = _controls.erase(it);
			delete cp;
		} else
			it++;
	}
};

void ControlPoint::deselect(){
	for (vector<ControlPoint*>::iterator it = ControlPoint::_controls.begin(); it != ControlPoint::_controls.end(); it++)
		(*it)->_selected = false;
	_selected_count = 0;
};

void ControlPoint::dragSelected(const Vec3& p){
	ControlPoint* selected = 0;
	for (vector<ControlPoint*>::iterator it = ControlPoint::_controls.begin(); it != ControlPoint::_controls.end(); it++)
		if ((*it)->_selected)
			(*(*it)->_p) = (*(*it)->_p) + p;
};

void TangentPoint::_drag(const Vec3& p){
	if (!_tp2)
		_p->set(*_p + p );
	if (_isparent){
		if (_tp0)
			_tp0->_p->set(*(_tp0->_p) + p );
		if (_tp1)
			_tp1->_p->set(*(_tp1->_p) + p );
		if (_tp2)
			_tp2->_drag(p);
	}else if (_tp0->_iscont){
		//continuous tangents
	}
	_shape->shapeChanged(LIGHT);
}

void TangentPoint::draw(){
	ControlPoint::draw();
	if (!_isparent){
		glColor3f(0,0,0);
		glLineWidth(1.0);
		glBegin(GL_LINES);
			glVertex3f(_tp0->_p->x, _tp0->_p->y, _tp0->_p->z);
			glVertex3f(_p->x, _p->y, _p->z);
		glEnd();
	}
}

TangentPoint* TangentPoint::create(ShapePtr sp, Vec3 * p, TangentPoint* t){
	TangentPoint* tp = new TangentPoint(sp, p, t);
	TangentPoint::_controls.push_back(tp);
	return tp;
}

void TangentPoint::set(TangentPoint* t, int i){
		if (!_isparent)
			return;
		//explixit set
		t->_tp0 = this; //set parent;

		if (i==0){ 
			_tp0 = t;
			if (_tp1){
				_tp1->_tp1 = t;
				t->_tp1 = _tp1;
			}
		}else if (i==1){
			_tp1 = t;
			if (_tp0){
				_tp0->_tp1 = t;
				t->_tp1 = _tp0;
			}
		}else{ //now set in order
			if (!_tp0)
				set(t, 0);
			else if (!_tp1)
				set(t, 1);
			else {
				if (!_tp2)
					_tp2 = new TangentPoint(_shape, _p, 0); // new hidden parent
				_tp2->set(t);
			}
		}
	}