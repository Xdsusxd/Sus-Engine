#version 450

// Quad vertices generated in the shader
const vec2 quadVertices[6] = vec2[](
    vec2(-0.5, -0.5),
    vec2( 0.5, -0.5),
    vec2( 0.5,  0.5),
    vec2( 0.5,  0.5),
    vec2(-0.5,  0.5),
    vec2(-0.5, -0.5)
);

layout(push_constant) uniform CameraData {
    mat4 viewProj;
    vec3 cameraRight;
    float padding1;
    vec3 cameraUp;
    float padding2;
} camera;

// Instance inputs
layout(location = 0) in vec3 inPos;
layout(location = 1) in float inSize;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragUV;

void main() {
    vec2 vertexPos = quadVertices[gl_VertexIndex];
    
    // Billboarding: use camera right and up vectors
    vec3 worldPos = inPos 
                  + camera.cameraRight * vertexPos.x * inSize 
                  + camera.cameraUp * vertexPos.y * inSize;
                  
    gl_Position = camera.viewProj * vec4(worldPos, 1.0);
    
    fragColor = inColor;
    fragUV = vertexPos + vec2(0.5); // 0 to 1
}
