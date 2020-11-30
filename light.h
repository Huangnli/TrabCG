#include <vector>
#include <cstdio>
#include <string>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

using namespace std;

class light{

public:
    unsigned int vao;
    unsigned int vbo;

    string nome;
    glm::vec3 lightPosition;
    glm::vec3 lightColor;
    vector<glm::vec3> lightBuffer;
    glm::mat4 view;
    glm::mat4 projection;

    light(string name, glm::vec3 position, unsigned int idvao, unsigned int idvbo, glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
    ~light();

    glm::vec3 getPosition();

    void setLightBuffer();
    
};
