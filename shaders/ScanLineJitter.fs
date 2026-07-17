#version 440 core
uniform sampler2D tex;
uniform float time;
in vec2 coord;
out vec4 FragColor;

float nrand(float x, float y){
  return fract(sin(dot(vec2(x, y), vec2(12.9898, 78.233))) * 43758.5453);
}

float u=coord.x;
float v=coord.y;

vec4 ScanLineJitter(){
  vec2 _ScanLineJitter=vec2(0.015,0.1);
  float jitter = nrand(v, time) * 2 - 1;
  return texture(tex,vec2(u + jitter*(step(_ScanLineJitter.y, abs(jitter)) * _ScanLineJitter.x), v));
}

void main(){
	FragColor=ScanLineJitter();
}