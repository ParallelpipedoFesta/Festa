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
float displaceNoise=1.0f;

void main(){
    float r=texture(tex,coord).r;
    float g=texture(tex,vec2(u+displaceNoise * 0.05 * nrand(7.0, time), v)).g;
    float b=texture(tex,vec2(u-displaceNoise * 0.05 * nrand(13.0, time), v)).b;
	FragColor=vec4(r,g,b,1.0);
}