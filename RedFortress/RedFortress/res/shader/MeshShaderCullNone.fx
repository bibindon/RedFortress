// BOMありのUTF8だとコンパイルできなくなる。そのため、シェーダーファイルだけはBOMなし

#include "Common.fx"
#include "MeshShader.fx"

technique TechniqueCullNone
{
    pass Pass1
    {
        CullMode = None;
        AlphaTestEnable = TRUE;
        AlphaFunc = GreaterEqual;
        AlphaRef = 128;

        VertexShader = compile vs_3_0 VertexShader1();
        PixelShader  = compile ps_3_0 PixelShader1();
    }
}
