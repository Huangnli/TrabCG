#include "objeto.h"
#include "string.h"

using namespace std;

objeto::objeto(string name, unsigned int idvao, unsigned int idvbo, vector<glm::vec3> dadosDoObjeto, glm::mat4 matrizmodel,
               glm::mat4 matrizview, glm::mat4 matrizprojection, glm::vec3 cor){
    nome = name;
    vao = idvao;
    vbo = idvbo;
    vertexBuffer = dadosDoObjeto;
    model = matrizmodel;
    view = matrizview;
    projection = matrizprojection;
    outColor = cor;
    Ka = 0.2f;
    Kd = 0.2f;
    Ks = 0.2f;
}

objeto::~objeto(){

}



// unsigned int objeto::getVBO(){
//     return vbo;
// }

// int objeto::getNumeroVertices(){
//     return nVertices;
// }

// vector<glm::vec3> objeto::getVertexBuffer(){
//     return vbuffer; 
// }

// unsigned int objeto::getVAO(){
//     return vao;
// }

// unsigned int objeto::getVBO(){
//     return vbo;
// }

// unsigned int getprogramId(){
//     return programId;
// }

// glm::mat4 objeto::getModel(){
//     return model;
// }
