#version 330 core
out vec4 FragColor;

struct Material {
    vec3 ambient, diffuse, specular;   
    float shininess, alpha;
}; 

struct Light {
    vec3 position, ambient, diffuse, specular;
    float constant, linear, quadratic;
};

#define NUM_LIGHTS 2

in vec3 FragPos;  
in vec3 Normal;
in vec4 FragPosLightSpaces[NUM_LIGHTS];

uniform float far_plane;
uniform sampler2D shadowMaps[NUM_LIGHTS];
uniform vec3 viewPos;
uniform Material material;
uniform Light lights[NUM_LIGHTS];
uniform bool blinn;
uniform bool shadows; 

float ShadowCalculation(vec4 fragPosLightSpace, int shadowMapId, vec3 norm, vec3 lightDir)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // depth check
    float closestDepth = texture(shadowMaps[shadowMapId], projCoords.xy).r; 
    float currentDepth = projCoords.z;
    float bias = max(0.005 * (1.0 - dot(norm, lightDir)), 0.0005);
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    if(projCoords.z>1.0)
        shadow=0.0;
    return shadow;
}

vec3 CalcPointLight(Light light, vec3 norm, vec3 fragPos, vec3 viewDir, vec4 fragPosLightSpace, int shadowMapId)
{
    // ambient
    vec3 ambient = light.ambient * material.ambient;
  	
    // diffuse 
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);
    
    // specular
    float spec;
    if(blinn)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    }
    else{
        vec3 reflectDir = reflect(-lightDir, norm);  
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }
    
    vec3 specular = light.specular * (spec * material.specular);  

    float distance    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    float shadow = ShadowCalculation(fragPosLightSpace, shadowMapId, norm, lightDir);
    vec3 result = ambient + (1.0-shadow)*attenuation*(diffuse + specular);
    return result;
}

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = vec3(0.0);
    for(int i = 0; i < NUM_LIGHTS; i++)
        result += CalcPointLight(lights[i], norm, FragPos, viewDir, FragPosLightSpaces[i], i); 

    FragColor = vec4(result, material.alpha);
} 