#pragma once

#include <dsound.h>
#include <string>
#include <vector>
#include <unordered_map>

class SoundEffect
{
public:
    static SoundEffect* get_ton();
    static void initialize(HWND hwnd);
    static void finalize(); // For memory leak check.

    bool load(const std::wstring& filename);
    void play(const std::wstring& filename, const int a_volume = 100);
    void stop(const std::wstring& filename);

private:
    static SoundEffect* single_ton_;
    // hide ctor
    SoundEffect(HWND hwnd);
    SoundEffect(const SoundEffect&) { }
    void operator=(const SoundEffect&) { }

    bool open_wave(
        const std::wstring& filepath,
        WAVEFORMATEX& waveFormatEx,
        std::vector<char>* ppData,
        DWORD& dataSize);

    int per_to_decibel(const int percent);

    LPDIRECTSOUND8 dx8sound_ { nullptr };
    std::unordered_map<std::wstring, LPDIRECTSOUNDBUFFER8> dx8sound_buffers_ { };
};
