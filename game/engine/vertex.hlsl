matrix _view_matrix, projection_matrix;

matrix _world_matrix;
matrix _inverse_world_matrix;

struct vertex_shader_in {
    float3 _position : POSITION;
    float3 _texture_coord : TEXCOORD0;
};
struct vertex_shader_out {
    float4 _position : SV_POSITION;
    float3 _texture_coord : TEXCOORD0;
};

vertex_shader_out main(vertex_shader_in In) {
    vertex_shader_out Out;
    Out._position = mul(float4(In._position, 1.f), _world_matrix);
    Out._position = mul(Out._position, _view_matrix);
    Out._position = mul(Out._position, projection_matrix);
    Out._texture_coord = In._texture_coord;
    return Out;
}