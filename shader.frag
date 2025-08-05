#version 300 es
#extension GL_OES_standard_derivatives : enable
precision highp float;

out vec4 out_color;

uniform vec2 u_resolution;
uniform int u_num_bodies;
uniform sampler2D u_data_texture;

const float TEXTURE_WIDTH = 2048.0;
const int MAX_BODIES = 2048;


uniform float u_initialization_radius;
uniform float u_zoom;

// Функция для отображения радиуса в цвет (фиолетовый-синий-зеленый-желтый-красный)
vec3 radiusToColor(float radius) {
    // The radius can be from 0.1 to 10. We use a logarithmic scale.
    float log_radius = log(radius);
    // Normalize log_radius from [log(0.1), log(10)] to [0, 1]
    // log(0.1) is approx -2.3, log(10) is approx 2.3
    float t = (log_radius + 2.302585) / 4.60517;
    t = clamp(t, 0.0, 1.0);

    vec3 violet = vec3(0.6, 0.2, 1.0);
    vec3 blue = vec3(0.2, 0.5, 1.0);
    vec3 green = vec3(0.5, 0.9, 0.2);
    vec3 yellow = vec3(1.0, 0.9, 0.2);
    vec3 red = vec3(1.0, 0.3, 0.2);

    if (t < 0.25) {
        return mix(violet, blue, t * 4.0);
    } else if (t < 0.5) {
        return mix(blue, green, (t - 0.25) * 4.0);
    } else if (t < 0.75) {
        return mix(green, yellow, (t - 0.5) * 4.0);
    } else {
        return mix(yellow, red, (t - 0.75) * 4.0);
    }
}

// Функция знакового расстояния для круга
float sdCircle(vec2 p, float r) {
    return length(p) - r;
}

void main() {
    float resolution = min(u_resolution.x, u_resolution.y);
    vec2 uv = (gl_FragCoord.xy * 2.0 - u_resolution.xy) / resolution;
    uv /= u_zoom;

    vec3 final_color = vec3(0.0);
    float total_alpha = 0.0;

    for (int i = 0; i < MAX_BODIES; i++) {
        if (i >= u_num_bodies) break;

        vec4 data = texture(u_data_texture, vec2((float(i) + 0.5) / TEXTURE_WIDTH, 0.5));
        vec2 body_position = data.xy;
        float body_radius = data.z;

        vec2 scaled_pos = body_position / u_initialization_radius;
        float scaled_radius = max(body_radius / u_initialization_radius, 2.0 / (resolution * u_zoom));

        float dist = sdCircle(uv - scaled_pos, scaled_radius);
        
        float edge_width = fwidth(dist);
        float alpha = 1.0 - smoothstep(-edge_width, edge_width, dist);

        if (alpha > 0.0) {
            vec3 body_color = radiusToColor(body_radius);
            final_color += body_color * alpha;
            total_alpha += alpha;
        }
    }

    if (total_alpha > 0.0) {
        final_color /= total_alpha;
    }

    out_color = vec4(final_color, min(total_alpha, 1.0));
}