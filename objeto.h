#include <vector>
#include <cstdio>
#include <string>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

using namespace std;

class objeto{

public:
    objeto(string name, unsigned int idvao, unsigned int idvbo , vector<glm::vec3> dadosDoObjeto, glm::mat4 matrizmodel, 
            glm::mat4 matrizview, glm::mat4 matrizprojection, glm::vec3 cor);
    ~objeto();


    string nome;
    unsigned int vao;
    unsigned int vbo;
    vector<glm::vec3> vertexBuffer;
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 outColor;

    float Ka, Kd, Ks;

};
