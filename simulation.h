#ifndef SIMULATION_H
#define SIMULATION_H

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

#endif // SIMULATION_H
