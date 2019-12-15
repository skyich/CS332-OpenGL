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

#define INDEX_BUFFER	0
#define POS_VB			1
#define NORMAL_VB		2
#define TEXCOORD_VB		3

GLuint m_VAO;
GLuint m_Buffers[4];

objl::Loader ldr;

GLuint Program;

GLint Attrib_vertex;
GLint Attrib_vertex1;
GLint Attrib_vertex2;

GLint Unif_light;

GLint Unif_affine;

GLuint texture;

float light[3] = { 0.0f, 3.0f, -10.0f };
float affineMatr[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };

struct vertex
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat tx;
	GLfloat ty;
};

void shaderLog(unsigned int shader)
{
	int infologLen = 0;
	int charsWritten = 0;
	char* infoLog;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);
	if (infologLen > 1)
	{
		infoLog = new char[infologLen];
		if (infoLog == NULL)
		{
			std::cout << "ERROR: Could not allocate InfoLog buffer\n";
			exit(1);
		}
		glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog);
		std::cout << "InfoLog: " << infoLog << "\n\n\n";
		delete[] infoLog;
	}
}

void initGL()
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.5, 0.3, 0.2, 0);
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

	//const char* vsSource =
	//	"#version 330 core\n"
	//	"attribute vec3 coord;\n"
	//	"attribute vec3 norm;\n"
	//	"attribute vec2 texCoord;\n"
	//	"uniform mat3 affine;\n"
	//	"uniform vec3 lightPoint;\n"
	//	"out vec3 Norm;\n"
	//	"out vec2 TexCoord;\n"
	//	"out vec3 LightVec;\n"
	//	"void main() {\n"
	//	"vec3 tmp = affine * coord;\n"
	//	"gl_Position = vec4(tmp, (0.1 * tmp.z + 1.0) * 1.0);\n"
	//	"TexCoord = -texCoord;\n"
	//	"Norm = norm;\n"
	//	"LightVec = tmp - lightPoint;\n"
	//	"}\n";
	//const char* fsSource =
	//	"#version 330 core\n"
	//	"in vec3 Norm;\n"
	//	"in vec2 TexCoord;\n"
	//	"in vec3 LightVec;\n"
	//	"uniform sampler2D ourTexture;\n"
	//	"out vec4 color;\n"
	//	"void main() {\n"
	//	"float cosTheta = clamp( -dot( Norm, LightVec ), 0, 1);\n"
	//	"color = texture(ourTexture, TexCoord) * (cosTheta + 0.2);\n"
	//	"}\n";

	std::string vs = readShader("vsShader.txt");
	std::string fs = readShader("fsShader.txt");
	const char* vsSource = vs.data();
	const char* fsSource = fs.data();

	GLuint vShader, fShader;
	vShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vShader, 1, &vsSource, NULL);
	glCompileShader(vShader);
	std::cout << "vertex shader \n";
	shaderLog(vShader);
	fShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fShader, 1, &fsSource, NULL);
	glCompileShader(fShader);
	std::cout << "fragment shader \n";
	shaderLog(fShader);
	Program = glCreateProgram();
	glAttachShader(Program, vShader);
	glAttachShader(Program, fShader);
	glLinkProgram(Program);

	int link_ok;
	glGetProgramiv(Program, GL_LINK_STATUS, &link_ok);
	if (!link_ok)
	{
		std::cout << "error attach shaders \n";
		return;
	}

	const char* attr_name0 = "coord";
	Attrib_vertex = glGetAttribLocation(Program, attr_name0);
	if (Attrib_vertex == -1)
	{
		std::cout << "could not bind attrib " << attr_name0 << std::endl;
		return;
	}

	const char* attr_name1 = "texCoord";
	Attrib_vertex1 = glGetAttribLocation(Program, attr_name1);
	if (Attrib_vertex1 == -1)
	{
		std::cout << "could not bind uniform " << attr_name1 << std::endl;
		return;
	}

	const char* attr_name2 = "norm";
	Attrib_vertex2 = glGetAttribLocation(Program, attr_name2);
	if (Attrib_vertex2 == -1)
	{
		std::cout << "could not bind uniform " << attr_name2 << std::endl;
		return;
	}

	const char* unif_name0 = "lightPoint";
	Unif_light = glGetUniformLocation(Program, unif_name0);
	if (Unif_light == -1)
	{
		std::cout << "could not bind uniform " << unif_name0 << std::endl;
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

void initVAO()
{
	ldr.LoadFile("african_head.obj");
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);
	glGenBuffers(4, m_Buffers);

	std::vector<objl::Vector3> Positions;
	std::vector<objl::Vector3> Normals;
	std::vector<objl::Vector2> TexCoords;
	std::vector<unsigned int> Indices;

	unsigned int NumVertices = ldr.LoadedMeshes[0].Vertices.size();
	unsigned int NumIndices = ldr.LoadedMeshes[0].Indices.size();

	Positions.reserve(NumVertices);
	Normals.reserve(NumVertices);
	TexCoords.reserve(NumVertices);
	Indices.reserve(NumIndices);

	for (unsigned int i = 0; i < NumVertices; i++) {
		Positions.push_back(ldr.LoadedMeshes[0].Vertices[i].Position);
		Normals.push_back(ldr.LoadedMeshes[0].Vertices[i].Normal);
		TexCoords.push_back(ldr.LoadedMeshes[0].Vertices[i].TextureCoordinate);
	}

	for (unsigned int i = 0; i < NumIndices; i++) {
		Indices.push_back(ldr.LoadedMeshes[0].Indices[i]);
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Positions[0]) * Positions.size(), &Positions[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(Attrib_vertex);
	glVertexAttribPointer(Attrib_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TEXCOORD_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoords[0]) * TexCoords.size(), &TexCoords[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(Attrib_vertex1);
	glVertexAttribPointer(Attrib_vertex1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Normals[0]) * Normals.size(), &Normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(Attrib_vertex2);
	glVertexAttribPointer(Attrib_vertex2, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * Indices.size(), &Indices[0], GL_STATIC_DRAW);

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

void initTexture()
{
	int width, height;
	unsigned char* image = SOIL_load_image("african_head_SSS.jpg", &width, &height, 0, SOIL_LOAD_RGB);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void resizeWindow(int width, int height)
{
	glViewport(0, 0, width, height);
}

void render()
{
	//glClear(GL_COLOR_BUFFER_BIT);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glUseProgram(Program);
	//static float red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	//static float green[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
	//static float blue[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	//glUniform4fv(Unif_color, 1, red);
	//glUniformMatrix3fv(Unif_affine, 1, GL_FALSE, affineMatr);
	//glEnableVertexAttribArray(Attrib_vertex);
	//glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glVertexAttribPointer(Attrib_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glDrawArrays(GL_TRIANGLES, 0, 12);
	//glUniform4fv(Unif_color, 1, green);
	//glDrawArrays(GL_TRIANGLES, 12, 12);
	//glUniform4fv(Unif_color, 1, blue);
	//glDrawArrays(GL_TRIANGLES, 24, 12);
	//glDisableVertexAttribArray(Attrib_vertex);
	//glUseProgram(0);
	//checkOpenGLerror();
	//glutSwapBuffers();

	glUseProgram(Program);
	glUniform3fv(Unif_light, 1, light);
	glUniformMatrix3fv(Unif_affine, 1, GL_FALSE, affineMatr);

	glBindVertexArray(m_VAO);
	glBindTexture(GL_TEXTURE_2D, texture);
	glEnable(GL_TEXTURE_2D);
	glDrawElements(GL_TRIANGLES,
		ldr.LoadedMeshes[0].Indices.size(),
		GL_UNSIGNED_INT,
		NULL);
	glDisable(GL_TEXTURE_2D);
	glBindVertexArray(0);

	glUseProgram(0);
	checkOpenGLerror();
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

void Keyboard(unsigned char key, int x, int y)
{
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

void SpecialKeys(int key, int x, int y)
{
	static float step = 0.1;
	float X, Z;
	switch (key)
	{
	case GLUT_KEY_UP:
		light[1] += step;
		break;
	case GLUT_KEY_DOWN:
		light[1] -= step;
		break;
	case GLUT_KEY_LEFT:
		X = std::cos(0.08727) * light[0] + std::sin(0.08727) * light[2];
		Z = std::cos(0.08727) * light[2] - std::sin(0.08727) * light[0];
		light[0] = X;
		light[2] = Z;
		break;
	case GLUT_KEY_RIGHT:
		X = std::cos(0.08727) * light[0] - std::sin(0.08727) * light[2];
		Z = std::cos(0.08727) * light[2] + std::sin(0.08727) * light[0];
		light[0] = X;
		light[2] = Z;
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
	initTexture();
	initVAO();
	glutReshapeFunc(resizeWindow);
	glutDisplayFunc(render);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeys);
	glutMainLoop();
	freeShader();
	freeVAO();
}
