#include <vector>
#include <cstdio>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

using namespace std;

class objeto
{
private:
    vector<glm::vec3> vertexPositions, vertexNormals;
    vector<glm::vec3> vbuffer;
    glm::mat4 model;

    int nVertices;
    unsigned int vbo;

public:
    objeto(const char *path);
    ~objeto();

    vector<glm::vec3> getVertexBuffer();
    glm::mat4 getModel();
    unsigned int getVBO();
    int getNumeroVertices();

    bool loadObj(const char *path);
};
