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
    void render(const std::vector<CelestialBody>& bodies, float min_radius, float max_radius);
    void handle_resize(int width, int height);
    void handle_touchstart(const EmscriptenTouchEvent *touchEvent);
    void handle_touchmove(const EmscriptenTouchEvent *touchEvent);
    void handle_touchend(const EmscriptenTouchEvent *touchEvent);
    void set_colors(const std::vector<float>& color_data, const std::vector<float>& weight_data);

private:
    int screen_width;
    int screen_height;
    float initialization_radius;
    double initial_touch_dist = 0;
    float zoom = 1.0;

    struct Color { float r, g, b; };
    std::vector<Color> colors;
    std::vector<float> weights;

    GLuint shader_program;
    GLint resolution_uniform_loc;
    GLint num_bodies_uniform_loc;
    GLint initialization_radius_uniform_loc;
    GLint zoom_uniform_loc;
    GLint min_radius_uniform_loc;
    GLint max_radius_uniform_loc;
    GLint num_colors_uniform_loc;
    GLint colors_uniform_loc;
    GLint weights_uniform_loc;

    GLuint particle_vbo;
    GLuint particle_vao;
    GLint body_pos_attrib_loc;
    GLint body_radius_attrib_loc;

    GLuint load_shader(GLenum type, const char* source);
    GLuint create_shader_program(const char* vs_source, const char* fs_source);
    std::string read_file(const std::string& path);

    static EM_BOOL touchstart_callback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
    static EM_BOOL touchmove_callback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
    static EM_BOOL touchend_callback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
};

#endif // RENDERER_H
