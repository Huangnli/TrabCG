// Compile command:
// Linux
// g++ --std=c++11 *.cpp -o main -lGLEW -lGL -lGLU -lglut
// To execute: ./main
// Compile command:
// Windows
// g++ --std=c++11 *.cpp -o main -lglew32 -lfreeglut -lglu32 -lopengl32

#include "objeto.h"
#include "openGLcontext.h"
#include <string>
#include <vector>
#include <iostream>
#include <cstdio>
#include <fstream>
#include <math.h>
#include <stdexcept>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> //transformations
#include <glm/trigonometric.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>
#include <GL/freeglut.h>  // ou glut.h - GLUT, include glu.h and gl.h
#include <GL/gl.h>

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#define MAX 50

namespace {
OpenGLContext* currentInstance = nullptr;
}
int nVertices;
OpenGLContext::OpenGLContext(int argc, char *argv[]) {
	glutInit(&argc, argv);     // Initialize GLUT

	glutInitContextVersion(3, 0);               // IMPORTANT!
	//glutInitContextProfile(GLUT_CORE_PROFILE); // IMPORTANT! or later versions, core was introduced only with 3.2
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // IMPORTANT! Double buffering!

	glutInitWindowSize(600, 600);
	glutCreateWindow("Cube");      // Create window with the given title

	glutDisplayFunc(OpenGLContext::glutRenderCallback); // Register callback handler for window re-paint event
	glutReshapeFunc(OpenGLContext::glutReshapeCallback); // Register callback handler for window re-size event

    //glutKeyboardFunc(keyPressed);

	GLenum error = glewInit();

	if (error != GLEW_OK) {
		throw std::runtime_error("Error initializing GLEW.");
	}

	currentInstance = this;

	this->initialize();
}

OpenGLContext::~OpenGLContext() {
	this->finalize();
}

void OpenGLContext::glutReshapeCallback(int width, int height)
{
    // Set the viewport to cover the new window
    glViewport(0, 0, width, height);
}

void OpenGLContext::glutRenderCallback(){
    //glutPostRedisplay();
    currentInstance->rendering();
}

unsigned int OpenGLContext::loadAndCompileShader(const std::string &filename,
                                                 const int glType) const
{
    std::string shaderCode;

    // Read the Vertex Shader code from the file
    {
        // Open file stream
        std::ifstream shaderStream(filename, std::ios::in);

        if (shaderStream.good() == false)
        {
            throw std::runtime_error(
                "Error when opening " + filename + ". Are you in the right directory ?");
        }

        // Copy file contents to string
        shaderCode.assign((std::istreambuf_iterator<char>(shaderStream)),
                          (std::istreambuf_iterator<char>()));

    } // End of scope, RAII shaderStream closed.

    // Some temporaries for using in the code ahead...
    char const *sourcePointer = shaderCode.c_str(); // C style pointer to string
    GLint result = GL_FALSE;                        // Return result for OGL API calls
    int infoLogLength = 0;                          // Length of log (in case of error)
    unsigned int shaderId = glCreateShader(glType); // Create a shader on GPU

    // Compile Shader
    std::cout << "Compiling shader : " << filename << std::endl;

    glShaderSource(shaderId, 1, &sourcePointer, nullptr);
    glCompileShader(shaderId);

    // Check Shader
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);

    // In case of error...
    if (result == GL_FALSE)
    {
        // Allocate vector of char to store error message
        std::vector<char> errorMessage(infoLogLength + 1);

        // Copy error message to string
        glGetShaderInfoLog(shaderId, infoLogLength, nullptr, &errorMessage[0]);

        // Print error message
        std::cerr << "Error compiling " << filename << '\n'
                  << "  "
                  << std::string(errorMessage.begin(), errorMessage.end())
                  << std::endl;
    }

    return shaderId;
}

unsigned int OpenGLContext::linkShaderProgram(unsigned int vertexShaderId, unsigned int fragmentShaderId) const{
    // Create shader program on GPU
    unsigned int shaderProgramId = glCreateProgram();

    // Attach shader to be linked
    glAttachShader(shaderProgramId, vertexShaderId);
    glAttachShader(shaderProgramId, fragmentShaderId);

    // Setup Vertex Attributes (only for GL < 3.3 and GLSL < 3.3)

    // Link the program!
    glLinkProgram(shaderProgramId);

    // Some temporaries for using in the code ahead...
    GLint result = GL_FALSE;
    int InfoLogLength = 0;

    // Check the program
    glGetProgramiv(shaderProgramId, GL_LINK_STATUS, &result);
    glGetProgramiv(shaderProgramId, GL_INFO_LOG_LENGTH, &InfoLogLength);

    // In case of error...
    if (result == GL_FALSE){
        // Allocate vector of char to store error message
        std::vector<char> errorMessage(InfoLogLength + 1);

        // Copy error message to string
        glGetProgramInfoLog(shaderProgramId, InfoLogLength, nullptr,
                            &errorMessage[0]);

        // Print error message
        std::cerr << std::string(errorMessage.begin(), errorMessage.end())
                  << std::endl;
    }

    // Detach...
    glDetachShader(shaderProgramId, vertexShaderId);
    glDetachShader(shaderProgramId, fragmentShaderId);

    // ...  and delete the shaders source code, work done
    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);

    return shaderProgramId;
}



void OpenGLContext::initialize() {
    // Set "clearing" or background color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black and opaque

    // Create and compile our GLSL program from the shaders
	GLint vertexShaderId = this->loadAndCompileShader("shader130/objeto.vp",
	GL_VERTEX_SHADER);
	GLint fragmentShaderId = this->loadAndCompileShader("shader130/objeto.fp",
	GL_FRAGMENT_SHADER);
	this->programId = this->linkShaderProgram(vertexShaderId, fragmentShaderId);

    // setting up cube vertex data
    objeto cubo("cube.obj");
    vector<glm::vec3> vertexData = cubo.getVertexBuffer();
    nVertices = vertexData.size()/2;
    
    //create and bind the vao
    glGenVertexArrays(1, static_cast<GLuint *>(&vao));
    glBindVertexArray(vao);

    //create the vbo buffer and bind it to vertexData and set the attribute pointer(s)
    glGenBuffers(1, (GLuint *)(&vbo));   // VBO    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size()*sizeof(glm::vec3), vertexData.data(), GL_STATIC_DRAW);

    //able the first buffer
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    //passando a localização dos atributos para o shader - 0= inicio do VBO
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), (GLvoid *)0);
    glBindAttribLocation(this->programId, 0, "vertexPosition");     //vertexPosition = name of attribute in shader
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3*sizeof(GLfloat)));
    glBindAttribLocation(this->programId, 0, "vertexNormal"); //vertexPosition = name of attribute in shader

    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void OpenGLContext::rendering( ) const {
    	
    glEnable(GL_DEPTH_TEST);
    
    // Clear the colorbuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //load everything back
    glUseProgram(this->programId);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));

    //fazendo a transformação na model
    glm::mat4 model = glm::mat4(1.0); //gera uma identidade 4x4
    model = glm::rotate(model, 45.0f, glm::vec3(1.0f, 1.0f, 1.0f));
    int modelLoc = glGetUniformLocation(programId, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

    //ajustando a camera
    glm::mat4 projection = glm::mat4(1.0);
    projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 5.0f, -5.0f);
    int projLoc = glGetUniformLocation(programId, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

    //setting up color from shader by uniform
    int location = glGetUniformLocation(programId, "outColor1");
    glUniform3f(location, 1.0f, 0.0f, 1.0f);
    
    glDrawArrays(GL_TRIANGLES, 0, 36);

    //clean things up
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    glutSwapBuffers();    //necessario para windows!
}

void OpenGLContext::printVersion() const {
	std::string glRender = reinterpret_cast<char const*>(glGetString(
	GL_RENDERER));
	std::string glVersion = reinterpret_cast<char const*>(glGetString(
	GL_VERSION));
	std::string glslVersion = reinterpret_cast<char const*>(glGetString(
	GL_SHADING_LANGUAGE_VERSION));

	std::cout << "OpenGL Renderer  : " << glRender << '\n'
			<< "OpenGL Version   : " << glVersion << '\n'
			<< "OpenGLSL Version : " << glslVersion << std::endl;
}

void OpenGLContext::runLoop() const {
	this->rendering();

	glutMainLoop();
}

void OpenGLContext::finalize() const {
	// Properly de-allocate all resources once they've outlived their purpose
	glDisableVertexAttribArray(0);
    glDeleteBuffers(1, (GLuint *)(&(this->vbo)));
    glUseProgram(0);
}

int main(int argc, char *argv[]) {
	OpenGLContext context { argc, argv };

	context.printVersion();
	context.runLoop();

	return 0;
}
