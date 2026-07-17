#version 440 core

uniform sampler1D map;
uniform float time;
uniform vec3 cameraPos;
uniform vec3 position;
uniform vec2 size;
uniform vec3 lightPos;
uniform float transparency;

in vec2 TexCoords;
in vec3 Pos;
in float zNear0;
in float zFar0;
out vec4 FragColor;

bool inCylinder(vec3 p){
    return 0.0<=p.y&&p.y<=size.y&&p.x*p.x+p.z*p.z<=size.x*size.x;
}

float intersect(vec3 p, vec3 d){
    float t0 = -p.y/d.y;
    float t1 = (size.y-p.y)/d.y;
    float tmin = max(0.0, min(t0, t1));
    float tmax = max(t0, t1);
    if(tmin>tmax)return -1.0;
    vec2 P = vec2(p.x, p.z);
    vec2 D = vec2(d.x, d.z);
    float a = dot(D, D);
    float b = 2.0*dot(P, D);
    float c = dot(P, P) - size.x*size.x;
    float delta = b*b-4.0*a*c;
    if(delta<0.0)return -1.0;
    delta = sqrt(delta);
    tmin = max(tmin, (-b-delta)/(2.0*a));
    tmax = min(tmax, (-b+delta)/(2.0*a));
    if(tmin>tmax)return -1.0;
    return tmin;
}


vec4 CalcDensity(vec3 world){
    if(!inCylinder(world-position))return vec4(0.0);
    return texture(map, (world.y-position.y)/size.y);
}

vec3 CalcDensityAlongRay(vec3 p, vec3 d, int numSteps, float step){
    vec3 density = vec3(0.0);
    for(int i=0;i<numSteps;i++){
        density += CalcDensity(p+d*i*step).xyz;
    }
    return density;
}

float CalcDepth(float dist){
    float z_view = -dist;
    float z_clip = -(zFar0+zNear0)/(zFar0-zNear0)*z_view - 2.0*zFar0*zNear0/(zFar0-zNear0);
    float z_ndc = z_clip/dist;
    return z_ndc*0.5+0.5;
}

float randomNoise(vec2 seed){
    return fract(sin(dot(seed, vec2(17.13, 3.71))*time) * 43758.5453);
}

vec2 f(vec2 x){
    vec2 t = vec2(1.0)/(1.0-exp(fract(x)));
    return floor(x)+t;//+pow(fract(x), vec2(6.0));
}

void main(){
    int numSteps = 20;
    float step = 0.2;

    vec3 dir = normalize(Pos - cameraPos);

    float t = intersect(cameraPos-position, dir);
    if(t < 0.0)discard;

    float depth = -1.0;
    vec3 densityAlongView = vec3(0.0);
    vec3 totalLight = vec3(0.0);
    //t+=2.0*randomNoise(floor(TexCoords*1.0));
    //dir+=vec3(0.0, 0.01*randomNoise(f(TexCoords*100.0)), 0.0);
    float noise = randomNoise(f(TexCoords*1000.0));
    float alpha = 0.0;
    for(int i=0;i<numSteps;i++){
        float d = i*step+t;
        vec3 pos = cameraPos+dir*d;
        pos.y += noise*min(abs(pos.y-position.y), abs(pos.y-position.y - size.y));
        //if(!inCylinder(pos-position))continue;
        vec4 color = CalcDensity(pos);
        vec3 density = color.xyz;
        alpha+=color.w;
        densityAlongView += density;
        vec3 densityAlongLightRay = CalcDensityAlongRay(pos, normalize(lightPos-pos), 8, 0.5);
        vec3 transmitted = exp(-densityAlongLightRay);
        vec3 inScatteredLight = transmitted * density;
        vec3 viewRayTransmittance = exp(-densityAlongView);
        totalLight += inScatteredLight * viewRayTransmittance;
        if(length(density)>0.0 && depth<0.0) depth = d;
    }
    gl_FragDepth = CalcDepth(depth) - 0.001;
    //alpha += (randomNoise(f(TexCoords*200.0))*0.5);
    alpha = pow(alpha/numSteps, transparency);
    FragColor = vec4(totalLight, alpha);
}