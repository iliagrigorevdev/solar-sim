#ifndef SIMULATION_H
#define SIMULATION_H

// Структура для представления небесного тела
struct CelestialBody {
    double x, y;    // Положение
    double vx, vy;  // Скорость
    double mass;    // Масса

    // Ускорение (вычисляется на каждом шаге)
    double ax = 0.0, ay = 0.0;
};

#endif // SIMULATION_H
