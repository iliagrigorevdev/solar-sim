#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <chrono>
#include "renderer.h"
#include "simulation.h"

// --- Константы для симуляции ---

// Гравитационная постоянная (масштабирована для лучшей визуализации)
const double G = 500.0;

// Количество небесных тел
const int NUM_BODIES = 100;

// Радиус круга для начальной генерации объектов
const double INITIALIZATION_RADIUS = 200.0;

// Шаг по времени для каждого обновления симуляции
const double DT = 0.01;

// Смягчающий фактор для предотвращения бесконечных сил на малых расстояниях
const double SOFTENING_FACTOR = 10.0;

// Максимальные значения для случайной генерации
const double MAX_MASS = 50.0;
const double MIN_MASS = 5.0;
const double MAX_INITIAL_VELOCITY = 10.0;


// Функция для инициализации небесных тел
void initialize_bodies(std::vector<CelestialBody>& bodies) {
    // Настройка генератора случайных чисел
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator(seed);
    std::uniform_real_distribution<double> dist_radius(0.0, INITIALIZATION_RADIUS);
    std::uniform_real_distribution<double> dist_angle(0.0, 2.0 * M_PI);
    std::uniform_real_distribution<double> dist_vel(-MAX_INITIAL_VELOCITY, MAX_INITIAL_VELOCITY);
    std::uniform_real_distribution<double> dist_mass(MIN_MASS, MAX_MASS);

    for (int i = 0; i < NUM_BODIES; ++i) {
        // Генерируем случайные полярные координаты и преобразуем их в декартовы
        // Использование sqrt() обеспечивает равномерное распределение по площади круга
        double r = std::sqrt(dist_radius(generator)) * INITIALIZATION_RADIUS;
        double angle = dist_angle(generator);

        double x = r * cos(angle);
        double y = r * sin(angle);

        CelestialBody new_body;
        new_body.x = x;
        new_body.y = y;
        new_body.vx = dist_vel(generator);
        new_body.vy = dist_vel(generator);
        new_body.mass = dist_mass(generator);
        bodies.push_back(new_body);
    }
}

// Функция для обновления состояния симуляции на один шаг
void update_simulation(std::vector<CelestialBody>& bodies) {
    // 1. Сброс ускорений
    for (auto& body : bodies) {
        body.ax = 0.0;
        body.ay = 0.0;
    }

    // 2. Вычисление сил и ускорений (O(N^2))
    for (size_t i = 0; i < bodies.size(); ++i) {
        for (size_t j = 0; j < bodies.size(); ++j) {
            if (i == j) continue;

            // Вектор расстояния от тела i до тела j
            double dx = bodies[j].x - bodies[i].x;
            double dy = bodies[j].y - bodies[i].y;

            // Квадрат расстояния с фактором смягчения
            double dist_sq = dx * dx + dy * dy + SOFTENING_FACTOR * SOFTENING_FACTOR;
            double dist = std::sqrt(dist_sq);

            // Сила гравитации: F = G * m1 * m2 / r^2
            double force_magnitude = (G * bodies[i].mass * bodies[j].mass) / dist_sq;

            // Направление силы (единичный вектор)
            double dir_x = dx / dist;
            double dir_y = dy / dist;

            // Добавляем компоненты силы к ускорению тела i
            // a = F/m, поэтому a = G * m2 / r^2
            bodies[i].ax += dir_x * (G * bodies[j].mass) / dist_sq;
            bodies[i].ay += dir_y * (G * bodies[j].mass) / dist_sq;
        }
    }

    // 3. Обновление скоростей и положений (интегрирование)
    for (auto& body : bodies) {
        // Обновляем скорость
        body.vx += body.ax * DT;
        body.vy += body.ay * DT;
        // Обновляем положение
        body.x += body.vx * DT;
        body.y += body.vy * DT;
    }
}


int main(int argc, char* argv[]) {
    std::vector<CelestialBody> bodies;
    initialize_bodies(bodies);

    Renderer renderer(800, 800);
    if (!renderer.init()) {
        return -1;
    }

    bool running = true;
    while(running) {
        running = renderer.handle_events();
        update_simulation(bodies);
        renderer.render(bodies);
    }

    return 0;
}
