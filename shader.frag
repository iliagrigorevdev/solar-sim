precision mediump float;

uniform vec2 u_resolution;
uniform vec2 u_body_pos;    // Позиция тела (x, y)
uniform float u_body_radius; // Радиус тела (масса)

// Функция знакового расстояния для круга
float sdCircle(vec2 p, float r) {
    return length(p) - r;
}

void main() {
    vec2 uv = (gl_FragCoord.xy * 2.0 - u_resolution.xy) / u_resolution.y;

    // Масштабируем позицию и радиус
    vec2 scaled_pos = u_body_pos / 1000.0;
    float scaled_radius = u_body_radius / 1000.0;

    float dist = sdCircle(uv - scaled_pos, scaled_radius);

    float color = 1.0 - smoothstep(0.0, 0.005, dist);

    gl_FragColor = vec4(vec3(color), 1.0);
}
