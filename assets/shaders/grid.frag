#version 450

layout(location = 0) in vec3 fragWorldPos;
layout(location = 0) out vec4 outColor;

float gridLine(float coord, float lineWidth) {
    float d = fract(coord + 0.5) - 0.5;
    float line = 1.0 - smoothstep(0.0, lineWidth, abs(d));
    return line;
}

void main() {
    // Grid lines every 1 unit
    float lineWidth = 0.02;
    float gx = gridLine(fragWorldPos.x, lineWidth);
    float gz = gridLine(fragWorldPos.z, lineWidth);
    float grid = max(gx, gz);

    // Major lines every 5 units
    float majorWidth = 0.03;
    float mx = gridLine(fragWorldPos.x / 5.0, majorWidth);
    float mz = gridLine(fragWorldPos.z / 5.0, majorWidth);
    float major = max(mx, mz);

    // Axis highlights
    float axisWidth = 0.05;
    float xAxis = gridLine(fragWorldPos.z, axisWidth); // X axis (z = 0)
    float zAxis = gridLine(fragWorldPos.x, axisWidth); // Z axis (x = 0)
    bool onXAxis = abs(fragWorldPos.z) < axisWidth * 2.0;
    bool onZAxis = abs(fragWorldPos.x) < axisWidth * 2.0;

    // Base color
    vec3 color = vec3(0.25) * grid;
    color = mix(color, vec3(0.4), major);

    // Red X-axis, Blue Z-axis
    if (onXAxis) color = mix(color, vec3(0.8, 0.2, 0.2), xAxis);
    if (onZAxis) color = mix(color, vec3(0.2, 0.2, 0.8), zAxis);

    // Fade with distance
    float dist = length(fragWorldPos.xz);
    float fade = 1.0 - smoothstep(20.0, 50.0, dist);
    float alpha = max(grid, major) * fade * 0.6;

    if (alpha < 0.01) discard;

    outColor = vec4(color, alpha);
}
