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


