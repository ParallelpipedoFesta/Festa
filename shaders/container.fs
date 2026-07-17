#version 330 core
#define eps 0.0001

struct Light {
    vec3 Position;
    vec3 Color;
    float Linear;
    float Quadratic;
    float Constant;
    float Radius;
};

struct DirLight{
    vec3 Direction;
    vec3 Color;
};
const int NR_LIGHTS = 15;
uniform Light lights[NR_LIGHTS];
uniform DirLight dirLights[NR_LIGHTS];

uniform vec3 viewPos;
out vec4 FragColor;

in vec3 N;
in vec3 FragPos;
in vec2 TexCoords;

bool eq(vec3 a, vec3 b){
	return dot(a,b)>0.99;
}

float cartoonize(float x, int numSteps){
    return float(ceil(x*float(numSteps)))/float(numSteps);
}

void main(){
	vec3 Diffuse = vec3(1.0);
	vec3 Specular = vec3(1.0);
    float shininess = 16.0;
	vec3 Normal = normalize(N);
	vec3 viewDir = normalize(viewPos-FragPos);
	vec3 lighting = Diffuse * 0.1;

    int numSteps = 10;
	
    for(int i = 0; i < NR_LIGHTS; ++i){
        float distance = length(lights[i].Position - FragPos);
        if(distance<lights[i].Radius){
            vec3 lightDir = normalize(lights[i].Position - FragPos);
            float diff = max(dot(Normal, lightDir), 0.0);
            //diff = cartoonize(diff, numSteps);
            vec3 diffuse = diff * Diffuse * lights[i].Color;
            // specular
            vec3 halfwayDir = normalize(lightDir + viewDir);  
            float spec = pow(max(dot(Normal, halfwayDir), 0.0), shininess);
            //spec = cartoonize(spec, numSteps);
            vec3 specular = spec * Specular * lights[i].Color;
            float attenuation = 1.0 / (lights[i].Constant + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);
            lighting += attenuation*(diffuse + specular);
        }
    }
    float alpha = length(lighting);
    //alpha = alpha*alpha;
    alpha = max(pow(alpha, 4.0), 0.2);
    FragColor = vec4(lighting, alpha);
    
}