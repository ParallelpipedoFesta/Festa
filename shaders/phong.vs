#version 330 core
layout(location=0)in vec3 pos;
layout(location=1)in vec3 normal;
layout(location=2)in vec2 texCoord;
layout(location=3)in vec4 boneids;
layout(location=4)in vec4 weights;

const int MAX_BONES=100;
uniform mat4 boneMatrices[MAX_BONES+1];

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 Normal;
out vec3 FragPos;
out vec2 texcoord;


void main(){
	vec4 position=vec4(0.0f);
	for(int i=0;i<4;i++){
		if(boneids[i]<0.0f)continue;
		else if(boneids[i]==0.0f||boneids[i]>MAX_BONES){
			position=vec4(pos,1.0f);
			break;
		}
		position+=boneMatrices[int(boneids[i])]*vec4(pos,1.0f);
	}
	gl_Position=projection*view*model*position;
	FragPos=(model*position).xyz;
	Normal=mat3(transpose(inverse(model)))*normal;
	texcoord=texCoord;
}