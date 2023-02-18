#include "gl_shaders.h"


std::string get_simple_material_vertex_shader()
  {
  return std::string(R"(#version 330 core
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;
uniform mat4 Projection; // columns
uniform mat4 Camera; // columns

out vec3 Normal;
out vec2 TexCoord;

void main() 
  {
  gl_Position = Projection*Camera*vec4(vPosition.xyz,1);
  Normal = (Camera*vec4(vNormal,0)).xyz;
  TexCoord = vTexCoord;
  }
)");
  }

std::string get_simple_material_fragment_shader()
  {
  return std::string(R"(#version 330 core
out vec4 FragColor;
  
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D Tex0;
uniform vec3 LightDir;
uniform vec4 Color;
uniform int TextureSample;
uniform float Ambient;

void main()
  {
  float l = clamp(dot(Normal,LightDir), 0, 1.0 - Ambient) + Ambient;
  vec4 clr = (texture(Tex0, TexCoord)*TextureSample + Color*(1-TextureSample))*l;
  FragColor = clr;
  }
)");
  }



std::string get_cubemap_material_vertex_shader()
  {
  return std::string(R"(#version 330 core
layout (location = 0) in vec3 vPosition;

uniform mat4 Projection; // columns
uniform mat4 Camera; // columns

out vec3 localPos;

void main()
{
    localPos = vPosition;
    mat4 rotView = mat4(mat3(Camera)); // remove translation from the view matrix
    vec4 clipPos = Projection * rotView * vec4(localPos, 1.0);

    gl_Position = clipPos;
}
)");
  }

std::string get_cubemap_material_fragment_shader()
  {
  return std::string(R"(#version 330 core
out vec4 FragColor;

in vec3 localPos;
  
uniform samplerCube environmentMap;
  
void main()
{
    vec3 envColor = texture(environmentMap, localPos).rgb;  
    FragColor = vec4(envColor, 1.0);
}
)");
  }
