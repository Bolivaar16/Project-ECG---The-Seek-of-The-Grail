#version 400

in vec2 textureCoord;
in vec3 norm;
in vec3 fragPos;

out vec4 fragColor;

uniform sampler2D texture1;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    vec2 uv = textureCoord;

    if(fragPos.y < -10.0) 
    {
        uv = uv * 50.0;
    }

    fragColor = texture(texture1, uv);
}