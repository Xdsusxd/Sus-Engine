#version 450

layout(push_constant) uniform PushConstants {
    mat4 viewProj;
} pc;

// Fullscreen quad positions
const vec3 gridPlane[6] = vec3[6](
    vec3(-50, 0, -50), vec3( 50, 0, -50), vec3( 50, 0,  50),
    vec3( 50, 0,  50), vec3(-50, 0,  50), vec3(-50, 0, -50)
);

layout(location = 0) out vec3 fragWorldPos;

void main() {
    vec3 pos = gridPlane[gl_VertexIndex];
    fragWorldPos = pos;
    gl_Position = pc.viewProj * vec4(pos, 1.0);
}
