#include <vector>
#include <cstdio>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

using namespace std;

class light{

public:
    unsigned int vao;
    unsigned int vbo;

    glm::vec3 lightPosition;
    glm::vec3 lightColor;
    vector<glm::vec3> lightBuffer;

    light(glm::vec3 position, unsigned int idvao, unsigned int idvbo);
    ~light();

    glm::vec3 getPosition();

    void setLightBuffer();
    
};
