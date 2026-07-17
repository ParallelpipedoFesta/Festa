#version 440 core
uniform sampler2D tex;
uniform vec3 solePos;
uniform vec2 winsize;
uniform float time;
in vec2 coord;
out vec4 FragColor;

float nrand(float x, float y){
  return (sin(dot(vec2(x, y), vec2(12.9898, 78.233))) * 43758.5453);
}

float u=coord.x;
float v=coord.y;



void main(){
    float radius=10.0f;
    vec3 dir=normalize(vec3(gl_FragCoord.x/winsize.x*2.0-1.0,gl_FragCoord.y/winsize.x*2.0-1.0, -1.0f));//zNear
    float x=dot(dir,normalize(solePos));

    float u=70.0;
    float factor=1-u+u*x;
    
    factor=min(factor,1.0);
    float m=pow(x,20);//+fract(nrand(time,factor))*0.01/x
    

    vec3 c1=vec3(1.0f, 0.77f, 0.98f);
    vec3 c2=vec3(1.0,1.0,1.0);
    vec3 col=vec3(texture(tex,coord))*(1.0f-m)+((c1+(c2-c1)*factor))*m;
    FragColor=vec4(col,1.0f);
}