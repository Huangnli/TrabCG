#include <vector>
#include <cstdio>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include "light.h"

using namespace std;

light::light(glm::vec3 position, glm::vec3 rgb){
    lightPosition = position;
    lightColor = rgb;
}

light::~light(){
}

glm::vec3 light::getPosition(){
    return lightPosition;
}

glm::vec3 light::getColor(){
    return lightColor;
}
