#version 440 core
uniform sampler2D tex;
uniform float time;
in vec2 coord;
out vec4 FragColor;

float randomNoise(vec2 seed){
    return fract(sin(dot(seed, vec2(17.13, 3.71))*time) * 43758.5453);
}

float u=coord.x;
float v=coord.y;

float _BlockSize=20.0;
float block = randomNoise(floor(coord*_BlockSize));
float displaceNoise = pow(block, 8.0) * pow(block, 3.0);
float splitRGBNoise = pow(randomNoise(vec2(7.2341)), 17.0);

vec2 offset=vec2((displaceNoise-splitRGBNoise*5.0)*randomNoise(vec2(13.0, 1.0)),
    (displaceNoise-splitRGBNoise*1.0)*randomNoise(vec2(7.0, 1.0)))*0.05;

void main(){
    float r=texture(tex,coord).r;
    float g=texture(tex,coord+offset).g;
    float b=texture(tex,coord-offset).b;
	FragColor=vec4(r,g,b,1.0);
}