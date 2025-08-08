#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>

#include "quadtree.h"

// Структура для представления небесного тела
struct CelestialBody {
  int id;
  float x, y;             // Положение
  float vx, vy;           // Скорость
  float mass;             // Масса
  float radius;           // Радиус
  bool collided = false;  // флаг для удаления

  // Ускорение (вычисляется на каждом шаге)
  float ax = 0.0f, ay = 0.0f;
};

// Структура для хранения параметров симуляции
struct SimulationParameters {
  float G = 10.0f;        // Гравитационная постоянная
  float DENSITY = 1.0f;   // Плотность
  int NUM_BODIES = 1000;  // Количество небесных тел
  float INITIALIZATION_RADIUS =
      100.0f;        // Радиус круга для начальной генерации объектов
  float DT = 0.05f;  // Шаг по времени
  float SOFTENING_FACTOR = 10.0f;     // Смягчающий фактор
  float MAX_MASS = 0.1f;              // Максимальная масса
  float MIN_MASS = 0.001f;            // Минимальная масса
  float CENTRAL_BODY_MASS = 1000.0f;  // Масса центрального объекта
  float THETA = 0.5f;                 // Точность для алгоритма Барнса-Хата
};

// Объявление функций
void initialize_bodies(std::vector<CelestialBody>& bodies,
                       const SimulationParameters& params);
void update_simulation(std::vector<CelestialBody>& bodies, Quadtree& qtree,
                       const SimulationParameters& params);

#endif  // SIMULATION_H
