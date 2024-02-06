#version 330 core

in vec2 texCoord;

out vec4 fragColor;

uniform sampler2D textureSampler;

void main() {
    fragColor = texture(textureSampler, texCoord);
}