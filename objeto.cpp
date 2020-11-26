
#include <vector>
#include <cstdio>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include "objeto.h"
#include "string.h"

#define MAX 50

using namespace std;

objeto::objeto(const char *path){
    
        //temporary variables to store the .obj's contents:
        vector<unsigned int> vertexIndices, normalIndices;
        vector<glm::vec3> temp_vertices;
        vector<glm::vec3> temp_normals;

        //reading the file data
        FILE *file = fopen(path, "r");
        char line[MAX];

        if (file == NULL){
            printf("erro ao abrir o arquivo!\n");
        }
        int res = fscanf(file, "%s", line); //read the first word
        while (res != EOF){

            if (strcmp(line, "v") == 0){ //verify is it's a vertex position

                glm::vec3 vertex;
                fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z); //copy the coordinates to auxiliar vertex
                temp_vertices.push_back(vertex);                             //copy the coordinates to the temporary vecVertex
            }
            else if (strcmp(line, "vn") == 0){ //verify if it's a normal data
                glm::vec3 normal;
                fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
                temp_normals.push_back(normal);
            }

            else if (strcmp(line, "f") == 0)
            { //verify if it's a face, and make a index contents
                unsigned int vertexIndex[3], textureIndex[3], normalIndex[3];
                int matches = fscanf(file, "%d/%d/%d  %d/%d/%d  %d/%d/%d\n", &vertexIndex[0], &textureIndex[0], &normalIndex[0],
                                                                            &vertexIndex[1], &textureIndex[0], &normalIndex[1],
                                                                            &vertexIndex[2], &textureIndex[0], &normalIndex[2]);
                if (matches != 9){
                    printf("error in reading file! try another way.\n\n");
                    
                }
                //indexação dos vertices e normais
                vertexIndices.push_back(vertexIndex[0]);
                vertexIndices.push_back(vertexIndex[1]);
                vertexIndices.push_back(vertexIndex[2]);

                normalIndices.push_back(normalIndex[0]);
                normalIndices.push_back(normalIndex[1]);
                normalIndices.push_back(normalIndex[2]);
            }
            res = fscanf(file, "%s", line); //continue reading
        }
        fclose(file);

        //passing by each vertex (v/vn) of each triangule (face)
        //put the right vertex positions and normals on the out_vertexData
        for (unsigned int i = 0; i < vertexIndices.size(); i++)    {

            unsigned int indexV = vertexIndices[i];
            unsigned int indexN = normalIndices[i];

            glm::vec3 vertex = temp_vertices[indexV - 1];
            glm::vec3 normal = temp_normals[indexN - 1];
            vertexBuffer.push_back(vertex);
            vertexBuffer.push_back(normal);
        }

        model = glm::mat4(1.0);
}

objeto::~objeto(){

}

vector<glm::vec3> objeto::getVertexBuffer(){
    return vertexBuffer; 

}
glm::mat4 objeto::getModel(){
    return model;
}
