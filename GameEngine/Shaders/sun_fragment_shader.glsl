#version 400

out vec4 fragColor;

uniform sampler2D texture1;

uniform vec3 objectColor;

void main()
{
    fragColor = vec4(objectColor, 1.0);
}