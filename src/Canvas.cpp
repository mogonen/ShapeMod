#include "Canvas.h"

Canvas* Canvas::_canvas = new Canvas();

int Canvas::TOOL = Canvas::MESH_TOOL;
int Canvas::MODE = Canvas::SKETCH;
bool Canvas::ISOLATE = false;
int Shape::_COUNT = 0;

bool Canvas::handleMouse(int state, const Vec3& p){

	if (MODE == SKETCH){

		if (_active){
			Vec3 p_ = p - _active->gT();
			if (state == MOUSE::UP)
				_active->exec(Command::CLICK, (void*)new Vec3(p_));
			if (state == MOUSE::SELECT)
				_active->exec(Command::CTRL_CLICK, (void*)new Vec3(p_));
			return false;
		}

		switch(state){
			case MOUSE::DRAGGED:
					strokeman->HandleMouseMotion(p);
			break;

			case MOUSE::UP:{
				Curve* c = strokeman->finalize();
				if (c)
					insert(c);
			}
			break;

			case MOUSE::DOWN:
				switch(TOOL){
					case STROKE_TOOL:
						strokeman->start();
					break;

					case SPLINE_TOOL:{
						insert(new BSpline());
					}
					break;

					case MESH_TOOL:{
						//insert(new FlatShape());
						insert(new BezierShape());
						//insert(new SubdivShape());
					}
					break;
				}
			break;
		}
		return false;
	}

	if (state == MOUSE::DRAGGED ){

		Vec3 mov =  p - _mp0;
		if (_mstate ==  MOUSE::DOWN && ControlPoint::active())
			ControlPoint::active()->drag(mov);
		else if (_mstate == MOUSE::DOWN && _active)
			_active->drag(mov);
		_mp0 = p;
		return 0;
	}

	if (state >= MOUSE::UP){

		if (state == MOUSE::SELECT ){
			if (ControlPoint::active())
				ControlPoint::active()->select();
		}

		ControlPoint::deactivate();

		if (state == MOUSE::SELECT && _active){
			//Select Additional Curves
			for(list<Curve*>::iterator it = _curves.begin(); it!=_curves.end(); it++){
				ShapePtr cp = (*it)->select(p);
				if (cp && !cp->is(SELECTED)){
					_selected.push_back((Curve*)cp);
					cp->set(SELECTED);
				}
			}
		} else if (!_active){//Select Active Shape
			for(list<ShapePtr>::iterator it = _shapes.begin(); it!=_shapes.end(); it++){
				(*it)->unset(SELECTED);
				if ((*it)->isChild())
					continue;
				ShapePtr sp = (*it)->select(p);
				if (sp)
					activate(sp);
			}
		}
	}

	if (state == MOUSE::DOWN){
		if (_active && _active->type() <= CURVE)
			ControlPoint::activate((Curve*)_active, (p - _active->gT()) );
		else if (_active)
			_active->click(p);
		_mp0 = p;
	}

	_mstate = state;
	return 0;
}

void Canvas::draw(){

	if (MODE == SHAPEMAP || MODE == ALPHA){
		drawCurvedShapes(false);
		return;
	}else if (MODE == OUTLINE){
		drawCurvedShapes(true);
		return;
	}

	if (!_shapes.size()){
		strokeman->draw();
		return;
	}

	if (!ISOLATE){
		float zoff = _shapes.size()*UNITZ + 1;
		for(list<ShapePtr>::iterator it = _shapes.begin(); it!=_shapes.end(); it++){

			if ( (*it)->isChild() || (*it) == _active )
				continue;

			glColor3f(0, 0, 0);
			int iss = (*it)->is(SELECTED);
			if (iss)
				glColor3f(0.2f, 0.6f, 0.6f);

			glPushMatrix();
				glTranslatef(0, 0, -zoff);
				(*it)->drawSelf(iss);
			glPopMatrix();
			zoff-= UNITZ;
		}
	}

	if ( _active){

		/*if (MODE == EDIT){
			glBegin(GL_POLYGON);
				glColor4f(1.0, 1.0 , 1.0, 0.5);
				glVertex3f(0, 1.5, 1);
				glVertex3f(2, 1.5, 1);
				glVertex3f(2, 0, 1);
				glVertex3f(0, 0, 1);
			glEnd();
		}*/

		glColor3f(0.2f, 1.0f, 1.0f);
		glLineWidth(3.0);		
		if (MODE == EDIT){
			glPushMatrix();
				Vec3 t = _active->gT();
				glTranslatef(t.x, t.y, 0);
				ControlPoint::draw(_active);
			glPopMatrix();
		}
		_active->drawSelf(2);
	}

	strokeman->draw();
};

//ShapeMap
void Canvas::drawCurvedShapes(bool drawchilds){
	float zoff = _shapes.size() * UNITZ + 1;
	glLineWidth(3.0);
	for(list<ShapePtr>::iterator it = _shapes.begin(); it!=_shapes.end(); it++){
		if ( (*it)->type()!= CURVED_SHAPE && (*it)->type()!= MESH_SHAPE )
			continue;
		if (drawchilds)
			(*it)->drawSelf(0);
		else
			(*it)->drawSelf(0);
		zoff-=UNITZ;
	}
}

int Canvas::saveTo(const char* fname){
	ofstream outf(fname);
	if(!outf) { 
		cout << "Cannot open file: "<<fname<<endl; 
		return -1; 
	} 
	for(list<ShapePtr>::iterator it = _shapes.begin(); it!=_shapes.end(); it++){
		if ((*it)->isChild())
			continue;
		outf<<"#newshape "<<(*it)->type()<<" "<<(*it)->id()<<endl;
		(*it)->save(outf);
		outf<<"#endshape"<<endl<<endl;
		Vec3 t = (*it)->T();
		if (t.norm()>0)
			outf<<"#translate "<<t.x<<" "<<t.y<<" "<<t.z<<endl;
	}	
	outf.close();
	return 0;
}

int Canvas::loadFrom(const char * fname){
	ifstream inf(fname);
	if(!inf) { 
		cout << "Cannot open file.\n"; 
		return -1; 
	}
	int i=0;
	Shape* shape = 0;
	while(!inf.eof()){
		char tag[255];
		inf>>tag;
		if (std::strcmp(tag, "#newshape") == 0){
			int type, id;
			inf >> type >> id;
			shape = ShapeFactory::loadShape(inf, type);
			_shapes.push_back(shape);
		}else if (shape && std::strcmp(tag, "#translate") == 0){
			Vec3 t;
			inf >> t.x >> t.y >> t.z;
			shape->drag(t);
		}
	}

	inf.close();
	return 0;
}