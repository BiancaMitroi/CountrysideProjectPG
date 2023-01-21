#version 410 core

out vec4 fColor;
uniform float param;
void main() 
{    
    fColor = vec4(1.0f, 1.0f, 1.0f, param);
}
