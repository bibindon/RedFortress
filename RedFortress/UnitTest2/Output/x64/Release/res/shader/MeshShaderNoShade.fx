//--------------------------------------------
// 陰を表示しないシェーダー
//
// BOMありのUTF8だとコンパイルできなくなる。そのため、シェーダーファイルだけはBOMなし
// グローバル変数は何も指定しなければ0で初期化されるので不要
//--------------------------------------------

float4x4 g_matWorldViewProj;
float4 g_vecDiffuse;

void VertexShader1(in  float4 inPos  : POSITION,
                   out float4 outPos : POSITION)
{
    outPos  = mul(inPos, g_matWorldViewProj);
}

void PixelShader1(in  float4 inDiffuse  : COLOR0,
                  out float4 outDiffuse : COLOR0)
{
    outDiffuse = g_vecDiffuse;
}

technique Technique1
{
    pass Pass1
    {
        VertexShader = compile vs_3_0 VertexShader1();
        PixelShader  = compile ps_3_0 PixelShader1();
    }
}
