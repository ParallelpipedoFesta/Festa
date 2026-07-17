#version 440 core
uniform sampler2D tex;
uniform vec3 solePos;
uniform float size;
in vec2 coord;
out vec4 FragColor;

float nrand(float x, float y){
  return (sin(dot(vec2(x, y), vec2(12.9898, 78.233))) * 43758.5453);
}


vec3 interpolate(vec3 a, vec3 b, float t){
  return a+(b-a)*t;
}

void main(){
    float radius=10.0f;
    vec3 dir=normalize(vec3(gl_FragCoord.x/size*2.0-1.0,gl_FragCoord.y/size*2.0-1.0, -1.0f));//zNear
    float x=dot(dir,normalize(solePos));

    float u=70.0;
    float factor=1-u+u*x;
    
    factor=min(factor,1.0);
    float m=pow(x,20);
    

    vec3 col=vec3(texture(tex,coord))*(1.0-m)+interpolate(vec3(1.0, 0.77, 0.98),vec3(1.0,1.0,1.0),factor)*m;
    FragColor=vec4(col,1.0f);
    return;
    float v=dot(col,vec3(1.0))*max(x,0.0);
    v=pow(v, 0.5);
    v=min(max(v,0.0), 1.0);
    col=interpolate(vec3(0.1,0.54,0.55), col, v);
    FragColor=vec4(col,1.0f);
}