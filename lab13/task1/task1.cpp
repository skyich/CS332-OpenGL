#include <functional>
#include <vector>
#include <iostream>
#include <random>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "SOIL.h"
#include "OBJ_Loader.h"

using namespace std;

static int w = 800, h = 600;

#define INDEX_BUFFER	0
#define POS_VB			1
#define NORMAL_VB		2

GLuint m_VAO;
GLuint m_Buffers[3];

objl::Loader loader;

GLuint Program;

GLint Attrib_vertex;
GLint Attrib_vertex2;
GLint Unif_affine;


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

float affineMatr[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };

void Init(void) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
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

void InitShader(void) {
	std::string vs = readShader("vsShaderP.txt");
	std::string fs = readShader("fsShaderP.txt");
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

	glUseProgram(Program);

	const char* attr_name0 = "coord";
	Attrib_vertex = glGetAttribLocation(Program, attr_name0);
	if (Attrib_vertex == -1)
	{
		std::cout << "could not bind attrib " << attr_name0 << std::endl;
		return;
	}

	const char* attr_name1 = "clr";
	Attrib_vertex2 = glGetAttribLocation(Program, attr_name1);
	if (Attrib_vertex2 == -1)
	{
		std::cout << "could not bind uniform " << attr_name1 << std::endl;
		return;
	}

	const char* unif_name1 = "affine";
	Unif_affine = glGetUniformLocation(Program, unif_name1);
	if (Unif_affine == -1)
	{
		std::cout << "could not bind uniform " << unif_name1 << std::endl;
		return;
	}
	checkOpenGLerror();
}

//https://triplepointfive.github.io/ogltutor/tutorials/tutorial32.html
void initVAO()
{
	loader.LoadFile("Bowl.obj");
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);
	glGenBuffers(3, m_Buffers);

	std::vector<objl::Vector3> Positions;
	std::vector<objl::Vector3> Normals;
	std::vector<unsigned int> Indices;

	unsigned int NumVertices = loader.LoadedMeshes[0].Vertices.size();
	unsigned int NumIndices = loader.LoadedMeshes[0].Indices.size();

	Positions.reserve(NumVertices);
	Normals.reserve(NumVertices);
	Indices.reserve(NumIndices);

	for (unsigned int i = 0; i < NumVertices; i++) {
		Positions.push_back(loader.LoadedMeshes[0].Vertices[i].Position);
		Normals.push_back(loader.LoadedMeshes[0].Vertices[i].Normal);
	}

	for (unsigned int i = 0; i < NumIndices; i++) {
		Indices.push_back(loader.LoadedMeshes[0].Indices[i]);
	}

	float* COLORS = new float[4 * Positions.size()];
	for (int i = 0; i < Positions.size(); ++i) {
		int r = std::rand() % 8;
		for (int j = 0; j < 4; ++j) {
			COLORS[i * 4 + j] = colors[r][j];
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Positions[0]) * Positions.size(), &Positions[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(Attrib_vertex);
	glVertexAttribPointer(Attrib_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * Positions.size(), COLORS, GL_STATIC_DRAW);
	glEnableVertexAttribArray(Attrib_vertex2);
	glVertexAttribPointer(Attrib_vertex2, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * Indices.size(), &Indices[0], GL_STATIC_DRAW);

	//glBindVertexArray(0);
	checkOpenGLerror();
}


void Reshape(int width, int height) {
	glViewport(0, 0, width, height);
}


void Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUniformMatrix3fv(Unif_affine, 1, GL_FALSE, affineMatr);
	glDrawElements(GL_TRIANGLES,
		loader.LoadedMeshes[0].Indices.size(),
		GL_UNSIGNED_INT,
		NULL);
	glutSwapBuffers();
}

void MatrProduct(float a[])
{
	float tmp[9];
	for (int i = 0; i < 9; ++i) tmp[i] = 0;

	for (int i = 0; i < 9; i += 3)
		for (int j = 0; j < 3; ++j)
			for (int k = 0; k < 3; ++k)
				tmp[i + j] += a[i + k] * affineMatr[3 * k + j];

	for (int i = 0; i < 9; ++i) affineMatr[i] = tmp[i];
}

void Keyboard(unsigned char key, int x, int y){
	float a[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
	switch (key)
	{
	case 'w':
		a[4] = std::cos(0.08727);
		a[5] = std::sin(0.08727);
		a[7] = -a[5];
		a[8] = a[4];
		break;
	case 's':
		a[4] = std::cos(0.08727);
		a[5] = -std::sin(0.08727);
		a[7] = -a[5];
		a[8] = a[4];
		break;
	case 'a':
		a[0] = std::cos(0.08727);
		a[2] = -std::sin(0.08727);
		a[6] = -a[2];
		a[8] = a[0];
		break;
	case 'd':
		a[0] = std::cos(0.08727);
		a[2] = std::sin(0.08727);
		a[6] = -a[2];
		a[8] = a[0];
		break;
	case 'q':
		a[0] = std::cos(0.08727);
		a[1] = std::sin(0.08727);
		a[3] = -a[1];
		a[4] = a[0];
		break;
	case 'e':
		a[0] = std::cos(0.08727);
		a[1] = -std::sin(0.08727);
		a[3] = -a[1];
		a[4] = a[0];
		break;
	case '+':
		a[0] = 1.1;
		a[4] = 1.1;
		a[8] = 1.1;
		break;
	case '-':
		a[0] = 1.0 / 1.1;
		a[4] = 1.0 / 1.1;
		a[8] = 1.0 / 1.1;
		break;
	}
	MatrProduct(a);
	glutPostRedisplay();
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(w, h);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Task 1");

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


	Init();
	InitShader();
	initVAO();

	glutReshapeFunc(Reshape);
	glutDisplayFunc(Render);
	glutKeyboardFunc(Keyboard);

	glutMainLoop();
}


