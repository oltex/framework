
struct pixel_shader_in {
    float4 vPosition : SV_POSITION;
    float3 _texture_coord : TEXCOORD0;
};
struct pixel_shader_out {
    float4 _color : SV_TARGET0;
};

pixel_shader_out main(pixel_shader_in In) {
    pixel_shader_out Out;
    Out._color = vector(1.f, 1.f, 1.f, 1.f);
    return Out;
}