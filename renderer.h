#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include <string>
#include <GLES2/gl2.h>
#include "simulation.h"
#include <emscripten/html5.h>

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    bool init(float initialization_radius);
    void render(const std::vector<CelestialBody>& bodies);
    void handle_resize(int width, int height);
    void handle_touchstart(const EmscriptenTouchEvent *touchEvent);
    void handle_touchmove(const EmscriptenTouchEvent *touchEvent);
    void handle_touchend(const EmscriptenTouchEvent *touchEvent);

private:
    int screen_width;
    int screen_height;
    float initialization_radius;
    double initial_touch_dist = 0;
    float zoom = 1.0;

    GLuint shader_program;
    GLint pos_attrib_loc;
    GLint resolution_uniform_loc;
    GLint num_bodies_uniform_loc;
    GLint body_positions_uniform_loc;
    GLint body_radii_uniform_loc;
    GLint initialization_radius_uniform_loc;
    GLint zoom_uniform_loc;

    GLuint load_shader(GLenum type, const char* source);
    GLuint create_shader_program(const char* vs_source, const char* fs_source);
    std::string read_file(const std::string& path);

    static EM_BOOL touchstart_callback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
    static EM_BOOL touchmove_callback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
    static EM_BOOL touchend_callback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
};

#endif // RENDERER_H
