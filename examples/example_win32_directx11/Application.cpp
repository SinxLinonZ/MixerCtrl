#include <iostream>
#include <atlstr.h>
// #include <psapi.h>

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

    void InitAudioDevice() {

        // Get enumerator for audio endpoint devices.
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
            NULL, CLSCTX_INPROC_SERVER,
            __uuidof(IMMDeviceEnumerator),
            (void**)&pEnumerator);
        EXIT_ON_ERROR(hr);

        // Get peak meter for default audio-rendering device.
        hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
        EXIT_ON_ERROR(hr);

        hr = pDevice->Activate(__uuidof(IAudioMeterInformation),
            CLSCTX_ALL, NULL, (void**)&pMeterInfo);
        EXIT_ON_ERROR(hr);

        hr = pDevice->Activate(__uuidof(IAudioSessionManager2),
            CLSCTX_ALL, NULL, (void**)&pSessionManager);
        EXIT_ON_ERROR(hr);

        return;

    Exit:
        if (FAILED(hr))
        {
            //MessageBox(NULL, TEXT("This program requires Windows Vista."),
                //TEXT("Error termination"), MB_OK);
        }
        SAFE_RELEASE(pEnumerator);
        SAFE_RELEASE(pDevice);
        SAFE_RELEASE(pMeterInfo);
        return;
    }

    void UpdateSession() {

        std::vector<DWORD> eraseList;

        hr = pSessionManager->GetSessionEnumerator(&pSessionEnumerator);
        EXIT_ON_ERROR(hr);

        int sessionCount;
        hr = pSessionEnumerator->GetCount(&sessionCount);
        EXIT_ON_ERROR(hr);

        // Remove exited session
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


        for (int i = 0; i < sessionCount; i++) {
            IAudioSessionControl* session;

            IAudioSessionControl2* session2;
            IAudioMeterInformation* audioMeterInformation;

            hr = pSessionEnumerator->GetSession(i, &session);
            EXIT_ON_ERROR(hr);

            hr = session->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&session2);
            EXIT_ON_ERROR(hr);
            hr = session->QueryInterface(__uuidof(IAudioMeterInformation), (void**)&audioMeterInformation);
            EXIT_ON_ERROR(hr);


            DWORD processId;
            hr = session2->GetProcessId(&processId);
            EXIT_ON_ERROR(hr);
            //if (processId == 0) continue;

            std::string mainWindowTitle;
            if (processId == 0) {
                mainWindowTitle = "System";
            }
            else {

                //HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);

                //if (hProcess)
                //{
                //    WCHAR wszImagePath[MAX_PATH];
                //    DWORD dwSize = sizeof(wszImagePath);
                //    if (GetProcessImageFileNameW(hProcess, wszImagePath, dwSize))
                //    {
                //        // WCHAR wszProgramName[MAX_PATH];
                //        auto wszProgramName = PathFindFileNameW(wszImagePath);
                //        std::wcout << L"Program name: " << wszProgramName << std::endl;
                //        mainWindowTitle = CW2A(wszProgramName);
                //    }
                //    else
                //    {
                //        std::cerr << "Error: GetProcessImageFileNameW failed" << std::endl;
                //    }
                //    CloseHandle(hProcess);
                //}
                //else
                //{
                //    std::cerr << "Error: OpenProcess failed" << std::endl;
                //}

                HANDLE handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
                if (handle)
                {
                    EnumWindowsData data = { processId, "" };
                    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&data));

                    if (data.title[0] == '\0') {
                        DWORD buffSize = 1024;
                        CHAR buffer[1024];
                        if (QueryFullProcessImageNameA(handle, 0, buffer, &buffSize))
                        {
                            mainWindowTitle = buffer;
                        }
                        CloseHandle(handle);
                    }
                    else {
                        mainWindowTitle = data.title;
                    }

                }
            }

            float _peak;
            hr = audioMeterInformation->GetPeakValue(&_peak);
            EXIT_ON_ERROR(hr);

            if (sessionMap[processId] == nullptr) {
                sessionMap[processId] = new AudioSessionInfo{
                        processId, mainWindowTitle, 0.0f
                };
            }

            sessionMap[processId]->DisplayName = mainWindowTitle;
            sessionMap[processId]->PeakValue = _peak;

            SAFE_RELEASE(audioMeterInformation);
            SAFE_RELEASE(session2);
            SAFE_RELEASE(session);
        }

        SAFE_RELEASE(pSessionEnumerator);
        return;

    Exit:
        if (FAILED(hr))
        {
            //MessageBox(NULL, TEXT("This program requires Windows Vista."),
                //TEXT("Error termination"), MB_OK);
        }
        SAFE_RELEASE(pEnumerator);
        SAFE_RELEASE(pDevice);
        SAFE_RELEASE(pMeterInfo);
        return;
    }

    void RenderUI() {
        ImGui::ShowDemoWindow(nullptr);

        hr = pMeterInfo->GetPeakValue(&masterPeak);

        ImGui::Begin("Meter");
        //char buf[32];
        //sprintf(buf, "%d/%d", (int)(peak * 100), 100);
        //ImGui::ProgressBar(peak, ImVec2(0.f, 0.f), buf);
        ImGui::ProgressBar(masterPeak, ImVec2(0.f, 0.f), "");
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::Text("Master");

        //ImDrawList* draw_list = ImGui::GetWindowDrawList();
        //ImVec2 gradient_size = ImVec2(ImGui::CalcItemWidth(), ImGui::GetFrameHeight());
        {

            /*
            ImVec2 p0 = ImGui::GetCursorScreenPos();
            ImVec2 p1 = ImVec2(p0.x + 5.0f, p0.y + 100.0f);
            draw_list->AddRectFilled(p0, p1, ImGui::GetColorU32(IM_COL32(100, 100, 100, 255)));
            ImGui::InvisibleButton("##gradient1", ImVec2(5.0f, 100.0f));

            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

            p0 = ImGui::GetCursorScreenPos();
            p1 = ImVec2(p0.x + 5.0f, p0.y + 100.0f);
            draw_list->AddRectFilled(p0, p1, ImGui::GetColorU32(IM_COL32(100, 100, 100, 255)));
            ImGui::InvisibleButton("##gradient1", ImVec2(5.0f, 100.0f));
            */

            //ImU32 col_a = ImGui::GetColorU32(IM_COL32(0, 0, 0, 255));
            //ImU32 col_b = ImGui::GetColorU32(IM_COL32(255, 255, 255, 255));

            //draw_list->AddRectFilledMultiColor(p0, p1, col_a, col_b, col_b, col_a);
            //ImGui::InvisibleButton("##gradient1", gradient_size);

        }


        //for (const auto& session : sessionMap) {
        //    ImGui::ProgressBar(session.second->PeakValue, ImVec2(0.0f, 0.0f));
        //    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        //    ImGui::Text(session.second->DisplayName.c_str());
        //}

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        int sessionCount = sessionMap.size();
        auto sessionIter = sessionMap.begin();
        float METER_HEIGHT = 120.0f;
        if (ImGui::BeginTable("table1", sessionCount,
            ImGuiTableFlags_SizingFixedSame | ImGuiTableFlags_NoHostExtendX
            //| ImGuiTableFlags_Resizable
            //| ImGuiTableFlags_NoSavedSettings
        ))
        {
            for (int column = 0; column < sessionCount; column++) {
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            }

            ImGui::TableNextRow();
            for (int column = 0; column < sessionCount; column++)
            {
                ImGui::TableSetColumnIndex(column);
                const AudioSessionInfo* session = sessionIter->second;

                ImVec2 start = ImGui::GetCursorScreenPos();
                ImVec2 p0, p1;
                p0.x = start.x;
                p1.x = start.x + 5.0f;

                //p0.y = start.y;
                //p1.y = start.y + METER_HEIGHT;
                //draw_list->AddRectFilled(p0, p1, IM_COL32(0, 0, 0, 255));

                p0.y = start.y;
                p1.y = start.y + METER_HEIGHT / 6;
                draw_list->AddRectFilled(p0, p1, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
                p0.y = p1.y;
                p1.y += METER_HEIGHT / 6;
                draw_list->AddRectFilled(p0, p1, ImGui::GetColorU32(IM_COL32(255, 255, 0, 255)));
                p0.y = p1.y;
                p1.y = start.y + METER_HEIGHT;
                draw_list->AddRectFilled(p0, p1, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));

                //p0.y = start.y + METER_HEIGHT - session->PeakValue * METER_HEIGHT;
                //p1.y = start.y + METER_HEIGHT;
                //draw_list->AddRectFilled(p0, p1, IM_COL32(255, 255, 255, 255));

                p0.y = start.y;
                p1.y = start.y + METER_HEIGHT - session->PeakValue * METER_HEIGHT;
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

                ++sessionIter;
            }

            ImGui::TableNextRow();
            sessionIter = sessionMap.begin();
            for (int column = 0; column < sessionCount; column++)
            {
                ImGui::TableSetColumnIndex(column);
                const AudioSessionInfo* session = sessionIter->second;

                ImGui::TextWrapped(std::to_string(session->PeakValue * 100).c_str());
                ++sessionIter;
            }


            ImGui::TableNextRow();
            sessionIter = sessionMap.begin();
            for (int column = 0; column < sessionCount; column++)
            {
                ImGui::TableSetColumnIndex(column);
                const AudioSessionInfo* session = sessionIter->second;

                ImGui::TextWrapped(session->DisplayName.c_str());
                ++sessionIter;
            }

            ImGui::EndTable();
        }


        ImGui::End();
    }

}
