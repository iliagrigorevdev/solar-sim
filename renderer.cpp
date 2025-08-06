#include <GLES2/gl2.h>
#include "renderer.h"
#include <iostream>
#include <fstream>
#include <sstream>

Renderer::Renderer(int width, int height) : screen_width(width), screen_height(height) {}

Renderer::~Renderer() {
}

std::string Renderer::read_file(const std::string& path) {
    std::ifstream file(path);
    std::stringstream buffer;
    if (file) {
        buffer << file.rdbuf();
        return buffer.str();
    }
    std::cerr << "Error: Could not open file " << path << std::endl;
    return "";
}

bool Renderer::init(float initialization_radius) {
    this->initialization_radius = initialization_radius;
    
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion = 1;
    attrs.minorVersion = 0;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
    if (context <= 0) {
        std::cerr << "Could not create WebGL context" << std::endl;
        return false;
    }
    emscripten_webgl_make_context_current(context);

    std::string vs_source = read_file("shader.vert");
    std::string fs_source = read_file("shader.frag");

    shader_program = create_shader_program(vs_source.c_str(), fs_source.c_str());
    if (!shader_program) {
        return false;
    }
    glUseProgram(shader_program);

    pos_attrib_loc = glGetAttribLocation(shader_program, "a_position");
    resolution_uniform_loc = glGetUniformLocation(shader_program, "u_resolution");
    data_texture_uniform_loc = glGetUniformLocation(shader_program, "u_data_texture");
    num_bodies_uniform_loc = glGetUniformLocation(shader_program, "u_num_bodies");
    initialization_radius_uniform_loc = glGetUniformLocation(shader_program, "u_initialization_radius");
    zoom_uniform_loc = glGetUniformLocation(shader_program, "u_zoom");
    min_radius_uniform_loc = glGetUniformLocation(shader_program, "u_min_radius");
    max_radius_uniform_loc = glGetUniformLocation(shader_program, "u_max_radius");

    emscripten_set_touchstart_callback("#canvas", this, true, touchstart_callback);
    emscripten_set_touchmove_callback("#canvas", this, true, touchmove_callback);
    emscripten_set_touchend_callback("#canvas", this, true, touchend_callback);

    glViewport(0, 0, screen_width, screen_height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // Включаем блендинг для корректного наложения объектов
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glGenTextures(1, &data_texture_loc);
    glBindTexture(GL_TEXTURE_2D, data_texture_loc);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return true;
}

void Renderer::render(const std::vector<CelestialBody>& bodies, float min_radius, float max_radius) {
    glClear(GL_COLOR_BUFFER_BIT);

    glUniform1f(initialization_radius_uniform_loc, initialization_radius);
    glUniform2f(resolution_uniform_loc, screen_width, screen_height);
    glUniform1f(zoom_uniform_loc, zoom);
    glUniform1f(min_radius_uniform_loc, min_radius);
    glUniform1f(max_radius_uniform_loc, max_radius);

    GLfloat vertices[] = {
        -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f
    };
    glVertexAttribPointer(pos_attrib_loc, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(pos_attrib_loc);

    const int num_bodies = bodies.size();
    glUniform1i(num_bodies_uniform_loc, num_bodies);

    const int texture_width = 2048;
    std::vector<float> texture_data(texture_width * 4, 0.0f);

    for (int i = 0; i < num_bodies; ++i) {
        texture_data[i * 4 + 0] = bodies[i].x;
        texture_data[i * 4 + 1] = bodies[i].y;
        texture_data[i * 4 + 2] = bodies[i].radius;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, data_texture_loc);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_width, 1, 0, GL_RGBA, GL_FLOAT, texture_data.data());
    glUniform1i(data_texture_uniform_loc, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

GLuint Renderer::load_shader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint info_len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);
        if (info_len > 1) {
            char* info_log = (char*)malloc(sizeof(char) * info_len);
            glGetShaderInfoLog(shader, info_len, NULL, info_log);
            std::cerr << "Error compiling shader:\n" << info_log << std::endl;
            free(info_log);
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint Renderer::create_shader_program(const char* vs_source, const char* fs_source) {
    GLuint vs = load_shader(GL_VERTEX_SHADER, vs_source);
    GLuint fs = load_shader(GL_FRAGMENT_SHADER, fs_source);

    if (vs == 0 || fs == 0) return 0;

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint info_len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_len);
        if (info_len > 1) {
            char* info_log = (char*)malloc(sizeof(char) * info_len);
            glGetProgramInfoLog(program, info_len, NULL, info_log);
            std::cerr << "Error linking program:\n" << info_log << std::endl;
            free(info_log);
        }
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void Renderer::handle_resize(int width, int height) {
    //std::cout << "Handling resize to: " << width << "x" << height << std::endl;
    screen_width = width;
    screen_height = height;
    glViewport(0, 0, screen_width, screen_height);
}

EM_BOOL Renderer::touchstart_callback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData) {
    Renderer* renderer = static_cast<Renderer*>(userData);
    if (renderer) {
        renderer->handle_touchstart(touchEvent);
    }
    if (touchEvent->numTouches >= 2) {
        return EM_FALSE;
    }
    return EM_TRUE;
}

EM_BOOL Renderer::touchmove_callback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData) {
    Renderer* renderer = static_cast<Renderer*>(userData);
    if (renderer) {
        renderer->handle_touchmove(touchEvent);
    }
    if (touchEvent->numTouches >= 2) {
        return EM_FALSE;
    }
    return EM_TRUE;
}

EM_BOOL Renderer::touchend_callback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData) {
    Renderer* renderer = static_cast<Renderer*>(userData);
    if (renderer) {
        renderer->handle_touchend(touchEvent);
    }
    if (touchEvent->numTouches >= 2) {
        return EM_FALSE;
    }
    return EM_TRUE;
}

void Renderer::handle_touchstart(const EmscriptenTouchEvent *touchEvent) {
    if (touchEvent->numTouches >= 2) {
        const EmscriptenTouchPoint *t1 = &touchEvent->touches[0];
        const EmscriptenTouchPoint *t2 = &touchEvent->touches[1];
        initial_touch_dist = std::sqrt(std::pow(t1->clientX - t2->clientX, 2) + std::pow(t1->clientY - t2->clientY, 2));
    }
}

void Renderer::handle_touchmove(const EmscriptenTouchEvent *touchEvent) {
    if (touchEvent->numTouches >= 2) {
        const EmscriptenTouchPoint *t1 = &touchEvent->touches[0];
        const EmscriptenTouchPoint *t2 = &touchEvent->touches[1];
        double current_touch_dist = std::sqrt(std::pow(t1->clientX - t2->clientX, 2) + std::pow(t1->clientY - t2->clientY, 2));
        zoom *= 1.0 + (current_touch_dist - initial_touch_dist) / initial_touch_dist;
        initial_touch_dist = current_touch_dist;
    }
}

void Renderer::handle_touchend(const EmscriptenTouchEvent *touchEvent) {
    initial_touch_dist = 0;
}