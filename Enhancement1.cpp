//inclusions
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SOIL2/SOIL2.h"
using namespace std;
#define WINDOW_TITLE "Bookshelf" //Window title

//Shader program
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version "\n" #Source
#endif

//variables for shader and window size
objects
GLint shaderProgram, WindowWidth = 800, WindowHeight = 600;
GLuint VBO, VAO, texture;
GLfloat degrees = glm::radians(0.0f); 

//light color
glm::vec3 lightColor(1.0f, 0.0f, 0.0f);
glm::vec3 secondLightColor(1.0f, 1.0f, 1.0f);
glm::vec3 lightPosition(1.0f, 0.5f, -3.0f); //light position and scale
glm::vec3 lightStrength(1.0f, 1.0f, 0.5f); // specular highlight

//camera rotation
float cameraRotation = glm::radians(-45.0f);
GLfloat cameraSpeed = 0.005f;
GLchar currentKey; //store key
GLfloat lastMouseX = 400, lastMouseY = 300; //lock cursor to center of screen
GLfloat mouseXOffset, mouseYOffset, yaw = 0.0f, pitch = 0.0f, zoom = 0.0f; 
GLfloat sensitivity = 0.005f; //sensitivity
bool mouseDetected = true; 
bool leftMouseButton = false;
bool rightMouseButton = false;
bool altDown = false;

//global vector declarations
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, -8.0f); 
position.placed - 8 units in z
glm::vec3 CameraUpY = glm::vec3(0.0f, 1.0f, 0.0f); 
glm::vec3 CameraForwardZ = glm::vec3(0.0f, 0.0f, -1.0f); 
glm::vec3 front; 
glm::vec3 cameraRotateAmt;

//function prototypes
void UResizeWindow(int, int);
void URenderGraphics(void);
void UCreateShader(void);
void UCreateBuffers(void);
void UMouseClick(int button, int state, int x, int y);
void UMousePressedMove(int x, int y);
void UGenerateTexture(void);

//vertex shader
const GLchar* vertexShaderSource = GLSL(330,
	layout(location = 0) in vec3 position; 
	layout(location = 2) in vec2 textureCoordinate;
	out vec2 mobileTextureCoordinate;

//global transform variables
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
	gl_Position = projection * view * model * vec4(position, 1.0f); 
	mobileTextureCoordinate = vec2(textureCoordinate.x, 1.0f - textureCoordinate.y); //flip texture
}
);

//fragment shader source code
const GLchar* fragmentShaderSource = GLSL(330,
	in vec2 mobileTextureCoordinate;
	out vec4 gpuTexture; //pass color to GPU
	uniform sampler2D uTexture; 
void main() {
	gpuTexture = texture(uTexture, mobileTextureCoordinate);
}
);

//VAP positions
const GLchar* lightVertexShaderSource = GLSL(330,
	layout(location = 0) in vec3 position; 
	layout(location = 1) in vec3 normal; 
	layout(location = 2) in vec2 textureCoordinate;
out vec3 Normal; //fnormals to fragment shader
out vec3 FragmentPos; //color and pixels to fragment shader
out vec2 mobileTextureCoordinate; //texture coordiantes

//transform matrices variables
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
	gl_Position = projection * view * model * vec4(position, 1.0f);
	Normal = mat3(transpose(inverse(model))) * normal; 
		FragmentPos = vec3(model * vec4(position, 1.0f)); 
		mobileTextureCoordinate = vec2(textureCoordinate.x, 1.0f - textureCoordinate.y); //flip texture
}
);
const GLchar* lightFragmentShaderSource = GLSL(330,
	in vec3 Normal; 
	in vec3 FragmentPos; 
	in vec2 mobileTextureCoordinate;
	out vec4 result; 

//variables for colors, lights, and camera
uniform vec3 lightColor;
uniform vec3 secondLightColor;
uniform vec3 lightPos;
uniform vec3 viewPosition;
uniform vec3 lightStrength;
uniform sampler2D uTexture; 

void main() {
	vec3 norm = normalize(Normal); 
	vec3 ambient = lightStrength.x * lightColor; //ambient light color
	vec3 ambientTwo = lightStrength.x * secondLightColor;//ambient light color 2
	vec3 lightDirection = normalize(lightPos - FragmentPos); //light direction
	float impact = max(dot(norm, lightDirection), 0.0); //diffuse impact
	vec3 diffuse = impact * lightColor; //diffuse light color
	vec3 viewDir = normalize(viewPosition - FragmentPos); //view direction
	vec3 reflectDir = reflect(-lightDirection, norm); //reflection
	float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), lightStrength.z);
	vec3 specular = lightStrength.y * specularComponent * lightColor;
	//phong result
	vec3 phongOne = (ambient + diffuse + specular) * vec3(texture(uTexture, mobileTextureCoordinate));
	//light position
	lightDirection = normalize(vec3(6.0f, 0.0f, -3.0f) - FragmentPos);
	impact = max(dot(norm, lightDirection), 0.0); //diffuse impact
	diffuse = impact * secondLightColor; //diffuse light color
	viewDir = normalize(viewPosition - FragmentPos); //view direction
	reflectDir = reflect(-lightDirection, norm); //reflection
	specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), lightStrength.z);
	//second light
	vec3 specularTwo = 0.1f * specularComponent * secondLightColor;
	vec3 phongTwo = (ambientTwo + diffuse + specularTwo) * vec3(texture(uTexture, mobileTextureCoordinate));
	result = vec4(phongOne + phongTwo, 1.0f); //send results to GPU
}
);

//main program
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutCreateWindow(WINDOW_TITLE);
	glutReshapeFunc(UResizeWindow);
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}
	UCreateShader();
	UCreateBuffers();
	UGenerateTexture();
	//shader program
	glUseProgram(shaderProgram);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //background color
	glutDisplayFunc(URenderGraphics);
	glutMouseFunc(UMouseClick); //mouse movement
	glutMotionFunc(UMousePressedMove); //mouse hold and move
	glutMainLoop();
	glDeleteVertexArrays(1, &VAO); 
	glDeleteBuffers(1, &VBO); //delete buffers once used
	return 0;
}

//resize window
void UResizeWindow(int w, int h)
{
	WindowWidth = w;
	WindowHeight = h;
	glViewport(0, 0, WindowWidth, WindowHeight);
}

//render graphics
void URenderGraphics(void)
{
	glEnable(GL_DEPTH_TEST); //z-depth
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear screen
	glBindVertexArray(VAO); //activate VAO
	front.x = 10.0f * cos(yaw);
	front.y = 10.0f * sin(pitch);
	front.z = sin(yaw) * cos(pitch) * 10.0f;
	//transform object
	glm::mat4 model;
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f)); //scale by 2
	model = glm::rotate(model, degrees, glm::vec3(0.0, 1.0f, 0.0f)); //rotate -45
	model = glm::translate(model, glm::vec3(0.0, 0.0f, 0.0f)); //put object at center
	
	//transform camera
	glm::mat4 view;
	view = glm::lookAt(cameraPosition, CameraForwardZ, CameraUpY);
	view = glm::rotate(view, cameraRotateAmt.x, glm::vec3(0.0f, 1.0f, 0.0f));
	view = glm::rotate(view, cameraRotateAmt.y, glm::vec3(1.0f, 0.0f, 0.0f));
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, zoom));
	
	//create projection
	glm::mat4 projection;
	projection = glm::perspective(45.0f, (GLfloat)WindowWidth / (GLfloat)WindowHeight, 0.1f, 100.0f); //pass to shader program
	GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
	GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
	GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
	GLint secondLightColorLoc, lightColorLoc, lightPositionLoc, lightStrengthLoc, viewPositionLoc;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
	lightPositionLoc = glGetUniformLocation(shaderProgram, "lightPos");
	lightStrengthLoc = glGetUniformLocation(shaderProgram, "lightStrength");
	secondLightColorLoc = glGetUniformLocation(shaderProgram, "secondLightColor");
	viewPositionLoc = glGetUniformLocation(shaderProgram, "viewPosition");
	//pass color, light, and camera to sahder program
	glUniform3f(lightColorLoc, lightColor.r, lightColor.g, lightColor.b);
	glUniform3f(secondLightColorLoc, secondLightColor.r, secondLightColor.g, secondLightColor.b);
	glUniform3f(lightPositionLoc, lightPosition.x, lightPosition.y, lightPosition.z);
	glUniform3f(lightStrengthLoc, lightStrength.x, lightStrength.y, lightStrength.z);
	glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
	glutPostRedisplay();
	glBindTexture(GL_TEXTURE_2D, texture);
	//draw traingles
	glDrawArrays(GL_TRIANGLES, 0, 48);
	glBindVertexArray(0); //deactivate VAO
	glutSwapBuffers();
}

//create shader program
void UCreateShader()
{
	//vertex shader
	GLint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &lightVertexShaderSource, NULL); 
	glCompileShader(vertexShader); 
	
	//fragment shader
	GLint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); 
	glShaderSource(fragmentShader, 1, &lightFragmentShaderSource, NULL); 
	glCompileShader(fragmentShader); 
	
	//shader Program
	shaderProgram = glCreateProgram(); 
	glAttachShader(shaderProgram, vertexShader); //attatch vertex shader
	glAttachShader(shaderProgram, fragmentShader); //attatch fragment shader
	glLinkProgram(shaderProgram); //link shaders
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader); //delete shaders once used
}
void UCreateBuffers()
{
	GLfloat vertices[] = {
		//bottom bookshelf
		//X Y Z normals    //textures
		0.0f, 0.5f, -0.25f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, //top left corner vertex 0
		0.0f, 0.5f, 0.25f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //front top left corner vertex 1
		0.0f, 0.0f, -0.25f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //back bottom left conrer vetex 2
		0.0f, 0.0f, -0.25f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //back bottom left corner vertex 3
		0.0f, 0.0f, 0.25f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //front bottom left corner vertex 4
		0.0f, 0.5f, 0.25f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, //front top left corner vertex 5
		1.0f, 0.5f, -0.25f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, //back top right corner vertex 6
		1.0f, 0.5f, 0.25f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //front top riggt corner vertex 7
		1.0f, 0.0f, -0.25f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //back bottom right corner vertex 8
		1.0f, 0.0f, -0.25f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //back bottom right corner vertex 9
		1.0f, 0.0f, 0.25f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //front bottom right corner vertex 10
		1.0f, 0.5f, 0.25f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, //front top right corner vertex 11
		0.0f, 0.5f, -0.25f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, //top back left corner vertex 12
		0.0f, 0.5f, 0.25f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, //top front left corner vertex 13
		1.0f, 0.5f, -0.25f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, //top back right corner vertex 14
		1.0f, 0.5f, -0.25f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, //top back right corner vertex 15
		0.0f, 0.5f, 0.25f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, //top front left corner vertex 16
		1.0f, 0.5f, 0.25f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, //top front right coirner vertex 17
		0.0f, 0.0f, -0.25f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, //bottom back elft corner vertex 18
		0.0f, 0.0f, 0.25f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, //bottom front left corner vertex 19
		1.0f, 0.0f, -0.25f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, //bottom right corner vertex 20
		1.0f, 0.0f, -0.25f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, //bottom back right corner vertex 21
		0.0f, 0.0f, 0.25f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, //bottom front left corner vertex 22
		1.0f, 0.0f, 0.25f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, //bottom front right corner vertex 23
		
		//top bookshelf
		//X Y Z normals    //textures
		0.0f, 1.0f, -0.25f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, //back bottom left corner vertex 24
		0.0f, 1.0f, 0.25f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //front top left corner vertex 25
		0.0f, 0.5f, -0.25f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //back bottom left corner vertex 26
		0.0f, 0.5f, -0.25f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //back bottom left corner vertex 27
		0.0f, 0.5f, 0.25f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //front bottom left corner vertex 28
		0.0f, 1.0f, 0.25f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, //front top left corner vertex 29
		1.0f, 1.0f, -0.25f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, //back top right corner vertex 30
		1.0f, 1.0f, 0.25f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //front top right corner vertex 31
		1.0f, 0.5f, -0.25f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //back bottom right corner vertex 32
		1.0f, 0.5f, -0.25f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //back bottom right corner vertex 33
		1.0f, 0.5f, 0.25f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //front bottom right corner vertex 34
		1.0f, 1.0f, 0.25f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, //front top right corner vertex 35
		0.0f, 1.0f, -0.25f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, //top back left corner vertex 36
		0.0f, 1.0f, 0.25f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, //top front left corner vertex 37
		1.0f, 1.0f, -0.25f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, //top back right corner vertex 38
		1.0f, 1.0f, -0.25f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, //top back right corner vertex 39
		0.0f, 1.0f, 0.25f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, //top front left corner vertex 40
		1.0f, 1.0f, 0.25f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, //top front right corner vertex 41
		
		//Center Divider
		//X Y Z normals    //textures
		0.5f, 1.0f, -0.25f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, //middle back top corner vertex 42
		0.5f, 1.0f, 0.25f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //middle front top left corner vertex 43
		0.5f, 0.0f, -0.25f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //middle back bottom left corner vertex 44
		0.5f, 0.0f, -0.25f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //middle back bottom left corner vertex 45
		0.5f, 0.0f, 0.25f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //middle front bottom left corner vertex 46
		0.5f, 1.0f, 0.25f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f //middle front top left corner vertex 47
	};
	
	//generate buffer ids
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO); //activate VAO
	glBindVertexArray(VAO); //activate VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); 
	//set attribute pointer 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0); 
	//set attribute pointer 1 
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	//set attribute pointer 2
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0); //unbind VAO
}

//generate and load texture
void UGenerateTexture() {
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	int width, height;
	unsigned char* image = SOIL_load_image("BookcaseTexture.jpg", &width, &height, 0, SOIL_LOAD_RGB);//loads texture file
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0); //unbind texture
}
//UMouseClick
void UMouseClick(int button, int state, int x, int y)
{
	altDown = glutGetModifiers(); //alt key held
	//mouse click states
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {
		leftMouseButton = true;
	}
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP)) {
		leftMouseButton = false;
	}
	if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN)) {
		rightMouseButton = true;
	}
	if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_UP)) {
		rightMouseButton = false;
	}
}
//UMouseMove
void UMousePressedMove(int x, int y)
{
	//replace mouse with new coordinates
	coordinates
		if (mouseDetected)
		{
			lastMouseX = x;
			lastMouseY = y;
			mouseDetected = false;
		}
	//x and y directions
	mouseXOffset = x - lastMouseX;
	mouseYOffset = lastMouseY - y; //invert y
	lastMouseX = x;
	lastMouseY = y;
	//sensitivity
	mouseXOffset *= sensitivity;
	mouseYOffset *= sensitivity;

	if (altDown == true) {
		//yaw and pitch
		if (leftMouseButton == true) {
			cameraRotateAmt.x += mouseXOffset;
			cameraRotateAmt.y += mouseYOffset;
		}
		if (rightMouseButton == true) {
			zoom += mouseYOffset; //zoom
		}
	}
	else {
		yaw += mouseXOffset;
		pitch += mouseYOffset;
	}
}