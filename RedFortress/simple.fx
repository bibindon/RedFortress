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
//
// スペキュラ光の実装は頂点シェーダーでもピクセルシェーダーでもできる
// 切り替えられたほうが良いかもしれない。
float4 g_colSpecular = { 1.0f, 1.0f, 1.0f, 0.0f };

// スペキュラ光の強さ
float g_specularIntensity = 0.9f;

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

float4x4 g_matWorld;
float4x4 g_matView;
float4 g_cameraPos = { 3.f, 3.f, 3.f, 1.f };

void VertexShader1(in float4 inPosition : POSITION,
                   in float4 inNormal : NORMAL0,
                   in float4 inTexCood : TEXCOORD0,

                   out float4 outPosition : POSITION,
                   out float4 outDiffuse : COLOR0,
                   out float4 outTexCood : TEXCOORD0,
                   out float3 outWorldPos : TEXCOORD1,
                   out float3 outNormalW : TEXCOORD2)
{
    float4 worldPos = mul(inPosition, g_matWorld);
    float3 normalW = normalize(mul(inNormal, (float3x3) g_matWorld));

    outPosition = mul(inPosition, g_matWorldViewProj);
    outDiffuse = g_colDiffuse * max(0, dot(normalW, g_lightNormal));
    outDiffuse += g_colAmbient;
    outDiffuse.a = 1.0f;
    outDiffuse = saturate(outDiffuse);
    outTexCood = inTexCood;

    outWorldPos = worldPos.xyz;
    outNormalW = normalW;
}

void PixelShader1(in float4 inScreenColor : COLOR0,
                  in float2 inTexCood : TEXCOORD0,
                  in float3 inWorldPos : TEXCOORD1,
                  in float3 inNormalW : TEXCOORD2,

                  out float4 outColor : COLOR)
{
    float4 texColor = tex2D(textureSampler, inTexCood);
    float4 baseColor = inScreenColor * texColor;

    // カメラからピクセルへのベクトル
    float3 viewDir = normalize(g_cameraPos.xyz - inWorldPos);

    // 反射ベクトル
    float3 reflectDir = reflect(-g_lightNormal.xyz, normalize(inNormalW));

    // スペキュラ項（Phongモデル）
    float specPower = 32.0f; // 固定値でもOK
    float spec = pow(saturate(dot(reflectDir, viewDir)), specPower);

    float3 specular = spec * g_colSpecular.rgb * g_specularIntensity;

    outColor.rgb = baseColor.rgb + specular;
    outColor.a = baseColor.a;
}

technique Technique1
{
   pass Pass1
   {
      VertexShader = compile vs_3_0 VertexShader1();
      PixelShader = compile ps_3_0 PixelShader1();
   }
}
