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
    vector<glm::vec3> vertexBuffer;
    glm::mat4 model;

public:
    objeto(const char *path);
    ~objeto();

    vector<glm::vec3> getVertexBuffer();
    glm::mat4 getModel();
    //vector<glm::vec3> getPositions();
    //vector<glm::vec3> getNormals();

    bool loadObj(const char *path);
};
