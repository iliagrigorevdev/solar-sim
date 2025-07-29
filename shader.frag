precision mediump float;

uniform vec2 u_resolution;
uniform int u_num_bodies;
// Структура для хранения данных о теле: x, y, радиус (масса)
uniform vec3 u_bodies[100]; 

// Функция знакового расстояния для круга
// p - точка, r - радиус
float sdCircle(vec2 p, float r) {
    return length(p) - r;
}

void main() {
    // Нормализуем координаты фрагмента и центрируем их
    vec2 uv = (gl_FragCoord.xy * 2.0 - u_resolution.xy) / u_resolution.y;

    float min_dist = 1e9; // Инициализируем минимальное расстояние большим значением

    // Итерируемся по всем телам, чтобы найти ближайшее
    for (int i = 0; i < 100; ++i) {
        // Выходим из цикла, если достигли последнего тела
        if (i >= u_num_bodies) break;

        vec3 body = u_bodies[i];
        vec2 pos = body.xy;
        float radius = body.z;

        // Масштабируем позицию и радиус из пространства симуляции в пространство экрана
        // Предполагаем, что основное действие симуляции происходит в диапазоне [-400, 400]
        vec2 scaled_pos = pos / 400.0;
        float scaled_radius = radius / 400.0;
        
        // Вычисляем расстояние до текущего тела и обновляем минимальное расстояние
        min_dist = min(min_dist, sdCircle(uv - scaled_pos, scaled_radius));
    }

    // Задаем цвет в зависимости от расстояния до ближайшего тела
    // Плавный переход для создания эффекта свечения/антиалиасинга
    float smooth_edge = 1.0 - smoothstep(0.0, 0.01, min_dist);
    
    // Цвет тела - белый, с плавной границей
    gl_FragColor = vec4(vec3(smooth_edge), 1.0);
}
