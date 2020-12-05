#include <vector>
#include <cstdio>
#include <string>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

using namespace std;

class axis{

public:
    
    unsigned int vao;
    unsigned int vbo;

    vector<glm::vec3> vertexLines;  //6 vértices
    vector<glm::vec3> colorAxis;    //serão 3 cores
    vector<glm::vec3> axisBuffer;

    glm::mat4 view;
    glm::mat4 projection;

    axis();
    axis(unsigned int idvao, unsigned int idvbo, glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
    ~axis();

    void setColorAxis();
    void setVertexLines();
    void setBuffer();
};

axis::axis(unsigned int idvao, unsigned int idvbo, glm::mat4 viewMatrix, glm::mat4 projectionMatrix){   
    vao = idvao;
    vbo = idvbo;
    view = viewMatrix;
    projection = projectionMatrix;
    setColorAxis();
    setVertexLines();
    setBuffer();    
}

axis::axis(){
}

axis::~axis(){
}

void axis::setColorAxis(){
    colorAxis.push_back(glm::vec3(1.0f, 0.0f, 0.0f)); //red x
    colorAxis.push_back(glm::vec3(0.0f, 1.0f, 0.0f)); //green y
    colorAxis.push_back(glm::vec3(0.0f, 0.0f, 1.0f)); //blue z
}

void axis::setVertexLines(){
    //eixo x
    vertexLines.push_back(glm::vec3(-2.0f, 0.0f, 0.0f));
    vertexLines.push_back(glm::vec3(2.0f, 0.0f, 0.0f));
    //eixo y
    vertexLines.push_back(glm::vec3(0.0f, -2.0f, 0.0f));
    vertexLines.push_back(glm::vec3(0.0f, 2.0f, 0.0f));
    //eixo z
    vertexLines.push_back(glm::vec3(0.0f, 0.0f, -2.0f));
    vertexLines.push_back(glm::vec3(0.0f, 0.0f, 2.0f));
}

void axis::setBuffer(){
    int j = 0;
    for (int i = 0; i < vertexLines.size()*2; i++){
        if(i >= 2 && i <= 3){
            j = 1;
        }
        if (i >= 4 && i <= 5){
            j = 2;
        }
        axisBuffer.push_back(vertexLines[i]);
        axisBuffer.push_back(colorAxis[j]);        
    }    
}
