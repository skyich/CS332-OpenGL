#include <functional>
#include <vector>
#include <iostream>
#include <random>

#include "GL/glew.h"
#include <GL/freeglut.h>

using namespace std;

static int w = 800, h = 600;

vector<double> rs;
int projection = 0;

double fRand(double fMin, double fMax)
{
	double f = (double)rand() / RAND_MAX;
	return fMin + f * (fMax - fMin);
}


void Init(void) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHT0);
	//glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	//glEnable(GL_NORMALIZE);
}

void randSize() {
	for (int i = 0; i < 100; ++i)
		rs.push_back(fRand(0.6, 1));
}

int randDist() {
	int r = rand() % 1000;
	if (r < 500)
		return 3;
	else
		return 4;
}

void Reshape(int width, int height) {
	width = w; height = h;
}

void modelTree(int x, int y, int z, double cf = 1) {
	glTranslated(x, y, z);
	glRotated(65, -1.0, 0.0, 0.0);
	glColor3ub(89, 60, 31);
	glutSolidCylinder(0.2 * cf, 1 * cf, 50, 50);
	glTranslated(0, 0, 1 * cf);
	glColor3ub(11, 74, 27);
	glutSolidCone(1 * cf, 2 * cf, 50, 50);
	glTranslated(0, 0, 1.8 * cf);
	glutSolidCone(0.8 * cf, 1.8 * cf, 50, 50);
	glTranslated(0, 0, 1.5 * cf);
	glutSolidCone(0.5 * cf, 1.2 * cf, 50, 50);
}

void drawTree() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (!projection) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-10, 10, -10, 10, -10, 10);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
	else {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(65.0f, w / h, 0.1f, 1000.0f);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(0.0f, 0.0f, 15.0f, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	}

	glPushMatrix();
	
	int i = 0;
	for (int x = -8; x < 15; x += randDist())
		for (int y = -10; y < 15; y += randDist()) {
			modelTree(x, y, 0, rs[i++]);
			glPopMatrix();
			glPushMatrix();
		}
	glPopMatrix();
	glFlush();
	glutSwapBuffers();
}

void specialKeys(int key, int x, int y) {
	switch ((int)key) {
	case GLUT_KEY_SHIFT_L: projection = 0; break;
	case GLUT_KEY_CTRL_L: projection = 1; break;
	}
	glutPostRedisplay();
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(w, h);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Task 3");
	randSize();
	glutReshapeFunc(Reshape);
	glutDisplayFunc(drawTree);
	glutSpecialFunc(specialKeys);

	Init();
	
	glutMainLoop();
}


