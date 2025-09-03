#pragma once

#include "../../RedFortressMenu/Menu/Menu.h"

class MenuManager
{
public:
    MenuManager();
    ~MenuManager();

    void InitMenu();
    void Finalize();
    void Draw();
    bool UseItem(const std::wstring& id, const int subId);
    void AddItem(const std::wstring& id, const int subId, const int durability = -1);
    void DeleteItem(const std::wstring& id, const int subId);
    std::wstring OperateMenu();

private:

    NSMenu::Menu m_menu;

    bool IsBagEquiped(const std::wstring& id, const int subId);
    void Equip(const std::wstring& id, const int subId);
    void Unequip(const std::wstring& id, const int subId);
};

