

struct Material
{
    float4  ambient;
    float4  diffuse;
    float4  specular;   //specular中第4个元素代表材质的表面光滑程度
};

struct DirLight
{
    float4  ambient;
    float4  diffuse;
    float4  specular;

    float3  dir;
    float   unused;
};


struct PointLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;

    float3 position;
    float range;

    float3 att; //attenuation (a0, a1, a2)
    float pad;
};

struct SpotLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;

    float3 position;
    float range;

    float3 direction;
    float spot;

    float3 att;
    float pad;
};


void ComputeDirLight(Material material, DirLight dirLight, float3 normal, float3 viewDirection,
    out float4 ambient, out float4 diffuse, out float4 specular)
{

    ambient = float4(0.0f, 0.0f, 0.f, 0.f);
    diffuse = float4(0.f, 0.f, 0.f, 0.f);
    specular = float4(0.f, 0.f, 0.f, 0.f);

    ambient = material.ambient * dirLight.ambient;

    float3 lightDir = -dirLight.dir;

    float lightIntensity = saturate(dot(normal, lightDir));

    if (lightIntensity > 0)
    {
        diffuse = saturate(material.diffuse * dirLight.diffuse * lightIntensity);

        float3 reflection = reflect(dirLight.dir, normal);

        float specFactor = pow(max(saturate(dot(reflection, viewDirection)), 0.f), material.specular.w);

        specular = material.specular * dirLight.specular * specFactor;
    }
}

void ComputePointLight(Material material, PointLight pointLight, float3 pos, float3 normal, float3 viewDirection,
    out float4 ambient, out float4 diffuse, out float4 specular)
{
    ambient = float4(0.0f, 0.0f, 0.f, 0.f);
    diffuse = float4(0.f, 0.f, 0.f, 0.f);
    specular = float4(0.f, 0.f, 0.f, 0.f);

    float3 lightDir = pointLight.position - pos;

    float d = length(lightDir);
    if( d > pointLight.range )
        return;

    ambient = material.ambient * pointLight.ambient;
    
     // Normalize the light vector.
    lightDir /= d; 

    float diffuseFactor = saturate(dot(normal, lightDir));

    if (diffuseFactor > 0)
    {
        diffuse = saturate(material.diffuse * pointLight.diffuse * diffuseFactor);

        float3 reflection = reflect(-lightDir, normal);

        float specFactor = pow(max(saturate(dot(reflection, viewDirection)), 0.f), material.specular.w);

        specular = material.specular * pointLight.specular * specFactor;
    }

    float att = 1.0f / dot(pointLight.att, float3(1.0f, d, d*d));
 
    diffuse *= att;
    specular *= att;
}


void ComputeSpotLight(Material material, SpotLight spotLight, float3 pos, float3 normal, float3 viewDirection,
    out float4 ambient, out float4 diffuse, out float4 specular)
{
    ambient = float4(0.0f, 0.0f, 0.f, 0.f);
    diffuse = float4(0.f, 0.f, 0.f, 0.f);
    specular = float4(0.f, 0.f, 0.f, 0.f);

    float3 lightDir = spotLight.position - pos;

    float d = length(lightDir);
    if (d > spotLight.range)
        return;

    ambient = material.ambient * spotLight.ambient;
    
     // Normalize the light vector.
    lightDir /= d;

    float diffuseFactor = saturate(dot(normal, lightDir));

    if (diffuseFactor > 0)
    {
        diffuse = saturate(material.diffuse * spotLight.diffuse * diffuseFactor);

        float3 reflection = reflect(-lightDir, normal);

        float specFactor = pow(max(saturate(dot(reflection, viewDirection)), 0.f), material.specular.w);

        specular = material.specular * spotLight.specular * specFactor;
    }

    float spot = pow(max(dot(-lightDir, spotLight.direction), 0.0f), spotLight.spot);

    float att = 1.0f / dot(spotLight.att, float3(1.0f, d, d * d));
    att *= spot;
 
    ambient *= spot;

    diffuse *= att;
    specular *= att;
}
