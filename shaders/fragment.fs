#version 460 core

in vec3 fragPos;
in vec3 normal;
out vec4 FragColor;

void main()
{
	//FragColor = vec4(1.0f,0.5f,0.2f,0.5f);
	vec3 eye_pos = vec3(0,0,0);
	FragColor = vec4(fragPos*1.5f,1.0f);
}
