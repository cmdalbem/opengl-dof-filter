#include <GL/glew.h>
#include <GL/glui.h>
#include <stdlib.h>
#include <math.h>
#include <jpeglib.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <iostream>
using namespace std;

#include "constants.h"
#include "imageloader.h"

#include "dof.h"



// GLOBALS /////////////////////////////////////////////////////////////

float	near=1, far=100.0;
int		texId;
int		useTex=0, useDof=1;
int 	mainWindow;
GLUI*	guiWindow;

#define CRAZY_TEAPOTS_NUM 42
#define CRAZY_BALLS_NUM 150
vector<mesh>	crazyTeapots;
vector<mesh>	crazyBalls;

// ------------

int sceneOpt = 0;
#define SCENE_COUNT 5

DofFilter *dofFilter;
float focus=4, focalLen=0.7, aperture=30;
int kernelType=0, boostBokeh=0;


// PROGRAM /////////////////////////////////////////////////////////////

void generateCrazyTeapots()
{
	srand(time(NULL));
	
	crazyTeapots.clear();
	
	for(int i=0; i<CRAZY_TEAPOTS_NUM; i++) {
		mesh t;
		
		t.x = rand()%100 / 10. - 5;
		t.y = rand()%100 / 10. - 5;
		t.z = - rand()%500 / 10. - 5;
		t.rotx = rand()%180;
		t.roty = rand()%180;
		t.rotz = rand()%180;
		t.shine = 0.6;
		t.amb.r = (rand()%50)/100.;
		t.amb.g = (rand()%50)/100.;
		t.amb.b = (rand()%50)/100.;
		t.dif.r = (rand()%100)/100.;
		t.dif.g = (rand()%100)/100.;
		t.dif.b = (rand()%100)/100.;
		t.spec.r = (rand()%100)/100.;
		t.spec.g = (rand()%100)/100.;
		t.spec.b = (rand()%100)/100.;
					
		crazyTeapots.push_back(t);
	}
}	

void generateCrazyBalls(bool spiral=false)
{
	srand(time(NULL));
	
	crazyBalls.clear();
	
	for(int i=0; i<CRAZY_BALLS_NUM; i++) {
		mesh t;
		
		if(spiral) {
			t.x = cos(i*M_PI/15.)*3;
			t.y = sin(i*M_PI/15.)*3;
			t.z = -i * CRAZY_BALLS_NUM/200.;			
		}
		else {
			t.x = rand()%300 / 10. - 15;
			t.y = rand()%300 / 10. - 15;	
			t.z = - rand()%500 / 10. - 5;
		}
		t.shine = 0.6;
		t.amb.r = (rand()%100)/100.;
		t.amb.g = (rand()%100)/100.;
		t.amb.b = (rand()%100)/100.;
		t.dif.r = (rand()%100)/100.;
		t.dif.g = (rand()%100)/100.;
		t.dif.b = (rand()%100)/100.;
		t.spec.r = (rand()%100)/100.;
		t.spec.g = (rand()%100)/100.;
		t.spec.b = (rand()%100)/100.;
					
		crazyBalls.push_back(t);
	}
}	

void init(void)
{
	dofFilter = new DofFilter(near,far);
	dofFilter->updateDistance();
	
	{
		Image *im;
		im = loadBMP("im/tex.bmp");
		texId = loadTexture(im);
		delete im;
	}
	
	generateCrazyTeapots();
	generateCrazyBalls(sceneOpt==4);
		
	GLfloat ambient[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat position[] = { 0.0, 3.0, 3.0, 0.0 };

	GLfloat lmodel_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat local_view[] = { 0.0 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, local_view);

	glFrontFace (GL_CW);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearAccum(0.0, 0.0, 0.0, 0.0); 
	
	dofFilter->allocMatrixes(WIDTH,HEIGHT);
}

void renderTeapot (GLfloat x, GLfloat y, GLfloat z, 
	GLfloat rotx, GLfloat roty, GLfloat rotz, 
	GLfloat ambr, GLfloat ambg, GLfloat ambb, 
	GLfloat difr, GLfloat difg, GLfloat difb, 
	GLfloat specr, GLfloat specg, GLfloat specb, GLfloat shine)
{
   GLfloat mat[4];

   glPushMatrix();
   glTranslatef (x, y, z);
   glRotatef (rotx, 1, 0, 0);
   glRotatef (roty, 0, 1, 0);
   glRotatef (rotz, 0, 0, 1);
   mat[0] = ambr; mat[1] = ambg; mat[2] = ambb; mat[3] = 1.0;   
   glMaterialfv (GL_FRONT, GL_AMBIENT, mat);
   mat[0] = difr; mat[1] = difg; mat[2] = difb; 
   glMaterialfv (GL_FRONT, GL_DIFFUSE, mat);
   mat[0] = specr; mat[1] = specg; mat[2] = specb;
   glMaterialfv (GL_FRONT, GL_SPECULAR, mat);
   glMaterialf (GL_FRONT, GL_SHININESS, shine*128.0);
   
   glutSolidTeapot(0.5);
   
   glPopMatrix();
}

void renderSphere (GLfloat x, GLfloat y, GLfloat z, 
	GLfloat rotx, GLfloat roty, GLfloat rotz, 
	GLfloat ambr, GLfloat ambg, GLfloat ambb, 
	GLfloat difr, GLfloat difg, GLfloat difb, 
	GLfloat specr, GLfloat specg, GLfloat specb, GLfloat shine)
{
   GLfloat mat[4];

   glPushMatrix();
   glTranslatef (x, y, z);
   glRotatef (rotx, 1, 0, 0);
   glRotatef (roty, 0, 1, 0);
   glRotatef (rotz, 0, 0, 1);
   mat[0] = ambr; mat[1] = ambg; mat[2] = ambb; mat[3] = 1.0;   
   glMaterialfv (GL_FRONT, GL_AMBIENT, mat);
   mat[0] = difr; mat[1] = difg; mat[2] = difb; 
   glMaterialfv (GL_FRONT, GL_DIFFUSE, mat);
   mat[0] = specr; mat[1] = specg; mat[2] = specb;
   glMaterialfv (GL_FRONT, GL_SPECULAR, mat);
   glMaterialf (GL_FRONT, GL_SHININESS, shine*128.0);
   
   glutSolidSphere(0.2, 20, 20);
   
   glPopMatrix();
}

void display(void)
{
	glutSetWindow(mainWindow);
	
	glMatrixMode(GL_PROJECTION);
	
	glLoadIdentity();
	gluPerspective (45.0, //fovy
				 (float)WIDTH/HEIGHT, //aspect
				 near, far); //near, far
	
	glMatrixMode(GL_MODELVIEW);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glLoadIdentity();	
	
	if(useTex) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texId);
	}

	switch(sceneOpt) {
		case 0: {
				int shine = 1;
				
				/*      ruby, gold, silver, emerald, and cyan teapots   */
				renderTeapot (-1.1+0.5, -0.3, -2, 0,0,0, 0.1745, 0.01175, 
								0.01175, 0.61424, 0.04136, 0.04136, 
								0.727811, 0.626959, 0.626959, shine);
				renderTeapot (-0.5+0.5, -0.3, -4, 0,0,0, 0.24725, 0.1995, 
								0.0745, 0.75164, 0.60648, 0.22648, 
								0.628281, 0.555802, 0.366065, shine);
				renderTeapot (0.2+0.5, -0.3, -6, 0,0,0, 0.19225, 0.19225, 
								0.19225, 0.50754, 0.50754, 0.50754, 
								0.508273, 0.508273, 0.508273, shine);
				renderTeapot (1.0+0.5, -0.3, -8, 0,0,0, 0.0215, 0.1745, 0.0215, 
								0.07568, 0.61424, 0.07568, 0.633, 
								0.727811, 0.633, shine);
				renderTeapot (1.8+0.5, -0.3, -10, 0,0,0, 0.0, 0.1, 0.06, 0.0, 
								0.50980392, 0.50980392, 0.50196078, 
								0.50196078, 0.50196078, shine);
			}
			break;
		case 1:
			renderTeapot (0,-0,-1.5, 0.1745, 0.01175, 0,0,0, 
							0.01175, 0.61424, 0.04136, 0.04136, 
							0.727811, 0.626959, 0.626959, 0.6);
			break;
		
		case 2:
			for(int i=0; i<CRAZY_TEAPOTS_NUM; i++) {
				mesh t = crazyTeapots[i];
				renderTeapot(t.x, t.y, t.z, t.rotx, t.roty, t.rotz, t.shine, t.amb.r, t.amb.g, t.amb.b, t.dif.r, t.dif.g, t.dif.b, t.spec.r, t.spec.g, t.spec.b);
			}
			break;
			
		case 3:
		case 4:
			for(int i=0; i<CRAZY_BALLS_NUM; i++) {
				mesh t = crazyBalls[i];
				renderSphere(t.x, t.y, t.z, t.rotx, t.roty, t.rotz, t.shine, t.amb.r, t.amb.g, t.amb.b, t.dif.r, t.dif.g, t.dif.b, t.spec.r, t.spec.g, t.spec.b);
			}
			break;
		case 5:
			glColor3f(1,1,1);
			glBegin(GL_TRIANGLES);
				glTexCoord2f(0,0);
				glVertex3f(-1, -0.3-0.5, -4);
				glTexCoord2f(0,1);
				glVertex3f(3, -1-0.5, -10);
				glTexCoord2f(1,1);
				glVertex3f(3, 1-0.5, -10);
			glEnd();
			glBegin(GL_TRIANGLES);
				glVertex3f(1, -0.3+0.5, -4);
				glVertex3f(-1, -1+0.5, -10);
				glVertex3f(-1, 1+0.5, -10);
			glEnd();
			break;
	}
	
	glDisable(GL_TEXTURE_2D);

	if(useDof) {
		dofFilter->setParams(focus, focalLen, aperture);
		dofFilter->apply();
	}

	glutSwapBuffers();
}

void apply( int nil=0 )
{
	dofFilter->updateDistance();
	display();
}

void reshape(int w, int h)
{
	//WIDTH = w;
	//HEIGHT = h;
		
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
}

void keyboard(unsigned char key, int x, int y)
{
	if( key >= 49 && key <= 49+SCENE_COUNT) {
		sceneOpt = key-49;
		apply();
	}
	
	if( key == 13 )
		apply();
	
	if( key == 'r' ) {
		generateCrazyTeapots();
		generateCrazyBalls();
		apply();
	}
}

void mouseFunc(int button, int state, int x, int y) {
 
	if( button == GLUT_LEFT_BUTTON || button == GLUT_RIGHT_BUTTON ) {
		focus = MPOS(dofFilter->db,HEIGHT-y,x);
		focus = near*far / (far - focus*(far-near));
		GLUI_Master.sync_live_all();
		
		if( button == GLUT_LEFT_BUTTON )
			apply();
	}
				
}
	

void specialFunc(int key, int x, int y)
{
	static float var = 0.5;
	
	if( key == GLUT_KEY_LEFT ) {

	}
	
	if( key == GLUT_KEY_RIGHT ) {

	}
	
	if( key == GLUT_KEY_DOWN ) {
		focus -= var;
		if(focus<var)
			focus = var;
	}
	
	if( key == GLUT_KEY_UP ) {
		focus += var;
	}
	
	GLUI_Master.sync_live_all();
}

void parseOptions(int argc, char **argv)
{
	char ch;

	while((ch = getopt(argc, argv, "s:k:")) != EOF) {
		
		switch(ch) {		
			case 's':
				sceneOpt = atoi(optarg);
				break;
			case 'k':
				dofFilter->loadKernelBMP(optarg);
				dofFilter->useKernel = 1;
				break;
		}
	}

}

void kernelTypeCB( int nil=0 )
{
	char files[8][64] = {
			"im/gaussian.bmp",
			"im/dirtylens.bmp",
			"im/lens.bmp",
			"im/heart.bmp",
			"im/star.bmp",
			"im/levin.bmp",
			"im/zhou.bmp",
			"im/veeraraghavan.bmp"	
			};
	
	if(kernelType==0)
		dofFilter->useKernel = 0;
	else {
		dofFilter->loadKernelBMP(files[kernelType-1]);
		dofFilter->useKernel = 1;
	}
		
}

void boostBokehCB( int nil=0 )
{
	dofFilter->boostBokeh = boostBokeh;
}

void createGui( int windowId )
{
	GLUI *glui = GLUI_Master.create_glui("GUI");
	guiWindow = glui;

	glui->add_checkbox( "Use DOF Filter", &useDof );
	glui->add_checkbox( "Use texture", &useTex );

	GLUI_Panel *p = glui->add_panel("Lens type");
		glui->add_spinner_to_panel( p, "Focal Length:", GLUI_SPINNER_FLOAT, &focalLen );
		glui->add_spinner_to_panel( p, "Aperture:", GLUI_SPINNER_FLOAT, &aperture );
		//glui->add_spinner_to_panel( p, "Distance:", GLUI_SPINNER_FLOAT, &sensorDist );
	
	p = glui->add_panel("Lens control");
		glui->add_spinner_to_panel( p, "Focus:", GLUI_SPINNER_FLOAT, &focus );
	
	p = glui->add_panel("Bokeh");
		glui->add_checkbox_to_panel(p, "Bokeh Boost", &boostBokeh, 0, boostBokehCB );
		GLUI_Listbox *kType = glui->add_listbox_to_panel(p,"Shape:", &kernelType, 0, kernelTypeCB);
			kType->add_item( 0, "None");
			kType->add_item( 1, "Inverse Gaussian");
			kType->add_item( 2, "Dirty Lens");
			kType->add_item( 3, "Lens Diaphragm");
			kType->add_item( 4, "Heart");
			kType->add_item( 5, "Star");
			kType->add_item( 6, "Levin's C.A.");
			kType->add_item( 7, "Zhou's C.A.");
			kType->add_item( 8, "Veeraraghavan");
	
				
	glui->add_button( "Apply", 0, apply );
	
	glui->set_main_gfx_window( windowId );
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB
						| GLUT_ACCUM | GLUT_DEPTH); 
	glutInitWindowSize (WIDTH, HEIGHT);
	mainWindow = glutCreateWindow (argv[0]);
	
	init();
	
	parseOptions(argc, argv);
	
	createGui(mainWindow);
	
	display();

	glutReshapeFunc(reshape);
	//GLUI_Master.set_glutIdleFunc(display);
	glutSpecialFunc(specialFunc);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouseFunc);
	glutMainLoop();
	return 0;
}
