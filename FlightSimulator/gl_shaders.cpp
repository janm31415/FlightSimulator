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
    clipPos.xyz = -clipPos.xyz;
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



std::string get_terrain_material_vertex_shader()
  {
  return std::string(R"(#version 330 core
layout (location = 0) in vec3 vPosition;
uniform mat4 Projection; // columns

void main() 
  {   
  gl_Position = Projection*vec4(vPosition.xyz,1); 
  }
)");
  }

std::string get_terrain_material_fragment_shader()
  {
  return std::string(R"(#version 330 core
uniform mat4 Camera;
uniform vec3 iResolution;
uniform sampler2D Heightmap;
uniform sampler2D Normalmap;
uniform sampler2D Colormap;

out vec4 FragColor;

vec2 scalePosition(in vec2 p)
{
  p = p*0.005;
  p = p+vec2(0.5);
  //p = mix(vec2(0.5, 0.0), vec2(1.0, 0.5), p);
  return p;
}

float terrain( in vec2 p)
{
   p = scalePosition(p);
   if (p.x < 0.0 || p.x >= 1.0 || p.y < 0.0 || p.y >= 1.0)
     return 0.0;   
   return texture( Heightmap, p).x*5;   
}

float map( in vec3 p )
{
    return p.y - terrain(p.xz);
}

vec4 getColor( in vec3 pos )
{
  vec2 p = scalePosition(pos.xz);
  if (p.x < 0.0 || p.x > 1.0 || p.y < 0.0 || p.y > 1.0)
    return vec4(0,0,0,0);
  return texture( Colormap, p);
}

vec3 calcNormal( in vec3 pos, float t )
{
#if 1
  vec2 p = scalePosition(pos.xz);
  if (p.x < 0.0 || p.x > 1.0 || p.y < 0.0 || p.y > 1.0)
    return vec3(0,1,0);
  return texture( Normalmap, p).rgb;
#else
	  //float e = 0.001;
	  float e = 0.001*t;
    vec3 eps = vec3(e,0.0,0.0);
    vec3 nor;
    nor.x = map(pos+eps.xyy) - map(pos-eps.xyy);
    nor.y = map(pos+eps.yxy) - map(pos-eps.yxy);
    nor.z = map(pos+eps.yyx) - map(pos-eps.yyx);
    return -normalize(nor);
#endif
}

float intersect( in vec3 ro, in vec3 rd )
{
    const float maxd = 40.0;
    const float precis = 0.001;
    float t = 0.0;
    for( int i=0; i<256; i++ )
    {
        float h = map( ro+rd*t );
        if( abs(h)<precis || t>maxd ) break;
        t += h*0.5;
    }
    return (t>maxd)?-1.0:t;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec2 xy = fragCoord.xy / iResolution.xy;
	vec2 s = (-1.0 + 2.0* xy) * vec2(iResolution.x/iResolution.y, 1.0);
    
  vec3 planeNormal = normalize(vec3(0.0, 1.0, 0.0));
  vec3 planeUp = abs(planeNormal.y) < 0.5
      ? vec3(0.0, 1.0, 0.0)
      : vec3(0.0, 0.0, 1.0);
  vec3 planeSide = normalize(cross(planeNormal, planeUp));
  planeUp = cross(planeSide, planeNormal);	    	
    
  vec3 ro = (Camera*vec4(0,0,0,1)).xyz;
  //ro.y += 3;
  vec3 rx = (Camera*vec4(1,0,0,0)).xyz;
  vec3 ry = (Camera*vec4(0,1,0,0)).xyz;
  vec3 rz = (Camera*vec4(0,0,1,0)).xyz;
  vec3 rd = normalize( s.x*rx + s.y*ry + 2.0*rz );

  // transform ray
  //ro = vec3(
  //    dot(ro, planeSide),
  //    dot(ro, planeNormal),
  //    dot(ro, planeUp)
  //);
  //rd = vec3(
  //    dot(rd, planeSide),
  //    dot(rd, planeNormal),
  //    dot(rd, planeUp)
  //);
    
    
  float t = intersect(ro, rd);
    
  if (t > 0.0)
    {	
		// Get some information about our intersection
		vec3 pos = ro + t * rd;
		vec3 normal = calcNormal(pos, t);       	

    vec4 texCol = getColor(pos);
    if (texCol.a > 0)
      {		
		  vec3 col = vec3(pow(texCol.rgb, vec3(0.5)));
		
      vec3 sunDir = normalize(vec3(0, +0.5, -1));

      col = col * clamp(-dot(normal, sunDir), 0.0f, 1.0f) * 0.9 + col*0.1;
      fragColor = vec4(pow(col*1.2, vec3(2.2)), texCol.a);
      }
    else
      {
      fragColor = vec4(0);
      }
	  }	
  else
    {
    fragColor = vec4(0);
    }
}

void main() 
  {
  mainImage(FragColor, gl_FragCoord.xy);
  }
)");
  }


std::string get_blit_material_vertex_shader()
  {
  return std::string(R"(#version 330 core
layout (location = 0) in vec3 vPosition;
//layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;
uniform mat4 Projection; // columns
uniform mat4 Camera; // columns
out vec2 TexCoord;

void main() 
  {
  gl_Position = Projection*Camera*vec4(vPosition.xyz,1);
  TexCoord = vTexCoord;
  }
)");
  }

std::string get_blit_material_fragment_shader()
  {
  return std::string(R"(#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D Tex0;
uniform sampler2D Tex1;

void main()
  {
  vec4 clr0 = texture(Tex0, TexCoord);
  vec4 clr1 = texture(Tex1, TexCoord);
  
  vec3 clr = clr0.rgb * clr0.a + clr1.rgb * (1-clr0.a);

  FragColor = vec4(clr,1);
  }
)");
  }
