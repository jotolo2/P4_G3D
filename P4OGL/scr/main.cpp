#include "BOX.h"
#include "auxiliar.h"
#include "PLANE.h"
#include "convolutionMasks.h"

#include <gl/glew.h>
#define SOLVE_FGLUT_WARNING
#include <gl/freeglut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <algorithm>
#include <iostream>
#include <cstdlib>

#define RAND_SEED 31415926
#define SCREEN_SIZE 500,500

//////////////////////////////////////////////////////////////
// Variables que nos dan acceso a Objetos OpenGL
//////////////////////////////////////////////////////////////

//VAO
unsigned int vao;

//VBO
unsigned int fbo;

//VBOs que forman parte del objeto
unsigned int posVBO;
unsigned int colorVBO;
unsigned int normalVBO;
unsigned int texCoordVBO;
unsigned int triangleIndexVBO;

unsigned int planeVAO;
unsigned int planeVertexVBO;

///////////
//Forward-rendering
///////////
unsigned int forwardVShader;
unsigned int forwardFShader;
unsigned int forwardProgram;

//Atributos
int inPos;
int inColor;
int inNormal;
int inTexCoord;

//Matrices Uniform
int uModelViewMat;
int uModelViewProjMat;
int uNormalMat;

//Identificadores de texturas Forward-rendering
unsigned int colorTexId;
unsigned int emiTexId;

//Texturas Uniform
int uColorTex;
int uEmiTex;

///////////
//Post-proceso
///////////
unsigned int postProccesVShader;
unsigned int postProccesFShader;
unsigned int postProccesProgram;

//Atributos
int inPosPP;

//Uniforms de la distancia focal
int uFocalDistance;
int uMaxDistanceFactor;
int uNear;
int uFar;

//Uniforms para las máscaras de convolución
int uTexIdx9;
int uTexIdx25;
int uMask9;
int uMask25;
int uMaskSelector;

//Identificadores de texturas Post-proceso
unsigned int colorBuffTexId;
unsigned int emiBuffTexId;
unsigned int depthBuffTexId;
unsigned int vertexBuffTexId;
unsigned int normalBuffTexId;

//Texturas Post-proceso
unsigned int uColorTexPP;
unsigned int uVertexTexPP;
unsigned int uNormalTexPP;
unsigned int uDepthTexPP;
unsigned int uEmiTexPP;

//Uniform para el post-procesado del noise
int uTime;
int uUseNoise;

//////////////////////////////////////////////////////////////
// Datos que se almacenan en la memoria de la CPU
//////////////////////////////////////////////////////////////

float angle = 0.0f;

//Matrices
glm::mat4	proj = glm::mat4(1.0f);
glm::mat4	view = glm::mat4(1.0f);
glm::mat4	model = glm::mat4(1.0f);

//Variables de control de la distancia focal
float focalDistance;
float maxDistanceFactor;

//Variables de control para las máscaras de convolución
int maskSelector;
float mask_9d[9];

//Variables de control del motion Blur
float motionAlpha, motionColor;
bool useBlend;
bool useNoise;

//Variables de control del near y far de la matriz de proyección y para el cálculo de profundidad
float projNear, projFar;


//////////////////////////////////////////////////////////////
// Funciones auxiliares
//////////////////////////////////////////////////////////////

//Declaración de CB
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);

void renderCube();

//Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void initShaderFw(const char* vname, const char* fname);
void initShaderPP(const char* vname, const char* fname);
void initObj();
void initPlane();
void initFBO();
void destroy();
void resizeFBO(unsigned int w, unsigned int h);

//Carga el shader indicado, devuele el ID del shader
//!Por implementar
GLuint loadShader(const char* fileName, GLenum type);

//Crea una textura, la configura, la sube a OpenGL,
//y devuelve el identificador de la textura
//!!Por implementar
unsigned int loadTex(const char* fileName);

//////////////////////////////////////////////////////////////
// Nuevas variables auxiliares
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// Nuevas funciones auxiliares
//////////////////////////////////////////////////////////////
//!!Por implementar

int main(int argc, char** argv)
{
	std::locale::global(std::locale("spanish"));// acentos ;)

	initContext(argc, argv);
	initOGL();
	initShaderFw("../shaders_P4/fwRendering.v0.vert", "../shaders_P4/fwRendering.v0.frag");
	initShaderPP("../shaders_P4/postProcessing.v0.vert", "../shaders_P4/postProcessing.v0.frag");

	initObj();
	initPlane();

	initFBO();

	glutMainLoop();

	destroy();

	return 0;
}

//////////////////////////////////////////
// Funciones auxiliares
void initContext(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	//glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(SCREEN_SIZE);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Prácticas GLSL");

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		exit(-1);
	}

	const GLubyte* oglVersion = glGetString(GL_VERSION);
	std::cout << "This system supports OpenGL Version: " << oglVersion << std::endl;

	glutReshapeFunc(resizeFunc);
	glutDisplayFunc(renderFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseFunc);
}

void initOGL()
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);

	projNear = 1.0f;
	projFar = 50.0f;
	proj = glm::perspective(glm::radians(60.0f), 1.0f, projNear, projFar);

	//Inicializamos la cámara en -25
	view = glm::mat4(1.0f);
	view[3].z = -25.0f;

	//Inicializamos los valores para el motion blur
	motionAlpha = 0.6;
	motionColor = 0.5;
	useBlend = false;
	useNoise = false;

	//Inicializamos los valores para controlar la distancia focal
	focalDistance = -25.0;
	maxDistanceFactor = 5;

	//Establecemos la máscara de convolución primera
	maskSelector = 0;
	for (int i = 0; i < 9; ++i)
		mask_9d[i] = defaultMask[i];
}

void destroy()
{
	glDetachShader(forwardProgram, forwardVShader);
	glDetachShader(forwardProgram, forwardFShader);
	glDeleteShader(forwardVShader);
	glDeleteShader(forwardFShader);
	glDeleteProgram(forwardProgram);

	glDetachShader(postProccesProgram, postProccesVShader);
	glDetachShader(postProccesProgram, postProccesFShader);
	glDeleteShader(postProccesVShader);
	glDeleteShader(postProccesFShader);
	glDeleteProgram(postProccesProgram);

	if (inPos != -1) glDeleteBuffers(1, &posVBO);
	if (inColor != -1) glDeleteBuffers(1, &colorVBO);
	if (inNormal != -1) glDeleteBuffers(1, &normalVBO);
	if (inTexCoord != -1) glDeleteBuffers(1, &texCoordVBO);
	glDeleteBuffers(1, &triangleIndexVBO);

	glDeleteVertexArrays(1, &vao);

	glDeleteTextures(1, &colorTexId);
	glDeleteTextures(1, &emiTexId);

	glDeleteBuffers(1, &planeVertexVBO);
	glDeleteVertexArrays(1, &planeVAO);

	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &colorBuffTexId);
	glDeleteTextures(1, &depthBuffTexId);
	glDeleteTextures(1, &vertexBuffTexId);
	glDeleteTextures(1, &emiBuffTexId);
	glDeleteTextures(1, &normalBuffTexId);
}

void initShaderFw(const char* vname, const char* fname)
{
	forwardVShader = loadShader(vname, GL_VERTEX_SHADER);
	forwardFShader = loadShader(fname, GL_FRAGMENT_SHADER);

	forwardProgram = glCreateProgram();
	glAttachShader(forwardProgram, forwardVShader);
	glAttachShader(forwardProgram, forwardFShader);

	glBindAttribLocation(forwardProgram, 0, "inPos");
	glBindAttribLocation(forwardProgram, 1, "inColor");
	glBindAttribLocation(forwardProgram, 2, "inNormal");
	glBindAttribLocation(forwardProgram, 3, "inTexCoord");

	glLinkProgram(forwardProgram);

	int linked;
	glGetProgramiv(forwardProgram, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(forwardProgram, GL_INFO_LOG_LENGTH, &logLen);

		char* logString = new char[logLen];
		glGetProgramInfoLog(forwardProgram, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;

		glDeleteProgram(forwardProgram);
		forwardProgram = 0;
		exit(-1);
	}

	uNormalMat = glGetUniformLocation(forwardProgram, "normal");
	uModelViewMat = glGetUniformLocation(forwardProgram, "modelView");
	uModelViewProjMat = glGetUniformLocation(forwardProgram, "modelViewProj");

	uColorTex = glGetUniformLocation(forwardProgram, "colorTex");
	uEmiTex = glGetUniformLocation(forwardProgram, "emiTex");

	inPos = glGetAttribLocation(forwardProgram, "inPos");
	inColor = glGetAttribLocation(forwardProgram, "inColor");
	inNormal = glGetAttribLocation(forwardProgram, "inNormal");
	inTexCoord = glGetAttribLocation(forwardProgram, "inTexCoord");
}

void initShaderPP(const char* vname, const char* fname)
{
	postProccesVShader = loadShader(vname, GL_VERTEX_SHADER);
	postProccesFShader = loadShader(fname, GL_FRAGMENT_SHADER);

	postProccesProgram = glCreateProgram();
	glAttachShader(postProccesProgram, postProccesVShader);
	glAttachShader(postProccesProgram, postProccesFShader);

	glBindAttribLocation(postProccesProgram, 0, "inPos");

	glLinkProgram(postProccesProgram);
	int linked;
	glGetProgramiv(postProccesProgram, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(postProccesProgram, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetProgramInfoLog(postProccesProgram, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete logString;
		glDeleteProgram(postProccesProgram);
		postProccesProgram = 0;
		exit(-1);
	}

	inPosPP = glGetAttribLocation(postProccesProgram, "inPos");

	glUseProgram(postProccesProgram);

	//Establecimiento de los localizadores de las texturas/buffers de post-proceso
	uColorTexPP = glGetUniformLocation(postProccesProgram, "colorTex");
	if (uColorTexPP != -1)
		glUniform1i(uColorTexPP, 0);

	uVertexTexPP = glGetUniformLocation(postProccesProgram, "vertexTex");
	if (uVertexTexPP != -1)
		glUniform1i(uVertexTexPP, 1);

	uNormalTexPP = glGetUniformLocation(postProccesProgram, "normalTex");
	if (uNormalTexPP != -1)
		glUniform1i(uNormalTexPP, 2);

	uEmiTexPP = glGetUniformLocation(postProccesProgram, "emiTex");
	if (uEmiTexPP != -1)
		glUniform1i(uEmiTexPP, 3);

	uDepthTexPP = glGetUniformLocation(postProccesProgram, "depthTex");
	if (uDepthTexPP != -1)
		glUniform1i(uDepthTexPP, 4);

	//Localizadores de las variables uniform
	uFocalDistance = glGetUniformLocation(postProccesProgram, "focalDistance");
	uMaxDistanceFactor = glGetUniformLocation(postProccesProgram, "maxDistanceFactor");
	uNear = glGetUniformLocation(postProccesProgram, "near");
	uFar = glGetUniformLocation(postProccesProgram, "far");

	uMask9 = glGetUniformLocation(postProccesProgram, "mask9");
	uTexIdx9 = glGetUniformLocation(postProccesProgram, "texIdx9");
	uMask25 = glGetUniformLocation(postProccesProgram, "mask25");
	uTexIdx25 = glGetUniformLocation(postProccesProgram, "texIdx25");
	uMaskSelector = glGetUniformLocation(postProccesProgram, "maskSelector");
	uTime = glGetUniformLocation(postProccesProgram, "time");
	uUseNoise = glGetUniformLocation(postProccesProgram, "useNoise");
}

void initObj()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	if (inPos != -1)
	{
		glGenBuffers(1, &posVBO);
		glBindBuffer(GL_ARRAY_BUFFER, posVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
			cubeVertexPos, GL_STATIC_DRAW);
		glVertexAttribPointer(inPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inPos);
	}

	if (inColor != -1)
	{
		glGenBuffers(1, &colorVBO);
		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
			cubeVertexColor, GL_STATIC_DRAW);
		glVertexAttribPointer(inColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inColor);
	}

	if (inNormal != -1)
	{
		glGenBuffers(1, &normalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
			cubeVertexNormal, GL_STATIC_DRAW);
		glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inNormal);
	}

	if (inTexCoord != -1)
	{
		glGenBuffers(1, &texCoordVBO);
		glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 2,
			cubeVertexTexCoord, GL_STATIC_DRAW);
		glVertexAttribPointer(inTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inTexCoord);
	}

	glGenBuffers(1, &triangleIndexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		cubeNTriangleIndex * sizeof(unsigned int) * 3, cubeTriangleIndex,
		GL_STATIC_DRAW);

	model = glm::mat4(1.0f);

	colorTexId = loadTex("../img/color2.png");
	emiTexId = loadTex("../img/emissive.png");
}

void initPlane()
{
	glGenVertexArrays(1, &planeVAO);
	glBindVertexArray(planeVAO);

	glGenBuffers(1, &planeVertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVertexVBO);

	glBufferData(GL_ARRAY_BUFFER, planeNVertex * sizeof(float) * 3,
		planeVertexPos, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
}

GLuint loadShader(const char* fileName, GLenum type)
{
	unsigned int fileLen;
	char* source = loadStringFromFile(fileName, fileLen);

	//////////////////////////////////////////////
	//Creación y compilación del Shader
	GLuint shader;
	shader = glCreateShader(type);
	glShaderSource(shader, 1,
		(const GLchar**)&source, (const GLint*)&fileLen);
	glCompileShader(shader);
	delete[] source;

	//Comprobamos que se compilo bien
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);

		char* logString = new char[logLen];
		glGetShaderInfoLog(shader, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;

		glDeleteShader(shader);
		exit(-1);
	}

	return shader;
}

unsigned int loadTex(const char* fileName)
{
	unsigned char* map;
	unsigned int w, h;
	map = loadTexture(fileName, w, h);

	if (!map)
	{
		std::cout << "Error cargando el fichero: "
			<< fileName << std::endl;
		exit(-1);
	}

	unsigned int texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, (GLvoid*)map);
	delete[] map;
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);

	return texId;
}

void renderFunc()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	///////////
	//Forward-rendering
	///////////
	glUseProgram(forwardProgram);

	//Texturas del forward-rendering
	if (uColorTex != -1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorTexId);
		glUniform1i(uColorTex, 0);
	}

	if (uEmiTex != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, emiTexId);
		glUniform1i(uEmiTex, 1);
	}

	//Dibujado de los distintos cubos
	model = glm::mat4(2.0f);
	model[3].w = 1.0f;
	model = glm::rotate(model, angle, glm::vec3(1.0f, 1.0f, 0.0f));
	renderCube();

	std::srand(RAND_SEED);
	for (unsigned int i = 0; i < 10; i++)
	{
		float size = float(std::rand() % 3 + 1);

		glm::vec3 axis(glm::vec3(float(std::rand() % 2),
			float(std::rand() % 2), float(std::rand() % 2)));
		if (glm::all(glm::equal(axis, glm::vec3(0.0f))))
			axis = glm::vec3(1.0f);

		float trans = float(std::rand() % 7 + 3) * 1.00f + 0.5f;
		glm::vec3 transVec = axis * trans;
		transVec.x *= (std::rand() % 2) ? 1.0f : -1.0f;
		transVec.y *= (std::rand() % 2) ? 1.0f : -1.0f;
		transVec.z *= (std::rand() % 2) ? 1.0f : -1.0f;

		model = glm::rotate(glm::mat4(1.0f), angle * 2.0f * size, axis);
		model = glm::translate(model, transVec);
		model = glm::rotate(model, angle * 2.0f * size, axis);
		model = glm::scale(model, glm::vec3(1.0f / (size * 0.7f)));
		renderCube();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	///////////
	//Post-procesing
	///////////
	glUseProgram(postProccesProgram);

	//Variables uniform
	//Variables de control del DOF
	if (uFocalDistance != -1)
		glUniform1f(uFocalDistance, focalDistance);
	if (uMaxDistanceFactor != -1)
		glUniform1f(uMaxDistanceFactor, 1 / maxDistanceFactor);

	//Variables para el cálculo de la profundidad
	if (uNear != -1)
		glUniform1f(uNear, projNear);
	if (uFar != -1)
		glUniform1f(uFar, projFar);

	//Variables para las máscaras de convolución
	if (uMaskSelector != 1)
		glUniform1i(uMaskSelector, maskSelector);
	if (uMask9 != -1)
		glUniform1fv(uMask9, 9, mask_9d);
	if (uMask25 != -1)
		glUniform1fv(uMask25, 25, mask25);

	if (uTexIdx9 != -1)
		for (int i = 0; i != 9; ++i)
		{
			GLint originsLoc = glGetUniformLocation(postProccesProgram, ("texIdx9[" + std::to_string(i) + "]").c_str());
			glUniform2f(originsLoc, texIdx9[i].x, texIdx9[i].y);
		}

	if (uTexIdx25 != -1)
		for (int i = 0; i != 25; ++i)
		{
			GLint originsLoc = glGetUniformLocation(postProccesProgram, ("texIdx25[" + std::to_string(i) + "]").c_str());
			glUniform2f(originsLoc, texIdx25[i].x, texIdx25[i].y);
		}

	//Variable para el efecto de post-procesado del noise
	if (uTime != -1)
		glUniform1f(uTime, glutGet(GLUT_ELAPSED_TIME));
	if (uUseNoise != -1)
		glUniform1i(uUseNoise, useNoise);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	if (useBlend)
	{
		glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendFunc(GL_CONSTANT_COLOR, GL_CONSTANT_ALPHA);
		glBlendColor(motionColor, motionColor, motionColor, motionAlpha);	//Control del motion Blur
		glBlendEquation(GL_FUNC_ADD);
	}

	//Establecimiento de los distintos bufferes para el post-procesado
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, colorBuffTexId);

	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, vertexBuffTexId);

	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, normalBuffTexId);

	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, emiBuffTexId);

	glActiveTexture(GL_TEXTURE0 + 4);
	glBindTexture(GL_TEXTURE_2D, depthBuffTexId);

	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glutSwapBuffers();
}

void renderCube()
{
	glm::mat4 modelView = view * model;
	glm::mat4 modelViewProj = proj * view * model;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));

	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE,
			&(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE,
			&(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE,
			&(normal[0][0]));

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, (void*)0);
}

void resizeFunc(int width, int height)
{
	//Mantenimiento del aspect ratio
	glViewport(0, 0, width, height);
	proj = glm::perspective(glm::radians(60.0f), float(width) / float(height), projNear, projFar);

	resizeFBO(width, height);

	glutPostRedisplay();
}

void idleFunc()
{
	angle = (angle > 3.141592f * 2.0f) ? 0 : angle + 0.02f;

	glutPostRedisplay();
}

void keyboardFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	//OBLIGATORIO 1
	//CONTROL DEL MOTION BLUR POR TECLADO
	//Alfa controlado con 1,2 y Color controlado con 3,4
	case('1'):
		motionAlpha = std::min(1.0f, motionAlpha + 0.01f);
		break;
	case('2'):
		motionAlpha = std::max(0.0f, motionAlpha - 0.01f);
		break;
	case('3'):
		motionColor = std::min(1.0f, motionColor + 0.01f);
		break;
	case('4'):
		motionColor = std::max(0.0f, motionColor - 0.01f);
		break;

	//OBLIGATORIO 2
	//CONTROL DE LOS PARÁMETROS DEL DOF POR TECLADO
	//Distancia focal controlada con 5,6 y Factor máximo de distancia con 7,8
	case('5'):
		focalDistance = std::max(-100.0f, focalDistance - 0.25f);
		break;
	case('6'):
		focalDistance = std::min(0.0f, focalDistance + 0.25f);
		break;
	case('7'):
		maxDistanceFactor = std::min(100.0f, maxDistanceFactor + 0.1f);
		break;
	case('8'):
		maxDistanceFactor = std::max(0.1f, maxDistanceFactor - 0.1f);
		break;

	//OBLIGATORIO 4
	//USO DE VARIAS MÁSCARAS DE CONVOLUCIÓN Y SU SELECCIÓN POR TECLADO
	case('9'):
		maskSelector = (maskSelector < 5) ? maskSelector + 1 : 0;
		switch (maskSelector)
		{
		case(0):
			for (int i = 0; i < 9; ++i)
				mask_9d[i] = defaultMask[i];
			break;
		case(1):
			for (int i = 0; i < 9; ++i)
				mask_9d[i] = laplacianEdgeFilterMask[i];
			break;
		case(2):
			for (int i = 0; i < 9; ++i)
				mask_9d[i] = northDirectionMask[i];
			break;
		case(3):
			for (int i = 0; i < 9; ++i)
				mask_9d[i] = embossFilterMask[i];
			break;
		}
		break;

	//OPCIONAL 2
	//CONCATENACIÓN DE DISTINTOS POSTPROCESOS
	//Control para usar o no usar el Motion Blur
	case('Q'):
	case('q'):
		useBlend = !useBlend;
		break;

	//Control para usar o no el Noise
	case('W'):
	case('w'):
		useNoise = !useNoise;
		break;
	}
}
void mouseFunc(int button, int state, int x, int y) {}

void initFBO()
{
	glGenFramebuffers(1, &fbo);
	glGenTextures(1, &colorBuffTexId);
	glGenTextures(1, &depthBuffTexId);
	glGenTextures(1, &vertexBuffTexId);
	glGenTextures(1, &normalBuffTexId);
	glGenTextures(1, &emiBuffTexId);

	resizeFBO(SCREEN_SIZE);
}

void resizeFBO(unsigned int w, unsigned int h)
{
	//Inicialización de todos los buffers del post-procesado
	glBindTexture(GL_TEXTURE_2D, colorBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, depthBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, vertexBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, normalBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, emiBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffTexId, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffTexId, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, vertexBuffTexId, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, normalBuffTexId, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, emiBuffTexId, 0);

	const GLenum buffs[4] = { GL_COLOR_ATTACHMENT0,  GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, buffs);

	if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER))
	{
		std::cerr << "Error configurando el FBO" << std::endl;
		exit(-1);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}