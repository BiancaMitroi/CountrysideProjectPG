#version 410 core

out vec4 fColor;
uniform float param;
void main() 
{    
    fColor = vec4(0.54f, 0.76f, 0.28f, param);
}
