#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <tchar.h>

// 同じ3Dモデルを沢山表示したいときに使うクラス
// 同じ3Dモデルを沢山表示したいとき、3Dモデルの読み込みは1度でよく
// 座標、回転、拡大率を再設定して再表示することで軽量化する。
class MeshClone
{
public:
    enum class eMeshType
    {
        TREE,
        GRASS,
        SOTETSU,
        WATER,
        SEA,
        OTHER,
    };

    MeshClone(const std::wstring&,
              const D3DXVECTOR3&,
              const D3DXVECTOR3&,
              const float scale,
              const float radius = 20.f);

    ~MeshClone();

    void Init();
    void SetPos(const D3DXVECTOR3& pos);
    D3DXVECTOR3 GetPos() const;
    float GetScale() const;
    void Render();
    void Begin();
    void Render2() const;
    void End();

    eMeshType GetMeshType() const;

    LPD3DXMESH GetD3DMesh() const;

    void SetWeapon(const bool arg);

    float GetRadius() const;

    std::wstring GetMeshName();
    bool ContainMeshName(const std::wstring& arg);

    static void ForceRelease();

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceLost();

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceReset();

private:
    std::wstring m_shaderFilename { _T("res\\shader\\MeshShader.fx") };

    static std::unordered_map<std::wstring, LPD3DXEFFECT> m_D3DEffectMap;
    static std::unordered_map<std::wstring, LPD3DXMESH> m_D3DMeshMap;
    static std::unordered_map<std::wstring, std::vector<LPDIRECT3DTEXTURE9>> m_vecTextureMap;
    static std::unordered_map<std::wstring, DWORD> m_materialCountMap;
    static std::unordered_map<std::wstring, std::vector<D3DXVECTOR4>> m_vecColorMap;

    // 最初に一回だけやればいい処理
    static std::unordered_map<std::wstring, bool> m_bFirstMap;

    D3DXVECTOR3 m_loadingPos { };
    D3DXVECTOR3 m_rotate { };

    D3DXVECTOR3 m_centerPos { 0.0f, 0.0f, 0.0f };
    float m_scale { 0.0f };

    float m_radius = 0.f;

    std::wstring m_meshName { _T("") };
    bool m_bIsInit { false };

    bool m_bWeapon = false;

    bool m_bPointLightEnablePrevious = false;

    eMeshType m_eMeshType = eMeshType::OTHER;
};

