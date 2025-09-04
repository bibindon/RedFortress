#include "MeshNoShade.h"
#include "Light.h"
#include "Common.h"
#include "Camera.h"
#include "SharedObj.h"

MeshNoShade::MeshNoShade(
    const std::wstring& xFilename,
    const D3DXVECTOR3& position,
    const D3DXVECTOR3& rotation,
    const float& scale)
    : m_meshName { xFilename }
    , m_loadingPos { position }
    , m_rotate { rotation }
    , m_scale { scale }
{
}

MeshNoShade::~MeshNoShade()
{
    SAFE_RELEASE(m_D3DMesh);
    SAFE_RELEASE(m_D3DEffect);
}

void MeshNoShade::Init()
{
    HRESULT result { 0 };
    D3DXCreateEffectFromFile(
        SharedObj::GetD3DDevice(),
        SHADER_FILENAME.c_str(),
        nullptr,
        nullptr,
        D3DXSHADER_OPTIMIZATION_LEVEL3,
        nullptr,
        &m_D3DEffect,
        nullptr);
    if (FAILED(result))
    {
        throw std::exception("Failed to create an effect file.");
    }

    LPD3DXBUFFER adjacencyBuffer { nullptr };
    LPD3DXBUFFER materialBuffer { nullptr };

    result = D3DXLoadMeshFromX(
        m_meshName.c_str(),
        D3DXMESH_SYSTEMMEM,
        SharedObj::GetD3DDevice(),
        &adjacencyBuffer,
        &materialBuffer,
        nullptr,
        &m_materialCount,
        &m_D3DMesh);

    if (FAILED(result))
    {
        throw std::exception("Failed to load a x-file.");
    }

    D3DVERTEXELEMENT9 decl[] = {
        {
            0,
            0,
            D3DDECLTYPE_FLOAT3,
            D3DDECLMETHOD_DEFAULT,
            D3DDECLUSAGE_POSITION,
            0},
        {
            0,
            12,
            D3DDECLTYPE_FLOAT3,
            D3DDECLMETHOD_DEFAULT,
            D3DDECLUSAGE_NORMAL,
            0
        },
        {
            0,
            24,
            D3DDECLTYPE_FLOAT2,
            D3DDECLMETHOD_DEFAULT,
            D3DDECLUSAGE_TEXCOORD,
            0
        },
        D3DDECL_END(),
    };

    LPD3DXMESH tempMesh { nullptr };
    result = m_D3DMesh->CloneMesh(D3DXMESH_MANAGED, decl, SharedObj::GetD3DDevice(), &tempMesh);

    if (FAILED(result))
    {
        throw std::exception("Failed 'CloneMesh' function.");
    }
    SAFE_RELEASE(m_D3DMesh);
    m_D3DMesh = tempMesh;
    DWORD* wordBuffer { static_cast<DWORD*>(adjacencyBuffer->GetBufferPointer()) };

    // ここをコメントアウトするとフラットシェーディングになる。
    result = D3DXComputeNormals(m_D3DMesh, wordBuffer);

    if (FAILED(result))
    {
        throw std::exception("Failed 'D3DXComputeNormals' function.");
    }

    result = m_D3DMesh->OptimizeInplace(
        D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE,
        wordBuffer,
        nullptr,
        nullptr,
        nullptr);

    SAFE_RELEASE(adjacencyBuffer);

    if (FAILED(result))
    {
        throw std::exception("Failed 'OptimizeInplace' function.");
    }

    m_vecColor.insert(begin(m_vecColor), m_materialCount, D3DCOLORVALUE { });

    D3DXMATERIAL* materials { static_cast<D3DXMATERIAL*>(materialBuffer->GetBufferPointer()) };

    std::wstring xFileDir = m_meshName;
    std::size_t lastPos = xFileDir.find_last_of(_T("\\"));
    xFileDir = xFileDir.substr(0, lastPos + 1);

    for (DWORD i = 0; i < m_materialCount; ++i)
    {
        m_vecColor.at(i) = materials[i].MatD3D.Diffuse;
    }
    SAFE_RELEASE(materialBuffer);

    m_bIsInit = true;
}

void MeshNoShade::SetPos(const D3DXVECTOR3& pos)
{
    m_loadingPos = pos;
}

D3DXVECTOR3 MeshNoShade::GetPos()
{
    return m_loadingPos;
}

void MeshNoShade::Render()
{
    if (m_bIsInit == false)
    {
        return;
    }

    D3DXMATRIX worldViewProjMatrix { };
    D3DXMatrixIdentity(&worldViewProjMatrix);
    {
        D3DXMATRIX mat { };

        D3DXMatrixTranslation(&mat, -m_centerPos.x, -m_centerPos.y, -m_centerPos.z);
        worldViewProjMatrix *= mat;

        D3DXMatrixScaling(&mat, m_scale, m_scale, m_scale);
        worldViewProjMatrix *= mat;

        D3DXMatrixRotationYawPitchRoll(&mat, m_rotate.y, m_rotate.x, m_rotate.z);
        worldViewProjMatrix *= mat;

        D3DXMatrixTranslation(&mat, m_loadingPos.x, m_loadingPos.y, m_loadingPos.z);
        worldViewProjMatrix *= mat;
    }
    worldViewProjMatrix *= Camera::GetViewMatrix();
    worldViewProjMatrix *= Camera::GetProjMatrix();

    m_D3DEffect->SetMatrix("g_matWorldViewProj", &worldViewProjMatrix);

    m_D3DEffect->Begin(nullptr, 0);

    D3DXVECTOR4 vec4Color;

    HRESULT result { S_FALSE };
    if (FAILED(result = m_D3DEffect->BeginPass(0)))
    {
        m_D3DEffect->End();
        throw std::exception("Failed 'BeginPass' function.");
    }

    for (DWORD i = 0; i < m_materialCount; ++i)
    {
        vec4Color.x = m_vecColor.at(i).r;
        vec4Color.y = m_vecColor.at(i).g;
        vec4Color.z = m_vecColor.at(i).b;
        vec4Color.w = m_vecColor.at(i).a;

        // TODO
//        vec4Color.x = 0.8f;
//        vec4Color.y = 0.2f;
//        vec4Color.z = 0.2f;
//        vec4Color.w = 1.f;
        result = m_D3DEffect->SetVector("g_vecDiffuse", &vec4Color);
        m_D3DEffect->CommitChanges();
        m_D3DMesh->DrawSubset(i);
    }
    m_D3DEffect->EndPass();
    m_D3DEffect->End();
}

LPD3DXMESH MeshNoShade::GetD3DMesh() const
{
    return m_D3DMesh;
}

// 解像度やウィンドウモードを変更したときのための関数
void MeshNoShade::OnDeviceLost()
{

}

// 解像度やウィンドウモードを変更したときのための関数
void MeshNoShade::OnDeviceReset()
{

}

