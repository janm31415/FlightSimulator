#include <metal_stdlib>
using namespace metal;

struct VertexIn {
  packed_float3 position;
  packed_float3 normal;
  packed_float2 textureCoordinates;
};

struct SimpleMaterialUniforms {
  float4x4 view_projection_matrix;
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
  out.position = input.view_projection_matrix * pos;
  out.normal = (input.camera_matrix * float4(vertices[vertexId].normal, 0)).xyz;
  out.texcoord = vertices[vertexId].textureCoordinates;
  return out;
}


fragment float4 simple_material_fragment_shader(const VertexOut vertexIn [[stage_in]], texture2d<float> texture [[texture(0)]], sampler sampler2d [[sampler(0)]], constant SimpleMaterialUniforms& input [[buffer(10)]]) {
  float l = clamp(dot(vertexIn.normal,input.light), 0.0, 1.0 - input.ambient) + input.ambient;
  return (texture.sample(sampler2d, vertexIn.texcoord)*input.texture_sample + input.color*(1-input.texture_sample))*l;
}
