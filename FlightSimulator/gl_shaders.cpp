#include "gl_shaders.h"

std::string get_basic_fragment_shader()
  {
  return std::string(
  R"(#version 330 core
out vec4 FragColor;

uniform vec3 u_SolidObjectColor;

void main()
{
	FragColor = vec4(u_SolidObjectColor.rgb, 1.0f);
})"
  );
  }

std::string get_basic_vertex_shader()
  {
  return std::string(
  R"(#version 330 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec2 a_Normal;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
	gl_Position = u_Projection * u_View * u_Model * vec4(a_Pos, 1.0f);
})"
  );
  }

std::string get_heightmap_fragment_shader()
  {
  return std::string(
    R"(#version 330 core

out vec4 FragColor;

uniform float u_Level;
uniform sampler2D u_Heightmap;
uniform sampler2D u_Normalmap;
uniform sampler2D u_Texture;

in vec3 Color;
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;
flat in int Factor;

vec3 calculateDirLight(vec3 direction, vec3 normal, vec3 color)
{
	float ka = 0.6;
	float kd = 1.0f;

	vec3 light_color = vec3(1.0, 1.0, 1.0);

    // ambient
    vec3 ambient = ka * light_color;
  	
    // diffuse 
    vec3 diffuse = max(dot(normalize(normal), normalize(direction)), 0.0) * light_color;

    return (ambient + diffuse) * color;
}


void main()
{
	if (gl_FrontFacing)
	{


		vec3 lightDir = vec3(-2.0, 4.0, -1.0);
		FragColor = vec4(calculateDirLight(lightDir, Normal, texture(u_Texture, TexCoord).rgb), 1.0);
		//FragColor = vec4(Normal, 1.0);
		//FragColor = texture(u_Texture, TexCoord);
		//FragColor = vec4(Color, 1.0); 
	}
	else
	{
		//FragColor = vec4(0.0, 0.0, 1.0, 1.0); // blue
	}
}
)"
  );
  }

std::string get_heightmap_vertex_shader()
  {
  return std::string(
    R"(#version 330 core
layout (location = 0) in vec3 a_Pos;

uniform mat4 u_View;
uniform mat4 u_Model;
uniform mat4 u_Projection;

uniform sampler2D u_Heightmap;
uniform sampler2D u_Normalmap;

uniform vec3 u_CameraPos;
uniform float u_Scale;
uniform float u_SegmentSize;
uniform float u_Level;

out vec3 Color;
out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoord;

out int Factor;

float scale(float input_val, float in_min, float in_max, float out_min, float out_max)
{
    return (input_val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float getHeight(vec2 uv)
{
    float height = texture(u_Heightmap, uv).r;
    float coarser = textureLod(u_Heightmap, uv, 1).r;

    float scale = 3000;
    float shift = 0;
    return scale * height + shift;
}

vec3 getNormal(vec2 uv)
{
    return normalize(texture(u_Normalmap, uv).rgb);
}

vec2 getUV(vec2 pos)
{
    vec2 coord = pos / Factor;
    coord.x = scale(coord.x, -1.0, 1.0, 0.0, 1.0);
    coord.y = scale(coord.y, -1.0, 1.0, 0.0, 1.0);
    return coord;
}

void main()
{
    Factor = 25000;

    FragPos = vec3(u_Model * vec4(a_Pos, 1.0));
    TexCoord = getUV(FragPos.xz);

    FragPos.y = getHeight(TexCoord);

    Normal = getNormal(TexCoord);

	Color = vec3(1.0, u_Level, 0.0);

    gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);

    float distanceFromCam = clamp(gl_Position.w, -0.1, 1000);
    float positionResolution = 128;
	gl_Position.xy = round(gl_Position.xy * (positionResolution / distanceFromCam)) / (positionResolution / distanceFromCam);

}




)"
  );
  }

std::string get_depth_fragment_shader()
  {
  return std::string(
    R"(#version 330 core
void main() {}
)"
  );
  }

std::string get_depth_vertex_shader()
  {
  return std::string(
    R"(#version 330 core
layout(location = 0) in vec3 a_Pos;

uniform mat4 u_Model;
uniform mat4 u_LightSpaceMatrix;

void main()
{
	gl_Position = u_LightSpaceMatrix * u_Model * vec4(a_Pos, 1.0);
}
)"
  );
  }

std::string get_phong_fragment_shader()
  {
  return std::string(
    R"(#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoords;
in vec4 FragPosLightSpace;
  
uniform vec3 u_CameraPosition; 

// phong lighting parameters
uniform float ka;
uniform float kd;
uniform float ks;
uniform float alpha;

uniform bool u_ReceiveShadow;
uniform sampler2D u_ShadowMap;

uniform bool u_UseTexture;
uniform sampler2D u_Texture1;

uniform vec3 u_SolidObjectColor;
uniform vec3 u_BackgroundColor;

struct Light {
	int type; // POINT = 0, DIRECTIONAL = 1
	vec3 color;
	vec3 position;  
};

#define MAX_LIGHTS 4
uniform int u_NumLights;
uniform Light u_Lights[MAX_LIGHTS];

vec3 getColor()
{
	return u_UseTexture ? vec3(texture(u_Texture1, TexCoords)) : u_SolidObjectColor;

}

float calculateAttenuation(float constant, float linear, float quadratic, float distance)
{
	return 1.0 / (constant + linear * distance + quadratic * (distance * distance));
}

float calculateShadow(vec4 fragPosLightSpace)
{
    vec3 projectionCoords = (fragPosLightSpace.xyz / fragPosLightSpace.w) * 0.5 + 0.5;
    float closestDepth = texture(u_ShadowMap, projectionCoords.xy).r; 
    float currentDepth = projectionCoords.z;
	float bias = 0.005;
    return (currentDepth - bias) > closestDepth ? 1.0 : 0.0;
}

vec3 calculateDirLight(Light light)
{
	vec3 direction = light.position;
	
	vec3 color = getColor();

    // ambient
    vec3 ambient = ka * light.color;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(direction);
    vec3 diffuse = kd * max(dot(norm, lightDir), 0.0) * light.color;
    
    // specular
    vec3 u_ViewDir = normalize(u_CameraPosition - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    vec3 specular = ks * pow(max(dot(u_ViewDir, reflectDir), 0.0), alpha) * light.color;

	float shadow = 0.0;
	if (u_ReceiveShadow)
	{
		shadow = calculateShadow(FragPosLightSpace);       
	}

    return (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
}

vec3 calculatePointLight(Light light)
{
	vec3 result;
	vec3 position = light.position;

	vec3 color = getColor();

    // ambient
    vec3 ambient = ka * light.color;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(position - FragPos);
    vec3 diffuse = kd * max(dot(norm, lightDir), 0.0) * light.color;
    
    // specular
    vec3 u_ViewDir = normalize(u_CameraPosition - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    vec3 specular = ks * pow(max(dot(u_ViewDir, reflectDir), 0.0), alpha) * light.color;

	// attenuation
	float constant		= 1.0;
	float linear		= 0.09;
	float quadratic		= 0.032;
	float distance		= length(position - FragPos);
	float attenuation	= calculateAttenuation(constant, linear, quadratic, distance);  
	
    result += (ambient + diffuse + specular) * attenuation * color;

#if 0
	// https://ijdykeman.github.io/graphics/simple_fog_shader
	vec3 cameraDir = u_CameraPosition - FragPos;
	vec3 cameraDir = -u_ViewDir;
	float b = length(light.position - u_CameraPosition);

	float h = length(cross(light.position - u_CameraPosition, cameraDir)) / length(cameraDir);
	float dropoff = 1.0;
	float fog = (atan(b / h) / (h * dropoff));

	float density = 0.1;

	// TODO: improve this
	float theta = dot(norm, u_ViewDir) >= 0 ? 1 : 0;

	vec3 scattered = light.color * (fog * density) * theta;
	scattered *= calculateAttenuation(constant, linear, quadratic, length(cameraDir));
	
	result += scattered;
#endif

	return result;
}

void main()
{
    //float depthValue = texture(u_ShadowMap, TexCoords).r;
    //FragColor = vec4(vec3(depthValue), 1.0); 
	//return;

	vec3 result;

	for (int i = 0; i < u_NumLights; i++)
	{
		switch (u_Lights[i].type) {
			case 0:
				result += calculatePointLight(u_Lights[i]);
				break;
			
			case 1:
				result += calculateDirLight(u_Lights[i]);
				break;
		}
	}

#if 0	
	// fog

	float tmp = dot(vec3(0,1,0), u_CameraPosition - FragPos);

	vec4 fogColor = vec4(u_BackgroundColor, 1.0);
	float fogMin = 4.1;
	float fogMax = 100.0;
	float dist = length(u_CameraPosition - FragPos);
	float fogFactor = (fogMax - dist) / (fogMax - fogMin);

	fogFactor = clamp(fogFactor, 0.0, 1.0);

    FragColor = mix(fogColor * tmp, vec4(result, 1.0), fogFactor);
#else
	FragColor  = vec4(result, 1.0);
#endif
})"
  );
  }

std::string get_phong_vertex_shader()
  {
  return std::string(
    R"(#version 330 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec4 FragPosLightSpace;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform mat4 u_LightSpaceMatrix;

void main()
{

    FragPos = vec3(u_Model * vec4(a_Pos, 1.0));
    gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);

	//apply nostalgic vertex jitter
    float distanceFromCam = clamp(gl_Position.w, -0.1, 1000);
    float positionResolution = 128;
	gl_Position.xy = round(gl_Position.xy * (positionResolution / distanceFromCam)) / (positionResolution / distanceFromCam);


    TexCoords = a_TexCoord;
    Normal = mat3(transpose(inverse(u_Model))) * a_Normal;  
    FragPosLightSpace = u_LightSpaceMatrix * vec4(FragPos, 1.0);
})"
  );
  }

std::string get_screen_fragment_shader()
  {
  return std::string(
    R"(#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_ShadowMap;

void main()
{             
    float depthValue = texture(u_ShadowMap, TexCoords).r;
    FragColor = vec4(vec3(depthValue), 1.0); 
})"
  );
  }

std::string get_screen_vertex_shader()
  {
  return std::string(
    R"(#version 330 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec2 a_TexCoords;

out vec2 TexCoords;

void main()
{
	TexCoords = a_TexCoords;
	gl_Position = vec4(a_Pos, 1.0);
})"
  );
  }

std::string get_skybox_fragment_shader()
  {
  return std::string(
    R"(#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube u_Skybox;

void main()
{    
    FragColor = texture(u_Skybox, TexCoords);
})"
  );
  }

std::string get_skybox_vertex_shader()
  {
  return std::string(
    R"(#version 330 core
layout (location = 0) in vec3 a_Pos;

out vec3 TexCoords;

uniform mat4 u_Projection;
uniform mat4 u_View;

void main()
{
    TexCoords = a_Pos;
    gl_Position = u_Projection * u_View * vec4(a_Pos, 1.0);
}  )"
  );
  }