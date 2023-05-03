struct VsOut
{
    float2 texC       : TEXCOORD;
    float4 posH       : SV_POSITION;
};

float4 main(VsOut input) : SV_Target {
    return float4(1, 0, 1, 1);
}
