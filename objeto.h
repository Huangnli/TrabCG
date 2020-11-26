#include <vector>
#include <cstdio>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

using namespace std;

class objeto{
private:
    unsigned int vao;
    vector<glm::vec3> vertexPositions, vertexNormals;
    vector<glm::mat4> model;
    

public:
    vector<glm::vec3> vertexBuffer;

    objeto(const char *path);
    ~objeto();
    vector<glm::vec3> getVertexBuffer();
    vector<glm::vec3> getPositions();
    vector<glm::vec3> getNormals();

    unsigned int getVAO();

    bool loadObj(const char *path);
};
