#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

GLuint VBO;
GLuint IBO;
GLuint MVPLocation;

class Camera {
public:
	glm::vec3 pos;
	glm::vec3 target;
	glm::vec3 direction;
	glm::vec3 updirection;
	glm::vec3 leftdirection;

	Camera() {}

	Camera(glm::vec3 _pos, glm::vec3 _target, glm::vec3 _updirection):pos(_pos), target(_target) {
		updirection = glm::normalize(_updirection);
		direction = glm::normalize(pos - target);
		leftdirection = glm::normalize(glm::cross(updirection, direction));
	}
	bool OnKeyboard(int Key) {
		bool Ret = false;
		switch (Key)
		{
		case GLUT_KEY_UP:
			pos += direction;
			target += direction;
			Ret = true;
			break;
		case GLUT_KEY_DOWN:
			pos -= direction;
			target -= direction;
			Ret = true;
			break;
		case GLUT_KEY_LEFT:
			pos += leftdirection;
			target += leftdirection;
			Ret = true;
			break;
		case GLUT_KEY_RIGHT:
			pos -= leftdirection;
			target -= leftdirection;
			Ret = true;
			break;
		}
		return Ret;
	}

};

Camera GameCamera;

static void RenderSceneCB()
{
	glClear(GL_COLOR_BUFFER_BIT);
	
	static float Scale = 0.0f;
	Scale += 0.001f;
	glm::mat4 Model = glm::rotate(Scale, glm::vec3(0.0f, 0.0f, 1.0f));

	glm::mat4 View = glm::lookAt(GameCamera.pos, GameCamera.target, GameCamera.updirection);

	glm::mat4 Projection = glm::perspective(
		45.0f,
		4.0f / 3.0f,
		0.1f,
		100.0f
	);

	glm::mat4 MVP = Projection*View*Model;
	glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, &MVP[0][0]);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	//glDrawArrays(GL_TRIANGLES, 0, 3);
	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);

	glutSwapBuffers();
}

static void SpecialKeyboardCB(int Key, int x, int y)
{
	GameCamera.OnKeyboard(Key);
}

static void InitializeGlutCallbacks()
{
	glutDisplayFunc(RenderSceneCB);
	glutIdleFunc(RenderSceneCB);
	glutSpecialFunc(SpecialKeyboardCB);
}

static void CreateVertexBuffer()
{
	glm::vec3 vertices[4];
	vertices[0] = glm::vec3(-1.0f, -1.0f, 0.0f);
	vertices[1] = glm::vec3(1.0f, -1.0f, 1.0f);
	vertices[2] = glm::vec3(1.0f, -1.0f, 0.0f);
	vertices[3] = glm::vec3(0.0f, 1.0f, 0.0f);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
}

static void CreateIndexBuffer()
{
	unsigned int Indices[] = { 0, 3, 1,
								1, 3, 2,
								2, 3, 0,
								0, 1, 2 };

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
}


static GLuint LoadShader( const char * ShaderPath, GLenum ShaderType)
{
	GLuint Shader = glCreateShader(ShaderType);
	if (Shader == 0) {
		printf("craete shader error!\n");
		exit(0);
	}
	std::string SourceCode;
	std::ifstream Shaderfin(ShaderPath, std::ios::in);
	if (Shaderfin.is_open()) {
		std::string Line = "";
		while (std::getline(Shaderfin, Line)) {
			SourceCode += "\n" + Line;
		}
		Shaderfin.close();
	} else {
		printf("open shader file error! file:%s\n", ShaderPath);
		exit(0);
	}
	char const * SourcePointer = SourceCode.c_str();
	glShaderSource(Shader, 1, &SourcePointer, NULL);
	glCompileShader(Shader);
	GLint Success;
	glGetShaderiv(Shader, GL_COMPILE_STATUS, &Success);
	if (!Success) {
		printf("compile shader error! file:%s\n", ShaderPath);
		GLchar InfoLog[1024];
		glGetShaderInfoLog(Shader, 1024, NULL, InfoLog);
		printf("compile log: %s\n", InfoLog);
		exit(0);
	}
	return Shader;
}

static GLuint CompileShaders()
{
	GLuint ShaderProgram = glCreateProgram();
	if (ShaderProgram == 0) {
		printf("create shader program error!\n");
		exit(0);
	}
	GLuint VertexShader = LoadShader("shader.vert", GL_VERTEX_SHADER);
	GLuint FragmentShader = LoadShader("shader.frag", GL_FRAGMENT_SHADER);
	glAttachShader(ShaderProgram, VertexShader);
	glAttachShader(ShaderProgram, FragmentShader);
	glLinkProgram(ShaderProgram);
	GLint Success;
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
	if (!Success) {
		printf("link shader error!\n");
		GLchar InfoLog[1024];
		glGetProgramInfoLog(ShaderProgram, 1024, NULL, InfoLog);
		printf("link log: %s\n", InfoLog);
		exit(0);
	}
	glDetachShader(ShaderProgram, VertexShader);
	glDetachShader(ShaderProgram, FragmentShader);
	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);
	glUseProgram(ShaderProgram);
	return ShaderProgram;
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(1024, 768);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("GLtest");


	GameCamera = Camera(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	InitializeGlutCallbacks();

	glewInit();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	CreateVertexBuffer();
	CreateIndexBuffer();

	GLuint Program = CompileShaders();
	MVPLocation = glGetUniformLocation(Program, "MVP");
	if (MVPLocation == 0xFFFFFFFF) {
		printf("Get Uniform Var failed! var:%s\n", "gScale");
		exit(0);
	}

	glutMainLoop();

	return 0;
}