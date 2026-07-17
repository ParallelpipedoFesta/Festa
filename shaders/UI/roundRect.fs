#version 330 core
in vec2 TexCoords;

uniform vec2 rectSize = vec2(100.0);
uniform vec4 fillColor = vec4(1.0, 1.0, 1.0, 1.0);
uniform vec4 borderColor = vec4(1.0, 1.0, 0.0, 1.0);
uniform float borderThickness = 10.0; 
uniform float radius = 30.0; 

uniform sampler2D textureMap;
uniform vec2 textureStride = vec2(0.0);
uniform vec2 textureOff = vec2(0.0);

out vec4 FragColor;

float RectSDF(vec2 p, vec2 b, float r){ // signed distance field
    vec2 d = abs(p) - b + vec2(r);
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - r;   
}

void main() {
    vec4 innerColor = textureOff.x<0.0f ? fillColor : texture(textureMap, textureOff + textureStride * TexCoords);
    vec2 pos = rectSize * TexCoords;
    float fDist = RectSDF(pos - rectSize/2.0, rectSize/2.0 - borderThickness/2.0 - 1.0, radius);
    float fBlendAmount = smoothstep(-1.0, 1.0, abs(fDist) - borderThickness / 2.0);
    
    vec4 FromColor = borderColor;
    vec4 ToColor = (fDist < 0.0) ? innerColor : vec4(0.0);
    
    //FragColor = vec4(vec3(mix(borderColor, ToColor, fBlendAmount)), ToColor.w);
    FragColor = vec4(mix(borderColor, ToColor, fBlendAmount));
}