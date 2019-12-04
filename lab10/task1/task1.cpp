#include<functional>
#include<vector>
#include<iostream>
#include<random>

#include "GL/glew.h"
#include <GL/freeglut.h>

using namespace std;

double rotate_x = 0;
double rotate_y = 0;
double rotate_z = 0;

static int w = 0, h = 0;

vector<function<void(void)>> funs;
int numfun = 0;

vector<double> curr_color;

vector<double> rand_color()
{
	vector<double> v;
	for (int i = 0; i < 12; ++i)
	{
		int r = rand() % 1000;
		v.push_back((double)r / 1000);
	}
	return v;
}

void renderRectangle()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glRotatef(rotate_x, 1.0, 0.0, 0.0);
	glRotatef(rotate_y, 0.0, 1.0, 0.0);
	glRotatef(rotate_z, 0.0, 0.0, 1.0);
	glBegin(GL_QUADS);
	glColor3f(curr_color[0], curr_color[1], curr_color[2]); glVertex2f(-0.5f, -0.5f);
	glColor3f(curr_color[3], curr_color[4], curr_color[5]); glVertex2f(-0.5f, 0.5f);
	glColor3f(curr_color[6], curr_color[7], curr_color[8]); glVertex2f(0.5f, 0.5f);
	glColor3f(curr_color[9], curr_color[10], curr_color[11]); glVertex2f(0.5f, -0.5f);
	glEnd();
	glFlush(); 
	glutSwapBuffers();
}

void renderTriangle()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glRotatef(rotate_x, 1.0, 0.0, 0.0);
	glRotatef(rotate_y, 0.0, 1.0, 0.0);
	glRotatef(rotate_z, 0.0, 0.0, 1.0);
	glBegin(GL_TRIANGLES);
	glColor3f(curr_color[0], curr_color[1], curr_color[2]); glVertex2f(-0.5f, -0.5f);
	glColor3f(curr_color[3], curr_color[4], curr_color[5]); glVertex2f(-0.5f, 0.5f);
	glColor3f(curr_color[6], curr_color[7], curr_color[8]); glVertex2f(0.5f, 0.5f);

	glEnd();
	glFlush(); 
	glutSwapBuffers();
}


void callFun()
{
	if (funs.size() > 0)
	{
		funs[numfun % funs.size()]();
	}
}

void specialKeys(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP: rotate_x += 5; break;
	case GLUT_KEY_DOWN: rotate_x -= 5; break;
	case GLUT_KEY_RIGHT: rotate_y += 5; break;
	case GLUT_KEY_LEFT: rotate_y -= 5; break;
	case GLUT_KEY_PAGE_UP: rotate_z += 5; break;
	case GLUT_KEY_PAGE_DOWN: rotate_z -= 5; break;
	}
	glutPostRedisplay();
}

void specialMouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		++numfun;
		curr_color = rand_color();
	}
}


void Reshape(int width, int height)
{
	w = width; h = height;
}


int main(int argc, char** argv)
{
	funs.push_back(renderRectangle);
	funs.push_back(renderTriangle);
	curr_color = rand_color();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(800, 600);                    // window size
	glutInitWindowPosition(100, 100);                // distance from the top-left screen
	glutCreateWindow("Task 1");    // message displayed on top bar window
	glutDisplayFunc(callFun);
	glutSpecialFunc(specialKeys);
	glutMouseFunc(specialMouse);
	glutMainLoop();
	return 0;

}



