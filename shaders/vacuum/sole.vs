#version 440 core
layout(location=0)in vec2 aPos;
out vec3 dir;

uniform mat4 inversedView;

uniform float left;
uniform float right;
uniform float top;
uniform float bottom;
uniform float zNear;
uniform float zFar;

vec3 ndc2view(vec3 v){
    float ze = 2.0f * zFar * zNear / (zFar - zNear) / (v.z - (zFar + zNear) / (zFar - zNear));
    float xe = (-v.x - (right + left) / (right - left)) * ze / (2 * zNear / (right - left));
    float ye = (-v.y - (top + bottom) / (top - bottom)) * ze / (2 * zNear / (top - bottom));
    return vec3(xe, ye, ze);
}


void main(){
    gl_Position = vec4(aPos, 0.0, 1.0);
    vec3 ndc = vec3(aPos, -1.0);
    dir = vec3(inversedView * vec4(ndc2view(ndc), 1.0));
}