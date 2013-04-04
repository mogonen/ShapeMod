#include "Stroke.h"

using namespace std;

StrokeManager * StrokeManager::_manager;

void Stroke::draw(){
	if (_curve)
		_curve->draw();
	else{
		glBegin(GL_LINE_STRIP);
		for(list<Vec3>::const_iterator it = _pts.begin(); it!=_pts.end(); it++)
			glVertex3f((*it).x, (*it).y, (*it).z);
		glEnd();
	}
}

Stroke::Stroke():Curve(){
	_curve = 0;
	_hcurve =0;
	_isclosed = false;
	cout<<"new stroke!"<<endl;
}

void Stroke::add(Vec3 p){
	_pts.push_back(p);
}

Vec3 Stroke::getP(double len){
	return _pts.front();
}

void Stroke::finalize(){

	if (_pts.size()<10)
		return;

	Vec3 * arr = new Vec3[_pts.size()];
	int i = 0;
	for(list<Vec3>::const_iterator it = _pts.begin(); it!=_pts.end(); it++)
		arr[i++] = (*it);
	ArrCurve * ac = new ArrCurve(arr, i, true);
	if (( ac->getP(0) - ac->last() ).norm()<0.05){
		ac->close();
		this->close();
	}
	Canvas::getCanvas()->insert(BSpline::create(ac, 0.1));
}

double Stroke::length(){
	if (!_curve)
		return 0;
	return _curve->length();
}

int Stroke::size(){
	if (!_curve)
		return 0;
	return _curve->size();
}

Vec3 * Stroke::toArr(int len){
	if (!_curve)
		return 0;
	return _curve->toArr();
}

void Stroke::makeNURBS(){
	NURBS* nc = NURBS::create(_curve, 0.05);	
	delete _curve;
	_curve = nc;
}

StrokeManager* StrokeManager::getManager(){
	if (!_manager)
		_manager = new StrokeManager();
	return _manager;
}

StrokePtr StrokeManager::newStroke(){
	active = new Stroke();
	//strokes.push_back(active);
	return active;
}

void StrokeManager::add(Vec3 _p){
	if (!active)
		newStroke();
	active->add(_p);
}

void StrokeManager::draw(){
	glColor3f(0.0f,0.0f,0.0f);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(3.0);
	for (list<StrokePtr>::iterator it = strokes.begin(); it != strokes.end(); it++)
		(*it)->draw();
	glLineWidth(1.0); 
	glDisable(GL_LINE_SMOOTH);

}

void StrokeManager::HandleMouseEvent(int button, int state, Vec3 p){
	int mode = glutGetModifiers();
	if ( mode == GLUT_ACTIVE_ALT || mode == GLUT_ACTIVE_CTRL)
		return;

	if (state == GLUT_UP){
		if (active){
			active->finalize();
			active = 0;
		} else
			ControlPoint::handleMouseAll(2, p);
	}
	
	if (!active && state == GLUT_DOWN){
		if (!ControlPoint::handleMouseAll(1, p))
			newStroke();
	}
}

void StrokeManager::HandleMouseMotion(Vec3 p){
	/*if ( (m_prev - p).norm()<0.01 )
		return;*/
	if (active)
		active->add(p);
}

list<StrokePtr>* StrokeManager::getStrokes(){return &strokes;}

void StrokeManager::add(StrokePtr s){
	strokes.push_back(s);
}

void StrokeManager::insertStrokes(int id){
	StrokePtr s0 = new Stroke();
	StrokePtr s1 = new Stroke();
	Vec3 p0(-0.5, 0.1, -PZ);
	Vec3 p1(0.5, -0.1, -PZ);
	Vec3 u(0.025, 0.01, 0);

	for(int i =0; i<40; i++){
		s0->add(p0);
		s1->add(p1);
		p0 = p0+u;
		p1 = p1-u;
	}
	s0->finalize();
	s1->finalize();
	add(s0);
	add(s1);
}

StrokeManager::StrokeManager(){
	active = 0;
}

void StrokeManager::makeNURBS(){	
	for (list<StrokePtr>::iterator it = strokes.begin(); it != strokes.end(); it++)
		((StrokePtr)(*it))->makeNURBS();
}

void StrokeManager::deleteLast(){
	if (!strokes.size())
		return;
	CurvePtr cp = strokes.back();
	strokes.pop_back();
	ControlPoint::remove(((StrokePtr)cp)->curve());
	delete cp;
}

void StrokeManager::reset(){
	strokes.clear();
	active = 0;
}

void StrokeManager::processStrokes(){	
}

void StrokeManager::setCurves(list<CurvePtr> clist){
	strokes.clear();
	for(list<CurvePtr>::iterator it = clist.begin(); it != clist.end(); it++){
		strokes.push_back(new Stroke(*it));
		cout<<"sss:"<< (*it)->size() <<endl;
	}
}

list<CurvePtr> StrokeManager::flushCurves(){
	list<CurvePtr> curves;
	for (list<StrokePtr>::iterator it = strokes.begin(); it != strokes.end(); it++){
		if ((*it)->curve())
			curves.push_back((*it)->curve());
	}
	for (list<StrokePtr>::iterator it = strokes.begin(); it != strokes.end(); it++)
		delete *it;
	strokes.clear();
	return curves;
}