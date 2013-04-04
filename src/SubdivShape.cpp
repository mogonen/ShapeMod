#include "SubdivShape.h"

int SubdivShape::SUBDIVS = 1;

void SubdivShape::draw(int m){

	if ( (_control && !_subdiv) ){
		glColor3f(0,0,0);
		_control->draw(Mesh::FACECOLOR_OVERRIDE | Mesh::WIRE_OVERRIDE);
	}

	if (_control && ( m || !_subdiv) )
		drawBorder();
		
	if (_subdiv){
		if (Mesh::DRAW_MESH && _shapechanged)
			onShapeChanged(HEAVY);
		_subdiv->draw(0);
	}
}

void SubdivShape::onShapeChanged(int type){
	if (!Mesh::DRAW_MESH ){
		_shapechanged = true;
		return;
	}else if (type == HEAVY && _subdiv){
		subdivide(true);
		for(int i=0 ;i < _subdivs; i++)
			subdivCC(_subdiv);
		_shapechanged = false;
	}
}

void SubdivShape::exec(int cmd, void* param){
	
	MeshShape::exec(cmd, param);

	if (cmd == Command::BUILD){
		subdivide();
		return;
	}

	if (cmd == Command::REBUILD){
		_subdivs = 0;
		subdivide(true);
		return;
	}

	if (cmd == Command::REVERT){
		_isreversed = !_isreversed;
		subdivide(true);
		return;
	}
}

void SubdivShape::colorSubdivMesh(){

	list<Vertex*> verts = _subdiv->verts();
	for(list<Vertex*>::iterator itv = verts.begin(); itv != verts.end(); itv++){
		Vertex* v = *itv;

		if ( (*itv)->isBorder() ){
			Vec3 tan = (v->next->P() - v->prev->P()).normalize();
			Vec3 n = (Vec3(0,0,1) % tan).normalize();
			if (_isreversed)
				n = -n;
			v->setC(n);
		}else
			v->setC(Vec4(0,0,1,1));
	}
}

void SubdivShape::subdivide(bool newsubdiv){

	buildControlMesh();

	bool first = false;
	if (!_subdiv || newsubdiv){
		if (_subdiv)
			delete _subdiv;
		_control->updateFaces();
		_subdiv = _control->deepCopy();
		first = true;
	}
	
	if (_subdivs < 6)
		subdivCC(_subdiv);
	if (first)
		colorSubdivMesh();
	else if (_subdivs < 6)
		_subdivs++;
}

void SubdivShape::subdivCC(Mesh * mesh){

	unsigned int DONE = 0x10;
	int vs = mesh->size();
	int es = vs*vs*2;
	Vertex** evs = new Vertex*[es];
		
	list<Vertex*> oldverts = mesh->verts();
	list<FacePtr> oldfaces = mesh->faces();

	for(list<Vertex*>::iterator it = oldverts.begin(); it!= oldverts.end();it++){
		Vertex* v = *it;
		if ( !(v->prev && v->next))
			continue;
		Vec3 p = v->P();
		Vec3 p0 = v->prev->P();	
		Vec3 p1 = v->next->P();
		v->v3_2.set((p0 + p1 + 6.0*p)/8.0);

		Vec4 c = v->C();
		Vec4 c0 = v->prev->C();	
		Vec4 c1 = v->next->C();
		v->v4_2.set((c0 + c1 + 6.0*c)/8.0);
	};

	mesh->resetFaces();
	for(list<FacePtr>::iterator it = oldfaces.begin(); it!= oldfaces.end();it++){
		Face* f = (*it);
		f->update();
		Vertex* vo = Vertex::create(mesh);
		vo->setP(f->P());
		Vec4 col = f->C();

		vo->setC(col);
		Vertex* ev0 = 0;

		for(int j = 0; j < f->size+1; j++){

			Vertex* v0 = f->v(j);
			Vertex* v1 = f->v(j+1);

			//face point contribuition
			if (j < f->size){
				v0->v3_0 = v0->v3_0 + f->P();
				v0->v4_0 = v0->v4_0 + f->C();
				v0->flag.inc();
			}

			int ei = ((v0->id()<v1->id())?( v0->id() + v1->id()*vs):(v1->id() + v0->id()*vs))*2;
			Vertex* ev = 0;
			if ((unsigned int)evs[ei] == DONE){
				ev = evs[ei+1];
				if (j < f->size){
					ev->setP(ev->P() + vo->P()*0.25 );
					ev->setC( ev->C() + vo->C()*0.25 );
				}

			}else{
				Vec3 pmid = (v0->P() + v1->P())*0.5;
				Vec4 cmid = (v0->C() + v1->C())*0.5; 
				ev = mesh->addVertex( pmid);
				ev->setC( cmid);

				//ControlPoint::create(this, ev->p0);
				evs[ei] = (Vertex*)DONE;
				evs[ei+1] = ev;

				/*if (v0->flag.is(MID) && v1->flag.is(MID))
					ev->flag.set(MID);*/

				if(v0->next == v1){
					v0->setNext(ev);
					ev->setNext(v1);
				}else{

					ev->setP(pmid*0.5 + vo->P()*0.25 );
					ev->setC(cmid*0.5 + vo->C()*0.25 );

				//WEdge point contribution
					v0->v3_1 = v0->v3_1 + pmid;
					v1->v3_1 = v1->v3_1 + pmid;

					v0->v4_1 = v0->v4_1 + cmid;
					v1->v4_1 = v1->v4_1 + cmid;
				}
			}
			if (ev0)
				mesh->addQuad(vo, ev0, v0, ev);
			ev0 = ev;
		}
	}
	for(list<Vertex*>::iterator it = oldverts.begin(); it!= oldverts.end();it++){
		Vertex* v = (*it);
		int n = v->flag.getC();

		if (!v->prev){
			v->setP( ( 2*(v->v3_0/n) + (v->v3_1/n) + (n-3) * v->P() ) / n  );
			v->setC( ( 2*(v->v4_0/n) + (v->v4_1/n) + (n-3) * v->C() ) / n  );
		}else{
			v->setP(v->v3_2);
			v->setC(v->v4_2);
		}

		v->flag.resetC();
		v->v3_0.set();v->v3_1.set();v->v3_2.set();
		v->v4_0.set();v->v4_1.set();v->v4_2.set();
	}
	delete evs;
}
