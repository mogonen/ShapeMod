#include "FlatShape.h"

void FlatShape::draw(int m){
	/*if (m)
		MeshShape::draw(m);*/
	if ( _control ){
		glColor3f(0,0,0);
		if (m)
			_control->draw(Mesh::FACECOLOR_OVERRIDE | Mesh::WIRE_OVERRIDE);
		else
			_control->draw(Mesh::FACECOLOR_OVERRIDE | Mesh::WIRE_OVERRIDE);
	}
}

void FlatShape::flatShade(){
	if (!_control->sizeF())
		return;

	list<Face*> faces = _control->faces();
 
	for(list<FacePtr>::iterator it = faces.begin(); it!= faces.end();it++){
		Face* f = *it;
		f->update();
		if (f->size!=4)
			continue;

		Vec2 pAB[2];
		for(int i=0;i<2;i++){
			pAB[i] = getIntersection(f->v(i)->P() ,(f->v(i+1)->P() - f->v(i)->P()).normalize(),  f->v(i+3)->P(), (f->v(i+2)->P() - f->v(i+3)->P()).normalize());
			tri[i].set(pAB[i].x, pAB[i].y, -1.0);
		}
		double k = pAB[0]*pAB[1];
		double x = (k/pAB[0].y - k/pAB[1].y) / ( pAB[0].x / pAB[0].y - pAB[1].x / pAB[1].y);
		double y = (k/pAB[0].x - k/pAB[1].x) / ( pAB[0].y / pAB[0].x - pAB[1].y / pAB[1].x);

		tri[2].set(x, y, -1.0);

		Vec3 n(x,y,-1);
		n = -n.normalize();
		//cout<<k<<"  "<<x<<","<<y<<endl;n.print();
		f->setC(Vec4(n));
		f->setN(n);
	}
}

void FlatShape::exec(int cmd, void* param){

	MeshShape::exec(cmd, param);
	if (cmd == Command::BUILD){
		buildControlMesh();
		return;
	}
}

void FlatShape::onShapeChanged(int type){

	if (type == HEAVY)
		flatShade();

	/*if (!Mesh::DRAW_MESH ){
		_shapechanged = true;
		return;
	}else if (type == HEAVY){
		flatShade();
		_shapechanged = false;
	}*/
}

int FlatShape::load(std::ifstream& inf){

	int err = MeshShape::load(inf);
	if (err == 0)
		flatShade();
	return err;
}