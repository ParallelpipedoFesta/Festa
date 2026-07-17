#version 330 core
layout(location=0)in vec3 aPos;
layout(location=1)in vec3 aNormal;
layout(location=2)in vec2 aTexCoords;
layout(location=3)in vec4 aBones;
layout(location=4)in vec4 aWeights;

const int MAX_BONES = 100;
uniform mat4 boneMatrices[MAX_BONES+1];

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 FragPos;
out vec3 N;
out vec2 TexCoords;


void main(){
	vec4 position = vec4(0.0);
	mat4 boneMat = mat4(0.0);
	for(int i=0; i<4; i++){
		if(aBones[i] < 0.0)continue;
		else if(aBones[i]==0.0 || aBones[i]>MAX_BONES){
			position = vec4(aPos, 1.0f);
			boneMat = mat4(1.0f);
			break;
		}
		position += aWeights[i] * boneMatrices[int(aBones[i])] * vec4(aPos, 1.0);
		boneMat += aWeights[i] * boneMatrices[int(aBones[i])];
	}
    vec4 worldPos = model*position;
	gl_Position = projection*view*model*position;
	FragPos = worldPos.xyz;
	N = transpose(inverse(mat3(boneMat*model))) * aNormal;
	TexCoords = vec2(aTexCoords.x, 1.0 - aTexCoords.y);
}