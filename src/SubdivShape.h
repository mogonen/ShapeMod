#ifndef __SUBDIVSHAPE_H__	
#define __SUBDIVSHAPE_H__

#include "MeshShape.h"

class SubdivShape:public MeshShape{
	Mesh * _subdiv;
	void colorSubdivMesh();

	void subdivide(bool newsubdiv = false);

	void colorize(Mesh*, BitMap *bmap, int);
	RGBA* pickFromQuad(const Vec3& p0, const Vec3& u, const Vec3& v, int, int, BitMap *bmap);
	int _subdivs;

public:
	SubdivShape():MeshShape(){
		_subdiv = 0;
		_subdivs = 0;
	}

	void onShapeChanged(int type=0);
	void exec(int cmd, void* param);
	void draw(int m=0);

	void subdivCC(Mesh * m);
	static int SUBDIVS;
};

#endif

/*


	Vertex** getRingVerts(const Vec3&, const Vec3&, Mesh*, int, double, bool c = true);
	void stichRings(Mesh*, Vertex**, Vertex**, int seg);
public:
	void make3DMesh();


	
Vertex** MeshShape::getRingVerts(const Vec3& o, const Vec3& tan, Mesh*m, int seg, double rad, bool control){
	Vertex**v = new Vertex*[seg];
	Vec3 nx(0,0,1);
	Vec3 ny = -( nx % tan).normalize();
	for(int i = 0; i < seg; i++){
		double a = 2 * PI * i / seg;
		Vec3 * p = new Vec3( o + nx*rad*cos(a) + ny*rad*sin(a) );
		v[i] = m->addVertex(p);
	}
	if (control){
		ControlPoint::create(this, p0);
		ControlPoint::create(this, p1);
	}
	return v;
}

void MeshShape::stichRings(Mesh* m, Vertex** v0s, Vertex** v1s, int seg){
	for(int i=0; i < seg / 2 + 1 ; i++ )
		m->addQuad(v0s[i], v0s[i+1], v1s[seg-2-i], v1s[seg-1-i]);
}

void MeshShape::make3DMesh(){

	int seg = 8;
	int osz = _control->size() * _control->size();
	OutlineMap omap = new Vertex**[osz];
	for(int i = 0; i < osz; i++)
		omap[i] = 0x00;

	Mesh* omesh = new Mesh();
	list<Vertex*> verts = _control->verts();

	for(list<Vertex*>::iterator it = verts.begin(); it!= verts.end();it++){
		//three type verts: Joint (val:3) / Bridge (val:2) / End (val:1)
		Vertex* v = (*it);	 
		if (v->val()>2){ // a joint
			Vertex** branches = v->sortTo(0);
			int sz = v->to.size();

			Face* fmid0 = new Face(sz);
			Face* fmid1 = new Face(sz);
			omesh->addFace(fmid0);
			omesh->addFace(fmid1);

			Vertex*** verts = new Vertex**[seg];
			for(int i = 0; i < sz; i++){
				Vec3 n = (branches[i]->P() - v->P()).normalize();
				Vec3 p = v->P() + n*RAD*2;
				verts[i] = getRingVerts(p, n, omesh, seg, RAD);
			}

			//now update omesh
			for(int i = 0; i< sz; i++ ){

				Vertex** v0s = verts[i];
				Vertex** v1s = verts[(i+1)%sz];
				for(int j=0; j<seg/2; j++){
					omesh->addQuad(v0s[j%seg], v1s[(2*seg-j)%seg], v1s[(2*seg-1-j)%seg], v0s[(j+1)%seg]);
				}
				fmid0->verts[i] = v0s[0];
				fmid1->verts[i] = v0s[seg/2];

				if (v1s = omap[branches[i]->edgeId(v)]){
					for(int j=0; j < seg; j++ )
						omesh->addQuad(v0s[j%seg], v1s[(2*seg-j)%seg], v1s[(2*seg-1-j)%seg], v0s[(j+1)%seg]);
						//omesh->addQuad(v0s[j%seg], v0s[(j+1)%seg], v1s[(2*seg-2-j)%seg], v1s[(2*seg-1-j)%seg]);
				}else{
					omap[v->edgeId(branches[i])] = v0s;
				}
			}
			fmid0->update();
			fmid1->update();
		}else{ //a bridge or an end vertex
			Vertex* v0 = ( v->val() == 2 )? v->to.back() : v;
			Vertex* v1 = v->to.front();
			Vec3 tan = (v1->P() - v0->P()).normalize();
			Vertex** vs = this->getRingVerts(v->P(), tan, omesh, seg, RAD);
			int e = 0;
			if (Vertex** v1s = omap[v1->edgeId(v)]){
				for(int j=0; j < seg; j++ )
					omesh->addQuad(vs[j%seg], v1s[(2*seg-j)%seg], v1s[(2*seg-1-j)%seg], vs[(j+1)%seg]);
				//omesh->addQuad(vs[0], vs1[1], vs1[0], vs[1]);
				
			}else{
				omap[v->edgeId(v1)] = vs;
			}
			if (v0!=v){ //inverse order in neg tangent direction

				if (Vertex** v1s = omap[v0->edgeId(v)]){
					for(int j = 0; j < seg; j++ )
						omesh->addQuad(vs[j%seg], v1s[j%seg], v1s[(j+1)%seg], vs[(j+1)%seg]);
					//omesh->addQuad(vs[1], vs1[1], vs1[0], vs[0]);
				}else{
					//Vertex* vs_i[2] = {vs[1], vs[0]}; // this causes error! figure it out!!!!
					Vertex** vs_i = new Vertex*[seg]; 
					for(int j=0; j < seg; j++ )
						vs_i[j] = vs[(2*seg-j)%seg];
					omap[v->edgeId(v0)] = vs_i;
				}
			}
		}
	}

	delete _control;
	_control = new CMesh(omesh);
}


*/