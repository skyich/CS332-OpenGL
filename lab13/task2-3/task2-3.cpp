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
#define TEXCOORD_VB		3

GLuint m_VAO;
GLuint m_Buffers[4];


GLuint Program;

GLint a_coord;
GLint a_norm;
GLint a_texcoord;
GLint a_transform_model;
GLint a_transform_viewProjection;
GLint a_transform_normal;
GLint a_transform_viewPosition;
GLint a_light_position;
GLint a_light_ambient;
GLint a_light_diffuse;
GLint a_light_specular;
GLint a_light_attenuation;
GLint a_material_texture;
GLint a_material_ambient;
GLint a_material_diffuse;
GLint a_material_specular;
GLint a_material_emission;
GLint a_material_shininess;
GLint a_use_texture;


GLuint texture;
size_t indices_size;

bool use_texture = true;

glm::vec3 eye;
glm::vec4 light_position, light_ambient, light_diffuse, light_specular;
glm::vec3 light_attenuation;

glm::vec4 material_ambient, material_diffuse, material_specular, material_emission;
GLfloat material_shininess;

float lightZ = 0;
float lightX = 2;
float lightAngle = 0;
float rotateX = 0;
float rotateY = 0;
float scaleX = 1;
float scaleY = 1;
glm::mat4 Matrix_projection;

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

void set_light() {

	light_position = { 0,0,0,0 };
	light_ambient = { 1,1,1,1 };
	light_diffuse = { 1,1,1,1 };
	light_specular = { 1,1,1,1 };
	light_attenuation = { 0,0,0 };
}

void set_material() {

	material_emission = { 0,0,0,1 };
	material_ambient = { 0.25, 0.25, 0.25, 0 };
	material_diffuse = { 0.3, 0.2, 0.25, 0 };
	material_specular = { 0.027, 0.027, 0.027, 0 };
	material_shininess = 10;
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
	std::string vs = readShader("vsPhong.txt");
	std::string fs = readShader("fsPhong.txt");
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

	attr_name = "norm";
	a_norm = glGetAttribLocation(Program, attr_name);

	attr_name = "texcoord";
	a_texcoord = glGetAttribLocation(Program, attr_name);

	const char* unif_name = "transform_model";
	a_transform_model = glGetUniformLocation(Program, unif_name);

	unif_name = "transform_viewProjection";
	a_transform_viewProjection = glGetUniformLocation(Program, unif_name);

	unif_name = "transform_normal";
	a_transform_normal = glGetUniformLocation(Program, unif_name);

	unif_name = "transform_viewPosition";
	a_transform_viewPosition = glGetUniformLocation(Program, unif_name);

	unif_name = "light_position";
	a_light_position = glGetUniformLocation(Program, unif_name);

	unif_name = "light_ambient";
	a_light_ambient = glGetUniformLocation(Program, unif_name);

	unif_name = "light_diffuse";
	a_light_diffuse = glGetUniformLocation(Program, unif_name);

	unif_name = "light_specular";
	a_light_specular = glGetUniformLocation(Program, unif_name);

	unif_name = "light_attenuation";
	a_light_attenuation = glGetUniformLocation(Program, unif_name);

	unif_name = "material_texture";
	a_material_texture = glGetUniformLocation(Program, unif_name);

	unif_name = "material_ambient";
	a_material_ambient = glGetUniformLocation(Program, unif_name);

	unif_name = "material_diffuse";
	a_material_diffuse = glGetUniformLocation(Program, unif_name);

	unif_name = "material_specular";
	a_material_specular = glGetUniformLocation(Program, unif_name);

	unif_name = "material_emission";
	a_material_emission = glGetUniformLocation(Program, unif_name);

	unif_name = "material_shininess";
	a_material_shininess = glGetUniformLocation(Program, unif_name);

	unif_name = "use_texture";
	a_use_texture = glGetUniformLocation(Program, unif_name);

	checkOpenGLerror();
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
	glGenBuffers(4, m_Buffers);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * indexed_vertices.size(), &indexed_vertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(a_coord);
	glVertexAttribPointer(a_coord, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TEXCOORD_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * indexed_texcoords.size(), &indexed_texcoords[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(a_texcoord);
	glVertexAttribPointer(a_texcoord, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * indexed_normals.size(), &indexed_normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(a_norm);
	glVertexAttribPointer(a_norm, 3, GL_FLOAT, GL_FALSE, 0, 0);

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

void initTexture()
{

	int width, height;
	unsigned char* image = SOIL_load_image("african_head_diffuse.jpg", &width, &height, 0, SOIL_LOAD_RGB);
	//tex1 = SOIL_load_OGL_texture("cat_diff.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_INVERT_Y);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);


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
	glm::vec4  lp = { lightX,0,0,1 };
	glm::mat4  m = glm::translate(glm::vec3(0, 0, lightZ));
	m = glm::rotate(m, lightAngle, glm::vec3(0, 0, 1));
	lp = m * lp;

	// vert shader part, never changes
	glm::mat4 transform_model = glm::rotate(rotateY, glm::vec3(0, 0, 1));
	transform_model = glm::rotate(transform_model, rotateX, glm::vec3(1, 0, 0));

	glUniformMatrix4fv(a_transform_model, 1, false, glm::value_ptr(transform_model));
	glUniformMatrix4fv(a_transform_viewProjection, 1, false, glm::value_ptr(Matrix_projection));
	glUniform3fv(a_transform_viewPosition, 1, glm::value_ptr(eye));

	glm::mat3 transform_normal = glm::inverseTranspose(glm::mat3(transform_model));
	glUniformMatrix3fv(a_transform_normal, 1, false, glm::value_ptr(transform_normal));

	glUniform4fv(a_light_position, 1, glm::value_ptr(lp));
	glUniform4fv(a_light_ambient, 1, glm::value_ptr(light_ambient));
	glUniform4fv(a_light_diffuse, 1, glm::value_ptr(light_diffuse));
	glUniform4fv(a_light_specular, 1, glm::value_ptr(light_specular));

	glUniform3fv(a_light_attenuation, 1, glm::value_ptr(light_attenuation));

	glUniform4fv(a_material_ambient, 1, glm::value_ptr(material_ambient));
	glUniform4fv(a_material_diffuse, 1, glm::value_ptr(material_diffuse));
	glUniform4fv(a_material_specular, 1, glm::value_ptr(material_specular));
	glUniform4fv(a_material_emission, 1, glm::value_ptr(material_emission));
	glUniform1f(a_material_shininess, material_shininess);

	glUniform1i(a_use_texture, use_texture);
	glUniform1i(a_material_texture, 0);
	
	glBindVertexArray(m_VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

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


void specialKeys(int key, int x, int y) {
	switch (key)
	{
	case GLUT_KEY_UP:
		lightZ += 0.1;
		break;
	case GLUT_KEY_DOWN:
		lightZ -= 0.1;
		break;
	case GLUT_KEY_LEFT:
		lightAngle -= 0.1;
		break;
	case GLUT_KEY_RIGHT:
		lightAngle += 0.1;
		break;
	case GLUT_KEY_F1:
		use_texture = !use_texture;
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
	initTexture();
	initVAO();
	set_light();
	set_material();
	glutReshapeFunc(resizeWindow);
	glutDisplayFunc(render);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKeys);
	glutMainLoop();
	freeShader();
	freeVAO();
}
