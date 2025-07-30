#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include <string>
#include <GLFW/glfw3.h>
#include <GLES2/gl2.h>
#include "simulation.h"

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    bool init();
    void render(const std::vector<CelestialBody>& bodies);
    bool handle_events();
    void handle_resize(int width, int height);

private:
    int screen_width;
    int screen_height;
    GLFWwindow* window = nullptr;

    GLuint shader_program;
    GLint pos_attrib_loc;
    GLint resolution_uniform_loc;
    GLint body_pos_uniform_loc;
    GLint body_radius_uniform_loc;

    GLuint load_shader(GLenum type, const char* source);
    GLuint create_shader_program(const char* vs_source, const char* fs_source);
    std::string read_file(const std::string& path);
};

#endif // RENDERER_H