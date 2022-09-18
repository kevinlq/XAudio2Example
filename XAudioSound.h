#pragma once

#include <QString>

class XAudioSoundImpl;

class XAudioSound
{
public:
    XAudioSound();
    ~XAudioSound();

    bool playAudio(const QStringList &audioFileList);
    bool play(const QString &audioFile);
    bool pause();
    bool resume();

private:
    XAudioSoundImpl *m_pImpl = nullptr;
};

