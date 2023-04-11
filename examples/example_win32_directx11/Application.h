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

    static HRESULT hr;
    static IMMDeviceEnumerator* pEnumerator = NULL;

    void init_audio_device();
    void register_endpoint_notification();

    void release_devices();



    // Struct to hold the data we need to pass to EnumWindowsProc
    struct EnumWindowsData
    {
        DWORD processId;
        std::string title;
    };

    static std::map<DWORD, AudioSessionInfo*> sessionMap;

    static IMMDevice* pRenderDevice = NULL;
    static IMMDevice* pCaptureDevice = NULL;

    static IAudioSessionManager2* pSessionManager = NULL;
    static IAudioSessionEnumerator* pSessionEnumerator = NULL;

    static IAudioMeterInformation* pRenderDeviceMeterInfo = NULL;
    static IAudioMeterInformation* pCaptureDeviceMeterInfo = NULL;
    static float renderMasterPeak = 0.0f;

    void render_ui();

    
    void update_sessions();

    void Tick();
}
