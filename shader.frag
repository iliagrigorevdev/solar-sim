precision mediump float;

uniform vec2 u_resolution;
uniform int u_num_bodies;

#define MAX_BODIES 100
uniform vec2 u_body_positions[MAX_BODIES];
uniform float u_body_radii[MAX_BODIES];

// Функция знакового расстояния для круга
float sdCircle(vec2 p, float r) {
    return length(p) - r;
}

void main() {
    vec2 uv = (gl_FragCoord.xy * 2.0 - u_resolution.xy) / min(u_resolution.x, u_resolution.y);

    float total_color = 0.0;

    for (int i = 0; i < MAX_BODIES; i++) {
        if (i >= u_num_bodies) break;

        // Масштабируем позицию и радиус
        vec2 scaled_pos = u_body_positions[i] / 200.0;
        float scaled_radius = u_body_radii[i] / 1000.0;

        float dist = sdCircle(uv - scaled_pos, scaled_radius);
        total_color += 1.0 - smoothstep(0.0, 0.005, dist);
    }

    gl_FragColor = vec4(vec3(total_color), 1.0);
}
