#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

void main() {
    // Simple soft circle alpha based on UV
    vec2 centerDist = fragUV - vec2(0.5);
    float dist = length(centerDist) * 2.0; // 0 at center, 1 at edge
    
    float alpha = 1.0 - smoothstep(0.8, 1.0, dist);
    
    if (alpha < 0.01) {
        discard;
    }
    
    outColor = fragColor * vec4(1.0, 1.0, 1.0, alpha);
}
