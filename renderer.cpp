#include "renderer.h"
#include <iostream>
#include <fstream>
#include <sstream>

Renderer::Renderer(int width, int height) : screen_width(width), screen_height(height) {}

Renderer::~Renderer() {
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
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

bool Renderer::init() {
    if (!glfwInit()) {
        std::cerr << "Could not initialize GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(screen_width, screen_height, "Solar System Simulation", NULL, NULL);
    if (!window) {
        std::cerr << "Could not create window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

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

    glViewport(0, 0, screen_width, screen_height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // Включаем блендинг для корректного наложения объектов
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    return true;
}

void Renderer::render(const std::vector<CelestialBody>& bodies) {
    glClear(GL_COLOR_BUFFER_BIT);

    glUniform2f(resolution_uniform_loc, screen_width, screen_height);

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

    glfwSwapBuffers(window);
}

bool Renderer::handle_events() {
    if (glfwWindowShouldClose(window)) {
        return false;
    }
    glfwPollEvents();
    return true;
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

void Renderer::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Renderer* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if (renderer) {
        renderer->handle_resize(width, height);
    }
}

void Renderer::handle_resize(int width, int height) {
    //std::cout << "Handling resize to: " << width << "x" << height << std::endl;
    screen_width = width;
    screen_height = height;
    glViewport(0, 0, screen_width, screen_height);
}
