#include <functional>
#include <vector>
#include <iostream>
#include <random>
#include <fstream>
#include <string>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "glm/glm.hpp"
#include "SOIL.h"
#include "objloader.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp> 
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>

using namespace std;

int w = 0, h = 0;

#define INDEX_BUFFER	0
#define POS_VB			1
#define NORMAL_VB		2

GLuint m_VAO;
GLuint m_Buffers[4];

GLuint Program;

GLint a_coord;
GLint a_clr;
GLint a_transform_model;
GLint a_transform_viewProjection;
GLint a_transform_normal;
GLint a_transform_viewPosition;

size_t indices_size;

glm::vec3 eye;

float rotateX = 0;
float rotateY = 0;
float scaleX = 1;
float scaleY = 1;
glm::mat4 Matrix_projection;

float colors[8][4] = {
	{ 1.0, 1.0, 1.0, 1.0 },
	{ 0.0, 1.0, 1.0, 1.0 },
	{ 1.0, 0.0, 1.0, 1.0 },
	{ 0.0, 0.0, 1.0, 1.0 },
	{ 1.0, 1.0, 0.0, 1.0 },
	{ 0.0, 1.0, 0.0, 1.0 },
	{ 1.0, 0.0, 0.0, 1.0 },
	{ 0.0, 0.0, 0.0, 1.0 }
};

void shaderLog(unsigned int shader)
{
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(shader, sizeof(InfoLog), NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", shader, InfoLog);
	}
}

void initGL()
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(1, 1, 1, 1);
}

void checkOpenGLerror()
{
	GLenum errCode;
	if ((errCode = glGetError()) != GL_NO_ERROR)
		std::cout << "OpenGl error! - " << gluErrorString(errCode);
}

std::string readShader(std::string filename)
{
	std::ifstream file(filename);
	std::string line, res = "";
	while (std::getline(file, line))
	{
		res += line + '\n';
	}
	return res;
}

void initShader()
{
	std::string vs = readShader("vsShader.txt");
	std::string fs = readShader("fsShader.txt");
	const char* vsSource = vs.data();
	const char* fsSource = fs.data();

	Program = glCreateProgram();

	GLuint vShader, fShader;
	vShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vShader, 1, &vsSource, NULL);
	glCompileShader(vShader);
	shaderLog(vShader);
	fShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fShader, 1, &fsSource, NULL);
	glCompileShader(fShader);
	shaderLog(fShader);

	glAttachShader(Program, vShader);
	glAttachShader(Program, fShader);
	glLinkProgram(Program);

	GLint success;
	glGetProgramiv(Program, GL_LINK_STATUS, &success);
	if (success == 0) {
		GLchar ErrorLog[1024];
		glGetProgramInfoLog(Program, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
	}

	const char* attr_name = "coord";
	a_coord = glGetAttribLocation(Program, attr_name);

	attr_name = "clr";
	a_clr = glGetAttribLocation(Program, attr_name);



	const char* unif_name = "transform_model";
	a_transform_model = glGetUniformLocation(Program, unif_name);

	unif_name = "transform_viewProjection";
	a_transform_viewProjection = glGetUniformLocation(Program, unif_name);

	unif_name = "transform_normal";
	a_transform_normal = glGetUniformLocation(Program, unif_name);

	unif_name = "transform_viewPosition";
	a_transform_viewPosition = glGetUniformLocation(Program, unif_name);

	checkOpenGLerror();
}

double DRand(double a, double b) {
	return a + std::fmod(static_cast<double>(std::rand()), b - a);
}

void initVAO()
{
	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> texcoords;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ("african_head.obj", vertices, texcoords, normals);
	printf("vertices: %d\ntexcoords: %d\nnormals: %d\n", vertices.size(), texcoords.size(), normals.size());

	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_texcoords;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, texcoords, normals, indices, indexed_vertices, indexed_texcoords, indexed_normals);
	printf("indexed_vertices: %d\nindexed_texcoords: %d\nindexed_normals: %d\nindices: %d\n", indexed_vertices.size(), indexed_texcoords.size(), indexed_normals.size(), indices.size());

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);
	glGenBuffers(3, m_Buffers);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * indexed_vertices.size(), &indexed_vertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(a_coord);
	glVertexAttribPointer(a_coord, 3, GL_FLOAT, GL_FALSE, 0, 0);

	std::vector<glm::vec4> colors;
	for (int i = 0; i < indexed_vertices.size(); ++i)
		colors.push_back(glm::vec4(static_cast <float> (rand()) / static_cast <float> (RAND_MAX), static_cast <float> (rand()) / static_cast <float> (RAND_MAX), static_cast <float> (rand()) / static_cast <float> (RAND_MAX), 0.3));

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * indexed_vertices.size(), &colors[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(a_clr);
	glVertexAttribPointer(a_clr, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
	indices_size = indices.size();

	glBindVertexArray(0);


	checkOpenGLerror();
}

void freeShader()
{
	glUseProgram(0);
	glDeleteProgram(Program);
}

void freeVAO()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(4, m_Buffers);
	glDeleteVertexArrays(1, &m_VAO);
}


void resizeWindow(int x, int y)
{
	if (y == 0 || x == 0) return;

	w = x;
	h = y;
	glViewport(0, 0, w, h);

	Matrix_projection = glm::perspective(80.0f, (float)w / h, 0.01f, 200.0f);
	eye = { 1.5,0,0 };
	glm::vec3 center = { 0,0,0 };
	glm::vec3 up = { 0,0,-1 };

	Matrix_projection *= glm::lookAt(eye, center, up);
}

void render()
{
	glMatrixMode(GL_MODELVIEW);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(Program);
	
	// vert shader part, never changes
	glm::mat4 transform_model = glm::rotate(rotateY, glm::vec3(0, 0, 1));
	transform_model = glm::rotate(transform_model, rotateX, glm::vec3(1, 0, 0));

	glUniformMatrix4fv(a_transform_model, 1, false, glm::value_ptr(transform_model));
	glUniformMatrix4fv(a_transform_viewProjection, 1, false, glm::value_ptr(Matrix_projection));
	glUniform3fv(a_transform_viewPosition, 1, glm::value_ptr(eye));

	glm::mat3 transform_normal = glm::inverseTranspose(glm::mat3(transform_model));
	glUniformMatrix3fv(a_transform_normal, 1, false, glm::value_ptr(transform_normal));

	glBindVertexArray(m_VAO);

	glDrawElements(GL_TRIANGLES,
		indices_size,
		GL_UNSIGNED_SHORT,
		(void*)0);

	glBindVertexArray(0);

	glUseProgram(0);
	checkOpenGLerror();
	glutSwapBuffers();
}


void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
		rotateX += 0.1;
		break;
	case 's':
		rotateX -= 0.1;
		break;
	case 'a':
		rotateY -= 0.1;
		break;
	case 'd':
		rotateY += 0.1;
		break;
	default:
		break;
	}

	glutPostRedisplay();
}


int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE);
	glutInitWindowSize(600, 600);
	glutCreateWindow("Simple shaders");
	GLenum glew_status = glewInit();
	if (GLEW_OK != glew_status)
	{
		std::cout << "Error: " << glewGetErrorString(glew_status) << "\n";
		return 1;
	}
	if (!GLEW_VERSION_2_0)
	{
		std::cout << "No support for OpenGL 2.0 found\n";
		return 1;
	}
	initGL();
	initShader();
	initVAO();
	glutReshapeFunc(resizeWindow);
	glutDisplayFunc(render);
	glutKeyboardFunc(keyboard);
	glutMainLoop();
	freeShader();
	freeVAO();
}
