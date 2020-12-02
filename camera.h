#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

class camera{
    //private:
    
    public:
        // camera Attributes
        glm::vec3 position;
        glm::vec3 lookat;
        glm::vec3 viewup;
   
        camera();
        ~camera();
        glm::vec3 getInitialPosition();
        glm::vec3 getInitialLookat();
        glm::vec3 getInitialUp();
        glm::mat4 getViewMatrix();
        glm::vec3 getCurrentPosition();

        void setLookat(glm::vec3 vecLookAt);
        void setPosition(glm::vec3 vecPos);
};

camera::camera(){
    position = getInitialPosition();
    lookat = getInitialLookat();
    viewup = getInitialUp();
}

camera::~camera(){
}

glm::vec3 camera::getInitialPosition(){
    return glm::vec3(0.0f, 0.0f, 0.0f);
}

glm::vec3 camera::getInitialLookat(){
    return glm::vec3(0.0f, 0.0f, -1.0f);
}

glm::vec3 camera::getInitialUp(){
    return glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::mat4 camera::getViewMatrix(){
    return glm::lookAt(position, lookat, viewup);
}

void camera::setLookat(glm::vec3 vecLookAt){
    lookat = vecLookAt;
}

void camera::setPosition(glm::vec3 vecPos){
    position = vecPos;
}

glm::vec3 camera::getCurrentPosition(){
    return position;
}