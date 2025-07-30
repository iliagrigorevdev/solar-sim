#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <chrono>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include "renderer.h"
#include "simulation.h"

// --- Константы для симуляции ---

// Гравитационная постоянная (масштабирована для лучшей визуализации)
const float G = 500.0f;

// Плотность (для расчета радиуса)
const float DENSITY = 1000.0f;

// Количество небесных тел
const int NUM_BODIES = 100;

// Радиус круга для начальной генерации объектов
const float INITIALIZATION_RADIUS = 200.0f;

// Шаг по времени для каждого обновления симуляции
const float DT = 0.05f;

// Смягчающий фактор для предотвращения бесконечных сил на малых расстояниях
const float SOFTENING_FACTOR = 10.0f;

// Максимальные значения для случайной генерации
const float MAX_MASS = 50.0f;
const float MIN_MASS = 5.0f;
const float MAX_INITIAL_VELOCITY = 10.0f;

struct SimulationContext {
    Renderer* renderer;
    std::vector<CelestialBody>* bodies;
};

// Функция для инициализации небесных тел
void initialize_bodies(std::vector<CelestialBody>& bodies) {
    // Настройка генератора случайных чисел
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator(seed);
    std::uniform_real_distribution<float> dist_radius(0.0f, INITIALIZATION_RADIUS);
    std::uniform_real_distribution<float> dist_angle(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> dist_vel(-MAX_INITIAL_VELOCITY, MAX_INITIAL_VELOCITY);
    std::uniform_real_distribution<float> dist_mass(MIN_MASS, MAX_MASS);

    for (int i = 0; i < NUM_BODIES; ++i) {
        // Генерируем случайные полярные координаты и преобразуем их в декартовы
        // Использование sqrt() обеспечивает равномерное распределение по площади круга
        float r = std::sqrt(dist_radius(generator)) * INITIALIZATION_RADIUS;
        float angle = dist_angle(generator);

        float x = r * cos(angle);
        float y = r * sin(angle);

        CelestialBody new_body;
        new_body.id = i;
        new_body.x = x;
        new_body.y = y;
        new_body.vx = dist_vel(generator);
        new_body.vy = dist_vel(generator);
        new_body.mass = dist_mass(generator);
        new_body.radius = std::cbrt(new_body.mass / DENSITY); // Радиус зависит от массы и плотности
        bodies.push_back(new_body);
    }
}

// Функция для обновления состояния симуляции на один шаг
void update_simulation(std::vector<CelestialBody>& bodies) {
    // 1. Сброс ускорений
    for (auto& body : bodies) {
        body.ax = 0.0f;
        body.ay = 0.0f;
    }

    // 2. Проверка столкновений и слияние
    for (size_t i = 0; i < bodies.size(); ++i) {
        for (size_t j = i + 1; j < bodies.size(); ++j) {
            if (bodies[i].collided || bodies[j].collided) continue;

            float dx = bodies[j].x - bodies[i].x;
            float dy = bodies[j].y - bodies[i].y;
            float dist_sq = dx * dx + dy * dy;
            float dist = std::sqrt(dist_sq);

            float r1 = bodies[i].radius;
            float r2 = bodies[j].radius;

            if (dist < r1 + r2) { // Обнаружено столкновение
                CelestialBody* smaller = (r1 < r2) ? &bodies[i] : &bodies[j];
                CelestialBody* larger = (r1 < r2) ? &bodies[j] : &bodies[i];

                float overlap = (smaller->radius + larger->radius) - dist;

                if (overlap > 0.25f * smaller->radius) {
                    // Сохранение импульса
                    float total_mass = larger->mass + smaller->mass;
                    larger->vx = (larger->vx * larger->mass + smaller->vx * smaller->mass) / total_mass;
                    larger->vy = (larger->vy * larger->mass + smaller->vy * smaller->mass) / total_mass;
                    
                    // Обновление массы и радиуса большего тела
                    larger->mass = total_mass;
                    larger->radius = std::cbrt(larger->mass / DENSITY);

                    // Помечаем меньшее тело для удаления
                    smaller->collided = true;
                }
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

    // 5. Вычисление сил и ускорений (O(N^2))
    for (size_t i = 0; i < bodies.size(); ++i) {
        for (size_t j = 0; j < bodies.size(); ++j) {
            if (i == j) continue;

            // Вектор расстояния от тела i до тела j
            float dx = bodies[j].x - bodies[i].x;
            float dy = bodies[j].y - bodies[i].y;

            // Квадрат расстояния с фактором смягчения
            float dist_sq = dx * dx + dy * dy + SOFTENING_FACTOR * SOFTENING_FACTOR;
            float dist = std::sqrt(dist_sq);

            // Сила гравитации: F = G * m1 * m2 / r^2
            float force_magnitude = (G * bodies[i].mass * bodies[j].mass) / dist_sq;

            // Направление силы (единичный вектор)
            float dir_x = dx / dist;
            float dir_y = dy / dist;

            // Добавляем компоненты силы к ускорению тела i
            // a = F/m, поэтому a = G * m2 / r^2
            bodies[i].ax += dir_x * (G * bodies[j].mass) / dist_sq;
            bodies[i].ay += dir_y * (G * bodies[j].mass) / dist_sq;
        }
    }

    // 6. Обновление скоростей и положений (интегрирование)
    for (auto& body : bodies) {
        // Обновляем скорость
        body.vx += body.ax * DT;
        body.vy += body.ay * DT;
        // Обновляем положение
        body.x += body.vx * DT;
        body.y += body.vy * DT;
    }
}

#ifdef __EMSCRIPTEN__
void main_loop(void* arg) {
    SimulationContext* context = static_cast<SimulationContext*>(arg);
    if (!context->renderer->handle_events()) {
        emscripten_cancel_main_loop();
    }
    update_simulation(*context->bodies);
    context->renderer->render(*context->bodies);
}
#endif

int main(int argc, char* argv[]) {
    std::vector<CelestialBody> bodies;
    initialize_bodies(bodies);

    Renderer renderer(800, 800);
    if (!renderer.init()) {
        return -1;
    }

    #ifdef __EMSCRIPTEN__
    SimulationContext context = { &renderer, &bodies };
    emscripten_set_main_loop_arg(main_loop, &context, 0, 1);
#else
    bool running = true;
    while(running) {
        running = renderer.handle_events();
        update_simulation(bodies);
        renderer.render(bodies);
    }
#endif

    return 0;
}
