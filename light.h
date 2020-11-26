#include <vector>
#include <cstdio>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

using namespace std;

class light{
private:
    
    glm::vec3 lightPosition;
    glm::vec3 lightColor;

public:
    light(glm::vec3 positions, glm::vec3 rgb);
    ~light();

    glm::vec3 getPosition();
    glm::vec3 getColor();
};
