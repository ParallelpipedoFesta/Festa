#version 440 core
uniform sampler2D frame;
uniform vec2 windowSize;
in vec2 TexCoords;
out vec4 FragColor;



void main(){
    vec2 offset = 1.0/windowSize;
    vec2 offsets[9] = vec2[](
        vec2(-offset.x,  offset.y), // top-left
        vec2( 0.0f,    offset.y), // top-center
        vec2( offset.x,  offset.y), // top-right
        vec2(-offset.x,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset.x,  0.0f),   // center-right
        vec2(-offset.x, -offset.y), // bottom-left
        vec2( 0.0f,   -offset.y), // bottom-center
        vec2( offset.x, -offset.y)  // bottom-right    
    );

    float kernel[9] = float[](
        1.0 / 16, 2.0 / 16, 1.0 / 16,
        2.0 / 16, 4.0 / 16, 2.0 / 16,
         1.0 / 16, 2.0 / 16, 1.0 / 16  
    );
    
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++){
        //col += vec3(texture(frame, TexCoords + offsets[i]))*kernel[i];
        col += vec3(texture(frame, TexCoords + offsets[i])) /9.0;
    }
    const float exposure = 0.5;
    const float gamma = 2.2;
    vec3 mapped = vec3(1.0) - exp(-col * exposure);
    //mapped = pow(mapped, vec3(1.0 / gamma));

    FragColor = vec4(mapped, 1.0);
}  