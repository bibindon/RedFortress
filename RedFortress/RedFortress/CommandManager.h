#pragma once

#include "Common.h"
#include "../../RedFortressCommand/Command/Command.h"
#include <string>

class CommandManager
{
public:
    enum class eType
    {
        Title,
        Title_Language,
        Main,
    };

    void Init(const eType type);
    std::wstring Operate();
    void Draw();
    void Finalize();
    void Upsert(const std::wstring& name, const bool visible);
    void SetPreviousState(const eBattleState previousState);
    eBattleState GetPreviousState() const;

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceLost();

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceReset();

private:

    // コマンドの作成
    // 0.5秒おきに呼ばれる？
    void BuildCommand();
    void BuildOpeningCommand();
    void BuildLangCommand();

    NSCommand::Command* m_commandLib = nullptr;

    // コマンド画面の表示カウント
    int m_counter = 0;

    eType m_eType = eType::Title;

    eBattleState m_previousState = eBattleState::NONE;
};

