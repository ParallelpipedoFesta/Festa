#version 440 core
uniform sampler2D tex;
uniform float time;
in vec2 coord;
out vec4 FragColor;

float nrand(float x, float y){
  return (sin(dot(vec2(x, y), vec2(12.9898, 78.233))) * 43758.5453);
}




void main()
{
    float sx=fract(nrand(time*10000.0,nrand(coord.x,coord.y)))*0.001;
    vec2 offset = vec2(1.0 / 256 +sx, 1.0/256);  //+fract(nrand(time*10.0,coord.y))*0.02
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
    
    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(tex, coord + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];
    
    //FragColor = vec4(col, 1.0);
    //col/=10.0f;
    
    col=pow(col,vec3(0.5));
    float x=dot(col,vec3(1.0));
    col=clamp(col,vec3(0.0),vec3(1.0));

    float u=3.0;
    //float factor=1-u+u*x;
    float factor=1.0/(1.0+exp(-x));
    
    factor=min(factor,1.0);
    //float m=pow(max(factor,0.0),20);
    float m=pow(x,2);

    vec3 c1=vec3(1.0f, 0.77f, 0.98f);
    vec3 c2=vec3(1.0,1.0,1.0);

    col=col*(1.0-m)+((c1+(c2-c1)*factor))*m;
    FragColor = vec4(col, 1.0);
}  