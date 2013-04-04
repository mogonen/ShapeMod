#include <stdio.h>
#include <stdlib.h>
#include <cstdio>
#include <cmath>
#include <time.h>
#include <string.h>
#include "LumoShape.h"
#include <gl/glui.h>
#include <IL/il.h>
#include <FI/FreeImage.h>


#include "Vector.h"
#include "Camera.h"
#include "Stroke.h"
#include "Canvas.h"
#include "CurvedShape.h"
#include "SubdivShape.h"

using namespace std;

Vec3 (*ControlPoint::toView)(Vec3 p);
//int* (*MeshShape::toScreen)(const Vec3&);

int WIDTH = 1200;
int HEIGHT = 900;
int persp_win;

GLUI * glui_window;
GLUI_FileBrowser *fbrowser;
GLUI_EditText* fname_box;
GLUI_String filename;

int draw = 1;				//  Related to Draw Check Box
int listbox_item_id = 12;	//  Id of the selected item in the list box
int tool_id = 0; //  Id of the selcted radio button

enum
{
	COLOR_LISTBOX = 0,
	TOOL_RADIOGROUP,
	TRANSLATION_XY,
	TRANSLATION_Z,
	ROTATION,
	CONTROLS_SPINNER,
	RADIUS_SPINNER,
	FLATNESS_SPINNER,
	STEP_SPINNER,
	CAPS_SPINNER,
	SPANS_SPINNER,
	FADEIN_SPINNER,
	FEATHER_SPINNER,
	FB,
	LOADB,
	SAVE_OBJ_BTN,
	SAVE_IMG_BTN,
	NEW_BTN,
	FNAMEBOX,
	QUIT_BUTTON
};

bool SHOW_CONTROL_POINTS = true;
int StrokeManager::CONTROL_POINTS = 10;
int showGrid = 0;
int file_id = 0;
int showBG = 0;
BitMap * bgmap = 0;
bool smoothing = true;
int glut_modifier;

Camera * camera;
//StrokeManager * strokeman;
Canvas * canvas = Canvas::getCanvas();
BSpline * bspline;

void setupGLUI ();
bool saveBuffer(const char *fname, int c=3);
void LoadBG(const char *fname);
//GLUI_CB glui_callback;

void makeGrid() {
  glColor3f(0.93, 0.93, 0.93);
  glLineWidth(1.0);

  for (float i=-2; i<2; i+=0.1) {
    for (float j=-2; j<2; j+=0.1) {
      glBegin(GL_LINES);
      glVertex3f(i, 0, j);
      glVertex3f(i, 0, j+1);
      glEnd();
      glBegin(GL_LINES);
      glVertex3f(i, 0, j);
      glVertex3f(i+1, 0, j);
      glEnd();

     if (j == 1.9){
		glBegin(GL_LINES);
		glVertex3f(i, 0, j+1);
		glVertex3f(i+1, 0, j+1);
		glEnd();
     }
     if (i == 1.9){
		glBegin(GL_LINES);
		glVertex3f(i+1, 0, j);
		glVertex3f(i+1, 0, j+1);
		glEnd();
      }
    }
  }

  glLineWidth(2.0);
  glBegin(GL_LINES);
  glVertex3f(-2, 0, 0);
  glVertex3f(2, 0, 0);
  glEnd();
  glBegin(GL_LINES);
  glVertex3f(0, 0, -2);
  glVertex3f(0, 0, 2);
  glEnd();
  glLineWidth(1.0);
}

Vec3 toView(Vec3 p){
	return camera->toView(p);
}

int* toScreen(const Vec3& p){
	return camera->toScreen(p);
}

void init() {

  // set up camera
  camera = new Camera(Eye::get()->P, Eye::get()->N, Eye::get()->U, PZ, 1000, FOV);
  glClearColor(0.98, 0.98, 0.98, 0.00);
  //glClearColor(0.781, 0.781, 0, 0.00);
  glShadeModel(GL_SMOOTH);
  glDepthRange(0.0, 1.0);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);

  //strokeman = new StrokeManager();
  ControlPoint::toView = toView;
 // MeshShape::toScreen  = toScreen;
}

void drawGLScene(){
	if (smoothing){
		//glEnable (GL_POLYGON_SMOOTH);
		glEnable(GL_LINE_SMOOTH);
	}else{
		//glDisable (GL_POLYGON_SMOOTH);
		glDisable (GL_LINE_SMOOTH);
	}
	//strokeman->draw();
	if (canvas)
		canvas->draw();
}

void PerspDisplay() {

	if (Canvas::MODE == Canvas::SKETCH)
	  glClearColor(0.98, 0.98, 0.98, 0);
  else if (Canvas::MODE == Canvas::EDIT)
	  glClearColor(0.781, 0.781, 0, 0);
  else if (Canvas::MODE == Canvas::ALPHA)
	  glClearColor(0,0,0,0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // draw the camera created in perspective
  //camera->PerspectiveDisplay(WIDTH, HEIGHT);
  camera->OrthoDisplay(WIDTH, HEIGHT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if (showGrid) 
    makeGrid();

  drawGLScene();
  if (showBG && bgmap){
	  glDrawPixels(WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, bgmap->get());
		/*glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0,1,0,1,0,1);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		
	    GLuint texID;
	    glGenTextures (1, &texID);
		glBindTexture(GL_TEXTURE_2D, texID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 640, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, bgmap);
		glBegin (GL_QUADS);
		glTexCoord2f (0, 0);
		glVertex2f (0, 0);
		glTexCoord2f (640, 0);
		glVertex2f (WIDTH, 0);
		glTexCoord2f (640, 480);
		glVertex2f (WIDTH, HEIGHT);
		glTexCoord2f (0, 480);
		glVertex2f (0, HEIGHT);
		glEnd();*/
  }

  glutSwapBuffers();
}

void mouseEventHandler(int button, int state, int x, int y) {
  glut_modifier = glutGetModifiers();
  // let the camera handle some specific mouse events (similar to maya)
  Vec3 mp = camera->HandleMouseEvent(button, state, x, y);
  if ( glut_modifier != GLUT_ACTIVE_ALT ){
		state =  (state == GLUT_UP)? MOUSE::UP: MOUSE::DOWN;
		if ( glut_modifier == GLUT_ACTIVE_CTRL && state == MOUSE::UP)
			state = MOUSE::SELECT;
		canvas->handleMouse(state, mp);
  }
  glutPostRedisplay();
}

void motionEventHandler(int x, int y) {
//int mode = glutGetModifiers();
  // let the camera handle some mouse motions if the camera is to be moved
  Vec3 mp = camera->HandleMouseMotion(x, y);
  if (camera->isInactive() && glut_modifier != GLUT_ACTIVE_CTRL){
		  canvas->handleMouse(MOUSE::DRAGGED, mp);
  }
  glutSetWindow(persp_win);
  glutPostRedisplay();
}

void keyboardEventHandler(unsigned char key, int x, int y) {
  //cout<<"="<<0+key<<endl;
  switch (key) {
  case '1':
    // reset the camera to its initial position
		camera->Reset();
		Eye::setCam(camera);
		Canvas::MODE = Canvas::SKETCH;
  break;

  case '2':
	  Canvas::MODE = Canvas::EDIT;
	  Mesh::DRAWING_MODE = Mesh::NORMALMAP;
  break;

  case '3':
	  Canvas::MODE = Canvas::SHAPEMAP;
	  Mesh::DRAWING_MODE = Mesh::NORMALMAP;
  break;

  case '4':
	  canvas->deselect();
	  Canvas::MODE = Canvas::OUTLINE;
	  Mesh::DRAWING_MODE = Mesh::SOLID;
  break;

  case '5':
	 canvas->deselect();
	 Canvas::MODE = Canvas::ALPHA;
	 Mesh::DRAWING_MODE = Mesh::ALPHAMAP;
  break;


  case 'f':
	  if (canvas->active())
			canvas->active()->exec(Command::BLEND_MESH_NORMALS);
    //camera->SetCenterOfFocus(Vec3(0, 0, 0));
  break;

  case 'g':
    showGrid = !showGrid;
	Mesh::DRAW_WIREFRAME = showGrid;
  break;

  case 'c':
	  SHOW_CONTROL_POINTS=!SHOW_CONTROL_POINTS;
  break;

  /*case '6':{
	  canvas->active()->exec(4, (void*)bgmap);
  }break;

 case '7':{
	  canvas->active()->exec(5, (void*)bgmap);
  }break;

 case '8':{
	  canvas->active()->exec(6, (void*)bgmap);
  }break;
  */

  case 127:
	  canvas->removeActive();
  break;

  case 13:
	  {
		  if (canvas->active() && canvas->active()->type() <= CURVE){
			  //CurvedShape* lp = new CurvedShape();
			  Shape* lp = new LumoShape();
			  lp->exec();
			  canvas->insert(lp);
		  }else if (canvas->active())
			canvas->active()->exec(Command::BUILD);
		  
	  }
  break;

  case 8:{
	  if (canvas->active())
		  canvas->active()->exec(Command::UNDO);
  }
  break;

  case 10:
	  canvas->active()->exec(Command::REBUILD);
  break;


/* case 'b':

  {
		 ShapePtr lp = new CurvedShape();
		 lp->exec(Command::BUILD_NO_SPINE);
		 canvas->insert(lp);
	  }
  break;*/

  case 'h':
	  Canvas::ISOLATE = ! Canvas::ISOLATE;
  break;

  case ' ':
	 Canvas::MODE = (Canvas::MODE == Canvas::SKETCH)? Canvas::EDIT : Canvas::SKETCH;
	 Mesh::DRAWING_MODE = Mesh::NORMALMAP;
  break;

  case 'p':
	  //project
	  camera->SetClippingPlanes(PZ,1000);
	  Eye::setCam(camera);
  break;

   case 's':
	   saveBuffer("c://temp//test.png", 4);
	   canvas->saveTo("c://temp//test.cnvs");
	   /*canvas->deselect();
	   showGrid = 0;
	   showBG = 0;
	   Mesh::DRAW_WIREFRAME = false;
	   Canvas::MODE = Canvas::EDIT;
	   glutPostRedisplay();
	   saveBuffer("c://temp//test.png");

	   Canvas::MODE = Canvas::OUTLINE;
	   Mesh::DRAWING_MODE = Mesh::SOLID;
	   glutPostRedisplay();
	   saveBuffer("c://temp//test_o.png");*/
	   file_id++;
   break;

   case 'l':
	   canvas->loadFrom("c://temp//test.cnvs");
	   break;

   case 'n':
	   /*strokeman->reset();
	   ControlPoint::clear();
	   camera->Reset();
	   Eye::setCam(camera);
	   canvas->clear();*/
   break;

    case 'q':
	   camera->SetClippingPlanes(0.1, 1000);
    break;

	case '0':
		if (canvas->active())
			canvas->active()->resetT();
	break;

	case 'r':
		if (canvas->active())
			canvas->active()->exec(Command::REVERT);
    break;

	case 'w' :
	  camera->SetClippingPlanes(PZ,1000);
	break;

	case '+':
		canvas->activeUp();
	break;
	case '-':
		canvas->activeDown();
	break;

	case '.':
		BezierShape::MODE= !BezierShape::MODE;
	break;

	case 27:
		canvas->deselectAll();
	break;

	case 'm':
		Mesh::DRAW_MESH = !Mesh::DRAW_MESH;
		//MeshShape::UPDATE_SUBDIV = Mesh::DRAW_MESH;
	break;
  }

  glutPostRedisplay();
}

void SpecialInput(int key, int x, int y)
{
	switch(key)
	{
		case GLUT_KEY_UP:
		//do something here
		break;	
		case GLUT_KEY_DOWN:
		//do something here
		break;
		case GLUT_KEY_LEFT:
			canvas->selectNext();
		//do something here
		break;
		case GLUT_KEY_RIGHT:
			canvas->selectPrev();
		//do something here
		break;
	}
	glutPostRedisplay();
}

static void initGLScene(){

	glShadeModel(GL_SMOOTH);
	//glDepthRange(0.0, 1.0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable( GL_ALPHA_TEST );
	glEnable(GL_TEXTURE_2D);
	//glAlphaFunc( GL_GREATER, 1.0 );

	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);

	/*GLfloat l0pos[]= { 0.0f, 0.0f, 5.0f, 1.0f };   
	glLightfv(GL_LIGHT0, GL_POSITION, l0pos);
	/*glEnable(GL_LIGHT1);

	glEnable(GL_LIGHT2);
	glEnable(GL_LIGHT3);
	glEnable(GL_LIGHT4);
	glEnable(GL_LIGHT5);
	glEnable(GL_DEPTH_TEST);*/
	//glCullFace (GL_BACK);
    //glEnable (GL_CULL_FACE);
    //glBlendFunc (GL_SRC_ALPHA_SATURATE, GL_ONE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glClear (GL_COLOR_BUFFER_BIT);
    //glDisable (GL_DEPTH_TEST);
	//glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
}


void initScene(){
}

int main(int argc, char *argv[]) {

  FreeImage_Initialise();
  LoadBG("c://temp//hands.png");
  initScene();
  // set up opengl window
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_ACCUM);
  

  glutInitWindowSize(WIDTH, HEIGHT);
  glutInitWindowPosition(50, 50);
  persp_win = glutCreateWindow("Scene");

  // initialize the camera and such
  init();
  initGLScene();
  //updateGLScene();

  // set up opengl callback functions
  glutDisplayFunc(PerspDisplay);
  glutMouseFunc(mouseEventHandler);
  glutMotionFunc(motionEventHandler);
  glutKeyboardFunc(keyboardEventHandler);
  glutSpecialFunc(SpecialInput);

  glutSetWindow(persp_win);
  setupGLUI();

  glutMainLoop();
  return(0);
}

void idle (){
	glutSetWindow (persp_win);
	glutPostRedisplay();
	//Sleep (50);
}

void glui_callback(int id){

	if (id == LOADB){
		canvas->loadFrom(fbrowser->get_file());
	}

	if (id == SAVE_OBJ_BTN){
		canvas->saveTo(fname_box->get_text());
	}

	if (id == FB){
		fname_box->set_text(fbrowser->get_file());
	}

	if (id == SAVE_IMG_BTN){
		saveBuffer(fname_box->get_text());
	}

	if (id == NEW_BTN){
		canvas->clear();
	}

	if (id == TOOL_RADIOGROUP){
		switch(tool_id){
			case 0:
				Canvas::TOOL = Canvas::MESH_TOOL;
			break;
			case 1:
				Canvas::TOOL = Canvas::SPLINE_TOOL;
			break;
			case 2:
				Canvas::TOOL = Canvas::STROKE_TOOL;
			break;
		}
	}

	glutPostRedisplay();
}

void setupGLUI ()
{
	//  Set idle function
	//GLUI_Master.set_glutIdleFunc (idle);
	
	//  Create GLUI window
	glui_window = GLUI_Master.create_glui ("Options", 0, 0, 20);
	
	//---------------------------------------------------------------------
	// 'Object Properties' Panel
	//---------------------------------------------------------------------

	//  Add the 'Object Properties' Panel to the GLUI window
	GLUI_Panel *op_panel = glui_window->add_panel ("Object Properties");

	//  Add the Draw Check box to the 'Object Properties' Panel
	glui_window->add_checkbox_to_panel (op_panel, "Caps", &CurvedShape::CAPS_ON );

	//  Add the Wireframe Check box to the 'Object Properties' Panel
	glui_window->add_checkbox_to_panel (op_panel, "Wireframe", &showGrid );
	glui_window->add_checkbox_to_panel (op_panel, "Background", &showBG );

	//  Add a separator
	glui_window->add_separator_to_panel (op_panel);
	
	GLUI_Spinner * numcontrols = glui_window->add_spinner_to_panel (op_panel, "Controls", GLUI_SPINNER_INT, & StrokeManager::CONTROL_POINTS, CONTROLS_SPINNER);
	numcontrols->set_int_limits(3, 99);

	GLUI_Spinner * radius = glui_window->add_spinner_to_panel (op_panel, "Radius", GLUI_SPINNER_FLOAT, &CurvedShape::RADIUS, RADIUS_SPINNER);
	radius->set_float_limits(0.001, 0.5);

	GLUI_Spinner * flatness = glui_window->add_spinner_to_panel (op_panel, "Flatness", GLUI_SPINNER_FLOAT, &CurvedShape::FLATNESS, FLATNESS_SPINNER);
	flatness->set_float_limits(0, 0.99);

	GLUI_Spinner * step = glui_window->add_spinner_to_panel (op_panel, "Step", GLUI_SPINNER_FLOAT, &CurvedShape::SPINE_STEP, STEP_SPINNER);
	step->set_float_limits(0.005, 0.25);

	GLUI_Spinner * caps = glui_window->add_spinner_to_panel (op_panel, "Caps", GLUI_SPINNER_INT, &CurvedShape::CAPS, CAPS_SPINNER);
	caps->set_int_limits(0, 64);

	GLUI_Spinner * spans = glui_window->add_spinner_to_panel (op_panel, "Spans", GLUI_SPINNER_INT, &CurvedShape::VNUM, SPANS_SPINNER);
	spans->set_int_limits(2, 12);

	GLUI_Spinner * fadein = glui_window->add_spinner_to_panel (op_panel, "Fade In", GLUI_SPINNER_FLOAT, &CurvedShape::FADEIN, FADEIN_SPINNER);
	fadein->set_float_limits(0, 0.6);

	GLUI_Spinner * feather = glui_window->add_spinner_to_panel (op_panel, "Feather", GLUI_SPINNER_FLOAT, &CurvedShape::FEATHER, FEATHER_SPINNER);
	feather->set_float_limits(0, 1.0);

	GLUI_Spinner * n_z = glui_window->add_spinner_to_panel (op_panel, "nZ", GLUI_SPINNER_FLOAT, &CurvedShape::N_Z, FEATHER_SPINNER);
	n_z->set_float_limits(0, 1.0);

	GLUI_Spinner * subdivs = glui_window->add_spinner_to_panel (op_panel, "Subdivs", GLUI_SPINNER_INT, &SubdivShape::SUBDIVS, FEATHER_SPINNER);
	subdivs->set_int_limits(1, 6);

	GLUI_Rollout *ot_rollout = glui_window->add_rollout ("Tool");

	//  Create radio button group
	GLUI_RadioGroup *ot_group = glui_window->add_radiogroup_to_panel 
								(ot_rollout, &tool_id, TOOL_RADIOGROUP, glui_callback);
 	
	//  Add the radio buttons to the radio group
	glui_window->add_radiobutton_to_group( ot_group, "Mesh" );
	glui_window->add_radiobutton_to_group( ot_group, "Spline" );
	glui_window->add_radiobutton_to_group( ot_group, "Stroke" );

	GLUI_Panel *fb_panel = glui_window->add_panel ("File");
	fbrowser = new GLUI_FileBrowser(fb_panel, "", false, FB, glui_callback);
	filename = " ";
	fname_box = glui_window->add_edittext_to_panel(fb_panel,"fname:", filename, FNAMEBOX,  glui_callback);
	fname_box->set_w(150);
	glui_window->add_button ("Save OBJ", SAVE_OBJ_BTN, glui_callback);
	glui_window->add_button ("Save IMG", SAVE_IMG_BTN, glui_callback);
	glui_window->add_button ("Load OBJ", LOADB, glui_callback);
	glui_window->add_button ("NEW", NEW_BTN, glui_callback);

	/*	
	//Add the Color listbox to the 'Object Properties' Panel
	GLUI_Listbox *color_listbox = glui_window->add_listbox_to_panel (op_panel, 
									"Color", &listbox_item_id, COLOR_LISTBOX, glui_callback);
	//  Add the items to the listbox
	color_listbox->add_item (1, "Black");
	color_listbox->add_item (2, "Blue");
	color_listbox->add_item (3, "Cyan");
	color_listbox->add_item (4, "Dark Grey");
	color_listbox->add_item (5, "Grey");
	color_listbox->add_item (6, "Green");
	color_listbox->add_item (7, "Light Grey");
	color_listbox->add_item (8, "Magenta");
	color_listbox->add_item (9, "Orange");
	color_listbox->add_item (10, "Pink");
	color_listbox->add_item (11, "Red");
	color_listbox->add_item (12, "White");
	color_listbox->add_item (13, "Yellow");

	//  Select the White Color by default
	color_listbox->set_int_val (12);

	//---------------------------------------------------------------------
	// 'Object Type' Panel
	//---------------------------------------------------------------------
	
	//  Add the 'Object Type' Panel to the GLUI window
	GLUI_Rollout *ot_rollout = glui_window->add_rollout ("Object Type");

	//  Create radio button group
	GLUI_RadioGroup *ot_group = glui_window->add_radiogroup_to_panel 
								(ot_rollout, &radiogroup_item_id, OBJECTYPE_RADIOGROUP, glui_callback);
 	
	//  Add the radio buttons to the radio group
	glui_window->add_radiobutton_to_group( ot_group, "Cube" );
	glui_window->add_radiobutton_to_group( ot_group, "Sphere" );
	glui_window->add_radiobutton_to_group( ot_group, "Cone" );
	glui_window->add_radiobutton_to_group( ot_group, "Torus" );
	glui_window->add_radiobutton_to_group( ot_group, "Dodecahedron" );
	glui_window->add_radiobutton_to_group( ot_group, "Octahedron" );
	glui_window->add_radiobutton_to_group( ot_group, "Tetrahedron" );
	glui_window->add_radiobutton_to_group( ot_group, "Icosahedron" );
	glui_window->add_radiobutton_to_group( ot_group, "Teapot" );

	

	//---------------------------------------------------------------------
	// 'Quit' Button
	//---------------------------------------------------------------------

	//  Add the Quit Button
	glui_window->add_button ("Quit", QUIT_BUTTON, glui_callback);
	*/

	//  Let the GLUI window know where its main graphics window is
	glui_window->set_main_gfx_window( persp_win );
}

void LoadBG(const char *fname){
	FIBITMAP* fi_bmp = FreeImage_Load(FIF_PNG, fname);
	if (!bgmap)
		bgmap = new BitMap(WIDTH, HEIGHT, 4);
	Byte* map = bgmap->get();
	RGBQUAD pix;
	for ( int i =0; i<WIDTH; i++)
		for ( int j =0; j<HEIGHT; j++){
	
			FreeImage_GetPixelColor(fi_bmp, i , j , &pix);
			int loc = (j*WIDTH + i)*4;
			map[loc] = pix.rgbRed;
			map[loc+1] = pix.rgbGreen;
			map[loc+2] = pix.rgbBlue;
			map[loc+3] = 100;
		}
}

bool saveBuffer(const char *fname, int c){

	unsigned char * bmp;
	bmp = (unsigned char *) malloc(c*WIDTH*HEIGHT*sizeof(char));
	glPixelStorei(GL_PACK_ALIGNMENT,1);
	glReadBuffer(GL_BACK_LEFT);
	glReadPixels(0,0, WIDTH, HEIGHT, (c==4)?GL_RGBA:GL_RGB, GL_UNSIGNED_BYTE, bmp);
	FIBITMAP * fi_bmp = FreeImage_Allocate(WIDTH, HEIGHT, 8*c);
	RGBQUAD pix;

	for(int i = 0; i<WIDTH; i++)
		for( int j = 0; j<HEIGHT; j++){
			int loc = (j*WIDTH+i)*c;
			pix.rgbRed = bmp[loc];
			pix.rgbGreen = bmp[loc+1];
			pix.rgbBlue = bmp[loc+2];

			if (c == 4)
				pix.rgbReserved = bmp[loc+3];

			FreeImage_SetPixelColor(fi_bmp, i , j , &pix);
		}

	FreeImage_Save(FIF_PNG, fi_bmp , fname , 0);
	return false;
}

