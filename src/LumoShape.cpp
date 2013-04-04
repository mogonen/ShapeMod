#include "LumoShape.h"

void LumoShape::draw(int){

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, _tmap);
	// Draw a textured quad
	glBegin(GL_QUADS);
		double x = _quad.x/2; 
		double y = _quad.y/2;
		glTexCoord2f(0, 0); glVertex3f(-x, y, 0);
		glTexCoord2f(0, 1); glVertex3f(x, y, 0);
		glTexCoord2f(1, 1); glVertex3f(x, -y, 0);
		glTexCoord2f(1, 0); glVertex3f(-x, -y, 0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

LumoShape::LumoShape(){
	_pmap = _vmap = 0;
	glGenTextures( 1, &_tmap );
	glBindTexture( GL_TEXTURE_2D, _tmap );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,	GL_LINEAR_MIPMAP_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,	GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);//_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);//_TO_EDGE );
}

void LumoShape::exec(int cmd , void* p){

	_curves = Canvas::getCanvas()->selectedCurves();
	Canvas::getCanvas()->flushSelectedCurves();
	renderToTex();
}

void LumoShape::renderToTex(){
	GLuint FramebufferName = 0;
	glGenFramebuffers(1, &FramebufferName);

	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
	
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, _tmap);
 
	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, 1024, 768, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);
 
	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _tmap, 0);
 
	// Set the list of draw buffers.
	GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	// Always check that our framebuffer is ok
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return;

	// Render to our framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
	glViewport(0, 0, 1200, 900); // Render on the whole framebuffer, complete from the lower left corner to the upper right

	for(list<Curve*>::iterator it = _curves.begin(); it!= _curves.end(); it++)
		(*it)->draw();
}

void LumoShape::render(){

	GLuint fbo;
	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, _tmap, 0);
	for(list<Curve*>::iterator it = _curves.begin(); it!= _curves.end(); it++)
		(*it)->draw();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); 

}