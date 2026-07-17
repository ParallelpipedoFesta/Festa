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

float _BlockSize=100.0f;
vec2 t=fract(coord * _BlockSize);
float block = nrand(t.x,t.y);
float displaceNoise = pow(block, 8.0) * pow(block, 3.0);

void main(){
    float r=texture(tex,coord).r;
    float g=texture(tex,fract(vec2(u+displaceNoise * 0.05 * nrand(7.0, time), v))).g;
    float b=texture(tex,fract(vec2(u-displaceNoise * 0.05 * nrand(13.0, time), v))).b;
	FragColor=vec4(r,g,b,1.0);
}