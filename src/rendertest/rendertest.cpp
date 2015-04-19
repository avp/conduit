#include "rendertest.hpp"

// angle of rotation for the camera direction
float angle = 0.0f;
// actual vector representing the camera's direction
float lx=0.0f,lz=-1.0f;
// XZ position of the camera
float x=0.0f, z=0.0f;
// the key states. These variables will be zero
//when no key is being presses
float deltaAngle = 0.0f;
float deltaMove = 0;

GLuint ourTexture;

void changeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0)
		h = 1;
	float ratio =  w * 1.0 / h;

	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);

	// Reset Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(45.0f, ratio, 0.1f, 100.0f);

	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW);
}

void drawSnowMan() {

	glColor3f(1.0f, 1.0f, 1.0f);

// Draw Body

	glTranslatef(0.0f ,0.75f, 0.0f);
	glutSolidSphere(0.75f,20,20);

// Draw Head
	glTranslatef(0.0f, 1.0f, 0.0f);
	glutSolidSphere(0.25f,20,20);
	// glutSolidCylinder(0.25f, 0.25f, 20, 20);

// Draw Eyes
	glPushMatrix();
	glColor3f(0.0f,0.0f,0.0f);
	glTranslatef(0.05f, 0.10f, 0.18f);
	glutSolidSphere(0.05f,10,10);
	glTranslatef(-0.1f, 0.0f, 0.0f);
	glutSolidSphere(0.05f,10,10);
	glPopMatrix();

// Draw Nose
	glColor3f(1.0f, 0.5f , 0.5f);
	glRotatef(0.0f,1.0f, 0.0f, 0.0f);
	glutSolidCone(0.08f,0.5f,10,2);
}

void computePos(float deltaMove) {

	x += deltaMove * lx * 0.1f;
	z += deltaMove * lz * 0.1f;
}

void computeDir(float deltaAngle) {

	angle += deltaAngle;
	lx = sin(angle);
	lz = -cos(angle);
}

GLUquadric* qobj;
void renderScene(void) {

	if (deltaMove)
		computePos(deltaMove);
	if (deltaAngle)
		computeDir(deltaAngle);

	// Clear Color and Depth Buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Reset transformations
	glLoadIdentity();
	// Set the camera
	gluLookAt(	x, 1.0f, z,
				x+lx, 1.0f,  z+lz,
				0.0f, 1.0f,  0.0f);

// Draw ground

	glColor3f(1,1,1);
	glPushMatrix();
	glRotatef(90.0,1.0,0.0,0.0);
	glTranslatef(0,0,-5);
	gluQuadricTexture(qobj, true);
	glBindTexture(GL_TEXTURE_2D, ourTexture);
	gluCylinder(qobj, 5.0, 5.0, 10.0, 20, 20);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();

	// glColor3f(0.9f, 0.9f, 0.9f);

	// glBegin(GL_QUADS);
	// 	glVertex3f(-100.0f, 0.0f, -100.0f);
	// 	glVertex3f(-100.0f, 0.0f,  100.0f);
	// 	glVertex3f( 100.0f, 0.0f,  100.0f);
	// 	glVertex3f( 100.0f, 0.0f, -100.0f);
	// glEnd();

// Draw 36 SnowMen
	for(int i = -3; i < 3; i++)
		for(int j=-3; j < 3; j++) {
			if (i == 0 && j == 0) continue;
			glPushMatrix();
			glTranslatef(i*10.0,0,j * 10.0);
			drawSnowMan();
			glPopMatrix();
		}

	glutSwapBuffers();
}


void pressKey(int key, int xx, int yy) {

	switch (key) {
		case GLUT_KEY_LEFT : deltaAngle = -0.01f; break;
		case GLUT_KEY_RIGHT : deltaAngle = 0.01f; break;
		case GLUT_KEY_UP : deltaMove = 0.5f; break;
		case GLUT_KEY_DOWN : deltaMove = -0.5f; break;
	}
}

void releaseKey(int key, int x, int y) {

	switch (key) {
		case GLUT_KEY_LEFT :
		case GLUT_KEY_RIGHT : deltaAngle = 0.0f;break;
		case GLUT_KEY_UP :
		case GLUT_KEY_DOWN : deltaMove = 0;break;
	}
}

GLuint loadTexture(const cv::Mat& image) {
  int height = image.rows;
  int width = image.cols;

  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  // build our texture
  // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
  //     GL_BGR, GL_UNSIGNED_BYTE, image.ptr());
  gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height,
      GL_BGR, GL_UNSIGNED_BYTE, image.ptr());

  glBindTexture(GL_TEXTURE_2D, 0);
  return texture;
}

int RenderTest::renderTest(int argc, char **argv, cv::Mat& image) {
	using std::cout;
	using std::cerr;
	using std::endl;

	if (!ovr_Initialize()) {
		cerr << "Failed to initialize OVR..." << endl;
	}

	ovrHmd hmd = ovrHmd_Create(0);
	if (!hmd) {
	  cout << "Failed to open Oculus device, using debug mode." << endl;
	  hmd = ovrHmd_CreateDebug(ovrHmd_DK2);
	  if (!hmd) {
	    cerr << "Failed to debug mode HMD." << endl;
	    exit(1);
	  }
	}

	cout << "HMD Initialized: " << hmd->ProductName << endl;

	// init GLUT and create window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(800,800);
	glutCreateWindow("Lighthouse3D - GLUT Tutorial");

	// register callbacks
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	glutIdleFunc(renderScene);

	glutSpecialFunc(pressKey);

	// here are the new entries
	glutIgnoreKeyRepeat(1);
	glutSpecialUpFunc(releaseKey);

	// OpenGL init
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	// Custom cylinder code
	qobj = gluNewQuadric();
	gluQuadricNormals(qobj, GLU_SMOOTH);
	ourTexture = loadTexture(image);

	// enter GLUT event processing cycle
	glutMainLoop();

	// TODO: Where to put these?
	ovrHmd_Destroy(hmd);
    ovr_Shutdown();

	return 1;
}
