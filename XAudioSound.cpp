#include "XAudioSound.h"

#include "AudioFile.h"

// xaudio lib file
#include <xaudio2.h>
#include <comdef.h>

// Qt lib file
#include <QDebug>

class VoiceCallback : public IXAudio2VoiceCallback
{
public:
    HANDLE hBufferEndEvent;
    VoiceCallback() : hBufferEndEvent(CreateEvent(nullptr, FALSE, FALSE, nullptr)) {}
    ~VoiceCallback() { CloseHandle(hBufferEndEvent); }

    // Called when the voice has just finished playing a contiguous audio stream.
    void OnStreamEnd() { SetEvent(hBufferEndEvent); }

    // Unused methods are stubs
    void OnVoiceProcessingPassEnd() {}
    void OnVoiceProcessingPassStart(UINT32 SamplesRequired)
    {
        Q_UNUSED(SamplesRequired);
        //qDebug() << "OnVoiceProcessingPassStart " << SamplesRequired;
    }
    void OnBufferEnd(void *pBufferContext)
    {
        Q_UNUSED(pBufferContext);
        qDebug() << "OnBufferEnd:" << pBufferContext;
    }
    void OnBufferStart(void *pBufferContext)
    {
        Q_UNUSED(pBufferContext);
        qDebug() << "OnBufferStart:" << pBufferContext;
    }
    void OnLoopEnd(void *pBufferContext) {Q_UNUSED(pBufferContext);}
    void OnVoiceError(void *pBufferContext, HRESULT Error)
    {
        Q_UNUSED(pBufferContext);
        Q_UNUSED(Error);
        qDebug() << "voice error:" << Error;
    }
};

class XAudioSoundImpl
{
public:
    XAudioSoundImpl();
    ~XAudioSoundImpl();
    bool init();
    bool unInit();

public:
    bool m_bInit = false;
    WAVEFORMATEX m_waveFormatex; //设置主声音的格式
    XAUDIO2_BUFFER buffer;

    IXAudio2 *m_pEngine = nullptr;
    IXAudio2MasteringVoice *m_pMasterVoice = nullptr;
    IXAudio2SourceVoice *m_pSourceVoice = nullptr;
    VoiceCallback           *m_pCallback = nullptr;
};

XAudioSoundImpl::XAudioSoundImpl()
{
    m_waveFormatex.nChannels = 2;
    m_waveFormatex.nSamplesPerSec = 48000;
    m_waveFormatex.wBitsPerSample = 32;
    m_waveFormatex.nAvgBytesPerSec = 384000;
    m_waveFormatex.nBlockAlign = 8;
    m_waveFormatex.wFormatTag = WAVE_FORMAT_PCM;

    m_pCallback = new VoiceCallback;
}

XAudioSoundImpl::~XAudioSoundImpl()
{
    unInit();

    delete m_pCallback;
    m_pCallback = nullptr;
}

bool XAudioSoundImpl::init()
{
    if(m_bInit)
    {
        qDebug() << "already init";
        return true;
    }

    // com初始化,// COINIT_MULTITHREADED,COINIT_APARTMENTTHREADED
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if(FAILED(hr))
    {
        qDebug() << "com init fail..";
        return false;
    }

    //hr = XAudio2Create(&m_pEngine, 0);
    hr = XAudio2Create(&m_pEngine, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if(FAILED(hr))
    {
        qDebug() << "create engine fail.";
        return false;
    }

    // get default device

    //hr = m_pEngine->CreateMasteringVoice(&m_pMasterVoice, m_waveFormatex.nChannels, m_waveFormatex.nSamplesPerSec);
    hr = m_pEngine->CreateMasteringVoice(&m_pMasterVoice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, 0, nullptr); //创建主声音。默认是输出当前扬声器
    if(FAILED(hr))
    {
        qDebug() << "create master voice fail.";
        return false;
    }

    m_bInit = true;

    return true;
}

bool XAudioSoundImpl::unInit()
{
    if(nullptr != m_pEngine)
    {
        m_pEngine->Release();
    }

    CoUninitialize();

    m_bInit = false;

    return true;
}

XAudioSound::XAudioSound()
    : m_pImpl(new XAudioSoundImpl)
{
}

XAudioSound::~XAudioSound()
{
    delete m_pImpl;
    m_pImpl = nullptr;
}

bool XAudioSound::playAudio(const QStringList &audioFileList)
{
    return false;
    if(!m_pImpl->init())
    {
        qDebug() << "play fail for init xaudio:" << audioFileList;
        return false;
    }

    XAUDIO2_SEND_DESCRIPTOR pSend;
    pSend.pOutputVoice = m_pImpl->m_pMasterVoice; //指定输出为mastering voice
    pSend.Flags = XAUDIO2_SEND_USEFILTER;

    XAUDIO2_VOICE_SENDS pSendList;
    pSendList.pSends = &pSend;
    pSendList.SendCount = 1;

    IXAudio2SubmixVoice *pSubmixVoice = NULL;
    //指定输出为mastering voice,作用：混音，将两路音频数据混为一路并输出到mastering voice
    HRESULT hr = m_pImpl->m_pEngine->CreateSubmixVoice(&pSubmixVoice, m_pImpl->m_waveFormatex.nChannels, m_pImpl->m_waveFormatex.nSamplesPerSec, 0, 0, &pSendList);
    if (FAILED(hr))
    {
        return false;
    }

    pSend.pOutputVoice = pSubmixVoice; //指定输出为SubmixVoice

    IXAudio2SubmixVoice *pSubmixVoice1 = nullptr;
    //指定输出为pSubmixVoice,作用：对文件1的音频数据进行重採样为mastering voice的採样率
    hr = m_pImpl->m_pEngine->CreateSubmixVoice(&pSubmixVoice1, m_pImpl->m_waveFormatex.nChannels, m_pImpl->m_waveFormatex.nSamplesPerSec, 0, 0, &pSendList);
    if (FAILED(hr))
    {
        return false;
    }

    IXAudio2SubmixVoice *pSubmixVoice2 = nullptr;
    //指定输出为pSubmixVoice,作用：对文件2的音频数据进行重採样为mastering voice的採样率
    hr = m_pImpl->m_pEngine->CreateSubmixVoice(&pSubmixVoice2, m_pImpl->m_waveFormatex.nChannels, m_pImpl->m_waveFormatex.nSamplesPerSec, 0, 0, &pSendList);
    if (FAILED(hr))
    {
        return false;
    }

    pSend.pOutputVoice = pSubmixVoice1; //指定输出为pSubmixVoice1


    WAVEFORMATEX *waveFormat1; //获取文件格式1
    WAVEFORMATEX *waveFormat2; //获取文件格式2

    AudioFile<float> wavAudioFile1(audioFileList[0].toStdString());
    waveFormat1->nChannels = wavAudioFile1.getNumChannels();
    waveFormat1->wBitsPerSample = wavAudioFile1.getBitDepth();
    waveFormat1->nSamplesPerSec = wavAudioFile1.getSampleRate();

    VoiceCallback voiceCallBack1;
    IXAudio2SourceVoice *pSourceVoice1 = NULL;
    //创建源声音。用来提交数据.指定输出为SubmixVoice1
    hr = m_pImpl->m_pEngine->CreateSourceVoice(&pSourceVoice1, waveFormat1, 0, 1.0f, &voiceCallBack1, &pSendList);
    if (FAILED(hr))
    {
        return false;
    }

    pSend.pOutputVoice = pSubmixVoice2; //指定输出为pSubmixVoice2

    return true;
}

bool XAudioSound::play(const QString &audioFile)
{
    if(!m_pImpl->init())
    {
        qDebug() << "play fail for init xaudio:" << audioFile;
        return false;
    }

    if(nullptr != m_pImpl->m_pEngine)
    {
        m_pImpl->m_pEngine->StopEngine();
        m_pImpl->m_pEngine->StartEngine();
    }

    // open wav audio file
    AudioFile<float> wavAudioFile;
    if(!wavAudioFile.load(audioFile.toLocal8Bit().toStdString()))
    {
        qDebug() << "load wav file fail.." << audioFile;
        return false;
    }

    int fs = wavAudioFile.getSampleRate();          // 8k,16k,44.1k,.......
    int bits = wavAudioFile.getBitDepth();          // 8,16,24,32,......
    int channels = wavAudioFile.getNumChannels();   // 1,2

    //DWORD transVPerSecond = fs * bits/8 * channels;

    WAVEFORMATEX waveFormat; //获取文件格式1
    memset(&waveFormat, 0, sizeof(waveFormat));
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.wBitsPerSample = bits;
    waveFormat.nChannels = channels;
    waveFormat.nSamplesPerSec = fs;
    waveFormat.nBlockAlign = (bits/8) *channels;
    waveFormat.nAvgBytesPerSec = fs * (bits/8) *channels;
    waveFormat.cbSize = sizeof(WAVEFORMATEX);

    qDebug() << "audio info";
    qDebug() << "wBitsPerSample:" << waveFormat.wBitsPerSample;
    qDebug() << "nChannels:" << waveFormat.nChannels;
    qDebug() << "nSamplesPerSec:" << waveFormat.nSamplesPerSec;
    qDebug() << "nBlockAlign:" << waveFormat.nBlockAlign;
    qDebug() << "nAvgBytesPerSec:" << waveFormat.nAvgBytesPerSec;
    qDebug() << "cbSize:" << waveFormat.cbSize;

    DWORD dataSize = wavAudioFile.getNumSamplesPerChannel() * channels;
    BYTE * pData = new BYTE[dataSize];
    memset(pData, 0, dataSize);

    FILE *inputFile = fopen(audioFile.toLocal8Bit(), "rb");
    if (nullptr == inputFile)
    {
        qDebug() << "fopen error.." << dataSize << audioFile;
        delete []pData;
        return false;
    }

    size_t readSize = fread(pData, sizeof(BYTE), dataSize, inputFile);
    if (readSize != dataSize)
    {
        qDebug() << "readSize error.." << readSize << dataSize;
        delete []pData;
        return false;
    }
    fclose(inputFile);

    m_pImpl->buffer = {0}; //将读取的文件数据，赋值XAUDIO2_BUFFER
    m_pImpl->buffer.AudioBytes = dataSize;
    m_pImpl->buffer.pAudioData = pData;
    m_pImpl->buffer.Flags = XAUDIO2_END_OF_STREAM;

    if(nullptr != m_pImpl->m_pSourceVoice)
    {
        m_pImpl->m_pSourceVoice->DestroyVoice();
        m_pImpl->m_pSourceVoice = nullptr;
    }


    //创建源声音。用来提交数据.
    HRESULT hr = m_pImpl->m_pEngine->CreateSourceVoice(&m_pImpl->m_pSourceVoice,&waveFormat, 0, 1.0f, m_pImpl->m_pCallback);
    if (FAILED(hr))
    {
        qDebug() << "CreateSourceVoice error..";
        return false;
    }

    {
        float fMatrix[2] = {0};
        m_pImpl->m_pSourceVoice->GetOutputMatrix(m_pImpl->m_pSourceVoice, 1, 2, fMatrix);
        fMatrix[0] = 1.0;
        fMatrix[1] = 0.0;
//        fMatrix[2] = 0.0;
//        fMatrix[3] = 0.0;
//        fMatrix[4] = 0.0;
//        fMatrix[5] = 0.0;
        hr = m_pImpl->m_pSourceVoice->SetOutputMatrix(m_pImpl->m_pSourceVoice,1,2, fMatrix);
        if (FAILED(hr))
        {
            qDebug() << "SetOutputMatrix error..";
            //return false;
        }
    }


    //提交内存数据
    hr = m_pImpl->m_pSourceVoice->SubmitSourceBuffer(&m_pImpl->buffer, nullptr);
    if (FAILED(hr))
    {
        qDebug() << "SubmitSourceBuffer error..";
        return false;
    }

    hr = m_pImpl->m_pSourceVoice->Start(0);

    if (FAILED(hr))
    {
        qDebug() << "Start error..";
        return false;
    }

    XAUDIO2_VOICE_STATE state;
    m_pImpl->m_pSourceVoice->GetState(&state); //获取状态
//    while (state.BuffersQueued)
//    {
//        WaitForSingleObject(voiceCallBack.hBufferEndEvent, INFINITE);
//        m_pImpl->m_pSourceVoice->GetState(&state);
//    }

    qDebug() << "end info m_pEngine|m_pMasterVoice|m_pSourceVoice|:" << m_pImpl->m_pEngine << m_pImpl->m_pMasterVoice << m_pImpl->m_pSourceVoice;

    return true;
}

bool XAudioSound::pause()
{
    if (nullptr == m_pImpl->m_pSourceVoice)
    {
        return false;
    }

    m_pImpl->m_pSourceVoice->Stop();

    return true;
}

bool XAudioSound::resume()
{
    if (nullptr == m_pImpl->m_pSourceVoice)
    {
        return false;
    }

    m_pImpl->m_pSourceVoice->Start();

    return true;
}
