#ifndef OPENGLCONTEXT_H
#define OPENGLCONTEXT_H
#include <string>
#include <vector>
#include <glm/fwd.hpp>


class OpenGLContext{

public:
    OpenGLContext(int argc, char *argv[]); // Constructor
    ~OpenGLContext();                      // Destructor

    void printVersion() const; // Show OpenGL Version
    void runLoop() const;
    void loadObj(const char *path, std::vector<glm::vec3> &vbuffer);

    void initialize(); // Init our render
    void rendering() const; // Render Loop
    void finalize() const;  // Finalize our render
    
private:
    unsigned int programId;
    unsigned int programNone;
    unsigned int programFlat;
    unsigned int programSmooth;
    unsigned int programPhong;
    unsigned int programAxis;
    unsigned int programLight;
    unsigned int vao;
    unsigned int vbo;
    
    static void glutRenderCallback();                       // Render window
    static void glutReshapeCallback(int width, int height); // Reshape window

    void createShadersNone();
    void createShadersFlat();
    void createShadersSmooth();
    void createShadersPhong();
    void createShadersLight();
    void createShadersAxis();


    unsigned int loadAndCompileShader(const std::string &filename, const int glType) const;
    unsigned int linkShaderProgram(unsigned int vertexShaderId, unsigned int fragmentShaderId) const;
};

#endif // OPENGLCONTEXT_H
