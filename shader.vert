#version 300 es

in vec2 a_body_pos;
in float a_body_radius;

uniform float u_initialization_radius;
uniform vec2 u_resolution;
uniform float u_zoom;

out float v_radius;

void main() {
    float resolution = min(u_resolution.x, u_resolution.y);
    vec2 scaled_pos = a_body_pos / u_initialization_radius;
    
    vec2 aspect_ratio_correction = u_resolution.x > u_resolution.y ? vec2(u_resolution.y / u_resolution.x, 1.0) : vec2(1.0, u_resolution.x / u_resolution.y);
    
    gl_Position = vec4(scaled_pos * aspect_ratio_correction * u_zoom, 0.0, 1.0);
    gl_PointSize = max(a_body_radius / u_initialization_radius * resolution * u_zoom, 2.0);
    v_radius = a_body_radius;
}
