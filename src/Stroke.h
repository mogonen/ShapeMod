#ifndef _H_Stroke
#define _H_Stroke

#include <GL/glut.h>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include "Vector.h"
//#include "Canvas.h"
#include "Curve.h"

using namespace std;

class StrokeManager{
	list<Vec3> _pts;
	Vec3 m_prev;
	bool _active;

public:
	static int CONTROL_POINTS;

	StrokeManager(){_active = 0;};
	void draw(){
		if (!_active)
			return;
		glColor3f(0.0f,0.0f,0.0f);
		//glEnable(GL_LINE_SMOOTH);
		glLineWidth(3.0);
			glBegin(GL_LINE_STRIP);
			for(list<Vec3>::const_iterator it = _pts.begin(); it!=_pts.end(); it++)
				glVertex3f((*it).x, (*it).y, (*it).z);
			glEnd();
		//glDisable(GL_LINE_SMOOTH);
	};


	/*
ArrCurve* finalize(int len = 0){
		if (!_active)
			return 0;
		_active = 0;
		if(_pts.size()<1)
			return 0;

		Vec3 * arr = new Vec3[_pts.size()];
		int i = 0;
		for(list<Vec3>::const_iterator it = _pts.begin(); it!=_pts.end(); it++)
			arr[i++] = (*it);
		_pts.clear();

		ArrCurve * ac = new ArrCurve(arr, i, true);
		if (( ac->getP(0) - ac->p1() ).Length()<0.05)
			ac->close();
		if (len)
			ac->resample(len);
		return ac;
	};	
	
	*/

	Curve* finalize(){
		_active = 0;
		if(_pts.size()<1)
			return 0;
		
		if (_pts.size()<10){
			Vec3 p = _pts.front();
			_pts.clear();
			return new PointCurve(p);
		}

		Vec3 * arr = new Vec3[_pts.size()];
		int i = 0;
		for(list<Vec3>::const_iterator it = _pts.begin(); it!=_pts.end(); it++)
			arr[i++] = (*it);
		ArrCurve * ac = new ArrCurve(arr, i, true);
		if (( ac->getP(0) - ac->last() ).norm()<0.05)
			ac->close();
		double len  = ac->length();
		_pts.clear();
		/*
		list<Vec3> clist = corners();
		cout<<"corners: "<<clist.size()<<endl;
		_pts.clear();
		Vec3 * arr = new Vec3[clist.size()];
		int i = 0;
		for(list<Vec3>::const_iterator it = clist.begin(); it!=clist.end(); it++)
			arr[i++] = (*it);
		return new BSpline(arr, clist.size(), 0, false);
		*/
		return new BSpline(ac->toArr(), ac->size(), ac->isClosed());
	};

	void start(){
		_active = 1;
	};

	void HandleMouseMotion(Vec3 p){
		if (_active)
			_pts.push_back(p);
	};

	void reset(){_active = 0;};

	list<Vec3> corners(){

		int i = 0;
		Vec3 * arr = new Vec3[_pts.size()];
		for(list<Vec3>::const_iterator it = _pts.begin(); it!=_pts.end(); it++)
			arr[i++] = (*it);
		ArrCurve * ac = new ArrCurve(arr, i, true);
		int sz = i;
		int straw = 2;
		double * len = new double[sz];
		double sum = 0;
		for(int j = 0; j < sz; j++){
			len[j] = ( ac->getP(j + straw) - ac->getP(j -straw) ).norm();
			sum+= len[j];
		}
		double med = 0;
		for(int j = 0; j < sz/2; j++){
			double min = 999999;
			for(int k = 0; k < sz; k++)
				if (len[k] < min && len[k] > med){
					min = len[k];
				}
			med = min;
			cout<<"med:"<<med<<endl;
		}

		double mean = sum / sz;
		list<Vec3> clist;
		for(int j = 0; j < sz; j++){
			if (len[j] < med*0.75 ){
				clist.push_back(ac->getP(j));
				cout<<j<<":"<<len[j]<<" < "<<(med*0.75) <<endl;
			}
		}
		return clist;
	}

};

#endif