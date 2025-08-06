#version 300 es
#extension GL_OES_standard_derivatives : enable
precision highp float;

out vec4 out_color;

uniform float u_min_radius;
uniform float u_max_radius;

in float v_radius;

// Функция для отображения радиуса в цвет (фиолетовый-синий-зеленый-желтый-красный)
vec3 radiusToColor(float radius) {
    // The radius can be from u_min_radius to u_max_radius. We use a logarithmic scale.
    float log_radius = log(radius);
    // Normalize log_radius from [log(u_min_radius), log(u_max_radius)] to [0, 1]
    float log_min = log(u_min_radius);
    float log_max = log(u_max_radius);
    float t = (log_radius - log_min) / (log_max - log_min);
    t = clamp(t, 0.0, 1.0);

    vec3 violet = vec3(0.6, 0.2, 1.0);
    vec3 blue = vec3(0.2, 0.5, 1.0);
    vec3 yellow = vec3(1.0, 0.9, 0.2);
    vec3 red = vec3(1.0, 0.3, 0.2);

    if (t < 0.5) {
        return mix(violet, blue, t * 2.0);
    } else if (t < 0.75) {
        return mix(blue, yellow, (t - 0.5) * 4.0);
    } else {
        return mix(yellow, red, (t - 0.75) * 4.0);
    }
}

void main() {
    float dist = length(gl_PointCoord - vec2(0.5));
    float edge_width = fwidth(dist);
    float alpha = 1.0 - smoothstep(0.5 - edge_width, 0.5 + edge_width, dist);

    if (alpha <= 0.0) {
        discard;
    }

    vec3 color = radiusToColor(v_radius);
    out_color = vec4(color, alpha);
}
