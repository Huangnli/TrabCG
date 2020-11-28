#include <vector>
#include <cstdio>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include "light.h"

using namespace std;

light::light(glm::vec3 position, unsigned int idvao, unsigned int idvbo){
    vao = idvao;
    vbo = idvbo;
    lightPosition = position;
    lightColor = glm::vec3(1.0f, 1.0f, 0.0f);
    setLightBuffer();
}

light::~light(){
}

glm::vec3 light::getPosition(){
    return lightPosition;
}

void light::setLightBuffer(){
    lightBuffer.push_back(lightPosition);
    lightBuffer.push_back(lightColor);
}
