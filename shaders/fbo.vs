#version 440 core
layout(location=0)in vec2 Pos;
out vec2 coord;
void main(){
   gl_Position=vec4(Pos,0.0f,1.0f);
   coord=vec2(Pos.x*0.5f+0.5f, Pos.y*0.5f+0.5f);
}