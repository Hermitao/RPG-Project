#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;    
    sampler2D emission;
    float shininess;
}; 

struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

struct SpotLight {
    bool on;

    vec3 position;  
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	
    float constant;
    float linear;
    float quadratic;
};

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;
  
uniform vec3 viewPos;

uniform Material material;
uniform DirLight dirLight;
#define NR_POINT_LIGHTS 1
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

uniform float time;

float near = 0.1;
float far = 50.0;

// ---- forward declaration ----

float LinearizeDepth(float depth);

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);  
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

float diffuseShading(vec3 normal, vec3 lightDir);
float specularShading(vec3 lightDir, vec3 normal, vec3 viewDir);

vec3 combineResults(DirLight light, float diff, float spec);
vec3 combineResults(PointLight light, vec3 fragPos, float diff, float spec);
vec3 combineResults(SpotLight light, vec3 fragPos, float diff, float spec);

// -----------------------------

vec3 reflectDir;
float spec;
//float diff;

void main()
{
    // properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // phase 1: Directional lighting
    vec3 result = calcDirLight(dirLight, norm, viewDir);
    // phase 2: Point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += calcPointLight(pointLights[i], norm, FragPos, viewDir);    
    // phase 3: Spot light
    if (spotLight.on)
        result += calcSpotLight(spotLight, norm, FragPos, viewDir);    
    
   // FragColor = vec4(result, 1.0);
    //FragColor = vec4(result, texture(material.diffuse, TexCoords).w);
    vec4 texColor = texture(material.diffuse, TexCoords);
    FragColor = vec4(result, texColor.w);

//    float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
//    FragColor = vec4(vec3(depth), 1.0);
}

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to normalized device coordinates (NDC)
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

// Returns clamped cosine of angle between polygon normal and
// light direction, used to determine diffuse light intensity.
float diffuseShading(vec3 normal, vec3 lightDir)
{
    float diff = max(dot(normal, lightDir), 0.0);   
    return diff;     
}

// Returns specular lighting multiplier, also see diffuseShading().
float specularShading(vec3 lightDir, vec3 normal, vec3 viewDir)
{
    reflectDir = reflect(-lightDir, normal);
    spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    return spec;
}

float lightAttenuation(PointLight light, float distance)
{
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));  

    return attenuation;
}

float lightAttenuation(SpotLight light, float distance)
{
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));  

    return attenuation;
}

// ----------------------------------------------------------------
// color combining function and overloads -------------------------
// ----------------------------------------------------------------

vec3 combineResults(DirLight light, float diff, float spec)                                     // directional lights
{
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords)) * light.diffuse;

    return (ambient + diffuse + specular);
}

vec3 combineResults(PointLight light, vec3 fragPos, float diff, float spec)                     // point lights
{
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords)) * light.diffuse;

    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = lightAttenuation (light, distance);

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

vec3 combineResults(SpotLight light, vec3 lightDir, vec3 fragPos, float diff, float spec)       // spot lights
{
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords)) * light.diffuse;
    vec3 emission = light.diffuse * diff * vec3(texture(material.emission, TexCoords));

    // spotlight (soft edges)
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = (light.cutOff - light.outerCutOff);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    diffuse  *= intensity;
    specular *= intensity;
    emission *= intensity * (sin(time * 1.0) * 0.3f + 0.6f);    // oscillate intensity

    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = lightAttenuation(light, distance);

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular/* + emission*/);
}

// ----------------------------------------------------------------
// ----------------------------------------------------------------

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = diffuseShading(normal, lightDir);
    // specular shading
    float spec = specularShading(lightDir, normal, viewDir);
    // combine results
    return combineResults(light, diff, spec);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = diffuseShading(normal, lightDir);
    // specular shading
    float spec = specularShading(lightDir, normal, viewDir);
    // combine results
    return combineResults(light, fragPos, diff, spec);
}

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    
    float diff = diffuseShading(normal, lightDir);

    float spec = specularShading(lightDir, normal, viewDir);

    return combineResults(light, lightDir, fragPos, diff, spec);
} 