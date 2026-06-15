#include "Light.h"

D3DXVECTOR4 Light::m_lightNormal { 1.0f, 1.0f, 0.0f, 0.0f };
float Light::m_Brightness = 1.0f;

D3DXVECTOR4 Light::GetLightNormal()
{
    return m_lightNormal;
}

void Light::SetLightNormal(const D3DXVECTOR4& normal)
{
    m_lightNormal = normal;
}

float Light::GetBrightness()
{
    return m_Brightness;
}

void Light::SetBrightness(const float brightness)
{
    m_Brightness = brightness;
}
