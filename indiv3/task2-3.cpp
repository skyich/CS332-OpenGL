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
#include "OBJ_Loader.h"

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

float translateX = 0;
float translateY = 0;
float translateZ = 0;
float angleX = 0;
float angleY = 0;
float angleZ = 0;

void checkOpenGLerror()
{
	GLenum errCode;
	if ((errCode = glGetError()) != GL_NO_ERROR)
		std::cout << "OpenGl error! - " << gluErrorString(errCode);
}

class Object {
public:
	GLint a_coord;
	GLint a_norm;
	GLint a_texcoord;
	GLint a_material_texture;
	GLint a_material_ambient;
	GLint a_material_diffuse;
	GLint a_material_specular;
	GLint a_material_emission;
	GLint a_material_shininess;
	GLint a_transform_model;

	GLuint texture;
	size_t indices_size;

	GLuint VAO;
	GLuint m_Buffers[4];

	glm::vec4 material_ambient, material_diffuse, material_specular, material_emission;
	GLfloat material_shininess;

	glm::mat4 transform_model;

	
	Object(string object_path, string texture_path, GLuint Program, glm::mat4 transform) {
		transform_model = transform;
		init_attributes(Program);
		
		// Read our .obj file
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec2> texcoords;
		std::vector<glm::vec3> normals;
		bool res = loadOBJ(object_path.data(), vertices, texcoords, normals);
		printf("vertices: %d\ntexcoords: %d\nnormals: %d\n", vertices.size(), texcoords.size(), normals.size());

		std::vector<unsigned short> indices;
		std::vector<glm::vec3> indexed_vertices;
		std::vector<glm::vec2> indexed_texcoords;
		std::vector<glm::vec3> indexed_normals;
		indexVBO(vertices, texcoords, normals, indices, indexed_vertices, indexed_texcoords, indexed_normals);
		printf("indexed_vertices: %d\nindexed_texcoords: %d\nindexed_normals: %d\nindices: %d\n", indexed_vertices.size(), indexed_texcoords.size(), indexed_normals.size(), indices.size());
		
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
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

		///////// Load Textures //////////
		int width, height;
		unsigned char* image = SOIL_load_image(texture_path.data(), &width, &height, 0, SOIL_LOAD_RGB);
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
		SOIL_free_image_data(image);
		glBindTexture(GL_TEXTURE_2D, 0);

		//////// Init Material /////////
		material_emission = { 0,0,0,1 };
		material_ambient = { 0.25, 0.25, 0.25, 0 };
		material_diffuse = { 0.2, 0.5, 0.2, 0 };
		material_specular = { 0.027, 0.027, 0.027, 0 };
		material_shininess = 10;
	}
	
	/*
	Object(string object_path, string texture_path, GLuint Program, glm::mat4 transform) {
		transform_model = transform;
		init_attributes(Program);
		
		// Read our .obj file
		objl::Loader loader;
		loader.LoadFile(object_path);
		unsigned int NumVertices = loader.LoadedMeshes[0].Vertices.size();
		unsigned int NumIndices = loader.LoadedMeshes[0].Indices.size();
		std::vector<unsigned int> indices;
		std::vector<objl::Vector3> indexed_vertices;
		std::vector<objl::Vector2> indexed_texcoords;
		std::vector<objl::Vector3> indexed_normals;

		for (unsigned int i = 0; i < NumVertices; i++) {
			indexed_vertices.push_back(loader.LoadedMeshes[0].Vertices[i].Position);
			indexed_normals.push_back(loader.LoadedMeshes[0].Vertices[i].Normal);
			indexed_texcoords.push_back(loader.LoadedMeshes[0].Vertices[i].TextureCoordinate);
		}

		for (unsigned int i = 0; i < NumIndices; i++) {
			indices.push_back(loader.LoadedMeshes[0].Indices[i]);
		}

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glGenBuffers(4, m_Buffers);

		glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(indexed_vertices[0]) * indexed_vertices.size(), &indexed_vertices[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(a_coord);
		glVertexAttribPointer(a_coord, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TEXCOORD_VB]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(indexed_texcoords[0]) * indexed_texcoords.size(), &indexed_texcoords[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(a_texcoord);
		glVertexAttribPointer(a_texcoord, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(indexed_normals[0]) * indexed_normals.size(), &indexed_normals[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(a_norm);
		glVertexAttribPointer(a_norm, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), &indices[0], GL_STATIC_DRAW);
		indices_size = indices.size();

		glBindVertexArray(0);

		checkOpenGLerror();

		///////// Load Textures //////////
		int width, height;
		unsigned char* image = SOIL_load_image(texture_path.data(), &width, &height, 0, SOIL_LOAD_RGB);
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
		SOIL_free_image_data(image);
		glBindTexture(GL_TEXTURE_2D, 0);

		//////// Init Material /////////
		material_emission = { 0,0,0,1 };
		material_ambient = { 0.25, 0.25, 0.25, 0 };
		material_diffuse = { 0.3, 0.2, 0.25, 0 };
		material_specular = { 0.027, 0.027, 0.027, 0 };
		material_shininess = 10;
	}
	*/

	void init_attributes(GLuint Program) {
		//transform_model = glm::rotate(rotateY, glm::vec3(0, 0, 1));
		//transform_model = glm::rotate(transform_model, rotateX, glm::vec3(1, 0, 0));

		const char* attr_name = "coord";
		a_coord = glGetAttribLocation(Program, attr_name);

		attr_name = "norm";
		a_norm = glGetAttribLocation(Program, attr_name);

		attr_name = "texcoord";
		a_texcoord = glGetAttribLocation(Program, attr_name);

		const char* unif_name = "material_texture";
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

		unif_name = "transform_model";
		a_transform_model = glGetUniformLocation(Program, unif_name);
	}
};

GLuint Program;
GLuint Phong;
GLuint Gouraud;
bool select = false;

GLint a_transform_viewProjection;
GLint a_transform_normal;
GLint a_transform_viewPosition;
GLint a_light_position;
GLint a_light_ambient;
GLint a_light_diffuse;
GLint a_light_specular;
GLint a_light_attenuation;

GLint a_use_texture;

bool use_texture = true;

glm::vec3 eye;
glm::vec4 light_position, light_ambient, light_diffuse, light_specular;
glm::vec3 light_attenuation;

float lightZ = 20;
float lightY = 20;
float lightX = 2;
float lightAngle = 0;
float scaleX = 1;
float scaleY = 1;
glm::mat4 Matrix_projection;

vector<Object> objects;

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

GLuint initShader(string vss, string fss)
{
	std::string vs = readShader(vss);
	std::string fs = readShader(fss);
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

	checkOpenGLerror();

	return Program;
}

void init_attributes() {
	const char* unif_name = "transform_viewProjection";
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

	unif_name = "use_texture";
	a_use_texture = glGetUniformLocation(Program, unif_name);
}

void freeShader()
{
	glUseProgram(0);
	glDeleteProgram(Program);
}

void freeVAO()
{
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glDeleteBuffers(4, m_Buffers);
	//for (auto x : VAOS)
		//glDeleteVertexArrays(1, &x);
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


void set_light() {

	light_position = { 0,0,0,0 };
	light_ambient = { 1,1,1,1 };
	light_diffuse = { 1,1,1,1 };
	light_specular = { 1,1,1,1 };
	light_attenuation = { 0,0,0 };
}

void load_objects() {
	objects.push_back(Object("objects/T_34.obj", "objects/T_34.jpg", Program, glm::mat4()));
	objects.push_back(Object("objects/IS7.obj", "objects/IS7.jpg", Program, glm::translate(glm::mat4(), glm::vec3(-10.0f, 0.0f, 3.0f))));
	objects.push_back(Object("objects/T_34.obj", "objects/T_34.jpg", Program, glm::translate(glm::mat4(), glm::vec3(-15.0f, 0.0f, 0.0f))));

	objects.push_back(Object("objects/Tiger.obj", "objects/Tiger.jpg", Program, glm::rotate(glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, 40.0f)), glm::radians(180.0f), glm::vec3(0, 1, 0)))) ;
	objects.push_back(Object("objects/Tiger2.obj", "objects/Tiger2.jpg", Program, glm::rotate(glm::translate(glm::mat4(), glm::vec3(-10.0f, 0.0f, 40.0f)), glm::radians(180.0f), glm::vec3(0, 1, 0))));

	objects.push_back(Object("objects/floor.obj", "objects/f.jpg", Program, glm::mat4()));
	objects[objects.size() - 1].material_diffuse = { 0.1, 0.2, 0.1, 0 };
}

void render()
{
	glMatrixMode(GL_MODELVIEW);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(Program);
	glm::vec4  lp = { lightX,lightY,0,1 };
	glm::mat4  m = glm::translate(glm::vec3(0, 0, lightZ));
	m = glm::rotate(m, lightAngle, glm::vec3(0, 0, 1));
	lp = m * lp;

	Matrix_projection = glm::translate(Matrix_projection, glm::vec3(translateX, translateY, translateZ));
	Matrix_projection = glm::rotate(Matrix_projection, glm::radians(angleX), glm::vec3(1, 0, 0));
	Matrix_projection = glm::rotate(Matrix_projection, glm::radians(angleY), glm::vec3(0, 1, 0));
	Matrix_projection = glm::rotate(Matrix_projection, glm::radians(angleZ), glm::vec3(0, 0, 1));
	glUniformMatrix4fv(a_transform_viewProjection, 1, false, glm::value_ptr(Matrix_projection));
	glUniform3fv(a_transform_viewPosition, 1, glm::value_ptr(eye));

	glUniform4fv(a_light_position, 1, glm::value_ptr(lp));
	glUniform4fv(a_light_ambient, 1, glm::value_ptr(light_ambient));
	glUniform4fv(a_light_diffuse, 1, glm::value_ptr(light_diffuse));
	glUniform4fv(a_light_specular, 1, glm::value_ptr(light_specular));

	glUniform3fv(a_light_attenuation, 1, glm::value_ptr(light_attenuation));
	glUniform1i(a_use_texture, use_texture);


	for (auto x : objects) {
		x.init_attributes(Program);
		glUniformMatrix4fv(x.a_transform_model, 1, false, glm::value_ptr(x.transform_model));
		glm::mat3 transform_normal = glm::inverseTranspose(glm::mat3(x.transform_model));
		glUniformMatrix3fv(a_transform_normal, 1, false, glm::value_ptr(transform_normal));
		glUniform4fv(x.a_material_ambient, 1, glm::value_ptr(x.material_ambient));
		glUniform4fv(x.a_material_diffuse, 1, glm::value_ptr(x.material_diffuse));
		glUniform4fv(x.a_material_specular, 1, glm::value_ptr(x.material_specular));
		glUniform4fv(x.a_material_emission, 1, glm::value_ptr(x.material_emission));
		glUniform1f(x.a_material_shininess, x.material_shininess);
		glUniform1i(x.a_material_texture, 0);
		glBindVertexArray(x.VAO); 
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, x.texture);
		glDrawElements(GL_TRIANGLES,
			x.indices_size,
			GL_UNSIGNED_SHORT,
			(void*)0);
		glBindVertexArray(0);
	}

	glUseProgram(0);
	checkOpenGLerror();
	glutSwapBuffers();
}


void keyboard(unsigned char key, int x, int y)
{
	angleX = angleY = angleZ = translateX = translateY = translateZ = 0;
	switch (key)
	{
	case 'w':
		translateX = 0.1;
		break;
	case 's':
		translateX = -0.1;
		break;
	case 'a':
		translateY = 0.1;
		break;
	case 'd':
		translateY = -0.1;
		break;
	case 'q':
		translateZ = 0.5;
		break;
	case 'e':
		translateZ = -0.5;
		break;
	case 't':
		lightZ += 1;
		break;
	case 'g':
		lightZ -= 2;
		break;
	case 'f':
		lightAngle -= 0.1;
		break;
	case 'h':
		lightAngle += 0.1;
		break;
	default:
		break;
	}

	glutPostRedisplay();
}


void specialKeys(int key, int x, int y) {
	angleX = angleY = angleZ = translateX = translateY = translateZ = 0;
	switch (key)
	{
	case GLUT_KEY_F1:
		use_texture = !use_texture;
		break;
	case GLUT_KEY_F2:
		select = !select;
		if (select) {
			Program = Phong;
			cout << "Phong shader\n";
		}
		else {
			Program = Gouraud;
			cout << "Gouraud shader\n";
		}
		init_attributes();
		//initVAO();
		break;
	case GLUT_KEY_UP: 
		angleX = 4;
		break;
	case GLUT_KEY_DOWN: 
		angleX = -4;
		break;
	case GLUT_KEY_RIGHT: 
		angleY = 4;
		break;
	case GLUT_KEY_LEFT: 
		angleY = -4;
		break;
	case GLUT_KEY_PAGE_UP: 
		angleZ = 4;
		break;
	case GLUT_KEY_PAGE_DOWN: 
		angleZ = -4;
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
	Phong = initShader("vsPhong.txt", "fsPhong.txt");
	Gouraud = initShader("vsShader.txt", "fsShader.txt");
	Program = Phong;
	init_attributes();
	load_objects();
	set_light();

	glutReshapeFunc(resizeWindow);
	glutDisplayFunc(render);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKeys);
	glutMainLoop();
	freeShader();
	freeVAO();
}
