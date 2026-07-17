#version 440 core
uniform sampler2D tex;
uniform float time;
in vec2 coord;
out vec4 FragColor;

void main(){
    vec4 color=texture(tex,coord);
	FragColor=vec4(1.0-color.r,1.0-color.g,1.0-color.b,1.0);
}