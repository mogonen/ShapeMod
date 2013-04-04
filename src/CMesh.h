#ifndef __MESHDLFL_H__	
#define __MESHDLFL_H__
#include "Mesh.h"

struct Corner;
class CEdge;
class CFace;
class CMesh;

struct Corner{
	
	Vertex* v;
	CFace* f;
	CEdge* e;

	Corner *prev, *next;

	Corner(Vertex* lv, CFace* lf){
		v = lv; f = lf;
		prev = next = 0;
	};

	void setNext(Corner* c){
		if (c){
			next = c;
			c->prev = this;
		}
	};

	Corner* vNext();
	Corner* vPrev();
};

class CFace:public Face{

public:
	CFace():Face(){};

	Corner* c0;
	void update(){
		size = 1;
		_p0.set();
		_c0.set();
		for(Corner* c = c0->next; c != c0; c = c->next)
			size++;
		int i = 0;
		delete verts;
		verts = new Vertex*[size];
		Corner* c = c0;
		Corner* cp = c->prev;
		for(int i=0; i<size; i++){
			verts[i] = c->v;
			_p0 = _p0 + verts[i]->P();
			_c0 = _c0 + verts[i]->C();
			cp = c;
			c = c->next;
		}
		_p0 = _p0 / size;
		_c0 = _c0 / size;
	};
};

class CEdge{

	CEdge(Corner* lc0=0, Corner* lc1=0){
		c0 = c1 = 0;
		if (lc0){
			c0 = lc0;
			c0->e = this;
		}

		if (lc1){
			c1 = lc1;
			c1->e = this;
		}
	};

public:

	Corner *c0, *c1;
	Corner* other(Corner* c){return (c==c0)?c1:c0;}

	Corner* split(double t=0.5, Corner* c=0);

	bool pick(const Vec3& p, double rad);

	void set(Corner*c, int i=0){
		if (i)
			c1 = c;
		else
			c0 = c;
		c->e = this;
	}

	Vec3* P0(){return c0->v->p0;};
	Vec3* P1(){return c0->next->v->p0;};

	static CEdge * insert(Corner* ic0, Corner* ic1);
	static CEdge * create(CMesh* m);

};

class CMesh:public Mesh{

	list<CEdge*> _edges;
public:

	CMesh():Mesh(){};
	CMesh(Mesh*m);
	list<CEdge*> edges(){return _edges;};
	int sizeEdge(){return _edges.size();};
	void add(CEdge * e){_edges.push_back(e);}//cout<<"edges:"<<_edges.size()<<":"<<e<<endl;}

	CEdge* pickEdge(const Vec3&);

	void draw(int m=0);
	//topological operations
	//void insertIsopam(CEdge*, double t=0.5);
};


#endif