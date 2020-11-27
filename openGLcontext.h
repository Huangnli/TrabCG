#ifndef OPENGLCONTEXT_H
#define OPENGLCONTEXT_H
#include <string>
#include <vector>
#include <glm/fwd.hpp>

class OpenGLContext
{
public:
    OpenGLContext(int argc, char *argv[]); // Constructor
    ~OpenGLContext();                      // Destructor

    void printVersion() const; // Show OpenGL Version
    void runLoop() const;

    void initialize(); // Init our render
    void rendering() const; // Render Loop
private:
    unsigned int programId;
    unsigned int programLight;
    unsigned int vao;
    unsigned int vbo[2];
    static void glutRenderCallback();                       // Render window
    static void glutReshapeCallback(int width, int height); // Reshape window

    void finalize() const;  // Finalize our render

    unsigned int loadAndCompileShader(const std::string &filename, const int glType) const;
    unsigned int linkShaderProgram(unsigned int vertexShaderId, unsigned int fragmentShaderId) const;
};

#endif // OPENGLCONTEXT_H
