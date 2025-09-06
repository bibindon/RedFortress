#pragma once

#include <d3d9.h>
#include <random>
#include "Player.h"

#include "../../RedFortressQuestSystem/QuestSystem/QuestSystem.h"
#include "VoyageManager.h"
#include "KeyBoard.h"

enum class eWindowStyle
{
    WINDOW,
    FULLSCREEN,
    BORDERLESS,
    NONE,
};

class SharedObj
{
public:
    static HWND GetWindowHandle();
    static void SetWindowHandle(const HWND hWnd);
    static LPDIRECT3DDEVICE9 GetD3DDevice();
    static void SetD3DDevice(const LPDIRECT3DDEVICE9 D3DDevice);
    static void Init();
    static void Finalize();
    static unsigned int GetRandom();
    static Player* GetPlayer();
    static void SetPlayer(Player* player);
    static Map* GetMap();
    static void SetMap(Map* map);
    static D3DXMATRIX GetRightHandMat();
    static void SetRightHandMat(D3DXMATRIX mat);

    static VoyageManager* Voyage();

    static void SetKeyBoard(IKeyBoard* keyboard);
    static IKeyBoard* KeyBoard();

    static void SetEnglish(const bool bEnglish);
    static bool IsEnglish();

    static eWindowStyle WindowStyleRequest();
    static void SetWindowStyleRequest(const eWindowStyle arg);

    // (0, 0)だったらリクエスト無し、と判断する
    static POINT WindowSizeRequest();
    static void SetWindowSizeRequest(const POINT& arg);

private:
    static LPDIRECT3DDEVICE9 m_D3DDevice;
    static HWND m_hWnd;
    static std::mt19937 m_Engine;
    static Player* m_player;
    static Map* m_map;
    static D3DXMATRIX m_rightHandMat;

    static IKeyBoard* m_keyBoard;
    static bool m_bEnglish;

    static eWindowStyle m_eWindowStyle;
    static POINT m_windowSize;
};

