#include <vector>
#include <cstdio>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

using namespace std;

class light{

public:
    glm::vec3 lightPosition;
    glm::vec3 lightColor;
    vector<glm::vec3> lightBuffer;

    light(glm::vec3 position);
    ~light();

    glm::vec3 getPosition();

    void setLightBuffer();
    
};
