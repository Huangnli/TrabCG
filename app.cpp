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
#include "camera.h"

#include <iostream>
#include <cstdio>
#include <fstream>
#include <stdexcept>

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>               // glm::mat4
#include <glm/gtc/matrix_transform.hpp> //transformations
#include <glm/trigonometric.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform2.hpp> //usar shear

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
vector<light*> lightVetor;
axis *axisScene;
camera *cam;
glm::mat4 view;

lerComando ler;

unsigned int vboid = 1;
unsigned int vaoid = 1;
unsigned int vboidAxis = 1;
unsigned int vaoidAxis = 1;
unsigned int vboidLight = 1;
unsigned int vaoidLight = 1;

int flat_on = 0;
int smooth_on = 0;
int phong_on = 0;
int lights_on;
int axis_on;
int wire = 0;

float Ka = 0.2f;
float Kd = 0.2f;
float Ks = 0.2f;

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

}

OpenGLContext::~OpenGLContext()
{

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

void OpenGLContext::createShadersNone(){

    // Create and compile our GLSL program from the shaders
    GLint vertexShaderId = this->loadAndCompileShader("shader130/none.vp",
                                                      GL_VERTEX_SHADER);
    GLint fragmentShaderId = this->loadAndCompileShader("shader130/none.fp",
                                                        GL_FRAGMENT_SHADER);
    this->programNone = this->linkShaderProgram(vertexShaderId, fragmentShaderId);
}

void OpenGLContext::createShadersFlat(){

    // Create and compile our GLSL program from the shaders
    GLint vertexShaderId = this->loadAndCompileShader("shader130/flat.vp",
                                                      GL_VERTEX_SHADER);
    GLint fragmentShaderId = this->loadAndCompileShader("shader130/flat.fp",
                                                        GL_FRAGMENT_SHADER);
    this->programFlat = this->linkShaderProgram(vertexShaderId, fragmentShaderId);
}

void OpenGLContext::createShadersSmooth(){

    // Create and compile our GLSL program from the shaders
    GLint vertexShaderId = this->loadAndCompileShader("shader130/smooth.vp",
                                                      GL_VERTEX_SHADER);
    GLint fragmentShaderId = this->loadAndCompileShader("shader130/smooth.fp",
                                                        GL_FRAGMENT_SHADER);
    this->programSmooth = this->linkShaderProgram(vertexShaderId, fragmentShaderId);
}

void OpenGLContext::createShadersPhong(){

    // Create and compile our GLSL program from the shaders
    GLint vertexShaderId = this->loadAndCompileShader("shader130/phong.vp",
                                                      GL_VERTEX_SHADER);
    GLint fragmentShaderId = this->loadAndCompileShader("shader130/phong.fp",
                                                        GL_FRAGMENT_SHADER);
    this->programPhong = this->linkShaderProgram(vertexShaderId, fragmentShaderId);
}

void OpenGLContext::createShadersAxis(){
    GLint vertexShaderAxisId = this->loadAndCompileShader("shader130/axis.vp",
                                                      GL_VERTEX_SHADER);
    GLint fragmentShaderAxisId = this->loadAndCompileShader("shader130/axis.fp",
                                                        GL_FRAGMENT_SHADER);
    this->programAxis = this->linkShaderProgram(vertexShaderAxisId, fragmentShaderAxisId);
}

void OpenGLContext::createShadersLight(){
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

    //ajustando a camera (view matrix)
    view = cam->getViewMatrix();   //inicialização    
    
    //ajustando a projeção
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f);
    //projection = glm::frustum(-1.0f, 1.0f, -1.0f, 1.0f, 0.5f, 2.0f);
    //criando axis antes pois cam e lookat usa ele tbm
    axis *aux = new axis(vaoidAxis, vboidAxis, view, projection);
    axisScene = aux;

    //define o lookAt
    if (ler.getEntrada().compare(0, 6, "lookat") == 0){

        int i = 7;
        string fl1, fl2, fl3;
        //pegar os floats
        while (ler.getEntrada().at(i) != ' '){
            fl1.push_back(ler.getEntrada().at(i));
            i++;
        }
        i++;
        while (ler.getEntrada().at(i) != ' '){
            fl2.push_back(ler.getEntrada().at(i));
            i++;
        }
        i++;
        while (i < ler.getEntrada().length()){
            fl3.push_back(ler.getEntrada().at(i));
            i++;
        }

        glm::vec3 vecLookAt = glm::vec3(stof(fl1), stof(fl2), stof(fl3));
        cam->setLookat(vecLookAt);

        //atualização para os objetos que serão desenhados após a mudança da camera
        view = cam->getViewMatrix();    

        //atualização para os objetos que já foram add à cena antes da mudança da camera
        for(int j = 0; j < objetoVetor.size(); j++){
				objetoVetor[j]->view = view;
        }
        
        for (int i = 0; i < lightVetor.size(); i++){
            lightVetor[i]->view = view;
        }

        axisScene->view = view;
    }

    //configura posição da camera
    if (ler.getEntrada().compare(0, 3, "cam") == 0){
        int i = 4;
        string fl1, fl2, fl3;
        //pegar os floats
        while (ler.getEntrada().at(i) != ' ')  {
            fl1.push_back(ler.getEntrada().at(i));
            i++;
        }
        i++;
        while (ler.getEntrada().at(i) != ' ') {
            fl2.push_back(ler.getEntrada().at(i));
            i++;
        }
        i++;
        while (i < ler.getEntrada().length()) {
            fl3.push_back(ler.getEntrada().at(i));
            i++;
        }

        glm::vec3 vecPos = glm::vec3(stof(fl1), stof(fl2), stof(fl3));
        cam->setPosition(vecPos);

        //atualização para os objetos que serão desenhados após a mudança da camera
        view = cam->getViewMatrix();    

        //atualização para os objetos que já foram add à cena antes da mudança da camera
        for(int j = 0; j < objetoVetor.size(); j++){
				objetoVetor[j]->view = view;
        }
        
        for (int i = 0; i < lightVetor.size(); i++){
            lightVetor[i]->view = view;
        }        
            
        axisScene->view = view;
    }

    //add cubo
    if(ler.getEntrada().compare(0, 14, "add_shape cube")  == 0){
        //creating shaders
        

        vector<glm::vec3> vertexData;
        string name;
        //printf("%s\n", ler.getEntrada().c_str());
        for (int i = 15; i < ler.getEntrada().length(); ++i) {
            name.push_back(ler.getEntrada().at(i));
        }
        //printf("%s\n", name.c_str());

        loadObj("objs/cube.obj", vertexData);
        
        //inicialização da model
        glm::mat4 model = glm::mat4(1.0); //gera uma identidade 4x4

        //cor 
        glm::vec3 cor = glm::vec3(1.0f,1.0f,1.0f);
    
        objeto *aux = new objeto(name, vaoid, vboid, vertexData, model, view, projection, cor);
        objetoVetor.push_back(aux);
    }
    
    //add cone
    if(ler.getEntrada().compare(0, 14, "add_shape cone")  == 0){
        

        vector<glm::vec3> vertexData;
        string name;
        // printf("%s\n", ler.getEntrada().c_str());
        for (int i = 15; i < ler.getEntrada().length(); ++i) {
            name.push_back(ler.getEntrada().at(i));
        }
        //printf("%s\n", name.c_str());

        loadObj("objs/cone.obj", vertexData);
       
        //inicialização da model
        glm::mat4 model = glm::mat4(1.0); //gera uma identidade 4x4
        //model = glm::rotate(model, 45.0f, glm::vec3(0.2f, 0.6f, 0.0f)); //para cone
        
        //cor 
        glm::vec3 cor = glm::vec3(1.0f,1.0f,1.0f);

        objeto *aux = new objeto(name, vaoid, vboid, vertexData, model, view, projection, cor);
        objetoVetor.push_back(aux);
    }
    
    //add torus
    if(ler.getEntrada().compare(0, 15, "add_shape torus")  == 0){
        

        vector<glm::vec3> vertexData;
        string name;
        // printf("%s\n", ler.getEntrada().c_str());
        for (int i = 16; i < ler.getEntrada().length(); ++i) {
            name.push_back(ler.getEntrada().at(i));
        }
        //printf("%s\n", name.c_str());

        loadObj("objs/torus.obj", vertexData);

        //inicialização da model
        glm::mat4 model = glm::mat4(1.0); //gera uma identidade 4x4
        //model = glm::rotate(model, 45.0f, glm::vec3(0.2f, 0.6f, 0.0f)); //para cone
       
        //cor 
        glm::vec3 cor = glm::vec3(1.0f,1.0f,1.0f);

        objeto *aux = new objeto(name, vaoid, vboid, vertexData, model, view, projection, cor);
        objetoVetor.push_back(aux);
    }

    //add sphere
    if(ler.getEntrada().compare(0, 16, "add_shape sphere")  == 0){
      
        vector<glm::vec3> vertexData;
        string name;
        // printf("%s\n", ler.getEntrada().c_str());
        for (int i = 17; i < ler.getEntrada().length(); ++i) {
            name.push_back(ler.getEntrada().at(i));
        }
        //printf("%s\n", name.c_str());

        loadObj("objs/sphere.obj", vertexData);
       
        //inicialização da model
        glm::mat4 model = glm::mat4(1.0); //gera uma identidade 4x4
        
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
    
    //add color to objects
    if(ler.getEntrada().compare(0, 5, "color")  == 0){
        string name;
        int i = 6;
        string fl1, fl2, fl3;
        //pegar o nome que foi digitado
        while (ler.getEntrada().at(i) != ' ' ) {
             name.push_back(ler.getEntrada().at(i));
            i++;
        }
        i++;
        while (ler.getEntrada().at(i) != ' ' ) {
             fl1.push_back(ler.getEntrada().at(i));
            i++;
        }
        i++;
        while (ler.getEntrada().at(i) != ' ' ) {
             fl2.push_back(ler.getEntrada().at(i));
            i++;
        }
        i++;
        while (i < ler.getEntrada().length()) {
             fl3.push_back(ler.getEntrada().at(i));
            i++;
        }
                
        // modifica a cor do objeto informado com 'name'
        for(int j = 0; j < objetoVetor.size(); j++){
			if(strcmp(name.c_str(), objetoVetor[j]->nome.c_str()) == 0){
				glm::vec3 color = glm::vec3(stof(fl1), stof(fl2), stof(fl3));
				objetoVetor[j]->outColor = color;
			}
        }
    }

    //add transladar
    if(ler.getEntrada().compare(0, 9, "translate")  == 0){
        string name;
        int i = 10;
        string fl1, fl2, fl3;
        //pegar o nome que foi digitado
        while (ler.getEntrada().at(i) != ' ' ) {
             name.push_back(ler.getEntrada().at(i));
            i++;
        }
        i++;
        while (ler.getEntrada().at(i) != ' ' ) {
             fl1.push_back(ler.getEntrada().at(i));
            i++;
        }
        i++;
        while (ler.getEntrada().at(i) != ' ' ) {
             fl2.push_back(ler.getEntrada().at(i));
            i++;
        }
        i++;
        while ( i < ler.getEntrada().length()) {
             fl3.push_back(ler.getEntrada().at(i));
            i++;
        }

        for(int j = 0; j < objetoVetor.size(); j++){
			if(strcmp(name.c_str(), objetoVetor[j]->nome.c_str()) == 0){
				glm::mat4 model = glm::translate( glm::mat4(1.0f), glm::vec3(stof(fl1),stof(fl2),stof(fl3)) );
				objetoVetor[j]->model = objetoVetor[j]->model * model;
			}
        }
    }

    //add escala
    if(ler.getEntrada().compare(0, 5, "scale")  == 0){
        string name;
        int i = 6;
        string fl1, fl2, fl3;
        //pegar o nome que foi digitado
        while (ler.getEntrada().at(i) != ' ' ) {
             name.push_back(ler.getEntrada().at(i));
            i++;
        }
        i++;
        while (ler.getEntrada().at(i) != ' ' ) {
             fl1.push_back(ler.getEntrada().at(i));
            i++;
        }
        i++;
        while (ler.getEntrada().at(i) != ' ' ) {
             fl2.push_back(ler.getEntrada().at(i));
            i++;
        }
        i++;
        while ( i < ler.getEntrada().length()) {
             fl3.push_back(ler.getEntrada().at(i));
            i++;
        }

        for(int j = 0; j < objetoVetor.size(); j++){
			if(strcmp(name.c_str(), objetoVetor[j]->nome.c_str()) == 0){
				glm::mat4 model = glm::scale( glm::mat4(1.0f), glm::vec3(stof(fl1),stof(fl2),stof(fl3)) );
				objetoVetor[j]->model = objetoVetor[j]->model * model;
			}
        }
    }

    //add shear
    if(ler.getEntrada().compare(0, 5, "shear")  == 0){
        string name;
        int i = 6;
        string fl1, fl2, fl3, fl4, fl5, fl6;
        //pegar o nome que foi digitado
        while (ler.getEntrada().at(i) != ' ' ) {
             name.push_back(ler.getEntrada().at(i));
            i++;
        }
        i++;
        while (ler.getEntrada().at(i) != ' ' ) {
             fl1.push_back(ler.getEntrada().at(i));
            i++;
        }
        i++;
        while (ler.getEntrada().at(i) != ' ' ) {
             fl2.push_back(ler.getEntrada().at(i));
            i++;
        }
        i++;
        while (ler.getEntrada().at(i) != ' ') {
             fl3.push_back(ler.getEntrada().at(i));
            i++;
        }
        i++;
        while (ler.getEntrada().at(i) != ' ' ) {
             fl4.push_back(ler.getEntrada().at(i));
            i++;
        }
        i++;
        while (ler.getEntrada().at(i) != ' ' ) {
             fl5.push_back(ler.getEntrada().at(i));
            i++;
        }
        i++;
        while ( i < ler.getEntrada().length()) {
             fl6.push_back(ler.getEntrada().at(i));
            i++;
        }

        for(int j = 0; j < objetoVetor.size(); j++){
			if(strcmp(name.c_str(), objetoVetor[j]->nome.c_str()) == 0){
				glm::mat4 model = glm::shearX3D( glm::mat4(1.0f), stof(fl1), stof(fl2));
                glm::mat4 model2 = glm::shearY3D( glm::mat4(1.0f), stof(fl3), stof(fl4));
                glm::mat4 model3 = glm::shearZ3D( glm::mat4(1.0f), stof(fl5), stof(fl6));
				objetoVetor[j]->model = objetoVetor[j]->model * model * model2 * model3;
			}
        }
    }
    
    //add rotacao
    if(ler.getEntrada().compare(0, 6, "rotate")  == 0){
        string name;
        int i = 7;
        string fl1, fl2, fl3, fl4;
        //pegar o nome que foi digitado
        while (ler.getEntrada().at(i) != ' ' ) {
             name.push_back(ler.getEntrada().at(i));
            i++;
        }
        i++;
        while (ler.getEntrada().at(i) != ' ' ) {
             fl1.push_back(ler.getEntrada().at(i));
            i++;
        }
        i++;
        while (ler.getEntrada().at(i) != ' ' ) {
             fl2.push_back(ler.getEntrada().at(i));
            i++;
        }
        i++;
        while (ler.getEntrada().at(i) != ' ') {
             fl3.push_back(ler.getEntrada().at(i));
            i++;
        }
        i++;
        while ( i < ler.getEntrada().length()) {
             fl4.push_back(ler.getEntrada().at(i));
            i++;
        }
        // printf("%s %f %f %f %f\n", name.c_str(), num, num2, num3, num4);
        // modifica a matriz model do objeto informado com 'name'
        for(int j = 0; j < objetoVetor.size(); j++){
			if(strcmp(name.c_str(), objetoVetor[j]->nome.c_str()) == 0){
				glm::mat4 model = glm::rotate( glm::mat4(1.0f), glm::radians(stof(fl1)) , glm::vec3(stof(fl2),stof(fl3),stof(fl4)) );
				objetoVetor[j]->model = objetoVetor[j]->model * model;
			}
        }
    }

    //show axis
    if (ler.getEntrada().compare(0, 7, "axis_on") == 0){
        //criando shaders para axis
        createShadersAxis();
        axis_on = 1;
    }   
    
    //hide axis
    if(ler.getEntrada().compare(0, 8, "axis_off")  == 0){
        //desliga o eixo  
		axis_on = 0;
        
    }    
    
    //add ponto de luz
    if(ler.getEntrada().compare(0, 9, "add_light")  == 0){
        //createShadersFlat();
        //createShadersSmooth();
        //createShadersPhong();

        if (lightVetor.size() == 10){
            printf("Número máximo de luzes atingido!\nRemova uma luz para adicionar outra nova.\n");
        }
        else{
            string name;
            int i = 10;
            string fl1, fl2, fl3;
            //pegar o nome que foi digitado
            while (ler.getEntrada().at(i) != ' ' ) {
                name.push_back(ler.getEntrada().at(i));
                i++;
            }
            i++;
            while (ler.getEntrada().at(i) != ' ' ) {
                fl1.push_back(ler.getEntrada().at(i));
                i++;
            }
            i++;
            while (ler.getEntrada().at(i) != ' ' ) {
                fl2.push_back(ler.getEntrada().at(i));
                i++;
            }
            i++;
            while (i < ler.getEntrada().length()) {
                fl3.push_back(ler.getEntrada().at(i));
                i++;
            }
            glm::vec3 pos = glm::vec3(stof(fl1), stof(fl2), stof(fl3));

            light *luz = new light(name, pos, vaoidLight, vboidLight, view, projection);
            lightVetor.push_back(luz);
   
        }
    }

    //pra remover a luz selecionada
    if(ler.getEntrada().compare(0, 12, "remove_light")  == 0){
        string name;
        //pegar o nome que foi digitado
        for (int i = 13; i < ler.getEntrada().length(); ++i) {
            name.push_back(ler.getEntrada().at(i));
        }
        //remover a luz escolhida
        for(int i = 0; i < lightVetor.size(); i++){
			if(strcmp(name.c_str(), lightVetor[i]->nome.c_str()) == 0) {
				lightVetor.erase(lightVetor.begin()+i);
			}
		}
    }

    
    //init lights    
    if (ler.getEntrada().compare(0, 9, "lights_on") == 0){
        lights_on = 1;
        //criando shaders para luzes
        createShadersLight();

    }
    
    //turns off pointLights
    if(ler.getEntrada().compare(0, 10, "lights_off") == 0){
        lights_on = 0;
    }

    //add reflexao informada
    if(ler.getEntrada().compare(0, 13, "reflection_on")  == 0){
        string type;
        int i = 14;
        string fl1;
        //pegar o nome que foi digitado
        while (ler.getEntrada().at(i) != ' ' ) {
             type.push_back(ler.getEntrada().at(i));
            i++;
        }
        i++;
        
        while (i < ler.getEntrada().length()) {
             fl1.push_back(ler.getEntrada().at(i));
            i++;
        }

        // modifica os coeficientes de iluminação globais de acordo com o 'type'
				
        if(strcmp(type.c_str(), "ambient") == 0){      
            Ka = stof(fl1);
        }
            
        else if (strcmp(type.c_str(), "diffuse") == 0){
            Kd = stof(fl1);
        }

        else if (strcmp(type.c_str(), "specular") == 0){
            Ks = stof(fl1);
        }
        

    }

    //desabilita reflexao informada
    if (ler.getEntrada().compare(0, 14, "reflection_off") == 0){
        string type;
        int i = 15;

        //pegar o nome que foi digitado
        while (i < ler.getEntrada().length()){
            type.push_back(ler.getEntrada().at(i));
            i++;
        }
       
        //zera os coeficientes globaisde iluminação conforme o 'type'   
        if (strcmp(type.c_str(), "ambient") == 0){
            Ka = 0.0f;
        }
        else if (strcmp(type.c_str(), "diffuse") == 0){
            Kd = 0.0f;
        }

        else if (strcmp(type.c_str(), "specular") == 0){
            Ks = 0.0f;
        }
        
    }
    
    //ligar wire
    if(ler.getEntrada().compare(0, 7, "wire_on") == 0) {
        wire = 1;
    }
    //desligar wire
    if(ler.getEntrada().compare(0, 8, "wire_off") == 0) {
        wire = 0;
    }

    //selecionando os shadings
    if (ler.getEntrada().compare(0, 7, "shading") == 0){

        string type;
        int i = 8;

        //pegar o nome que foi digitado
        while (i < ler.getEntrada().length()){
            type.push_back(ler.getEntrada().at(i));
            i++;
        }
       
        //seleciona o tipo de shading conforme o 'type'
        if (strcmp(type.c_str(), "flat") == 0){
            flat_on = 1;
            smooth_on = 0;
            phong_on = 0;
        }
        else if (strcmp(type.c_str(), "smooth") == 0){
            smooth_on = 1;
            flat_on = 0;
            phong_on = 0;
        }

        else if (strcmp(type.c_str(), "phong") == 0){
            phong_on = 1;
            flat_on = 0;
            smooth_on = 0;
            
        }
        else{   //none padrão
            flat_on = 0;
            smooth_on = 0;
            phong_on = 0;
        }          

    }
    
    if (flat_on == 1){
        createShadersFlat();
    }
    else if (smooth_on == 1){
        createShadersSmooth();
    }
    else if(phong_on == 1){
        createShadersPhong();
    }
    else{
        createShadersNone();
    }
    
    
    if (lights_on == 1){
        createShadersLight();
    }
    if (axis_on == 1){
        createShadersAxis();
    }    
    

    //init the objects
    for(int i = 0; i < objetoVetor.size(); i++){
        glGenVertexArrays(1, static_cast<GLuint *>(&objetoVetor[i]->vao));
        glBindVertexArray(objetoVetor[i]->vao);

        glGenBuffers(1, static_cast<GLuint *>(&objetoVetor[i]->vbo));
        glBindBuffer(GL_ARRAY_BUFFER, objetoVetor[i]->vbo);
        glBufferData(GL_ARRAY_BUFFER, objetoVetor[i]->vertexBuffer.size() * sizeof(glm::vec3), objetoVetor[i]->vertexBuffer.data(), GL_STATIC_DRAW);
        //able the first buffer
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);


        //passando a localização dos atributos para os shaders de acordo com o shading
        if(flat_on == 1){

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
            glBindAttribLocation(this->programFlat, 0, "vertexPosition"); //vertexPosition = name of attribute in shader
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
            glBindAttribLocation(this->programFlat, 0, "vertexNormal");
        }
        else if (smooth_on == 1){

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
            glBindAttribLocation(this->programSmooth, 0, "vertexPosition"); 
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
            glBindAttribLocation(this->programSmooth, 0, "vertexNormal"); 
        }
        else if (phong_on == 1){

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
            glBindAttribLocation(this->programPhong, 0, "vertexPosition");
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
            glBindAttribLocation(this->programPhong, 0, "vertexNormal");
        }
        else{   //shader none - passamos apenas os vértices e a cor por uniforme no rendering

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
            glBindAttribLocation(this->programNone, 0, "vertexPosition");
        }
        

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        vaoid++;
        vboid++;
    }

    //incializar a luz mesmo sem ter light_on
    for(int i = 0; i < lightVetor.size(); i++){
        glGenVertexArrays(1, static_cast<GLuint *>(&lightVetor[i]->vao));
        glBindVertexArray(lightVetor[i]->vao);

        glGenBuffers(1, static_cast<GLuint *>(&lightVetor[i]->vbo));
        glBindBuffer(GL_ARRAY_BUFFER, lightVetor[i]->vbo);
        glBufferData(GL_ARRAY_BUFFER, lightVetor[i]->lightBuffer.size() * sizeof(glm::vec3), lightVetor[i]->lightBuffer.data(), GL_STATIC_DRAW);
        
        
        //able the first buffer
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        //passando a localização dos atributos para o shader - 0= inicio do VBO
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
        glBindAttribLocation(this->programLight, 0, "lightPosition"); //vertexPosition = name of attribute in shader
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
        glBindAttribLocation(this->programLight, 0, "lightColor"); //vertexPosition = name of attribute in shader


        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        vaoidLight++;
        vboidLight++;
    }
    if (axis_on == 1){
           
        //init axis
        glGenVertexArrays(1, static_cast<GLuint *>(&axisScene->vao));
        glBindVertexArray(axisScene->vao);

        glGenBuffers(1, static_cast<GLuint *>(&axisScene->vbo));
        glBindBuffer(GL_ARRAY_BUFFER, axisScene->vbo);
        glBufferData(GL_ARRAY_BUFFER, axisScene->axisBuffer.size() * sizeof(glm::vec3), axisScene->axisBuffer.data(), GL_STATIC_DRAW);
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

}

void OpenGLContext::rendering() const{
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // Clear the colorbuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //desenhando formas
    for(int i = 0; i < objetoVetor.size(); i++){
        //printf("%d\n", objetoVetor[i]->vertexBuffer.size()/2);

        if (flat_on == 1){

            glUseProgram(this->programFlat);
            glBindVertexArray(objetoVetor[i]->vao);
            glBindBuffer(GL_ARRAY_BUFFER, objetoVetor[i]->vbo);

            //model
            int modelLoc = glGetUniformLocation(programFlat, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &objetoVetor[i]->model[0][0]);

            //view
            int viewLoc = glGetUniformLocation(programFlat, "view");
            glUniformMatrix4fv(viewLoc, 1, false, &objetoVetor[i]->view[0][0]);

            //ajustando a camera
            int projLoc = glGetUniformLocation(programFlat, "projection");
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, &objetoVetor[i]->projection[0][0]);

            //setting up color from shader by uniform
            int colorLoc = glGetUniformLocation(programFlat, "outColor");
            glUniform3fv(colorLoc, 1, &objetoVetor[i]->outColor[0]);

            //passando os coeficientes de iluminação para o fragment shader
            int kaLoc = glGetUniformLocation(programFlat, "Ka");
            glUniform1f(kaLoc, Ka);

            int kdLoc = glGetUniformLocation(programFlat, "Kd");
            glUniform1f(kdLoc, Kd);

            int ksLoc = glGetUniformLocation(programFlat, "Ks");
            glUniform1f(ksLoc, Ks);

            //printf("ka=%f kd=%f ks=%f\n", Ka, Kd, Ks);

            //passando a posição da camera para o fragment shader
            int viewerLoc = glGetUniformLocation(programFlat, "viewerPosition");
            glUniform3fv(viewerLoc, 1, &cam->position[0]);
            //glUniform3f(viewerLoc, -1.0f, 1.0f, 1.0f);
        

            //verificar se há luzes na cena - passar o vetor de luzes
            if (!lightVetor.empty()){

                int n = lightVetor.size();
                glUniform1i(glGetUniformLocation(programFlat, "nLuzes"), n);

                vector<glm::vec3> lights;
                for (int j = 0; j < n; j++){
                    //printf("entrou no for\n");
                    lights.push_back(lightVetor[j]->lightPosition);
                }
                //printf("saiu do for\n");
                //enviando a posição da luz para os shaders dos objetos
                int lightLoc = glGetUniformLocation(programFlat, "lights");
                glUniform3fv(lightLoc, n, &lights[0][0]);
                    
            }        
                
        }
        else if (smooth_on == 1){
            glUseProgram(this->programSmooth);
            glBindVertexArray(objetoVetor[i]->vao);
            glBindBuffer(GL_ARRAY_BUFFER, objetoVetor[i]->vbo);

            //model
            int modelLoc = glGetUniformLocation(programSmooth, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &objetoVetor[i]->model[0][0]);

            //view
            int viewLoc = glGetUniformLocation(programSmooth, "view");
            glUniformMatrix4fv(viewLoc, 1, false, &objetoVetor[i]->view[0][0]);

            //ajustando a camera
            int projLoc = glGetUniformLocation(programSmooth, "projection");
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, &objetoVetor[i]->projection[0][0]);

            //setting up color from shader by uniform
            int colorLoc = glGetUniformLocation(programSmooth, "outColor");
            glUniform3fv(colorLoc, 1, &objetoVetor[i]->outColor[0]);

            //passando os coeficientes de iluminação para o fragment shader
            int kaLoc = glGetUniformLocation(programSmooth, "Ka");
            glUniform1f(kaLoc, Ka);

            int kdLoc = glGetUniformLocation(programSmooth, "Kd");
            glUniform1f(kdLoc, Kd);

            int ksLoc = glGetUniformLocation(programSmooth, "Ks");
            glUniform1f(ksLoc, Ks);

            //printf("ka=%f kd=%f ks=%f\n", Ka, Kd, Ks);

            //passando a posição da camera para o fragment shader
            int viewerLoc = glGetUniformLocation(programSmooth, "viewerPosition");
            glUniform3fv(viewerLoc, 1, &cam->position[0]);
            //glUniform3f(viewerLoc, -1.0f, 1.0f, 1.0f);
        

            //verificar se há luzes na cena - passar o vetor de luzes
            if (!lightVetor.empty()){

                int n = lightVetor.size();
                glUniform1i(glGetUniformLocation(programSmooth, "nLuzes"), n);

                vector<glm::vec3> lights;
                for (int j = 0; j < n; j++){
                    //printf("entrou no for\n");
                    lights.push_back(lightVetor[j]->lightPosition);
                }
                //printf("saiu do for\n");
                //enviando a posição da luz para os shaders dos objetos
                int lightLoc = glGetUniformLocation(programSmooth, "lights");
                glUniform3fv(lightLoc, n, &lights[0][0]);
                    
            }     
        }
        else if (phong_on == 1){
            glUseProgram(this->programPhong);
            glBindVertexArray(objetoVetor[i]->vao);
            glBindBuffer(GL_ARRAY_BUFFER, objetoVetor[i]->vbo);

            //model
            int modelLoc = glGetUniformLocation(programPhong, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &objetoVetor[i]->model[0][0]);

            //view
            int viewLoc = glGetUniformLocation(programPhong, "view");
            glUniformMatrix4fv(viewLoc, 1, false, &objetoVetor[i]->view[0][0]);

            //ajustando a camera
            int projLoc = glGetUniformLocation(programPhong, "projection");
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, &objetoVetor[i]->projection[0][0]);

            //setting up color from shader by uniform
            int colorLoc = glGetUniformLocation(programPhong, "outColor");
            glUniform3fv(colorLoc, 1, &objetoVetor[i]->outColor[0]);

            //passando os coeficientes de iluminação para o fragment shader
            int kaLoc = glGetUniformLocation(programPhong, "Ka");
            glUniform1f(kaLoc, Ka);

            int kdLoc = glGetUniformLocation(programPhong, "Kd");
            glUniform1f(kdLoc, Kd);

            int ksLoc = glGetUniformLocation(programPhong, "Ks");
            glUniform1f(ksLoc, Ks);

            //printf("ka=%f kd=%f ks=%f\n", Ka, Kd, Ks);

            //passando a posição da camera para o fragment shader
            int viewerLoc = glGetUniformLocation(programPhong, "viewerPosition");
            glUniform3fv(viewerLoc, 1, &cam->position[0]);
            //glUniform3f(viewerLoc, -1.0f, 1.0f, 1.0f);
        

            //verificar se há luzes na cena - passar o vetor de luzes
            if (!lightVetor.empty()){

                int n = lightVetor.size();
                glUniform1i(glGetUniformLocation(programPhong, "nLuzes"), n);

                vector<glm::vec3> lights;
                for (int j = 0; j < n; j++){
                    //printf("entrou no for\n");
                    lights.push_back(lightVetor[j]->lightPosition);
                }
                //printf("saiu do for\n");
                //enviando a posição da luz para os shaders dos objetos
                int lightLoc = glGetUniformLocation(programPhong, "lights");
                glUniform3fv(lightLoc, n, &lights[0][0]);
                    
            }        
        }       
        else{   //ativa o shading None - padrão
                    
            glUseProgram(this->programNone);
            glBindVertexArray(objetoVetor[i]->vao);
            glBindBuffer(GL_ARRAY_BUFFER, objetoVetor[i]->vbo);

            //model
            int modelLoc = glGetUniformLocation(programNone, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &objetoVetor[i]->model[0][0]);

            //view
            int viewLoc = glGetUniformLocation(programNone, "view");
            glUniformMatrix4fv(viewLoc, 1, false, &objetoVetor[i]->view[0][0]);

            //ajustando a camera
            int projLoc = glGetUniformLocation(programNone, "projection");
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, &objetoVetor[i]->projection[0][0]);

            //setting up color from shader by uniform
            int colorLoc = glGetUniformLocation(programNone, "outColor");
            glUniform3fv(colorLoc, 1, &objetoVetor[i]->outColor[0]);

        }

        //verificar se eh pra desenhar com wire ou normal
        if (wire == 0)
            glDrawArrays(GL_TRIANGLES, 0, objetoVetor[i]->vertexBuffer.size() / 2);
        else
            glDrawArrays(GL_LINE_STRIP, 0, objetoVetor[i]->vertexBuffer.size() / 2);         
        
        

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);
    }

    //desenhando eixos
    if (axis_on == 1){
        
        glUseProgram(this->programAxis);
        glBindVertexArray(axisScene->vao);
        glBindBuffer(GL_ARRAY_BUFFER, axisScene->vbo);

        //ajustando a view
        int viewLoc = glGetUniformLocation(programAxis, "view");
        glUniformMatrix4fv(viewLoc, 1, false, &axisScene->view[0][0]);

        //ajustando a camera
        int projLoc = glGetUniformLocation(programAxis, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &axisScene->projection[0][0]);

        glDrawArrays(GL_LINES, 0, axisScene->axisBuffer.size()/2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);
    }


    //show lights
    if (lights_on == 1){

        //desenhando fontes de luz
        for (int i = 0; i < lightVetor.size(); i++){

            glUseProgram(this->programLight);
            glBindVertexArray(lightVetor[i]->vao);
            glBindBuffer(GL_ARRAY_BUFFER, lightVetor[i]->vbo);

            //ajustando a view
            int viewLoc = glGetUniformLocation(programLight, "view");
            glUniformMatrix4fv(viewLoc, 1, false, &lightVetor[i]->view[0][0]);

            //ajustando a camera
            int projLoc = glGetUniformLocation(programLight, "projection");
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, &lightVetor[i]->projection[0][0]);

            glPointSize(10.0f);
            glDrawArrays(GL_POINTS, 0, lightVetor[i]->lightBuffer.size() / 2);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            glUseProgram(0);
        }

    }
    
    
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

void OpenGLContext::finalize() const{
    // Properly de-allocate all resources once they've outlived their purpose

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    for (int i = 0; i < objetoVetor.size(); i++){
        glDeleteBuffers(1, (GLuint *)(&(objetoVetor[i]->vbo)));
        glDeleteVertexArrays(1, &(objetoVetor[i]->vao));
    }

    for (int i = 0; i < lightVetor.size(); i++){   
        glDeleteBuffers(1, (GLuint *)(&(lightVetor[i]->vbo)));
        glDeleteVertexArrays(1, &(lightVetor[i]->vao));
    }

    glDeleteBuffers(1, (GLuint *)(&(axisScene->vbo)));
    glDeleteVertexArrays(1, &(axisScene->vao));
        
    
    glUseProgram(0);

    for (int i = 0; i < objetoVetor.size(); i++) {
        delete objetoVetor[i];
    }
    for (int i = 0; i < lightVetor.size(); i++) {
        delete lightVetor[i];
    }

    delete axisScene;
    delete cam;
}

int main(int argc, char *argv[]){

    cam = new camera();

    // leitura do comando
    ler.ler();
    OpenGLContext context{argc, argv};  
    
    while( ler.getEntrada().compare("quit") != 0){ //comparar se a entrada eh igual quite
        
        context.initialize();
        context.rendering();

        ler.deleteEntrada();
        ler.ler();
    }
    ler.deleteEntrada();
    context.printVersion();

    context.finalize();

    return 0;
}
