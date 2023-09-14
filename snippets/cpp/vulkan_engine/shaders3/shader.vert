#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 0) out vec3 fragColor;

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

struct ObjectData{
	mat4 model;
};

layout(std140, binding = 1) readonly buffer ObjectBuffer{
	ObjectData objects[];
} objectBuffer;


void main() {
    mat4 modelMatrix = objectBuffer.objects[gl_InstanceIndex].model;
    gl_Position = ubo.proj * ubo.view * modelMatrix * vec4(inPosition, 1.0);
    fragColor = inColor;
}
