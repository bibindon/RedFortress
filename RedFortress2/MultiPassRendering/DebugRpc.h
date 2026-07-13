#pragma once

#include <Windows.h>
#include <functional>
#include <string>

class DebugRpc
{
public:
    using CommandHandler = std::function<std::string(const std::string&)>;

    DebugRpc();
    ~DebugRpc();

    void Initialize();
    void Poll(const CommandHandler& handler);
    void Finalize();

private:
    void DisconnectClient();

    HANDLE m_pipe;
    bool m_connected;
    std::string m_receiveBuffer;
};
