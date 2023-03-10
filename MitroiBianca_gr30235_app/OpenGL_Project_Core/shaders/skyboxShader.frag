#version 410 core

in vec3 textureCoordinates;
out vec4 color;

uniform samplerCube skybox;
uniform int fog;

void main()
{
	if(fog == 1){
		color = vec4(0.7f, 0.7f, 0.7f, 1.0f);
	}else{
		color = texture(skybox, textureCoordinates);
	}
}
