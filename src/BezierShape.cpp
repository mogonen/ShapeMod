#include "BezierShape.h"

int BPatch::N;
int BPatch::Ni;
int BPatch::NN;
double BPatch::T;
bool BezierShape::MODE = false;
void BezierShape::draw(int m){
	updateBPatch();
	glLineWidth(3.0);
	glColor3f(0, 0, 0.25);
	if (m && Mesh::DRAW_MESH)
	for(list<Curve*>::iterator it =_curves.begin(); it!=_curves.end(); it++)
		(*it)->draw();
	
	glLineWidth(1.0);
	glColor3f(0.25, 0.25, 0.25);

	/*for(list<Patch*>::iterator it =_patches.begin(); it!=_patches.end(); it++)
		(*it)->draw(50, 50);*/

	for(map<Vertex*, VertexNormal*>::iterator it_vt = _vtmap.begin(); it_vt!=_vtmap.end(); it_vt++)
		it_vt->second->draw();

	for(list<BPatch*>::iterator it =_patches.begin(); it!=_patches.end(); it++)
		(*it)->draw();

	MeshShape::draw(m);
}


void BezierShape::exec(int cmd, void* param){

	BPatch::setN(10);
	MeshShape::exec(cmd, param);

	ControlPoint::remove(this);
	tpmap.clear();
	_patches.clear();
	_vtmap.clear();
	_curves.clear();

	if (cmd == Command::BUILD){
		buildControlMesh();
		buildCurves2();
		return;
	}

	if (cmd == Command::REBUILD){
		buildCurves2();
		return;
	}
}

Bezier* BezierShape::edgeBezier(Vec3* p0, Vec3* p1){

	Vec3 tan = (*p1) - (*p0);

	Vec3* p0_t = new Vec3(*p0 + tan*0.25);
	Vec3* p1_t = new Vec3(*p1 - tan*0.25);

	Bezier* c = new Bezier(3);
	_curves.push_back(c);

	c->insert(p0);
	c->insert(p0_t); 
	c->insert(p1_t); 
	c->insert(p1);

	map<Vec3*, TangentPoint*>::iterator ittp = tpmap.find(p0);
	if ( ittp == tpmap.end()){
		TangentPoint* tp = TangentPoint::create(this, p0);
		TangentPoint::create(this, p0_t, tp);
		tpmap[p0] = tp;
	}else
		TangentPoint::create(this, p0_t, tpmap[p0]);

	ittp = tpmap.find(p1);
	if ( ittp == tpmap.end()){
		TangentPoint* tp = TangentPoint::create(this, p1);
		TangentPoint::create(this, p1_t, tp);
		tpmap[p1] = tp;
	}else
		TangentPoint::create(this, p1_t, tpmap[p1]);
		
	return c;
}

void BezierShape::buildCurves2(){

	list<CEdge*> edges = _control->edges();
	list<Face*> faces = _control->faces();
	int vs = _control->size();
	int es = vs*vs;
	Bezier** bzmap = new Bezier*[es];

	for(int i=0; i<es; i++ )
		bzmap[i] = 0x00;

	for(list<Face*>::iterator it =  faces.begin(); it!=faces.end(); it++){
		CFace* f = (CFace*)*it;
		f->update();
		if (f->size!=4)
			continue;
		Curve** bs = new Curve*[4];
		Corner* c0 = f->c0;
		BPatch* patch = new BPatch();
		_patches.push_back(patch);

		for(int i = 0; i<4; i++){

			bs[i] = bzmap[f->edgeUid(i)];
			int i0 = i;
			int i1 = i+1;
			if (i>1){
				i0 = i+1;
				i1 = i;
			}

			if (c0->e->c1)
				patch->setSeam(i);

			if (!bs[i])
				bs[i] = (bzmap[f->edgeUid(i)] = edgeBezier(f->v(i0)->p0, f->v(i1)->p0));

			map<Vertex*, VertexNormal*>::iterator it_vn = _vtmap.find(f->v(i));
			VertexNormal* vn = 0;

			if (it_vn != _vtmap.end()){
				vn = it_vn->second;
			}else{
				vn = new VertexNormal(f->v(i)->isBorder());
				_vtmap[f->v(i)] = vn;
			}

			if (c0->vNext())
				vn->v0 = patch->preCornerPtr(i);
			else
				vn->v1 = patch->postCornerPtr(i);

			vn->nlist.push_back(patch->cornerNormalPtr(i));
			if (vn->quad)
				vn->quad[i] = patch->cornerQuadDiagonalPtr(i);

			c0 = c0->next;
		}
		patch->setCurves(bs);
	}

}

void BezierShape::buildCurves(){

	buildControlMesh();
	list<CEdge*> edges = _control->edges();
	
	for(list<CEdge*>::iterator it =  edges.begin(); it!=edges.end(); it++){

		CEdge* e = (*it);

		/*if (e->c1)
			continue;*/

		//Create Curve for each Edge
		Vec3* p0 = e->P0();
		Vec3* p1 = e->P1();

		Vec3 tan = (*p1) - (*p0);

		Vec3* p0_t = new Vec3(*p0 + tan*0.25);
		Vec3* p1_t = new Vec3(*p1 - tan*0.25);

		Curve* c = new Bezier(3);
		_curves.push_back(c);

		c->insert(p0);
		c->insert(p0_t); 
		c->insert(p1_t); 
		c->insert(p1);

		map<Vec3*, TangentPoint*>::iterator ittp = tpmap.find(p0);
		if ( ittp == tpmap.end()){
			TangentPoint* tp = TangentPoint::create(this, p0);
			TangentPoint::create(this, p0_t, tp);
			tpmap[p0] = tp;
		}else
			TangentPoint::create(this, p0_t, tpmap[p0]);

		ittp = tpmap.find(p1);
		if ( ittp == tpmap.end()){
			TangentPoint* tp = TangentPoint::create(this, p1);
			TangentPoint::create(this, p1_t, tp);
			tpmap[p1] = tp;
		}else
			TangentPoint::create(this, p1_t, tpmap[p1]);
	}
}

void BezierShape::updateBPatch(){

	for(list<BPatch*>::iterator it =_patches.begin(); it!=_patches.end(); it++){
		(*it)->writeCurves();
		(*it)->initCornerNormals();
	}

	for(map<Vertex*, VertexNormal*>::iterator it_vt = _vtmap.begin(); it_vt!=_vtmap.end(); it_vt++)
		it_vt->second->update();
	
}

void VertexNormal::update(){
	
	if (nlist.size()<2)
		return;

	Vec3 n;

	if (isborder)
		n= -((*v1) - (*v0)).normalize() % Vec3(0,0,1);
	else //this is an innner, valance should be 4
		n = quadNormal(*quad[0],*quad[1], *quad[2],*quad[3]);

	/*for(list<Vec3*>::iterator it = nlist.begin(); it!=nlist.end(); it++)
		n= n+ *(*it);
	n = (n/nlist.size()).normalize();*/

	for(list<Vec3*>::iterator it = nlist.begin(); it!=nlist.end(); it++)
		(*it)->set(n);
}

void VertexNormal::draw(){
if (v0 && v1){
	glBegin(GL_LINES);
	glVertex3f(v0->x, v0->y, v0->z);
	glVertex3f(v1->x, v1->y, v1->z);
	glEnd();
}
}

Vec3 BPatch::interpolate(int i, int j, Vec3* d){
	double t = (i*1.0 / Ni);
	double s = (j*1.0 / Ni);
	Vec3 p0 = d[ind(i, 0)]*(1.0-s) + d[ind(i, Ni)]*s;
	p0 = p0 + d[ind(0, j)]*(1.0 - t) + d[ind(Ni, j)]*t;
	p0 = p0 - d[ind(0,0)]*(1-s)*(1.0-t) - d[ind(Ni, 0)]*(1-s)*t - d[ind(Ni, Ni)]*s*t - d[ind(0, Ni)]*s*(1-t);
	return p0;
}

void BPatch::update(bool nz){

	for(int j=1; j<Ni;j++)
		for(int i=1; i<Ni; i++){
			 Vec3 p = interpolate(i, j, ps);
			 Vec3 n = interpolate(i, j, ns);

			//double l = n.norm();
			if (nz)
				 n.z = sqrt(1 - n.x*n.x - n.y*n.y);
			 
			ps[ind(i,j)] = p;
			ns[ind(i,j)] = n;
		}

}

void BPatch::writeCurves(){

	for(int i=0; i<N; i++)
		ps[ind(i, 0)] = fs[0]->getP(i*T);

	for(int i=0; i<N; i++)
		ps[ind(Ni, i)] = fs[1]->getP(i*T); 

	for(int i=0; i<N; i++)
		ps[ind(i, Ni)] = fs[2]->getP(i*T); 

	for(int i=0; i<N; i++)
		ps[ind(0, i)] = fs[3]->getP(i*T);
}

void BPatch::initCornerNormals(){
	
	ns[ind(0, 0)] = -(p(1, 0) - p(0,1)).normalize() % Vec3(0,0,1);
	ns[ind(Ni, 0)] = -(p(Ni, 1) - p(Ni-1,0)).normalize() % Vec3(0,0,1);
	ns[ind(Ni, Ni)] = -(p(Ni-1, Ni) - p(Ni, Ni-1)).normalize() % Vec3(0,0,1);
	ns[ind(0, Ni)] = -(p(0, Ni-1) - p(1, Ni)).normalize() % Vec3(0,0,1);
}

void BPatch::computeNormals(){

	for(int i=1; i < Ni; i++)
		ns[ind(i, 0)] = (n(0,0)*(1-i*T) + n(Ni,0)*(i*T));

	for(int i=1; i < Ni; i++)
			ns[ind(Ni, i)] = (n(Ni,0)*(1-i*T) + n(Ni,Ni)*(i*T));

	for(int i=1; i < Ni; i++)
			ns[ind(i, Ni)] = (n(0,Ni)*(1-i*T) + n(Ni,Ni)*(i*T));

	for(int i=1; i < Ni; i++)
			ns[ind(0, i)] = (n(0,0)*(1-i*T) + n(0,Ni)*(i*T));
}

void BPatch::computeNormals2(){

	if (isSeam(0))
		for(int i=1; i < Ni; i++)
			ns[ind(i, 0)] = (n(0,0)*(1-i*T) + n(Ni,0)*(i*T));
	else 
		for(int i=1; i < Ni; i++)
			ns[ind(i, 0)] = -(p(i+1,0) - p(i-1,0)).normalize() % Vec3(0,0,1);
	
	if (isSeam(1)){
		for(int i=1; i < Ni; i++)
			ns[ind(Ni, i)] = (n(Ni,0)*(1-i*T) + n(Ni,Ni)*(i*T));
	}else
		for(int i=1; i<Ni; i++)
			ns[ind(Ni, i)] = -(p(Ni, i+1) - p(Ni, i-1)).normalize() % Vec3(0,0,1);

	if (isSeam(2)){
		for(int i=1; i < Ni; i++)
			ns[ind(i, Ni)] = (n(0,Ni)*(1-i*T) + n(Ni,Ni)*(i*T));
	}else
		for(int i=1; i<Ni; i++)
			ns[ind(i, Ni)] = -(p(i-1, Ni) - p(i+1, Ni)).normalize() % Vec3(0,0,1);

	if (isSeam(3)){
		for(int i=1; i < Ni; i++)
			ns[ind(0, i)] = (n(0,0)*(1-i*T) + n(0,Ni)*(i*T));
	}else
		for(int i=1; i<Ni; i++)
			ns[ind(0, i)] = -(p(0, i-1) - p(0, i+1)).normalize() % Vec3(0,0,1);
}

Vec3* BPatch::cornerNormalPtr(int i){
	if (i==0)
		return &ns[ind(0,0)];
	else if (i==1)
		return &ns[ind(Ni,0)];
	else if (i==2)
		return &ns[ind(Ni,Ni)];
	else if (i==3)
		return &ns[ind(0,Ni)];
	return 0;
}

Vec3* BPatch::preCornerPtr(int i){
	if (i==0)
		return &ps[ind(0,1)];
	else if (i==1)
		return &ps[ind(Ni-1,0)];
	else if (i==2)
		return &ps[ind(Ni,Ni-1)];
	else if (i==3)
		return &ps[ind(1,Ni)];
	return 0;
}

Vec3* BPatch::postCornerPtr(int i){
	if (i==0)
		return &ps[ind(1,0)];
	else if (i==1)
		return &ps[ind(Ni,1)];
	else if (i==2)
		return &ps[ind(Ni-1,Ni)];
	else if (i==3)
		return &ps[ind(0,Ni-1)];
	return 0;
}

Vec3* BPatch::cornerQuadDiagonalPtr(int i){
	if (i==0)
		return &ps[ind(1, 1)];
	else if (i==1)
		return &ps[ind(Ni-1, 1)];
	else if (i==2)
		return &ps[ind(Ni-1, Ni-1)];
	else if (i==3)
		return &ps[ind(1, Ni-1)];
	return 0;
}

void BPatch::draw(){

	if (BezierShape::MODE)
		computeNormals2();
	else
		computeNormals();
	update();

	for(int j=0; j < Ni; j++){
		for(int i = 0; i< Ni; i++){
	
			Vec3 p[4];
			p[0] = ps[ind(i, j)];
			p[1] = ps[ind(i+1, j)];
			p[2] = ps[ind(i+1, j+1)];
			p[3] = ps[ind(i, j+1)];
		
			Vec3 n[4];
			n[0] = ns[ind(i, j)];
			n[1] = ns[ind(i+1, j)];
			n[2] = ns[ind(i+1, j+1)];
			n[3] = ns[ind(i, j+1)];
	
			if (Mesh::DRAW_WIREFRAME){
				glColor3f(0,0,0);
				glLineWidth(1.0);
				glBegin(GL_LINE_LOOP);
				for(int k=0; k<4; k++)
					glVertex3f(p[k].x, p[k].y, p[k].z);
				glEnd();
			}

			glBegin(GL_POLYGON);
			for(int k=0; k<4; k++){
				glColor3f((n[k].x+1)/2, (n[k].y+1)/2, n[k].z );
				glVertex3f(p[k].x, p[k].y, p[k].z);
			}
			glEnd();
		}
	}
}

/*
		//Curve f0
		Bezier* f0 = bzmap[f->edgeUid(0)];
		if (!f0)	
			bzmap[f->edgeUid(0)] = (f0 = edgeBezier(f->v(0)->p0, f->v(1)->p0));

		Bezier* f1 = bzmap[f->edgeUid(2)];
		if (!f1)	
			bzmap[f->edgeUid(2)] = (f1 = edgeBezier(f->v(2)->p0, f->v(3)->p0));

		Bezier* g0 = bzmap[f->edgeUid(3)];
		if (!g0)	
			bzmap[f->edgeUid(3)] = (g0 = edgeBezier(f->v(1)->p0, f->v(2)->p0));

		Bezier* g1 = bzmap[f->edgeUid(1)];
		if (!g1)	
		bzmap[f->edgeUid(1)] = (g1 = edgeBezier(f->v(3)->p0, f->v(0)->p0));

seams

if (isSeam(0)){
		ns[ind(0, 0)] = -(p(0,0) - p(0,1)).normalize() % Vec3(0,0,1);
		ns[ind(Ni, 0)] = -(p(Ni, 1) - p(Ni,0)).normalize() % Vec3(0,0,1);
		for(int i=1; i < Ni; i++)
			ns[ind(i, 0)] = (n(0,0)*(1-i*T) + n(Ni,0)*(i*T));
	}else

if (isSeam(1)){
		ns[ind(Ni, 0)]  = -(p(Ni, 0) - p(Ni-1,0)).normalize() % Vec3(0,0,1);
		ns[ind(Ni, Ni)] = -(p(Ni-1, Ni) - p(Ni, Ni)).normalize() % Vec3(0,0,1);
		for(int i=1; i < Ni; i++)
			ns[ind(Ni, i)] = (n(Ni,0)*(1-i*T) + n(Ni,Ni)*(i*T));
	}else

if (isSeam(2)){
		ns[ind(Ni, Ni)] = -(p(Ni, Ni) - p(Ni, Ni-1)).normalize() % Vec3(0,0,1);
		ns[ind(0, Ni)] = -(p(0, Ni-1) - p(0, Ni)).normalize() % Vec3(0,0,1);
		for(int i=1; i < Ni; i++)
			ns[ind(i, Ni)] = (n(0,Ni)*(1-i*T) + n(Ni,Ni)*(i*T));
	}else

	if (isSeam(3)){
		ns[ind(0, Ni)] = -(p(0, Ni) - p(1, Ni-1)).normalize() % Vec3(0,0,1);
		ns[ind(0, 0)] = -(p(1, 0) - p(0, 0)).normalize() % Vec3(0,0,1);

		for(int i=1; i < Ni; i++)
			ns[ind(0, i)] = (n(0,0)*(1-i*T) + n(0,Ni)*(i*T));
	}else



	if (isSeam(0))
		for(int i=1; i < Ni; i++)
			ns[ind(i, 0)] = (n(0,0)*(1-i*T) + n(Ni,0)*(i*T));
	else 
		for(int i=1; i < Ni; i++)
			ns[ind(i, 0)] = -(p(i+1,0) - p(i-1,0)).normalize() % Vec3(0,0,1);
	
	if (isSeam(1)){
		for(int i=1; i < Ni; i++)
			ns[ind(Ni, i)] = (n(Ni,0)*(1-i*T) + n(Ni,Ni)*(i*T));
	}else
		for(int i=1; i<Ni; i++)
			ns[ind(Ni, i)] = -(p(Ni, i+1) - p(Ni, i-1)).normalize() % Vec3(0,0,1);

	if (isSeam(2)){
		for(int i=1; i < Ni; i++)
			ns[ind(i, Ni)] = (n(0,Ni)*(1-i*T) + n(Ni,Ni)*(i*T));
	}else
		for(int i=1; i<Ni; i++)
			ns[ind(i, Ni)] = -(p(i-1, Ni) - p(i+1, Ni)).normalize() % Vec3(0,0,1);

	if (isSeam(3)){
		for(int i=1; i < Ni; i++)
			ns[ind(0, i)] = (n(0,0)*(1-i*T) + n(0,Ni)*(i*T));
	}else
		for(int i=1; i<Ni; i++)
			ns[ind(0, i)] = -(p(0, i-1) - p(0, i+1)).normalize() % Vec3(0,0,1);


map<Vertex*, VertexNormal*>::iterator it_vn = _vtmap.find(f->v(i));
			VertexNormal* vn = 0;

			if (it_vn != _vtmap.end())
				vn = it_vn->second;
			else{
				vn = new VertexNormal(f->v(i)->isBorder());
				_vtmap[f->v(i)] = vn;
			}

			if (c0->vNext())
				vn->v0 = patch->preCornerPtr(i);
			else
				vn->v1 = patch->postCornerPtr(i);

			vn->nlist.push_back(patch->cornerNormalPtr(i));
			vn->cqlist.push_back(patch->cornerQuadDiagonalPtr(i));

*/