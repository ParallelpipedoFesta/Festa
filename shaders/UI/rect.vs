#version 330 core
layout(location=0)in vec2 aPos;
out vec2 TexCoords;
uniform mat4 model;

void main(){
    gl_Position = model * vec4(aPos,0.0f,1.0f);
	TexCoords=vec2(aPos.x*0.5f+0.5f, -aPos.y*0.5f+0.5f);
}