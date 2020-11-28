#include <vector>
#include <cstdio>
#include <string>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>

using namespace std;

class axis{

public:
    
    unsigned int vao;
    unsigned int vbo;

    vector<glm::vec3> vertexLines;  //6 vértices
    vector<glm::vec3> colorAxis;    //serão 3 cores
    vector<glm::vec3> axisBuffer;

    axis(unsigned int idvao, unsigned int idvbo);
    ~axis();

    void setColorAxis();
    void setVertexLines();
    void setBuffer();
};


