#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <chrono>
#include <algorithm>
#include <functional>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/bind.h>
#endif
#include "renderer.h"
#include "simulation.h"
#include "quadtree.h"

struct SimulationContext {
    Renderer* renderer;
    std::vector<CelestialBody>* bodies;
    SimulationParameters* params;
    Quadtree** quadtree;
};

#ifdef __EMSCRIPTEN__
// Global context for the callback
SimulationContext* g_context = nullptr;

EM_BOOL on_web_display_size_changed(int event_type, const EmscriptenUiEvent* ui_event, void* user_data) {
    double width, height;
    emscripten_get_element_css_size("canvas", &width, &height);
    emscripten_set_canvas_element_size("#canvas", (int)width, (int)height);
    if (g_context && g_context->renderer) {
        g_context->renderer->handle_resize((int)width, (int)height);
    }
    return EM_TRUE;
}
#endif

// Функция для инициализации небесных тел
void initialize_bodies(std::vector<CelestialBody>& bodies, const SimulationParameters& params) {
    bodies.clear(); // Очищаем существующие тела

    // Центральный объект
    CelestialBody central_body;
    central_body.id = 0;
    central_body.x = 0;
    central_body.y = 0;
    central_body.vx = 0;
    central_body.vy = 0;
    central_body.mass = params.CENTRAL_BODY_MASS;
    central_body.radius = std::cbrt(central_body.mass / params.DENSITY);
    bodies.push_back(central_body);

    // Настройка генератора случайных чисел
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator(seed);
    std::uniform_real_distribution<float> dist_uniform_0_1(0.0f, 1.0f);
    std::uniform_real_distribution<float> dist_angle(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> dist_mass(params.MIN_MASS, params.MAX_MASS);

    for (int i = 1; i < params.NUM_BODIES; ++i) {
        // Генерируем случайные полярные координаты и преобразуем их в декартовы
        float r = params.INITIALIZATION_RADIUS * std::sqrt(dist_uniform_0_1(generator));
        if (r < 10.0f) r = 10.0f; // Предотвращаем слишком близкое расположение тел к центру
        float angle = dist_angle(generator);

        float x = r * cos(angle);
        float y = r * sin(angle);

        // Скорость для кругового движения: v = sqrt(G * M / r)
        float speed = std::sqrt(params.G * bodies[0].mass / r);
        float vx = -speed * sin(angle);
        float vy = speed * cos(angle);

        CelestialBody new_body;
        new_body.id = i;
        new_body.x = x;
        new_body.y = y;
        new_body.vx = vx;
        new_body.vy = vy;
        new_body.mass = dist_mass(generator);
        new_body.radius = std::cbrt(new_body.mass / params.DENSITY);
        bodies.push_back(new_body);
    }
}

// Функция для обновления состояния симуляции на один шаг
void update_simulation(std::vector<CelestialBody>& bodies, Quadtree& qtree, const SimulationParameters& params) {
    // 1. Очищаем и строим квадродерево
    qtree.clear();
    for (auto& body : bodies) {
        qtree.insert(&body);
    }

    // 2. Проверка столкновений и слияние (с использованием квадродерева)
    for (auto& body_i : bodies) {
        if (body_i.collided) continue;

        std::vector<CelestialBody*> potential_colliders;
        Boundary query_range = { body_i.x, body_i.y, body_i.radius * 2.0f };
        qtree.query(query_range, potential_colliders);

        for (CelestialBody* body_j : potential_colliders) {
            if (body_i.id >= body_j->id) continue;
            if (body_j->collided) continue;

            float dx = body_j->x - body_i.x;
            float dy = body_j->y - body_i.y;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist < body_i.radius + body_j->radius) {
                CelestialBody* smaller = (body_i.radius < body_j->radius) ? &body_i : body_j;
                CelestialBody* larger = (body_i.radius < body_j->radius) ? body_j : &body_i;

                // Сохранение импульса
                float total_mass = larger->mass + smaller->mass;
                larger->vx = (larger->vx * larger->mass + smaller->vx * smaller->mass) / total_mass;
                larger->vy = (larger->vy * larger->mass + smaller->vy * smaller->mass) / total_mass;
                
                // Обновление массы и радиуса
                larger->mass = total_mass;
                larger->radius = std::cbrt(larger->mass / params.DENSITY);

                smaller->collided = true;
            }
        }
    }

    // 3. Удаление "слипшихся" тел
    bodies.erase(
        std::remove_if(bodies.begin(), bodies.end(), [](const CelestialBody& body) {
            return body.collided;
        }),
        bodies.end()
    );

    // 4. Сброс ускорений
    for (auto& body : bodies) {
        body.ax = 0.0f;
        body.ay = 0.0f;
    }

    // 5. Вычисление сил и ускорений (с использованием алгоритма Барнса-Хата)
    qtree.compute_mass_distribution();
    for (auto& body : bodies) {
        qtree.calculate_force(body, params.THETA, params.G, params.SOFTENING_FACTOR);
    }

    // 6. Обновление скоростей и положений
    for (auto& body : bodies) {
        body.vx += body.ax * params.DT;
        body.vy += body.ay * params.DT;
        body.x += body.vx * params.DT;
        body.y += body.vy * params.DT;
    }
}

// Глобальные переменные для симуляции
std::vector<CelestialBody> g_bodies;
SimulationParameters g_params;
Renderer* g_renderer = nullptr;
Quadtree* g_quadtree = nullptr;

// Функция для сброса симуляции
void reset_simulation() {
    initialize_bodies(g_bodies, g_params);
    if (g_quadtree) {
        delete g_quadtree;
    }
    Boundary boundary = { 0.0f, 0.0f, g_params.INITIALIZATION_RADIUS * 2.0f };
    g_quadtree = new Quadtree(boundary, 4);
}

#ifdef __EMSCRIPTEN__
void main_loop(void* arg) {
    SimulationContext* context = static_cast<SimulationContext*>(arg);
    update_simulation(*context->bodies, **context->quadtree, *context->params);
    context->renderer->render(*context->bodies);
}
#endif

int main(int argc, char* argv[]) {
    // Устанавливаем параметры по умолчанию
    g_params.G = 10.0f;
    g_params.DENSITY = 1.0f;
    g_params.NUM_BODIES = 1000;
    g_params.INITIALIZATION_RADIUS = 100.0f;
    g_params.DT = 0.05f;
    g_params.SOFTENING_FACTOR = 10.0f;
    g_params.MAX_MASS = 0.1f;
    g_params.MIN_MASS = 0.001f;
    g_params.CENTRAL_BODY_MASS = 1000.0f;

    initialize_bodies(g_bodies, g_params);

    Boundary boundary = { 0.0f, 0.0f, g_params.INITIALIZATION_RADIUS * 2.0f };
    g_quadtree = new Quadtree(boundary, 4);

    int width, height;
#ifdef __EMSCRIPTEN__
    emscripten_get_canvas_element_size("#canvas", &width, &height);
#else
    width = 800;
    height = 800;
#endif

    Renderer renderer(width, height);
    g_renderer = &renderer;
    if (!g_renderer->init(g_params.INITIALIZATION_RADIUS)) {
        return -1;
    }

#ifdef __EMSCRIPTEN__
    static SimulationContext context_instance = { g_renderer, &g_bodies, &g_params, &g_quadtree };
    g_context = &context_instance;
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, g_context, EM_FALSE, on_web_display_size_changed);
    on_web_display_size_changed(0, nullptr, g_context); // Initial call
    emscripten_set_main_loop_arg(main_loop, g_context, 0, 1);
#endif

    return 0;
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_BINDINGS(simulation_module) {
    emscripten::value_object<SimulationParameters>("SimulationParameters")
        .field("G", &SimulationParameters::G)
        .field("DENSITY", &SimulationParameters::DENSITY)
        .field("NUM_BODIES", &SimulationParameters::NUM_BODIES)
        .field("INITIALIZATION_RADIUS", &SimulationParameters::INITIALIZATION_RADIUS)
        .field("DT", &SimulationParameters::DT)
        .field("SOFTENING_FACTOR", &SimulationParameters::SOFTENING_FACTOR)
        .field("MAX_MASS", &SimulationParameters::MAX_MASS)
        .field("MIN_MASS", &SimulationParameters::MIN_MASS)
        .field("CENTRAL_BODY_MASS", &SimulationParameters::CENTRAL_BODY_MASS)
        .field("THETA", &SimulationParameters::THETA);

    emscripten::function("getSimulationParameters", emscripten::select_overload<SimulationParameters()>([]() -> SimulationParameters {
        return g_params;
    }));

    emscripten::function("setSimulationParameters", emscripten::select_overload<void(SimulationParameters)>([](SimulationParameters new_params) {
        g_params = new_params;
        reset_simulation();
    }));

    emscripten::function("resetSimulation", &reset_simulation);
}
#endif
