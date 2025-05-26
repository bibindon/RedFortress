float4x4 g_matWorldViewProj;
float4 g_lightNormal = { 0.3f, 1.0f, 0.5f, 0.0f };

// ディヒューズ色
// 基本色
// テクスチャで隠れてしまうので意味がないように思えるが、
// テクスチャの色とミックスして表示するので無意味ではない。
float4 g_colDiffuse = { 0.5f, 0.5f, 0.5f, 0.5f };

// スペキュラ光の色
// 鏡面反射の色
// 通常は白であるが、金や銅の時に白以外が使われる
float4 g_colSpecular = { 1.0f, 1.0f, 1.0f, 0.0f };

// スペキュラ光の強さ
float g_specularIntensity = 0.5f;

// アンビエント色
// 環境光の色
float4 g_colAmbient = { 0.5f, 0.5f, 0.5f, 0.5f };

texture texture1;

// 異方性フィルタを設定
// TODO ゲーム中に変えられるようにする
sampler textureSampler = sampler_state {
    Texture = (texture1);
    MipFilter = LINEAR;
    MinFilter = ANISOTROPIC;
    MagFilter = ANISOTROPIC;

    MaxAnisotropy = 8;
};

void VertexShader1(in  float4 inPosition  : POSITION,
                   in  float4 inNormal    : NORMAL0,
                   in  float4 inTexCood   : TEXCOORD0,

                   out float4 outPosition : POSITION,
                   out float4 outDiffuse  : COLOR0,
                   out float4 outTexCood  : TEXCOORD0)
{
    outPosition = mul(inPosition, g_matWorldViewProj);

    float lightIntensity = dot(inNormal, g_lightNormal);
    lightIntensity = max(0, lightIntensity);;

    outDiffuse = g_colDiffuse * lightIntensity;

    outDiffuse += g_colAmbient;
    outDiffuse.a = 1.0f;

    // 0.0〜1.0 にクランプする
    outDiffuse = saturate(outDiffuse);

    outTexCood = inTexCood;
}

void PixelShader1(in float4 inScreenColor : COLOR0,
                  in float2 inTexCood     : TEXCOORD0,

                  out float4 outColor     : COLOR)
{
    float4 workColor = (float4)0;
    workColor = tex2D(textureSampler, inTexCood);
    outColor = inScreenColor * workColor;
}

technique Technique1
{
   pass Pass1
   {
      VertexShader = compile vs_3_0 VertexShader1();
      PixelShader = compile ps_3_0 PixelShader1();
   }
}
