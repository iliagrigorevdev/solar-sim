#version 300 es
#extension GL_OES_standard_derivatives : enable
precision highp float;

#define MAX_COLORS 8

out vec4 out_color;

uniform float u_min_radius;
uniform float u_max_radius;
uniform int u_num_colors;
uniform vec3 u_colors[MAX_COLORS];
uniform float u_weights[MAX_COLORS];

in float v_radius;

vec3 get_color_from_gradient(float t) {
    if (u_num_colors == 0) {
        return vec3(1.0, 1.0, 1.0); // Default to white if no colors
    }
    if (u_num_colors == 1) {
        return u_colors[0];
    }

    // Find the two colors to interpolate between
    for (int i = 0; i < u_num_colors - 1; i++) {
        if (t >= u_weights[i] && t <= u_weights[i+1]) {
            // Normalize t within this segment
            float segment_t = (t - u_weights[i]) / (u_weights[i+1] - u_weights[i]);
            return mix(u_colors[i], u_colors[i+1], segment_t);
        }
    }

    // Handle edge cases (t outside of defined weights)
    if (t < u_weights[0]) {
        return u_colors[0];
    }
    if (t > u_weights[u_num_colors - 1]) {
        return u_colors[u_num_colors - 1];
    }
    
    return vec3(1.0, 0.0, 1.0); // Magenta for error
}

void main() {
    float dist = length(gl_PointCoord - vec2(0.5));
    float edge_width = fwidth(dist);
    float alpha = 1.0 - smoothstep(0.5 - edge_width, 0.5 + edge_width, dist);

    if (alpha <= 0.0) {
        discard;
    }

    // Normalize radius on a logarithmic scale
    float log_radius = log(v_radius);
    float log_min = log(u_min_radius);
    float log_max = log(u_max_radius);
    float t = (log_radius - log_min) / (log_max - log_min);
    t = clamp(t, 0.0, 1.0);

    vec3 color = get_color_from_gradient(t);
    out_color = vec4(color, alpha);
}