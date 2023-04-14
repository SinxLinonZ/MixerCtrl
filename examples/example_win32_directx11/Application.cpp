#include <iostream>
#include <atlstr.h>
//#include <psapi.h>
#include <propkeydef.h>
#include <Functiondiscoverykeys_devpkey.h>

#include <winver.h>
#pragma comment(lib,"Version.lib")

// #include <shlwapi.h>
// #pragma comment(lib, "shlwapi.lib") // link with shlwapi.lib

#include "imgui.h"
#include "Application.h"
#include <vector>




namespace MixerCtrl {

    // Callback function for EnumWindows
    BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
    {
        EnumWindowsData* pData = reinterpret_cast<EnumWindowsData*>(lParam);
        DWORD processId = 0;
        GetWindowThreadProcessId(hwnd, &processId);
        if (processId == pData->processId)
        {
            wchar_t titleW[256];
            GetWindowTextW(hwnd, titleW, 256);
            int len = WideCharToMultiByte(CP_UTF8, 0, titleW, -1, NULL, 0, NULL, NULL);
            char* title = new char[len];
            WideCharToMultiByte(CP_UTF8, 0, titleW, -1, title, len, NULL, NULL);
            pData->title = title;
            delete[] title;

            if (pData->title[0] != '\0') return FALSE;
        }
        return TRUE;
    }

    std::string get_file_description(const wchar_t* filePath)
    {
        int versionInfoSize = GetFileVersionInfoSize(filePath, NULL);
        if (!versionInfoSize) {
            std::cerr << "Error: GetFileVersionInfoSize failed with error " << GetLastError() << std::endl;
            return "";
        }

        auto versionInfo = new BYTE[versionInfoSize];
        std::unique_ptr<BYTE[]> versionInfo_automatic_cleanup(versionInfo);
        if (!GetFileVersionInfo(filePath, NULL, versionInfoSize, versionInfo)) {
            std::cerr << "Error: GetFileVersionInfo failed with error " << GetLastError() << std::endl;
            delete[] versionInfo;
            return "";
        }

        struct LANGANDCODEPAGE {
            WORD wLanguage;
            WORD wCodePage;
        } *translationArray;

        UINT translationArrayByteLength = 0;
        if (!VerQueryValue(versionInfo, L"\\VarFileInfo\\Translation", (LPVOID*)&translationArray, &translationArrayByteLength)) {
            std::cerr << "Error: VerQueryValue failed with error " << GetLastError() << std::endl;
            delete[] versionInfo;
            return "";
        }

        for (unsigned int i = 0; i < (translationArrayByteLength / sizeof(LANGANDCODEPAGE)); i++) {
            wchar_t fileDescriptionKey[256];
            wsprintf(
                fileDescriptionKey,
                L"\\StringFileInfo\\%04x%04x\\FileDescription",
                translationArray[i].wLanguage,
                translationArray[i].wCodePage
            );

            wchar_t* fileDescription = NULL;
            UINT fileDescriptionSize;
            if (VerQueryValue(versionInfo, fileDescriptionKey, (LPVOID*)&fileDescription, &fileDescriptionSize)) {
                int len = WideCharToMultiByte(CP_UTF8, 0, fileDescription, -1, NULL, 0, NULL, NULL);
                char* _description = new char[len];
                WideCharToMultiByte(CP_UTF8, 0, fileDescription, -1, _description, len, NULL, NULL);

                return _description;
            }
        }

        return "";
    }

    void release_devices() {
        SAFE_RELEASE(pEnumerator);
        SAFE_RELEASE(pRenderDevice);
        SAFE_RELEASE(pCaptureDevice);
        SAFE_RELEASE(pRenderDeviceMeterInfo);
        SAFE_RELEASE(pCaptureDeviceMeterInfo);
        SAFE_RELEASE(pRenderDeviceVolume);
        SAFE_RELEASE(pCaptureDeviceVolume);

        SAFE_RELEASE(pSessionManager);
    }





    void register_endpoint_notification() {
        //hr = pEnumerator->RegisterEndpointNotificationCallback(this);
        //EXIT_ON_ERROR(hr);

    }





    void init_audio_device() {
        // Get enumerator for audio endpoint devices.
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
            NULL, CLSCTX_INPROC_SERVER,
            __uuidof(IMMDeviceEnumerator),
            (void**)&pEnumerator);
        EXIT_ON_ERROR(hr);

        hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pRenderDevice);
        EXIT_ON_ERROR(hr);
        hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pCaptureDevice);
        EXIT_ON_ERROR(hr);

        // Meter information
        hr = pRenderDevice->Activate(__uuidof(IAudioMeterInformation),
            CLSCTX_ALL, NULL, (void**)&pRenderDeviceMeterInfo);
        EXIT_ON_ERROR(hr);
        hr = pCaptureDevice->Activate(__uuidof(IAudioMeterInformation),
            CLSCTX_ALL, NULL, (void**)&pCaptureDeviceMeterInfo);
        EXIT_ON_ERROR(hr);

        // Volume control
        hr = pRenderDevice->Activate(__uuidof(IAudioEndpointVolume),
            CLSCTX_ALL, NULL, (void**)&pRenderDeviceVolume);
        EXIT_ON_ERROR(hr);
        hr = pCaptureDevice->Activate(__uuidof(IAudioEndpointVolume),
            CLSCTX_ALL, NULL, (void**)&pCaptureDeviceVolume);
        EXIT_ON_ERROR(hr);

        // Session control
        hr = pRenderDevice->Activate(__uuidof(IAudioSessionManager2),
            CLSCTX_ALL, NULL, (void**)&pSessionManager);
        EXIT_ON_ERROR(hr);

        // Get device name
        wchar_t* renderDeviceId;
        wchar_t* captureDeviceId;
        hr = pRenderDevice->GetId(&renderDeviceId);
        EXIT_ON_ERROR(hr);
        hr = pCaptureDevice->GetId(&captureDeviceId);
        EXIT_ON_ERROR(hr);

        IPropertyStore* pRenderProps = NULL;
        IPropertyStore* pCaptureProps = NULL;
        hr = pRenderDevice->OpenPropertyStore(STGM_READ, &pRenderProps);
        EXIT_ON_ERROR(hr);
        hr = pCaptureDevice->OpenPropertyStore(STGM_READ, &pCaptureProps);
        EXIT_ON_ERROR(hr);

        PROPVARIANT varName;
        PropVariantInit(&varName);
        hr = pRenderProps->GetValue(PKEY_Device_FriendlyName, &varName);
        EXIT_ON_ERROR(hr);
        renderDeviceName = CW2A(varName.pwszVal);
        PropVariantClear(&varName);

        hr = pCaptureProps->GetValue(PKEY_Device_FriendlyName, &varName);
        EXIT_ON_ERROR(hr);
        captureDeviceName = CW2A(varName.pwszVal);
        PropVariantClear(&varName);

        SAFE_RELEASE(pRenderProps);
        SAFE_RELEASE(pCaptureProps);

        // Get device icon
        // TODO
    }

    void Tick() {

        hr = pSessionManager->GetSessionEnumerator(&pSessionEnumerator);
        EXIT_ON_ERROR(hr);
        remove_exited_session();
        update_session_info();
        SAFE_RELEASE(pSessionEnumerator);

        // Update master volume
        hr = pRenderDeviceMeterInfo->GetPeakValue(&renderMasterPeak);
        EXIT_ON_ERROR(hr);
        hr = pCaptureDeviceMeterInfo->GetPeakValue(&captureMasterPeak);
        EXIT_ON_ERROR(hr);

        render_ui();
    }

    void remove_exited_session() {
        std::vector<DWORD> eraseList;

        for (auto& audioSession : sessionMap) {
            if (audioSession.first == 0) continue;
            HANDLE handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, audioSession.first);
            DWORD lpExitCode;
            GetExitCodeProcess(handle, &lpExitCode);

            if (lpExitCode != STILL_ACTIVE) {
                delete audioSession.second;
                eraseList.push_back(audioSession.first);
            }
        }
        for (auto& processId : eraseList) {
            sessionMap.erase(processId);
        }
    }

    void update_session_info() {
        int sessionCount;
        hr = pSessionEnumerator->GetCount(&sessionCount);
        EXIT_ON_ERROR(hr);

        for (int i = 0; i < sessionCount; i++) {
            IAudioSessionControl* session = nullptr;

            IAudioSessionControl2* session2 = nullptr;
            IAudioMeterInformation* audioMeterInformation = nullptr;
            ISimpleAudioVolume* simpleAudioVolume = nullptr;

            hr = pSessionEnumerator->GetSession(i, &session);
            EXIT_ON_ERROR(hr);

            hr = session->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&session2);
            EXIT_ON_ERROR(hr);
            hr = session->QueryInterface(__uuidof(IAudioMeterInformation), (void**)&audioMeterInformation);
            EXIT_ON_ERROR(hr);
            hr = session->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&simpleAudioVolume);
            EXIT_ON_ERROR(hr);


            DWORD processId;
            hr = session2->GetProcessId(&processId);
            EXIT_ON_ERROR(hr);

            std::string mainWindowTitle;
            if (processId == 0) {
                mainWindowTitle = "System Sounds";
            }
            else {
                HANDLE handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
                if (handle)
                {
                    EnumWindowsData data = { processId, "" };
                    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&data));

                    if (data.title[0] != '\0') {
                        mainWindowTitle = data.title;
                    }
                    else {
                        DWORD buffSize = 1024;
                        char buffer[1024];
                        if (QueryFullProcessImageNameA(handle, 0, buffer, &buffSize))
                        {
                            wchar_t buffer_wchar[1024];
                            mbstowcs_s(NULL, buffer_wchar, buffer, 1024);
                            mainWindowTitle = get_file_description(buffer_wchar);
                        }
                        CloseHandle(handle);
                    }
                }
            }

            float _peak;
            hr = audioMeterInformation->GetPeakValue(&_peak);
            EXIT_ON_ERROR(hr);

            float _volume;
            hr = simpleAudioVolume->GetMasterVolume(&_volume);
            EXIT_ON_ERROR(hr);

            // Insert to session list if not is new session
            if (sessionMap[processId] == nullptr) {
                sessionMap[processId] = new AudioSessionInfo(processId, mainWindowTitle, 0.0f, 0.0f);
            }

            sessionMap[processId]->DisplayName = mainWindowTitle;
            sessionMap[processId]->OriginPeakValue = _peak;
            sessionMap[processId]->Volume = _volume;

            SAFE_RELEASE(audioMeterInformation);
            SAFE_RELEASE(simpleAudioVolume);
            SAFE_RELEASE(session2);
            SAFE_RELEASE(session);
        }
    }

    void render_ui() {
        ImGui::ShowDemoWindow(nullptr);

        ImGui::Begin("Meter");


        ImGui::Text("Playback: ");
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::Text(renderDeviceName.c_str());

        ImGui::ProgressBar(renderMasterPeak, ImVec2(0.f, 0.f), "");
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::Text("Master peak");
        ImGui::SliderFloat("##Capture volume", &f1, 0.0f, 1.0f, "Volume: %.3f");






        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        int sessionCount = sessionMap.size();
        auto sessionIter = sessionMap.begin();
        float METER_HEIGHT = 120.0f;

        for (int column = 0; column < sessionCount; column++)
        {
            const AudioSessionInfo* session = sessionIter->second;

            if (ImGui::TreeNode((void*)(intptr_t)session->ProcessId, session->DisplayName.c_str()))
            {
                // Meter bar
                ImVec2 start = ImGui::GetCursorScreenPos();
                ImVec2 p0, p1;
                p0.x = start.x;
                p1.x = start.x + 5.0f;

                p0.y = start.y;
                p1.y = start.y + METER_HEIGHT / 6;
                draw_list->AddRectFilled(p0, p1, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
                p0.y = p1.y;
                p1.y += METER_HEIGHT / 6;
                draw_list->AddRectFilled(p0, p1, ImGui::GetColorU32(IM_COL32(255, 255, 0, 255)));
                p0.y = p1.y;
                p1.y = start.y + METER_HEIGHT;
                draw_list->AddRectFilled(p0, p1, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));

                p0.y = start.y;
                p1.y = start.y + METER_HEIGHT - session->OriginPeakValue * METER_HEIGHT;
                draw_list->AddRectFilled(p0, p1, IM_COL32(0, 0, 0, 255));

                //p1.x = start.x + 1.0f;
                p0.x = start.x; p0.y = start.y;
                p1.y = start.y + METER_HEIGHT / 6;
                draw_list->AddRectFilled(p0, p1, ImGui::GetColorU32(IM_COL32(127, 0, 0, 128)));
                p0.y = p1.y;
                p1.y += METER_HEIGHT / 6;
                draw_list->AddRectFilled(p0, p1, ImGui::GetColorU32(IM_COL32(127, 127, 0, 128)));
                p0.y = p1.y;
                p1.y = start.y + METER_HEIGHT;
                draw_list->AddRectFilled(p0, p1, ImGui::GetColorU32(IM_COL32(0, 127, 0, 128)));

                ImGui::InvisibleButton("##gradient1", ImVec2(5.0f, METER_HEIGHT));
                // End of meter bar

                ImGui::TreePop();
            }

            ++sessionIter;
        }

        ImGui::NewLine();
        ImGui::Text("Capture: ");
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::Text(captureDeviceName.c_str());

        ImGui::ProgressBar(captureMasterPeak, ImVec2(0.f, 0.f), "");
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::Text("Master peak");
        ImGui::SliderFloat("##Capture volume", &f1, 0.0f, 1.0f, "Volume: %.3f");

        ImGui::End();
    }

}
