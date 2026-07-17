#version 330 core

uniform sampler2D textureMap;
uniform mat4 color = mat4(1.0);
in vec2 TexCoords;
out vec4 FragColor;

void main(){
    FragColor = color * texture(textureMap, TexCoords);
    //FragColor = vec4(1.0);
}