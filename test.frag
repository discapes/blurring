#version 460 core

out vec3 color;

in vec2 texCoord;

uniform sampler2D tex;

void main() {
    vec3 texCol = texture(tex, texCoord).rgb;
    vec3 baseCol = vec3(texCoord.st, 0.0f);
	color = mix(texCol, baseCol, 0.5f);
 //   color = vec3(1.0f, 0.5f, 0.2f);
}