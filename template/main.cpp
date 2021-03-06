#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include "generalCuda.h"
// includes, system

#include <string.h>
#include <time.h>
// includes, GL
#include <GL/glew.h>

#include <GL/glut.h>

extern "C" void callMallocSystemMemory(ParticleSystem *sys,float **h_system, float **d_system);
extern "C" void callClearMemory(float *d_system, float *h_system);

//variables of particle system
ParticleSystem *sys = NULL;
float * state = NULL;
float *h_system =NULL;
float *d_system = NULL;
int force_mask=0;
////////////////////////////////////////////////////////////////////////////////
 unsigned int window_width = 512;
 unsigned int window_height = 512;
// constants

const unsigned int mesh_width = 256;
const unsigned int mesh_height = 256;

const float DEG2RAD=3.14159/180;

float anim = 0.0;

// mouse controls
int mouse_old_x, mouse_old_y;
int mouse_buttons = 0;
float rotate_x = 0.0, rotate_y = 0.0;
float translate_z = -50.0;
float translate_x = 0.0;
float translate_y = 0.0;

unsigned int timer = 0;

// Auto-Verification Code
int fpsCount = 0;        // FPS count for averaging
clock_t start_calc,end_calc;
unsigned int frameCount = 0;

#define MAX(a,b) ((a > b) ? a : b)
#define VCOLORING 0
#define DEFAULTCOLORING 1
////////////////////////////////////////////////////////////////////////////////
// declaration, forward
bool runTest(int argc, char** argv);
void cleanup();

// GL functionality
bool initGL(int *argc, char** argv);

// rendering callbacks
void display();
void keyboard(unsigned char key, int x, int y);
void specialKeyboard(int key, int, int);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void reshape(int x, int y);

//drawing functions
void drawCircle(float x, float y, float radius);
void drawSystem(ParticleSystem *p);
int typeOfBalls;
int typeOfColoring;
void createSystem();
void initMemoryForSystem();
////////////////////////////////////////////////////////////////////////////////
// Program main
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
//	char temp[256];

//    fpsLimit = frameCheckNumber;
 //   sprintf(temp, "Projekt GPCA B_4");
//	glutSetWindowTitle(temp);
	createSystem();
	initMemoryForSystem();

    runTest(argc, argv);
}

void initMemoryForSystem(){
}
void createSystem(){
	sys = //createParticleSystem(30,0.5f,2.0f);
	 createColisionTestingParticleSystem(500,0.2f,5.0f);//debug
	typeOfBalls = 
		GL_POLYGON;
		//GL_LINE_LOOP;
	typeOfColoring = 
		//VCOLORING;
		DEFAULTCOLORING;
	callMallocSystemMemory(sys,&h_system,&d_system);
}
////////////////////////////////////////////////////////////////////////////////
//! Initialize GL
////////////////////////////////////////////////////////////////////////////////
bool initGL(int *argc, char **argv)
{
    glutInit(argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(window_width, window_height);
    glutCreateWindow("Projekt GPCA B_4 glut");
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMotionFunc(motion);
	glutMouseFunc(mouse);
	glutSpecialFunc(specialKeyboard);
	glutReshapeFunc(reshape);


	// initialize necessary OpenGL extensions
    glewInit();
    if (! glewIsSupported("GL_VERSION_2_0 ")) {
        fprintf(stderr, "ERROR: Support for necessary OpenGL extensions missing.");
        fflush(stderr);
        return false;
    }

    // default initialization
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glDisable(GL_DEPTH_TEST);

    // viewport
    glViewport(0, 0, window_width, window_height);

    // projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)window_width / (GLfloat) window_height, 0.1, 1000.0);


    return true;
}


bool runTest(int argc, char** argv)
{
	if ( initGL(&argc, argv)==false) {
		return false;
	}
	
	atexit(cleanup);
	glutMainLoop();
	return true;
}


void reshape(int x, int y)
{
	 int viewport_side = 0;
    
    //  Viewport x and y positions 
    int viewport_x = 0, viewport_y = 0;
    
    //  Calculate viewport side
    viewport_side = (x > y) ? y : x;
    
    //  Calculate viewport position (Center viewport)
    viewport_x = (x - viewport_side) / 2;
    viewport_y = (y - viewport_side) / 2;
    
    
    //  Set subwindow width and height
	window_width = x;
	window_height = y;

    //  Preserve aspect ratio
    glViewport (viewport_x, viewport_y, viewport_side, viewport_side);
   
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(translate_x, translate_y, translate_z);
    glRotatef(rotate_x, 1.0, 0.0, 0.0);
    glRotatef(rotate_y, 0.0, 1.0, 0.0);
	gluPerspective(60.0, (GLfloat)x / (GLfloat) y, 0.1, 1000.0);
}

float computeFPS(){
		end_calc=clock();
		int milisec = (end_calc-start_calc)*1000/(CLOCKS_PER_SEC);
		float fps = 1000.000f/ (float)milisec;
		//fpsCount = fps;
		start_calc=clock();
		//frameCount=0;
		return fps;

}
void display()
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set view matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(translate_x, translate_y, translate_z);
    glRotatef(rotate_x, 1.0, 0.0, 0.0);
    glRotatef(rotate_y, 0.0, 1.0, 0.0);

   float fps_no = computeFPS();
   char fps[256];

   if(frameCount>3){
   sprintf(fps,"FPS: %.3f",fps_no);
   glutSetWindowTitle(fps);
   frameCount=0;
   }else{
	   frameCount++;
   }
	//drawCircle(0.0f,0, 2.0f);
	EulerStep(sys,1.000f/(fps_no),force_mask,h_system,d_system );
	drawSystem(sys);

    glutSwapBuffers();
    glutPostRedisplay();

    anim += 0.01;

  
}

void cleanup()
{
  if(state!=NULL) free(state);
  if(sys!=NULL) cleanupParticleSystem(sys);
  callClearMemory(d_system,h_system);

}

void specialKeyboard(int key, int, int){
	switch(key){
	case(GLUT_KEY_LEFT):
		translate_x-=0.5;
	break;

	case(GLUT_KEY_RIGHT):
		translate_x+=0.5;
	break;
	
	case(GLUT_KEY_UP):
		translate_y+=0.5;
	break;

	case(GLUT_KEY_DOWN):
		translate_y-=0.5;
	break;

	}
}
////////////////////////////////////////////////////////////////////////////////
//! Keyboard events handler
////////////////////////////////////////////////////////////////////////////////
void keyboard(unsigned char key, int /*x*/, int /*y*/)
{
    switch(key) {
    case(27) :
        exit(0);
    break;
	
	case ('c'):
		if(force_mask&CUDASIMULATION) {
			force_mask-=CUDASIMULATION;
			printf("\n CUDA Simulation OFF");
		}else{
			force_mask+=CUDASIMULATION;
			printf("\n CUDA Simulation ON");
		}
	break;

	case ('w'):
		translate_z+=0.5;
	break;
	
	case ('s'):
		translate_z-=0.5;
	break;

	case ('g'):
		if(force_mask&GRAVITY){
			force_mask-=GRAVITY;
			printf("\n Gravity force OFF");
		}else{
			force_mask+=GRAVITY;
			printf("\n Gravity force ON");
		}
	break;
    
    }
						
}

////////////////////////////////////////////////////////////////////////////////
//! Mouse event handlers
////////////////////////////////////////////////////////////////////////////////
void mouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN) {
        mouse_buttons |= 1<<button;
    } else if (state == GLUT_UP) {
        mouse_buttons = 0;
    }

    mouse_old_x = x;
    mouse_old_y = y;
    glutPostRedisplay();
}

void motion(int x, int y)
{
    float dx, dy;
    dx = x - mouse_old_x;
    dy = y - mouse_old_y;

    if (mouse_buttons & 1) {
        rotate_x += dy * 0.2;
        rotate_y += dx * 0.2;
    } else if (mouse_buttons & 4) {
        translate_z += dy * 0.01;
    }

    mouse_old_x = x;
    mouse_old_y = y;
}

void drawCircle(float x, float y, float radius)
{
	glBegin(typeOfBalls);
	for (int i = 0; i < 360; i++)
	{
		float degInRad = i * DEG2RAD;
		glVertex2f(x+cos(degInRad)*radius, y+sin(degInRad)*radius);
	}
	glEnd();
}

void drawSystem(ParticleSystem *p){
	for(int i =0; i<p->n;i++){
		//dodać przesunięcie osi układu wyświetlania 
		
		if(typeOfColoring==VCOLORING){
		if(valueOfVector(p->p[i].v)>0.0f) glColor3f(1.0f,1.0f,1.0f);

		if(valueOfVector(p->p[i].v)>5.0f) glColor3f(1.0f,0.0f,0.0f);
		
		if(valueOfVector(p->p[i].v)>10.0f) glColor3f(0.0f,1.0f,0.0f);
		
		if(valueOfVector(p->p[i].v)>15.0f) glColor3f(1.0f,0.0f,1.0f);
		
		if(valueOfVector(p->p[i].v)>20.0f) glColor3f(0.0f,0.0f,1.0f);
		}else if(typeOfColoring==DEFAULTCOLORING){
			glColor3f(1.0f,1.0f,1.0f);
		}
		/*glColor3f(1.0f,1.0f,1.0f);

		if(i>p->n/5) glColor3f(1.0f,0.0f,0.0f);
		
		if(i>2*p->n/5) glColor3f(0.0f,1.0f,0.0f);
		
		if(i>3*p->n/5) glColor3f(1.0f,0.0f,1.0f);
		
		if(i>4*p->n/5) glColor3f(0.0f,0.0f,1.0f);*/

		drawCircle(p->p[i].x[0],p->p[i].x[1],p->p[i].R);
	}
	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_LINE_LOOP);
	glVertex2f(-20.0f,-20.0f);
	glVertex2f(20.0f,-20.0f);
	glVertex2f(20.0f,20.0f);
	glVertex2f(-20.0f,20.0f);
	glEnd();

}
