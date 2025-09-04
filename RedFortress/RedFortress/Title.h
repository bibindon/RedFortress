#pragma once

#include "Common.h"
#include "Sprite.h"
#include "CommandManager.h"
#include "SeqBattle.h"
#include <thread>
#include <atomic>

//----------------------------------------------------------
// タイトル画面
//
// ・最初は島全体が見えるようにカメラが離れている。
// ・Continueを選んだらカメラがプレイヤーの位置に移動する
// ・カメラの移動が完了するまではタイトル画面の担当範囲。
// ・Startを選んだら暗転してOpening画面
// ・セーブデータがあったら、タイトル画面が表示されたときすでにセーブデータが読まれている。
// ・セーブデータがあるのに初めからを選んだら、初期データは読んでいないので
//   くるくるを表示して読み直す必要がある。
// ・カメラ移動のフェードイン・フェードアウトと暗転によるフェードイン・フェードアウトがある。
//----------------------------------------------------------

class Title
{
public:
    // blackFadeIn: 暗転によるフェードインか、カメラによるフェードインか。
    Title(const bool blackFadeIn, const bool bFirst);
    ~Title();
    void Update(eSequence* sequence, eBattleState* eState);
    void Render();

    bool GetLoading() const;
    bool GetFirst() const { return m_bFirst; }

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceLost();

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceReset();

private:
    Sprite* m_sprite1 { nullptr };
    Sprite* m_sprite3 { nullptr };
    Sprite* m_spriteBlack { nullptr };
    Sprite* m_spriteTimer { nullptr };

    CommandManager* m_titleCommand = nullptr;

    Sprite* m_sprBack = nullptr;
    Sprite* m_sprLogo = nullptr;
    Sprite* m_sprClock = nullptr;
    Sprite* m_sprLoading = nullptr;
    LPD3DXFONT m_font = nullptr;

    const int FADE_IN = 60;
    bool m_bFadeIn = false;
    int m_fadeInCount = 0;
    int m_fadeInAlpha = 0;

    const int FADE_OUT = 1;
    bool m_bFadeOut = false;
    int m_fadeOutCount = 0;
    int m_fadeOutAlpha = 0;

    bool m_bCameraFadeIn = false;
    int m_cameraFadeInCount = 0;

    bool m_bCameraFadeOut = false;
    int m_cameraFadeOutCount = 0;

    std::thread* m_thread = nullptr;
    std::atomic<bool> m_loaded = false;
    bool m_bLoading = false;

    eTitleMenu m_eMenu = eTitleMenu::NOT_DECIDE;

    // 起動してから初めてタイトル画面が表示された。
    bool m_bFirst = true;
};

