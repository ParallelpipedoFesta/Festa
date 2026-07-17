#version 440 core
uniform sampler2D tex;
uniform float time;
in vec2 coord;
out vec4 FragColor;

float nrand(float x, float y){
  return (sin(dot(vec2(x, y), vec2(12.9898, 78.233))) * 43758.5453);
}

float u=coord.x;
float v=coord.y;

float _verticalJump=0.2f;
float jump = mix(v, fract(v + time* _verticalJump * 11.3f), _verticalJump);

void main(){
	FragColor=texture(tex,fract(vec2(u,jump)));
}