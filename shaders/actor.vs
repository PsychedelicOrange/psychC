#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec4 aTangent;
layout (location = 3) in vec2 aTexcoords;
layout (location = 4) in vec4 aJoints;
layout (location = 5) in vec4 aWeights;

out vec3 fragPos;
out vec3 normal;
uniform mat4 u_jointMat[100];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	mat4 skinMat =
		aWeights.x * u_jointMat[int(aJoints.x)] +
		aWeights.y * u_jointMat[int(aJoints.y)] +
		aWeights.z * u_jointMat[int(aJoints.z)] +
		aWeights.w * u_jointMat[int(aJoints.w)];
	normal = aNormal;
	vec4 animatedpos = skinMat * vec4(aPos,1.0f);
	gl_Position = projection * view * model * animatedpos;
	//gl_Position = projection * view * model * vec4(aPos,1.0f);
	fragPos = vec3(aPos.x,aPos.y,aPos.z);
}
