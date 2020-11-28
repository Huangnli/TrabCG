// Compile command:
// Linux
// g++ --std=c++11 *.cpp -o main -lGLEW -lGL -lGLU -lglut
// To execute: ./main
// Compile command:
// Windows
// g++ --std=c++11 *.cpp -o main -lglew32 -lfreeglut -lglu32 -lopengl32

#include "OpenGLContext.h"
#include "lerComando.h"
#include "objeto.h"
#include "axis.h"
#include "light.h"

#include <iostream>
#include <cstdio>
#include <fstream>
#include <math.h>
#include <stdexcept>

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>               // glm::mat4
#include <glm/gtc/matrix_transform.hpp> //transformations
#include <glm/trigonometric.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>
#include <GL/freeglut.h> // ou glut.h - GLUT, include glu.h and gl.h
#include <GL/gl.h>

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#define MAX 50

namespace
{
    OpenGLContext *currentInstance = nullptr;
}
vector<objeto*> objetoVetor;
vector<axis*> axisVetor;
vector<light*> lightVetor;
lerComando ler;
int nVertices;
unsigned int vboid = 1;
unsigned int vaoid = 1;
unsigned int vboidAxis = 1;
unsigned int vaoidAxis = 1;
unsigned int vboidLight = 1;
unsigned int vaoidLight = 1;

OpenGLContext::OpenGLContext(int argc, char *argv[]){
    glutInit(&argc, argv); // Initialize GLUT

    glutInitContextVersion(3, 0); // IMPORTANT!
    //glutInitContextProfile(GLUT_CORE_PROFILE); // IMPORTANT! or later versions, core was introduced only with 3.2
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // IMPORTANT! Double buffering!

    glutInitWindowSize(640, 640);
    glutCreateWindow("Geometry"); // Create window with the given title

    glutDisplayFunc(OpenGLContext::glutRenderCallback);  // Register callback handler for window re-paint event
    glutReshapeFunc(OpenGLContext::glutReshapeCallback); // Register callback handler for window re-size event

    //glutKeyboardFunc(keyPressed);

    GLenum error = glewInit();

    if (error != GLEW_OK)
    {
        throw std::runtime_error("Error initializing GLEW.");
    }

    currentInstance = this;

    //this->initialize();
}

OpenGLContext::~OpenGLContext()
{
    this->finalize();
}

void OpenGLContext::glutReshapeCallback(int width, int height)
{
    // Set the viewport to cover the new window
    glViewport(0, 0, width, height);
}

void OpenGLContext::glutRenderCallback()
{
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
    //std::cout << "Compiling shader : " << filename << std::endl;

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

unsigned int OpenGLContext::linkShaderProgram(unsigned int vertexShaderId, unsigned int fragmentShaderId) const
{
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
    if (result == GL_FALSE)
    {
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

void OpenGLContext::createShaderObjects(){

    // Create and compile our GLSL program from the shaders
    GLint vertexShaderId = this->loadAndCompileShader("shader130/objeto.vp",
                                                      GL_VERTEX_SHADER);
    GLint fragmentShaderId = this->loadAndCompileShader("shader130/objeto.fp",
                                                        GL_FRAGMENT_SHADER);
    this->programId = this->linkShaderProgram(vertexShaderId, fragmentShaderId);
}

void OpenGLContext::createShaderAxis(){
    GLint vertexShaderAxisId = this->loadAndCompileShader("shader130/axis.vp",
                                                      GL_VERTEX_SHADER);
    GLint fragmentShaderAxisId = this->loadAndCompileShader("shader130/axis.fp",
                                                        GL_FRAGMENT_SHADER);
    this->programAxis = this->linkShaderProgram(vertexShaderAxisId, fragmentShaderAxisId);
}

void OpenGLContext::createShaderLight(){
    GLint lightVShaderId = this->loadAndCompileShader("shader130/light.vp",
                                                          GL_VERTEX_SHADER);
    GLint lightFShaderId = this->loadAndCompileShader("shader130/light.fp",
                                                          GL_FRAGMENT_SHADER);
    this->programLight = this->linkShaderProgram(lightVShaderId, lightFShaderId);

}

void OpenGLContext::loadObj(const char *path, vector<glm::vec3> &vbuffer){
    //temporary variables to store the .obj's contents:
        vector<unsigned int> vertexIndices, normalIndices;
        vector<glm::vec3> temp_vertices;
        vector<glm::vec3> temp_normals;

        //reading the file data
        FILE *file = fopen(path, "r");
        char line[MAX];

        if (file == NULL){
            printf("erro ao abrir o arquivo!\n");
        }
        int res = fscanf(file, "%s", line); //read the first word
        while (res != EOF){

            if (strcmp(line, "v") == 0){ //verify is it's a vertex position

                glm::vec3 vertex;
                fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z); //copy the coordinates to auxiliar vertex
                temp_vertices.push_back(vertex);                             //copy the coordinates to the temporary vecVertex
            }
            else if (strcmp(line, "vn") == 0){ //verify if it's a normal data
                glm::vec3 normal;
                fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
                temp_normals.push_back(normal);
            }

            else if (strcmp(line, "f") == 0)
            { //verify if it's a face, and make a index contents
                unsigned int vertexIndex[3], textureIndex[3], normalIndex[3];
                int matches = fscanf(file, "%d/%d/%d  %d/%d/%d  %d/%d/%d\n", &vertexIndex[0], &textureIndex[0], &normalIndex[0],
                                                                            &vertexIndex[1], &textureIndex[0], &normalIndex[1],
                                                                            &vertexIndex[2], &textureIndex[0], &normalIndex[2]);
                if (matches != 9){
                    printf("error in reading file! try another way.\n\n");
                    
                }
                //indexação dos vertices e normais
                vertexIndices.push_back(vertexIndex[0]);
                vertexIndices.push_back(vertexIndex[1]);
                vertexIndices.push_back(vertexIndex[2]);

                normalIndices.push_back(normalIndex[0]);
                normalIndices.push_back(normalIndex[1]);
                normalIndices.push_back(normalIndex[2]);
            }
            res = fscanf(file, "%s", line); //continue reading
        }
        fclose(file);

        //passing by each vertex (v/vn) of each triangule (face)
        //put the right vertex positions and normals on the out_vertexData
        for (unsigned int i = 0; i < vertexIndices.size(); i++)    {

            unsigned int indexV = vertexIndices[i];
            unsigned int indexN = normalIndices[i];

            glm::vec3 vertex = temp_vertices[indexV - 1];
            glm::vec3 normal = temp_normals[indexN - 1];
            vbuffer.push_back(vertex);
            vbuffer.push_back(normal);
        }
}

void OpenGLContext::initialize(){

    glEnable(GL_DEPTH_TEST);
    // Set "clearing" or background color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black and opaque

    if(ler.getEntrada().compare(0, 14, "add_shape cube")  == 0){
        //creating shaders
        createShaderObjects();

        vector<glm::vec3> vertexData;
        string name;
        //printf("%s\n", ler.getEntrada().c_str());
        for (int i = 15; i < ler.getEntrada().length(); ++i) {
            name.push_back(ler.getEntrada().at(i));
        }
        //printf("%s\n", name.c_str());

        loadObj("cube.obj", vertexData);
        nVertices = vertexData.size()/2;
        //printf("%d %u %u\n", nVertices, vaoid, vboid);
        
        //fazendo a transformação na model
        glm::mat4 model = glm::mat4(1.0); //gera uma identidade 4x4
        //model = glm::rotate(model, 45.0f, glm::vec3(0.2f, 0.6f, 0.0f)); //para cone
        //view
        glm::mat4 view =glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        //ajustando a camera
        glm::mat4 projection = glm::mat4(1.0);
        projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f);
        //cor 
        glm::vec3 cor = glm::vec3(1.0f,0.0f,1.0f);
    
        objeto *aux = new objeto(name, vaoid, vboid, vertexData, model, view, projection, cor);
        objetoVetor.push_back(aux);
    }

    if(ler.getEntrada().compare(0, 14, "add_shape cone")  == 0){
        createShaderObjects();

        vector<glm::vec3> vertexData;
        string name;
        // printf("%s\n", ler.getEntrada().c_str());
        for (int i = 15; i < ler.getEntrada().length(); ++i) {
            name.push_back(ler.getEntrada().at(i));
        }
        //printf("%s\n", name.c_str());

        loadObj("cone.obj", vertexData);
        nVertices = vertexData.size()/2;
        //printf("%d %u %u\n", nVertices, vaoid, vboid);

        //fazendo a transformação na model
        glm::mat4 model = glm::mat4(1.0); //gera uma identidade 4x4
        model = glm::rotate(model, 45.0f, glm::vec3(0.2f, 0.6f, 0.0f)); //para cone
        //view
        glm::mat4 view =glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        //ajustando a camera
        glm::mat4 projection = glm::mat4(1.0);
        projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f);
        //cor 
        glm::vec3 cor = glm::vec3(1.0f,1.0f,1.0f);

        objeto *aux = new objeto(name, vaoid, vboid, vertexData, model, view, projection, cor);
        objetoVetor.push_back(aux);
    }

    if(ler.getEntrada().compare(0, 15, "add_shape torus")  == 0){
        createShaderObjects();

        vector<glm::vec3> vertexData;
        string name;
        // printf("%s\n", ler.getEntrada().c_str());
        for (int i = 16; i < ler.getEntrada().length(); ++i) {
            name.push_back(ler.getEntrada().at(i));
        }
        //printf("%s\n", name.c_str());

        loadObj("torus.obj", vertexData);
        nVertices = vertexData.size()/2;
        //printf("%d %u %u\n", nVertices, vaoid, vboid);

        //fazendo a transformação na model
        glm::mat4 model = glm::mat4(1.0); //gera uma identidade 4x4
        model = glm::rotate(model, 45.0f, glm::vec3(0.2f, 0.6f, 0.0f)); //para cone
        //view
        glm::mat4 view =glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        //ajustando a camera
        glm::mat4 projection = glm::mat4(1.0);
        projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f);
        //cor 
        glm::vec3 cor = glm::vec3(1.0f,1.0f,1.0f);

        objeto *aux = new objeto(name, vaoid, vboid, vertexData, model, view, projection, cor);
        objetoVetor.push_back(aux);
    }

    if(ler.getEntrada().compare(0, 16, "add_shape sphere")  == 0){
        createShaderObjects();
        vector<glm::vec3> vertexData;
        string name;
        // printf("%s\n", ler.getEntrada().c_str());
        for (int i = 17; i < ler.getEntrada().length(); ++i) {
            name.push_back(ler.getEntrada().at(i));
        }
        //printf("%s\n", name.c_str());

        loadObj("sphere.obj", vertexData);
        nVertices = vertexData.size()/2;
        //printf("%d %u %u\n", nVertices, vaoid, vboid);

        //fazendo a transformação na model
        glm::mat4 model = glm::mat4(1.0); //gera uma identidade 4x4
        model = glm::rotate(model, 45.0f, glm::vec3(0.2f, 0.6f, 0.0f)); //para cone
        //view
        glm::mat4 view =glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        //ajustando a camera
        glm::mat4 projection = glm::mat4(1.0);
        projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f);
        //cor 
        glm::vec3 cor = glm::vec3(1.0f,1.0f,1.0f);

        objeto *aux = new objeto(name, vaoid, vboid, vertexData, model, view, projection, cor);
        objetoVetor.push_back(aux);
    }
    //pra remover os objetos
    if(ler.getEntrada().compare(0, 12, "remove_shape")  == 0){
        string name;
        //pegar o nome que foi digitado
        for (int i = 13; i < ler.getEntrada().length(); ++i) {
            name.push_back(ler.getEntrada().at(i));
        }
        //apagar o objeto
        for(int i = 0; i < objetoVetor.size(); i++){
			if(strcmp(name.c_str(), objetoVetor[i]->nome.c_str()) == 0) {
				objetoVetor.erase(objetoVetor.begin()+i);
			}
		}
    }

    for(int i = 0; i < objetoVetor.size(); i++){
        glGenVertexArrays(1, static_cast<GLuint *>(&objetoVetor[i]->vao));
        glBindVertexArray(objetoVetor[i]->vao);

        glGenBuffers(1, static_cast<GLuint *>(&objetoVetor[i]->vbo));
        glBindBuffer(GL_ARRAY_BUFFER, objetoVetor[i]->vbo);
        glBufferData(GL_ARRAY_BUFFER, objetoVetor[i]->vertexBuffer.size() * sizeof(glm::vec3), objetoVetor[i]->vertexBuffer.data(), GL_STATIC_DRAW);
        //able the first buffer
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        //passando a localização dos atributos para o shader - 0= inicio do VBO
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
        glBindAttribLocation(this->programId, 0, "vertexPosition"); //vertexPosition = name of attribute in shader
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
        glBindAttribLocation(this->programId, 0, "vertexNormal"); //vertexPosition = name of attribute in shader

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        vaoid++;
        vboid++;
    }

    
    if (ler.getEntrada().compare(0, 7, "axis_on") == 0){
        //criando shaders para axis
        createShaderAxis();

        axis *aux = new axis(vaoidAxis, vboidAxis);
        axisVetor.push_back(aux);
    }

    if(ler.getEntrada().compare(0, 8, "axis_off")  == 0){
        //apagar o objeto
		axisVetor.erase(axisVetor.begin());
    }

    for(int i = 0; i < axisVetor.size(); i++){
        glGenVertexArrays(1, static_cast<GLuint *>(&axisVetor[i]->vao));
        glBindVertexArray(axisVetor[i]->vao);

        glGenBuffers(1, static_cast<GLuint *>(&axisVetor[i]->vbo));
        glBindBuffer(GL_ARRAY_BUFFER, axisVetor[i]->vbo);
        glBufferData(GL_ARRAY_BUFFER, axisVetor[i]->axisBuffer.size() * sizeof(glm::vec3), axisVetor[i]->axisBuffer.data(), GL_STATIC_DRAW);
        //able the first buffer
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        //passando a localização dos atributos para o shader - 0= inicio do VBO
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
        glBindAttribLocation(this->programAxis, 0, "axisPosition");
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
        glBindAttribLocation(this->programAxis, 0, "colorAxis");

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        vaoidAxis++;
        vboidAxis++;
    }

        // glm::vec3 position = glm::vec3(1.0f, 0.0f, 0.0f);
        // glm::vec3 color = glm::vec3(1.0f, 1.0f, 0.0f);
        // light light(position, color);
        // vector<glm::vec3> lightData;
        // lightData.push_back(light.getPosition());
        // lightData.push_back(light.getColor());

        
        // glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        // glBufferData(GL_ARRAY_BUFFER, lightData.size() * sizeof(glm::vec3), lightData.data(), GL_STATIC_DRAW);

        // //able the first buffer
        // glEnableVertexAttribArray(0);
        // glEnableVertexAttribArray(1);

        // //passando a localização dos atributos para o shader - 0= inicio do VBO
        // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
        // glBindAttribLocation(this->programLight, 0, "lightPosition");
        // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
        // glBindAttribLocation(this->programLight, 0, "outcolor");

    }

void OpenGLContext::rendering() const
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // Clear the colorbuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //desenhando formas
    for(int i = 0; i < objetoVetor.size(); i++){
        
        //printf("%d\n", objetoVetor[i]->vertexBuffer.size()/2);
        glUseProgram(this->programId);
        glBindVertexArray(objetoVetor[i]->vao);
        glBindBuffer(GL_ARRAY_BUFFER, objetoVetor[i]->vbo);

        //model
        int modelLoc = glGetUniformLocation(programId, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &objetoVetor[i]->model[0][0]);

        //view
        int viewLoc = glGetUniformLocation(programId, "view");
	    glUniformMatrix4fv(viewLoc, 1, false, &objetoVetor[i]->view[0][0]);

        //ajustando a camera
        int projLoc = glGetUniformLocation(programId, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &objetoVetor[i]->projection[0][0]);

        //setting up color from shader by uniform
        int colorLoc = glGetUniformLocation(programId, "outColor");
        glUniform3fv(colorLoc, 1, &objetoVetor[i]->outColor[0]);

        glDrawArrays(GL_TRIANGLES, 0, objetoVetor[i]->vertexBuffer.size()/2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);
    }

    //desenhando eixos
    for(int i = 0; i < axisVetor.size(); i++){        
        //printf("%d\n", objetoVetor[i]->vertexBuffer.size()/2);
        glUseProgram(this->programAxis);
        glBindVertexArray(axisVetor[i]->vao);
        glBindBuffer(GL_ARRAY_BUFFER, axisVetor[i]->vbo);

        glDrawArrays(GL_LINES, 0, axisVetor[i]->axisBuffer.size()/2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);
    }


    // glUseProgram(this->programLight);

    // glBindVertexArray(vao);
    // glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    // glEnableVertexAttribArray(0);
    // glEnableVertexAttribArray(1);
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));

    // glPointSize(10.0f);
    // glDrawArrays(GL_POINTS, 0, 1);

    glutSwapBuffers(); //necessario para windows!
}

void OpenGLContext::printVersion() const
{
    std::string glRender = reinterpret_cast<char const *>(glGetString(
        GL_RENDERER));
    std::string glVersion = reinterpret_cast<char const *>(glGetString(
        GL_VERSION));
    std::string glslVersion = reinterpret_cast<char const *>(glGetString(
        GL_SHADING_LANGUAGE_VERSION));

    std::cout << "OpenGL Renderer  : " << glRender << '\n'
              << "OpenGL Version   : " << glVersion << '\n'
              << "OpenGLSL Version : " << glslVersion << std::endl;
}

void OpenGLContext::runLoop() const
{
    this->rendering();
    glutMainLoop();
    
}

void OpenGLContext::finalize() const
{
    // Properly de-allocate all resources once they've outlived their purpose
    glDisableVertexAttribArray(0);
    glDeleteBuffers(1, (GLuint *)(&(this->vbo)));
    glUseProgram(0);
}

int main(int argc, char *argv[])
{
    OpenGLContext context{argc, argv};
    ler.ler();
    while( ler.getEntrada().compare("quite") != 0){ //comparar se a entrada eh igual quite
        // leitura do comando
        //printf("oi%s\n", ler.getEntrada().c_str());
        
        context.initialize();
        context.rendering();

        ler.deleteEntrada();
        ler.ler();
        //printf("%s\n", ler.getEntrada().c_str());
    }
    ler.deleteEntrada();
    context.printVersion();
    //context.runLoop();
    return 0;
}
