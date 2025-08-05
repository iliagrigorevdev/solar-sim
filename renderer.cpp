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
    attrs.majorVersion = 2;
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
    body_positions_uniform_loc = glGetUniformLocation(shader_program, "u_body_positions");
    body_radii_uniform_loc = glGetUniformLocation(shader_program, "u_body_radii");
    num_bodies_uniform_loc = glGetUniformLocation(shader_program, "u_num_bodies");
    initialization_radius_uniform_loc = glGetUniformLocation(shader_program, "u_initialization_radius");
    zoom_uniform_loc = glGetUniformLocation(shader_program, "u_zoom");

    emscripten_set_touchstart_callback("#canvas", this, true, touchstart_callback);
    emscripten_set_touchmove_callback("#canvas", this, true, touchmove_callback);
    emscripten_set_touchend_callback("#canvas", this, true, touchend_callback);

    glViewport(0, 0, screen_width, screen_height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // Включаем блендинг для корректного наложения объектов
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
}

void Renderer::render(const std::vector<CelestialBody>& bodies) {
    glClear(GL_COLOR_BUFFER_BIT);

    glUniform1f(initialization_radius_uniform_loc, initialization_radius);
    glUniform2f(resolution_uniform_loc, screen_width, screen_height);
    glUniform1f(zoom_uniform_loc, zoom);

    GLfloat vertices[] = {
        -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f
    };
    glVertexAttribPointer(pos_attrib_loc, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(pos_attrib_loc);

    const int num_bodies = bodies.size();
    glUniform1i(num_bodies_uniform_loc, num_bodies);

    std::vector<float> positions(num_bodies * 2);
    std::vector<float> radii(num_bodies);

    for (int i = 0; i < num_bodies; ++i) {
        positions[i * 2] = bodies[i].x;
        positions[i * 2 + 1] = bodies[i].y;
        radii[i] = bodies[i].radius;
    }

    glUniform2fv(body_positions_uniform_loc, num_bodies, positions.data());
    glUniform1fv(body_radii_uniform_loc, num_bodies, radii.data());

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