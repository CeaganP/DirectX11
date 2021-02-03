#pragma once 
#include <xaudio2.h>

ref class SoundEffect
{
internal:
    SoundEffect() { m_audioAvailable = false; };
    void Initialize(
        _In_ IXAudio2* masteringEngine,
        _In_ WAVEFORMATEX* sourceFormat,
        _In_ Platform::Array<byte>^ soundData
    );

    void PlaySound(_In_ float volume);

protected private:
    bool                    m_audioAvailable;
    IXAudio2SourceVoice* m_sourceVoice;
    Platform::Array<byte>^ m_soundData;
};