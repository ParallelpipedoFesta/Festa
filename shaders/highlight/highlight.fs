#version 330 core

uniform vec4 col = vec4(1.0, 1.0, 1.0, 0.5);
out vec4 FragColor;
void main(){
	FragColor=col;
}