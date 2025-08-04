#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>

// Структура для представления небесного тела
struct CelestialBody {
    int id;
    float x, y;    // Положение
    float vx, vy;  // Скорость
    float mass;    // Масса
    float radius;  // Радиус
    bool collided = false; // флаг для удаления

    // Ускорение (вычисляется на каждом шаге)
    float ax = 0.0f, ay = 0.0f;
};

// Структура для хранения параметров симуляции
struct SimulationParameters {
    float G;                  // Гравитационная постоянная
    float DENSITY;            // Плотность
    int NUM_BODIES;           // Количество небесных тел
    float INITIALIZATION_RADIUS; // Радиус круга для начальной генерации объектов
    float DT;                 // Шаг по времени
    float SOFTENING_FACTOR;   // Смягчающий фактор
    float MAX_MASS;           // Максимальная масса
    float MIN_MASS;           // Минимальная масса
};

// Объявление функций
void initialize_bodies(std::vector<CelestialBody>& bodies, const SimulationParameters& params);
void update_simulation(std::vector<CelestialBody>& bodies, const SimulationParameters& params);

#endif // SIMULATION_H
