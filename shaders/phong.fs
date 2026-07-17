#version 330 core

struct DirLight{
	vec3 dir;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight{
	vec3 pos;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float constant;
	float linear;
	float quadratic;
};

struct SpotLight{
	vec3 pos;
	vec3 dir;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float cutOff;
	float outerCutOff;
};

struct Material{
	sampler2D diffuseMap;
	sampler2D specularMap;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

#define N_POINT_LIGHTS 10
#define N_DIR_LIGHTS 10
#define N_SPOT_LIGHTS 10
#define eps 0.0001f
uniform DirLight dirLights[N_DIR_LIGHTS];
uniform PointLight pointLights[N_POINT_LIGHTS];
uniform SpotLight spotLights[N_SPOT_LIGHTS];

uniform Material material;
uniform vec3 viewPos;
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 texcoord;

bool eq(vec3 a,vec3 b){
	return dot(a,b)>0.99;
}

vec3 calcDirLight(DirLight light,vec3 normal,vec3 viewDir,vec3 ambient,vec3 diffuse,vec3 specular){
	ambient*=light.ambient;
	vec3 lightDir=normalize(-light.dir);
	diffuse*=light.diffuse*max(dot(normal,lightDir),0.0);
	float spec=max(dot(viewDir,reflect(-lightDir,normal)),0.0f);
	specular*=light.specular*pow(spec,material.shininess);
	return ambient+diffuse+specular;
}
vec3 calcPointLight(PointLight light,vec3 normal,vec3 viewDir,vec3 ambient,vec3 diffuse,vec3 specular){
	vec3 lightDir=normalize(light.pos-FragPos);
	vec3 reflectDir=reflect(-lightDir,normal);
	ambient*=light.ambient;
	diffuse*=max(dot(normal,lightDir),0.0)*light.diffuse;
	specular*=pow(max(dot(viewDir,reflectDir),0.0f),material.shininess)*light.specular;
	//specular=vec3(0.0f);
	if(light.constant<eps&&light.linear<eps&&light.quadratic<eps)return ambient+diffuse+specular;
	float dist=length(light.pos-FragPos);
	float attenution=1.0f/(light.constant+light.linear*dist+light.quadratic*dist*dist);
	return attenution*(ambient+diffuse+specular);
}

vec3 calcSpotLight(SpotLight light,vec3 normal,vec3 viewDir,vec3 ambient,vec3 diffuse,vec3 specular){
	ambient*=light.ambient;
	vec3 lightDir=normalize(light.pos-FragPos);
	float theta=dot(normalize(-light.dir),lightDir);
	if(theta<light.cutOff)return ambient;
	diffuse*=light.diffuse*max(dot(normal,lightDir),0.0);
	float spec=max(dot(viewDir,reflect(-lightDir,normal)),0.0f);
	specular*=light.specular*pow(spec,material.shininess);
	if(light.outerCutOff>1.0f)return ambient+diffuse+specular;
	float epsilon   = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0); 
	return ambient+intensity*(diffuse+specular);
}

void main(){
	float alpha=1.0f;
	vec3 ambient=material.ambient,diffuse=material.diffuse,specular=material.specular;
	if(eq(diffuse,vec3(-1.0f))){
		vec4 temp=texture(material.diffuseMap,texcoord);
		alpha=temp.w;
		ambient=temp.xyz;
		diffuse=ambient;
	}
	if(eq(specular,vec3(-1.0f)))specular=texture(material.specularMap,texcoord).xyz;
	vec3 normal=normalize(Normal);
	vec3 viewDir=normalize(viewPos-FragPos);
	vec3 result=vec3(0.0f);
	for(int i=0;i<N_DIR_LIGHTS;i++)
		result+=calcDirLight(dirLights[i],normal,viewDir,ambient,diffuse,specular);
	for(int i=0;i<N_POINT_LIGHTS;i++)
		result+=calcPointLight(pointLights[i],normal,viewDir,ambient,diffuse,specular);
	for(int i=0;i<N_SPOT_LIGHTS;i++)
		result+=calcSpotLight(spotLights[i],normal,viewDir,ambient,diffuse,specular);
	FragColor=vec4(result,alpha);
}