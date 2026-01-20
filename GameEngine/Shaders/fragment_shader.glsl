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

    // EL TRUCO:
    // El suelo lo pusimos en Y = -20. Las ruinas están en Y = 0.
    // Si la altura (fragPos.y) es menor que -10, sabemos que es el suelo.
    if(fragPos.y < -10.0) 
    {
        uv = uv * 50.0; // Repetimos la textura 50 veces (Efecto Mosaico)
    }

    fragColor = texture(texture1, uv);
}