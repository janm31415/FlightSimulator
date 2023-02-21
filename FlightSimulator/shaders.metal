#include <metal_stdlib>
using namespace metal;

struct VertexIn {
  packed_float3 position;
  packed_float3 normal;
  packed_float2 textureCoordinates;
};

struct SimpleMaterialUniforms {
  float4x4 projection_matrix;
  float4x4 camera_matrix;
  float4 color;
  float3 light;
  int texture_sample;
  float ambient;
  int tex0;
};

struct VertexOut {
  float4 position [[position]];
  float3 normal;
  float2 texcoord;
};

vertex VertexOut simple_material_vertex_shader(const device VertexIn *vertices [[buffer(0)]], uint vertexId [[vertex_id]], constant SimpleMaterialUniforms& input [[buffer(10)]]) {
  float4 pos(vertices[vertexId].position, 1);
  VertexOut out;
  out.position = input.projection_matrix * input.camera_matrix * pos;
  out.normal = (input.camera_matrix * float4(vertices[vertexId].normal, 0)).xyz;
  out.texcoord = vertices[vertexId].textureCoordinates;
  return out;
}


fragment float4 simple_material_fragment_shader(const VertexOut vertexIn [[stage_in]], texture2d<float> texture [[texture(0)]], sampler sampler2d [[sampler(0)]], constant SimpleMaterialUniforms& input [[buffer(10)]]) {
  float l = clamp(dot(vertexIn.normal,input.light), 0.0, 1.0 - input.ambient) + input.ambient;
  return (texture.sample(sampler2d, vertexIn.texcoord)*input.texture_sample + input.color*(1-input.texture_sample))*l;
}

struct CubeMaterialUniforms {
  float4x4 projection_matrix;
  float4x4 camera_matrix;
};

struct CubemapVertexOut {
  float4 position [[position]];
  float3 localpos;
};


vertex CubemapVertexOut cubemap_material_vertex_shader(const device VertexIn *vertices [[buffer(0)]], uint vertexId [[vertex_id]], constant CubeMaterialUniforms& input [[buffer(10)]]) {
  CubemapVertexOut out;
  out.localpos = vertices[vertexId].position;
  float4 pos(vertices[vertexId].position, 1);
  float4x4 rotView = input.camera_matrix;
  rotView.columns[3][0] = 0;
  rotView.columns[3][1] = 0;
  rotView.columns[3][2] = 0;
  out.position = input.projection_matrix * rotView * pos;
  out.position.y = -out.position.y;
  out.position.x = -out.position.x;
  return out;
}

fragment float4 cubemap_material_fragment_shader(const CubemapVertexOut vertexIn [[stage_in]], texturecube<float> texture [[texture(0)]], sampler cubesampler [[sampler(0)]], constant CubeMaterialUniforms& input [[buffer(10)]]) {
  return texture.sample(cubesampler, vertexIn.localpos);
}


struct TerrainMaterialUniforms {
  float4x4 projection_matrix;
  float4x4 camera_matrix;
  float3 resolution;
  int heightmap_handle;
  int normalmap_handle;
  int colormap_handle;
};

struct TerrainVertexOut {
  float4 position [[position]];
};

vertex TerrainVertexOut terrain_material_vertex_shader(const device VertexIn *vertices [[buffer(0)]], uint vertexId [[vertex_id]], constant TerrainMaterialUniforms& input [[buffer(10)]]) {
  TerrainVertexOut out;
  float4 pos(vertices[vertexId].position, 1);
  out.position = input.projection_matrix * pos;
  return out;
}


float2 scalePosition(float2 p)
{
#if 0
  p = p*0.02;
  p = p+float2(0.5);
  p = mix(float2(0.5, 0.0), float2(1.0, 0.5), p);
#else
  p = p*0.005;
  p = p+float2(0.5);
#endif
  return p;
}

float terrain( float2 pos, texture2d<float> Heightmap, sampler sampler2d)
{
  float2 p = scalePosition(pos);
  if (p.x < 0.0 || p.x > 1.0 || p.y < 0.0 || p.y > 1.0)
    return 0.0;
  return Heightmap.sample(sampler2d, p).r*5.0;
}

float map( float3 p,  texture2d<float> Heightmap, sampler sampler2d)
{
    return p.y - terrain(p.xz, Heightmap, sampler2d);
}

float intersect( float3 ro, float3 rd, texture2d<float> Heightmap, sampler sampler2d)
{
    const float maxd = 40.0;
    const float precis = 0.001;
    float t = 0.0;
    for( int i=0; i<256; i++ )
    {
        float h = map( ro+rd*t, Heightmap, sampler2d);
        if( abs(h)<precis || t>maxd ) break;
        t += h*0.5;
    }
    return (t>maxd)?-1.0:t;
}

float3 calcNormal( float3 pos, float t, texture2d<float> Normalmap, sampler sampler2d)
{
  float2 p = scalePosition(pos.xz);
  if (p.x < 0.0 || p.x > 1.0 || p.y < 0.0 || p.y > 1.0)
    return float3(0,1,0);
  return Normalmap.sample(sampler2d, p).rgb;
}

float4 getColor(float3 pos, texture2d<float> Colormap, sampler sampler2d)
{
  float2 p = scalePosition(pos.xz);
  if (p.x < 0.0 || p.x > 1.0 || p.y < 0.0 || p.y > 1.0)
    return float4(0,0,0,0);
  return Colormap.sample(sampler2d, p);
}

fragment float4 terrain_material_fragment_shader(const TerrainVertexOut vertexIn [[stage_in]], texture2d<float> heightmap [[texture(0)]], texture2d<float> normalmap [[texture(1)]], texture2d<float> colormap [[texture(2)]], sampler sampler2d [[sampler(0)]], constant TerrainMaterialUniforms& input [[buffer(10)]]) {
  //return colormap.sample(sampler2d, vertexIn.position.xy/input.resolution.xy);
  float2 xy = vertexIn.position.xy / input.resolution.xy;
  xy.y = 1-xy.y;
	float2 s = (-1.0 + 2.0* xy) * float2(input.resolution.x/input.resolution.y, 1.0);
    
  float3 planeNormal = normalize(float3(0.0, 1.0, 0.0));
  float3 planeUp = abs(planeNormal.y) < 0.5
      ? float3(0.0, 1.0, 0.0)
      : float3(0.0, 0.0, 1.0);
  float3 planeSide = normalize(cross(planeNormal, planeUp));
  planeUp = cross(planeSide, planeNormal);
    
  float3 ro = (input.camera_matrix*float4(0,0,0,1)).xyz;
  float3 rx = (input.camera_matrix*float4(1,0,0,0)).xyz;
  float3 ry = (input.camera_matrix*float4(0,1,0,0)).xyz;
  float3 rz = (input.camera_matrix*float4(0,0,1,0)).xyz;
  float3 rd = normalize( s.x*rx + s.y*ry + 2.0*rz );
    
    
  float t = intersect(ro, rd, heightmap, sampler2d);
    
  if(t > 0.0)
    {
		// Get some information about our intersection
		float3 pos = ro + t * rd;
		float3 normal = calcNormal(pos, t, normalmap, sampler2d);
		float4 texCol = getColor(pos, colormap, sampler2d);
    if (texCol.a > 0)
      {
      float3 col = float3(pow(texCol.rgb, float3(0.5)));
		  float3 sunDir = normalize(float3(0, 0.5, -1));
      col = col * clamp(-dot(normal, sunDir), 0.0f, 1.0f) * 0.9 + col * 0.1;
      return float4(pow(col*1.2, float3(2.2)), texCol.a);
      }
	  }
	
	return float4(0.0);
}

struct BlitMaterialUniforms {
  float4x4 projection_matrix;
  float4x4 camera_matrix;
};

struct BlitVertexOut {
  float4 position [[position]];
  float2 texcoord;
};

vertex BlitVertexOut blit_material_vertex_shader(const device VertexIn *vertices [[buffer(0)]], uint vertexId [[vertex_id]], constant BlitMaterialUniforms& input [[buffer(10)]]) {
  float4 pos(vertices[vertexId].position, 1);
  BlitVertexOut out;
  out.position = input.projection_matrix * input.camera_matrix * pos;
  out.texcoord = vertices[vertexId].textureCoordinates;
  return out;
}


fragment float4 blit_material_fragment_shader(const BlitVertexOut vertexIn [[stage_in]], texture2d<float> texture0 [[texture(0)]], texture2d<float> texture1 [[texture(1)]], sampler sampler2d [[sampler(0)]], constant BlitMaterialUniforms& input [[buffer(10)]]) {
  float4 clr0 = texture0.sample(sampler2d, vertexIn.texcoord);
  float4 clr1 = texture1.sample(sampler2d, vertexIn.texcoord);
  float3 clr = clr0.rgb * clr0.a + clr1.rgb * (1.0 - clr0.a);
  return float4(clr, 1);
}
