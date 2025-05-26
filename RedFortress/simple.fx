float4x4 g_matWorldViewProj;
float4 g_lightNormal = { 0.3f, 1.0f, 0.5f, 0.0f };

// �f�B�q���[�Y�F
// ��{�F
// �e�N�X�`���ŉB��Ă��܂��̂ňӖ����Ȃ��悤�Ɏv���邪�A
// �e�N�X�`���̐F�ƃ~�b�N�X���ĕ\������̂Ŗ��Ӗ��ł͂Ȃ��B
float4 g_colDiffuse = { 0.5f, 0.5f, 0.5f, 0.5f };

// �X�y�L�������̐F
// ���ʔ��˂̐F
// �ʏ�͔��ł��邪�A���⓺�̎��ɔ��ȊO���g����
float4 g_colSpecular = { 1.0f, 1.0f, 1.0f, 0.0f };

// �X�y�L�������̋���
float g_specularIntensity = 0.5f;

// �A���r�G���g�F
// �����̐F
float4 g_colAmbient = { 0.5f, 0.5f, 0.5f, 0.5f };

texture texture1;
sampler textureSampler = sampler_state {
    Texture = (texture1);
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
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
