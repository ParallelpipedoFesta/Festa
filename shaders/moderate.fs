#version 440 core
uniform sampler2D frame;
in vec2 TexCoords;
out vec4 FragColor;

float cartoonize(float x, int numSteps){
    return float(ceil(x*float(numSteps)))/float(numSteps);
}

void main(){
    vec3 col = vec3(texture(frame, TexCoords));
    int numSteps = 7;
    float factor = max(col.x, max(col.y, col.z))/(col.x+col.y+col.z);
    factor = pow(factor, 0.5);
    FragColor = vec4(col * cartoonize(factor, numSteps), 1.0);
}  