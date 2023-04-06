#pragma once

#include <map>
#include <string>

#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <Audiopolicy.h>

#include "AudioSessionInfo.h"

#define EXIT_ON_ERROR(hr)  \
              if (FAILED(hr)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }


namespace MixerCtrl {

    // Struct to hold the data we need to pass to EnumWindowsProc
    struct EnumWindowsData
    {
        DWORD processId;
        std::string title;
    };

    static std::map<DWORD, AudioSessionInfo*> sessionMap;

    static HRESULT hr;
    static IMMDeviceEnumerator* pEnumerator = NULL;
    static IMMDevice* pDevice = NULL;
    static IAudioSessionManager2* pSessionManager = NULL;
    static IAudioSessionEnumerator* pSessionEnumerator = NULL;

    static IAudioMeterInformation* pMeterInfo = NULL;
    static float masterPeak = 0.0f;

    void RenderUI();

    void InitAudioDevice();
    void UpdateSession();
}
