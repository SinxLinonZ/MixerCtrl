#pragma once

#include <map>
#include <string>

#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <Audiopolicy.h>

#include "AudioSessionInfo.h"

#define EXIT_ON_ERROR(hr)  \
              if (FAILED(hr)) { release_devices(); return; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }


namespace MixerCtrl {

    void Tick();
    inline HRESULT hr;


    inline std::string renderDeviceName;
    inline std::string captureDeviceName;

    void init_audio_device();
    inline IMMDeviceEnumerator* pEnumerator = nullptr;
    inline IMMDevice* pRenderDevice = nullptr;
    inline IMMDevice* pCaptureDevice = nullptr;
    inline IAudioEndpointVolume* pRenderDeviceVolume = nullptr;
    inline IAudioEndpointVolume* pCaptureDeviceVolume = nullptr;
    inline IAudioMeterInformation* pRenderDeviceMeterInfo = nullptr;
    inline IAudioMeterInformation* pCaptureDeviceMeterInfo = nullptr;
    inline IAudioSessionManager2* pSessionManager = nullptr;
    inline float renderMasterPeak = 0.0f;
    inline float captureMasterPeak = 0.0f;


    void remove_exited_session();
    void update_session_list();
    inline IAudioSessionEnumerator* pSessionEnumerator = nullptr;







    void register_endpoint_notification();

    void release_devices();



    // Struct to hold the data we need to pass to EnumWindowsProc
    struct EnumWindowsData
    {
        DWORD processId;
        std::string title;
    };

    inline std::map<DWORD, AudioSessionInfo*> sessionMap;


    

    void render_ui();

    void update_session_info();

}
