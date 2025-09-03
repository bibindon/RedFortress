#include "SharedObj.h"
#include "Common.h"

HWND SharedObj::m_hWnd = nullptr;
LPDIRECT3DDEVICE9 SharedObj::m_D3DDevice = nullptr;
std::mt19937 SharedObj::m_Engine;
Player* SharedObj::m_player = nullptr;
Map* SharedObj::m_map = nullptr;
D3DXMATRIX SharedObj::m_rightHandMat;

IKeyBoard* SharedObj::m_keyBoard = nullptr;
bool SharedObj::m_bEnglish = false;

eWindowStyle SharedObj::m_eWindowStyle = eWindowStyle::BORDERLESS;

LPDIRECT3DDEVICE9 SharedObj::GetD3DDevice()
{
    return m_D3DDevice;
}

HWND SharedObj::GetWindowHandle()
{
    return m_hWnd;
}

void SharedObj::SetWindowHandle(const HWND hWnd)
{
    m_hWnd = hWnd;
}

void SharedObj::Finalize()
{
    SAFE_RELEASE(m_D3DDevice);
    SAFE_DELETE(m_map);
    SAFE_DELETE(m_player);
    SAFE_DELETE(m_keyBoard);
}

unsigned int SharedObj::GetRandom()
{
    return m_Engine();
}

Player* SharedObj::GetPlayer()
{
    return m_player;
}

void SharedObj::SetPlayer(Player* player)
{
    m_player = player;
}

Map* SharedObj::GetMap()
{
    return m_map;
}

void SharedObj::SetMap(Map* map)
{
    m_map = map;
}

D3DXMATRIX SharedObj::GetRightHandMat()
{
    return m_rightHandMat;
}

void SharedObj::SetRightHandMat(D3DXMATRIX mat)
{
    m_rightHandMat = mat;
}

VoyageManager* SharedObj::Voyage()
{
    return VoyageManager::Get();
}

void SharedObj::SetKeyBoard(IKeyBoard* keyboard)
{
    m_keyBoard = keyboard;
}

IKeyBoard* SharedObj::KeyBoard()
{
    return m_keyBoard;
}

void SharedObj::SetEnglish(const bool bEnglish)
{
    m_bEnglish = bEnglish;
}

bool SharedObj::IsEnglish()
{
    return m_bEnglish;
}

eWindowStyle SharedObj::WindowStyleRequest()
{
    return m_eWindowStyle;
}

void SharedObj::SetWindowStyleRequest(const eWindowStyle arg)
{
    m_eWindowStyle = arg;
}

void SharedObj::SetD3DDevice(const LPDIRECT3DDEVICE9 D3DDevice)
{
    m_D3DDevice = D3DDevice;
}

void SharedObj::Init()
{
    std::random_device seed_gen;
    m_Engine = std::mt19937(seed_gen());
}

