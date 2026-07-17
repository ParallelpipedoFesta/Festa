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

vec4 ColorDrift(){
  float _colorDrift=0.55f;
  float jump=1.0f;
  float drift = sin(jump + time * 606.11f) * (_colorDrift * 0.04f);
  vec4 src1 = texture(tex, (vec2(u , v)));
  vec4 src2 = texture(tex, (vec2(u + drift, v)));
  return vec4(src1.r, src2.g, src1.b, 1);
}



void main(){
	FragColor=ColorDrift();
}