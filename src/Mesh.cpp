#include <iostream> 
#include <fstream>
#include <map>
#include "Mesh.h"
#include "Eye.h"

bool Mesh::DRAW_MESH = true;
bool Mesh::DRAW_WIREFRAME = false;
int Mesh::DRAWING_MODE = Mesh::NORMALMAP;


Mesh::~Mesh(){
	for(list<Vertex*>::iterator it = _verts.begin(); it != _verts.end(); it++)
		delete *it;

	for(list<FacePtr>::iterator itf = _faces.begin(); itf!=_faces.end(); itf++)
		delete *itf;
}

void Mesh::addFace(FacePtr f){
	_faces.push_back(f);
}

FacePtr Mesh::addQuad(Vertex* v0, Vertex* v1, Vertex* v2, Vertex* v3){
	FacePtr f = new Face(4);
	_faces.push_back(f);
	f->verts[0] = v0;
	f->verts[1] = v1;
	f->verts[2] = v2;
	f->verts[3] = v3;
	f->update();
	return f;
}

FacePtr Mesh::addTriangle(Vertex* v0, Vertex* v1, Vertex* v2){
	FacePtr f = new Face(3);
	_faces.push_back(f);
	f->verts[0] = v0;
	f->verts[1] = v1;
	f->verts[2] = v2;
	f->update();
	return f;
}

//not finished yet!
Face* Mesh::splitFace(Face* f, int i0, int i1 = -1){

	if (i1==-1)
		i1 = (i0 + f->size/2)%f->size;

	if (i0>i1){
		int t = i0;
		i0 = i1;
		i1 = t;
	}

	Vec3 mid0 = f->v(i0)->P()*0.5 + f->v(i0+1)->P()*0.5;
	Vec3 mid1 = f->v(i1)->P()*0.5 + f->v(i1+1)->P()*0.5;

	Vertex* v0 = addVertex(mid0);
	Vertex* v1 = addVertex(mid1);

	int s1 = ((i1 - i0)+f->size)%f->size + 2;
	Face* f1 = new Face(s1);
	f1->verts[0] = v0;
	f1->verts[s1-1] = v1;

	int s0 = (i0 + f->size - i1)%f->size + 2;
	Vertex** verts = new Vertex*[s0];
	verts[0] = f->verts[0];
	verts[s0-1] = v0;

	return f1;
}

Vertex* Mesh::addVertex(Vec3* p){
	Vertex* v = Vertex::create(this);
	v->setP(p);
	return v;
}

Vertex* Mesh::addVertex(const Vec3& p){
	Vertex* v = Vertex::create(this);
	v->setP(p);
	return v;
}

int Mesh::addVertex(Vertex* v){
	_verts.push_back(v);
	return _verts.size()-1;
}

//warning faces are not updated!!!
void Mesh::removeVertex(Vertex* v){
	if (!v || v->mesh() != this)
		return;

	if (v->prev && v->next) //update border
		v->prev->setNext(v->next);

	_verts.remove(v);
	if (v->val()){
		for(list<Vertex*>::const_iterator it = v->to.cbegin(); it!=v->to.cend(); it++)
			(*it)->to.remove(v);
	}
	delete v;
	//_voff--;
}

void Mesh::drawLinks(){

	glPointSize(6.0);
	glBegin(GL_POINTS);
		for(list<Vertex*>::iterator it = _verts.begin(); it != _verts.end(); it++){
			if (!(*it)->p0)
				continue;
			Vec3 p0 = (*it)->P();
			glVertex3f(p0.x, p0.y, p0.z);
		}

	glEnd();

	glBegin(GL_LINES);
		for(list<Vertex*>::iterator it = _verts.begin(); it != _verts.end(); it++){
			if (!(*it)->p0)
				continue;

			Vec3 p0 = (*it)->P();
			for(list<Vertex*>::iterator itv = (*it)->to.begin(); itv != (*it)->to.end(); itv++){
				Vec3 p1 = (*itv)->P();
				glVertex3f(p0.x, p0.y, p0.z);
				glVertex3f(p1.x, p1.y, p1.z);
			}
		}
	glEnd();
}

void Mesh::draw(int mode){

	if (!DRAW_MESH && !mode)
		return;

	if (!_faces.size()){ // this should be just a graph 
		drawLinks();
		return;
	}

	for(list<FacePtr>::iterator itf = _faces.begin(); itf!=_faces.end(); itf++){

		FacePtr f = (*itf);

		glColor3f(0, 0, 0);

		if (DRAW_WIREFRAME || (mode & WIRE_OVERRIDE) ){
			glLineWidth(1.0);
			glColor3f(0.0f,0.0f,0.0f);
			glBegin(GL_LINE_LOOP);
			for(int i = 0; i < f->size; i++){
				Vec3 p = f->verts[i]->P();
				glVertex3f(p.x, p.y, p.z);
			}
			glEnd();
			//draw normal
			glBegin(GL_LINES);
				Vec3 p0 = f->P();
				Vec3 p1 = p0 + f->N()*0.1;
				glVertex3f(p0.x, p0.y, p0.z);
				glVertex3f(p1.x, p1.y, p1.z);
			glEnd();
			/*glPointSize(.0); //draw vertices
			glBegin(GL_POINTS);
			for(int i=0; i<f->size; i++){
				Vec3 p = f->verts[i]->P();
				glVertex3f(p.x, p.y, p.z);
			}
			glEnd();//*/
		}

		if (DRAWING_MODE == SHADED || mode & FLATSHADE_OVERRIDE)
			glEnable(GL_LIGHTING);
		if (!(mode ^ WIRE_OVERRIDE)) // if only set to be WIRE
			continue;

		glColor3f(0.5, 0.2, 0.2);
		glBegin(GL_POLYGON);
		for(int i=0; i<f->size;i++){

			Vec3 p = f->verts[i]->P();
			Vec4 c = (mode & FACECOLOR_OVERRIDE)? f->C() : f->verts[i]->C();

			if (DRAWING_MODE == NORMALMAP)
				glColor4f((c.x+1)/2.0 , (c.y+1)/2.0  , c.z, c.w ); //glColor4f( c.x , c.y  , c.z, c.w );
			else if  (DRAWING_MODE == ALPHAMAP)
				glColor4f(0, 0, 1 , c.z*c.w );
			else if (DRAWING_MODE == SOLID)
				glColor4f(1.0, 1.0, 1.0, 1.0);

			glVertex3f(p.x, p.y, p.z);
		}
		glEnd();
		if (DRAWING_MODE == SHADED)
			glEnable(GL_LIGHTING);
	}
}

Vertex* * Vertex::sortTo(Vertex* v0){//sorts linked verticies with respet to v0

	int sz = to.size();
	if (!v0)
		v0 = to.front();
	Vertex* * vs = new Vertex*[sz];
	Vec3 x = (v0->P() - P()).normalize();
	Vec3 y = (x%Vec3(0,0,1)).normalize();
	vs[0] = v0;
	int k = 1;
	for(list<Vertex*>::iterator it = to.begin(); it != to.end(); it++)
		if (*it != v0 )
			vs[k++] = (*it);

	double * deg = new double[sz];
	for(int i = 0; i<sz; i++ ){
		Vec3 n((vs[i]->P() - P()).normalize());
		deg[i] = (n * x);
		if (n*y<0)
			deg[i] = -2 - deg[i];
	}

	//now sort:
	for(int i = 1; i<sz; i++)
		for(int j=i+1; j<sz; j++)
			if (deg[i] < deg[j]){
				//swap
				double d = deg[j];
				deg[j] = deg[i];
				deg[i] = d;
				Vertex* tmp = vs[j];
				vs[j] = vs[i];
				vs[i] = tmp;
			}
			
	return vs;
}

list<Edge*> Edge::createEdges(Mesh* m){
	list<Edge*> edges;
	list<Face*> faces = m->faces();
	unsigned int DONE = 0x10;
	int vs = m->size();
	int es = vs*vs*2;
	Edge** ematrix = new Edge*[es];
	for(list<Face*>::const_iterator it = faces.cbegin(); it != faces.cend(); it++){
		Face* f = *it;
		for(int i = 0; i < f->size; i++ ){
			int euid = f->edgeUid(i);
			if ((unsigned int)ematrix[euid] == DONE){
				ematrix[euid+1]->f1 = f;
			} else {
				Edge* e = new Edge(f->v(i), f->v(i+1), f);
				edges.push_back(e);
				ematrix[euid] = (Edge*)DONE;
				ematrix[euid+1] = e;
			}
		}
	}
	delete ematrix;
	return edges;
}

Edge** Edge::createEdgeMatrix(Mesh* m, bool uns){
	list<Face*> faces = m->faces();
	int vs = m->size();
	int es = vs*vs;
	Edge** ematrix = new Edge*[es];
	for(int i=0; i < es; i++)
		ematrix[i] = 0x00;
	for(list<Face*>::const_iterator it = faces.cbegin(); it != faces.cend(); it++){
		Face* f = *it;
		for(int i = 0; i < f->size; i++ ){
			int eid = (uns)? f->edgeUid(i) : f->edgeId(i);
			if (ematrix[eid]){
				ematrix[eid]->f1 = f;
			} else {
				Edge* e = new Edge(f->v(i), f->v(i+1), f);
				ematrix[eid] = e;
			}
		}
	}
	return ematrix;
}

bool Edge::pick(const Vec3& p, double rad){
	Vec3 p0 = v0->P();
	Vec3 p1 = v1->P();
	Vec3 p0p1 = (p1 - p0);
	double p0p1_len = p0p1.norm();
	p0p1 = p0p1 / p0p1_len;
	double pp_len = (p-p0)*p0p1;
	if (pp_len>0 && pp_len < p0p1_len){
		double d = (p - (p0 + p0p1*pp_len) ).norm();
		if (d < rad)
			return true;	
	}
	return false;
}

void Mesh::initNeighbors(){
	unsigned int DONE = 0x10;
	int vs = _verts.size();
	int es = vs*vs;
	Neighbor** ematrix = new Neighbor*[es];
	for(int i=0; i<es; i++)
		ematrix[i] = 0;
	for(list<Face*>::const_iterator it = _faces.cbegin(); it != _faces.cend(); it++){
		Face* f = *it;
		f->initNeighbors();
		for(int i = 0; i < f->size; i++ ){
			int euid = f->edgeUid(i);
			Neighbor* nb = new Neighbor(f, i);
			if (ematrix[euid]){
				Neighbor* nb1 = ematrix[euid];
				f->neighbors[i] = ematrix[euid];
				nb1->f->neighbors[nb1->vi] = nb;
			}else 
				ematrix[euid] = nb;
		}
	}
	delete ematrix;
}

/*
Vertex* * Vertex::sortTo(Vertex* v0){//sorts linked verticies with respet to v0

	Vertex* * vs = new Vertex*[to.size()];
	int i=0;
	for(list<Vertex*>::iterator it = to.begin(); it != to.end(); it++){
			vs[i++] = (*it);
	}
	return vs;
}
*

/*
WEdgePtr * Vertex::sortWEdges(WEdgePtr e0){

	WEdgePtr * es = new WEdgePtr[WEdges.size()-1];
	Vec3 x = (e0->other(this)->P() - P()).normalize();
	Vec3 y = (x%Vec3(0,0,1)).normalize();
	int sz = 0; 
	for(list<WEdgePtr>::iterator it = WEdges.begin(); it != WEdges.end(); it++){
		if (*it != e0)
			es[sz++] = (*it);
	}
	
	double * deg = new double[sz];
	for(int i = 0; i<sz; i++ ){
		Vec3 n((es[i]->other(this)->P() - P()).normalize());
		deg[i] = (n * x);
		if (n*y<0)
			deg[i] = -2 - deg[i];
	}

	//now sort:
	for(int i = 0; i<sz; i++)
		for(int j=i+1; j<sz; j++)
			if (deg[i] < deg[j]){
				//swap
				double d = deg[j];
				deg[j] = deg[i];
				deg[i] = d;
				WEdgePtr e = es[j];
				es[j] = es[i];
				es[i] = e;
			}
			
	return es;
}
*/

void Mesh::updateFaces(){
		for(list<FacePtr>::iterator itf = _faces.begin(); itf!=_faces.end(); itf++)
			(*itf)->update();
}

Mesh* Mesh::deepCopy(){
	Mesh * m = new Mesh();
	int vsize = _verts.size();
	Vertex** verts = new Vertex*[vsize];
	for(list<Vertex*>::iterator itv = _verts.begin(); itv != _verts.end(); itv++){
		verts[(*itv)->id()] = m->addVertex((*itv)->P());
		verts[(*itv)->id()]->setC((*itv)->C());
	}

	for(list<Vertex*>::iterator itv = _verts.begin(); itv != _verts.end(); itv++){

		if ((*itv)->next)
			verts[(*itv)->id()]->next = verts[(*itv)->next->id()];

		if ((*itv)->prev)
			verts[(*itv)->id()]->prev = verts[(*itv)->prev->id()];

		if ((*itv)->to.size()){
			for(list<Vertex*>::iterator itvv = (*itv)->to.begin(); itvv != (*itv)->to.end(); itvv++)
			verts[(*itv)->id()]->to.push_back(verts[(*itvv)->id()]);
		}
	}

	for(list<FacePtr>::iterator itf = _faces.begin(); itf!=_faces.end(); itf++){
		Face* f = (*itf);
		Face* nf = new Face(f->size);
		for(int j=0; j<f->size; j++){
			nf->verts[j] = verts[ f->verts[j]->id()];
		}
		nf->update();
		m->addFace(nf);
	}
	if (border_v0)
		m->border_v0 = verts[border_v0->id()];
	return m;
}

int Mesh::exportOBJ(char * fname){
	ofstream out(fname);
	if(!out) { 
		cout << "Cannot open file.\n"; 
		return -1; 
	} 
	int err = exportOBJ(out);
	out.close();
	return 0;//err;
}

int Mesh::exportOBJ(std::ofstream& out){
 // map<Vertex*, int> vtable;
  out<<"#vertices"<<endl;
  //int ind = 1;
  for(list<Vertex*>::iterator itv = _verts.begin(); itv != _verts.end(); itv++){
	  Vertex* v = (*itv);
	  out<<"v "<< v->P().x <<" "<< v->P().y <<" "<< v->P().z << endl;
	  //vtable[v] = ind++;
  }

  out<<"#faces"<<endl;
  for(list<FacePtr>::iterator itf = _faces.begin(); itf!=_faces.end(); itf++){
	  out<<"f ";
	 for(int i=0; i<(*itf)->size; i++)
		 out<<((*itf)->verts[i]->id()+1)<<" ";
		  //out<<vtable[ (*itf)->verts[i] ]<<" ";
	  out<<endl;
  }
  return 0;
}

int Mesh::loadOBJ(std::ifstream& inf){

	map<int, Vertex*> vtable;
	int vid = 0;
	while(!inf.eof()){

		char cline[255];
		if (!(inf >> cline))
			break;

		if ( cline[0]=='#' && cline[1]=='e' && cline[2]=='n' && cline[3]=='d')
			break; 

		if (cline[0]=='#')
			continue;
	
		if (cline[0]=='v'){
			Vec3 p;
			inf >> p.x >> p.y >> p.z; 
			Vertex* v = addVertex(p);
			vtable[vid++] = v;
		}

		if (cline[0]=='f'){
			int id = 0;
			int ids[50];
			int sz = 0;
			char idline[255];
			inf.getline(idline, 255);
			char * tok = std::strtok(idline," ");
			while(tok!=NULL){
				ids[sz++] = atoi(tok);
				tok = std::strtok(NULL, " ");
			}

			Face* f = new Face(sz);
			for(int i = 0; i < sz; i++){
				f->verts[i] = vtable[ids[i]-1];
			}
			addFace(f);
		}
	}
	return 0;
}

int Mesh::save(std::ofstream& outf){
	return exportOBJ(outf);
}

int Mesh::load(std::ifstream& inf){
	return loadOBJ(inf);
}

Vertex* Mesh::extractBorder(){

	int * WEdges = new int[_verts.size()*_verts.size()];
	for(list<FacePtr>::iterator itf = _faces.begin(); itf!=_faces.end(); itf++){
		Face* f = (*itf);
		f->update();
		for(int i=0; i<f->size; i++){
			int eid = f->edgeUid(i);
			if (WEdges[eid] == 1)
				WEdges[eid]++;
			else
				WEdges[eid] = 1;
		}
	}

	for(list<FacePtr>::iterator itf = _faces.begin(); itf!=_faces.end(); itf++){
		Face* f = (*itf);
		for(int i=0; i<f->size; i++){
			int eid = f->edgeUid(i);
			if (WEdges[eid] != 1)
				continue;
			f->v(i)->setNext(f->v(i+1));
			if (!border_v0)
				border_v0 = f->v(i);
		}
	}

	return border_v0;
}

int Face::pick(const Vec3& p, double rad){
	Vec3 p0 = verts[0]->P();
	for(int i=1; i < size+1; i++){
		Vec3 p1 = verts[i%size]->P();
		Vec3 p0p1 = (p1 - p0);
		double p0p1_len = p0p1.norm();
		p0p1 = p0p1 / p0p1_len;
		double pp_len = (p-p0)*p0p1;
		if (pp_len>0 && pp_len < p0p1_len){
			double d = (p - (p0 + p0p1*pp_len) ).norm();
			if (d < rad)
				return (i-1);	
		}
		p0 = p1;
	}
	return -1;
}

bool Face::isIn(const Vec3& p){
	// works with lazy projection
	bool  oddNodes = false;
	Vec3 p1 = verts[0]->P();
	Vec3 p0 = verts[size-1]->P();
	for (int t = 0; t < size; t++) {
		if ( (p1.y < p.y && p0.y >= p.y) ||  (p0.y < p.y && p1.y >= p.y)) {
		  if  ( p1.x + (p.y-p1.y) / ( p0.y - p1.y)*( p0.x - p1.x )< p.x ) {
			oddNodes=!oddNodes; 
		  }
		}
		p0.set(p1);
		p1 = v(t+1)->P();
	}
	return oddNodes; 
}

/*
void Mesh::subdivCC(){

	list<WEdgePtr> added;
	for(list<WEdgePtr>::iterator it = _WEdges.begin(); it!=_WEdges.end();it++){
		Vertex* v = Vertex::create(this);
		WEdgePtr e = (*it)->split(v, 0.5);
		added.push_back(e);
		(*it)->flag.store(e);
	}
	_WEdges.splice(_WEdges.end(), added);

	list<FacePtr> newfaces;
	for(list<FacePtr>::iterator it = _faces.begin(); it!=_faces.end();it++){
		Vertex* v = Vertex::create(this);
		v->setP((*it)->P());
		WEdgePtr * ee0 = 0;
		WEdgePtr en0 = 0;
		WEdgePtr ee00 = 0;
		WEdgePtr en00 = 0;
		int pick = -1, pick0 = 0;
		FacePtr f = (*it);
		FacePtr ff;
		for(list<WEdgePtr>::iterator ite = f->WEdges.begin(); ite != f->WEdges.end(); ite++){

			WEdgePtr * ee = new WEdgePtr[2];
			ee[0] = (*ite);
			ee[1] = (WEdgePtr)(*ite)->flag.retrieve();
			WEdgePtr en = this->addWEdge(v, ee[1]->v0);

			if (ee0){
				if (pick!=-1){
					int pi = 0;
					Vertex* c = ee0[pick]->corner(ee[pi]);
					if (!c)
						c = ee0[pick]->corner(ee[++pi]);
					if (c){
						FacePtr fn = new Face(4);
						fn->setWEdges(en0, ee0[pick], ee[pi], en);
						newfaces.push_back(fn);
						pick = 1-pi;
					}
				} else{
					int pi = 0, pi0 = 0;
					Vertex* c = ee0[pi0]->corner(ee[pi]);
					if (!c)
						c = ee0[pi0]->corner(ee[++pi]);
					if (!c)
						c = ee0[++pi0]->corner(ee[pi]);
					if (!c)
						c = ee0[pi0]->corner(ee[--pi]);
					if (c){
						ff = new Face(4);
						ff->addWEdge(en0);
						ff->addWEdge(ee0[pi0]);
						ff->addWEdge(ee[pi]);
						ff->addWEdge(en);
						pick = 1-pi;
						en00 = en0;
						ee00 = ee0[1-pi0];
					}
				}
			}

			ee0 = ee;
			en0 = en;
		}
		if (ff){
			f->WEdges.swap(ff->WEdges);
			f->updateVertsByWEdges();
		}
		{
			FacePtr fn = new Face(4);
			fn->setWEdges(en0, ee0[pick], ee00, en00);
			newfaces.push_back(fn);
		}
	}
	_faces.splice(_faces.end(), newfaces);
	//cout<<"faces:"<<_faces.size()<<endl;
}
*/