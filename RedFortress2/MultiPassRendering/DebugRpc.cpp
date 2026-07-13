#include "DebugRpc.h"

#include <stdexcept>

namespace
{
    const wchar_t* kDebugPipePath = L"\\\\.\\pipe\\RedFortress.Debug";
    const DWORD kPipeBufferSize = 4096;
}

DebugRpc::DebugRpc()
    : m_pipe(INVALID_HANDLE_VALUE)
    , m_connected(false)
{
}

DebugRpc::~DebugRpc()
{
    Finalize();
}

void DebugRpc::Initialize()
{
    if (m_pipe != INVALID_HANDLE_VALUE)
    {
        return;
    }

    m_pipe = CreateNamedPipeW(kDebugPipePath,
                              PIPE_ACCESS_DUPLEX,
                              PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT,
                              1,
                              kPipeBufferSize,
                              kPipeBufferSize,
                              0,
                              nullptr);
    if (m_pipe == INVALID_HANDLE_VALUE)
    {
        throw std::runtime_error("Failed to create the RedFortress debug RPC pipe.");
    }
}

void DebugRpc::Poll(const CommandHandler& handler)
{
    if (m_pipe == INVALID_HANDLE_VALUE)
    {
        return;
    }

    if (!m_connected)
    {
        const BOOL connected = ConnectNamedPipe(m_pipe, nullptr);
        if (connected != FALSE)
        {
            m_connected = true;
        }
        else
        {
            const DWORD error = GetLastError();
            if (error == ERROR_PIPE_CONNECTED)
            {
                m_connected = true;
            }
            else if (error == ERROR_PIPE_LISTENING || error == ERROR_NO_DATA)
            {
                return;
            }
            else
            {
                throw std::runtime_error("Failed to accept a RedFortress debug RPC client.");
            }
        }
    }

    char buffer[kPipeBufferSize];
    DWORD bytesRead = 0;
    const BOOL readSucceeded = ReadFile(m_pipe, buffer, sizeof(buffer), &bytesRead, nullptr);
    if (readSucceeded == FALSE)
    {
        const DWORD error = GetLastError();
        if (error == ERROR_NO_DATA || error == ERROR_PIPE_LISTENING)
        {
            return;
        }
        if (error == ERROR_BROKEN_PIPE)
        {
            DisconnectClient();
            return;
        }
        if (error != ERROR_MORE_DATA)
        {
            throw std::runtime_error("Failed to read a RedFortress debug RPC command.");
        }
    }

    if (bytesRead == 0)
    {
        return;
    }

    m_receiveBuffer.append(buffer, bytesRead);
    const std::size_t lineEnd = m_receiveBuffer.find('\n');
    if (lineEnd == std::string::npos)
    {
        return;
    }

    std::string command = m_receiveBuffer.substr(0, lineEnd);
    m_receiveBuffer.erase(0, lineEnd + 1);
    if (!command.empty() && command.back() == '\r')
    {
        command.pop_back();
    }

    std::string response = handler(command);
    response += "\n";

    DWORD bytesWritten = 0;
    const BOOL writeSucceeded = WriteFile(m_pipe,
                                          response.data(),
                                          static_cast<DWORD>(response.size()),
                                          &bytesWritten,
                                          nullptr);
    if (writeSucceeded == FALSE || bytesWritten != response.size())
    {
        const DWORD error = GetLastError();
        if (error == ERROR_BROKEN_PIPE || error == ERROR_NO_DATA)
        {
            DisconnectClient();
            return;
        }
        throw std::runtime_error("Failed to write a RedFortress debug RPC response.");
    }
}

void DebugRpc::Finalize()
{
    if (m_pipe == INVALID_HANDLE_VALUE)
    {
        return;
    }

    if (m_connected)
    {
        FlushFileBuffers(m_pipe);
        DisconnectNamedPipe(m_pipe);
    }

    CloseHandle(m_pipe);
    m_pipe = INVALID_HANDLE_VALUE;
    m_connected = false;
    m_receiveBuffer.clear();
}

void DebugRpc::DisconnectClient()
{
    DisconnectNamedPipe(m_pipe);
    m_connected = false;
    m_receiveBuffer.clear();
}
