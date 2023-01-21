#version 410 core

out vec4 fColor;
uniform float param;
void main() 
{    
    fColor = vec4(0.99f, 0.59f, 0.0f, param);
}
