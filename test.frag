#version 460 core
layout(location = 0) out vec3 color;

in vec2 texcoords;

uniform sampler2D ourTexture;

void main() {
    vec3 texc = texture(ourTexture, texcoords).rgb;
    vec3 basec = vec3(texcoords.x, texcoords.y, 0.0f);
    color = mix(texc, basec, 0.5f);
 //   color = vec3(1.0f, 0.5f, 0.2f);
}