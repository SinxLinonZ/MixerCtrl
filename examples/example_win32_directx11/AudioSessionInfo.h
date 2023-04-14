#pragma once

#include <Windows.h>

namespace MixerCtrl {

    struct AudioSessionInfo {
    public:
        AudioSessionInfo() = delete;

        AudioSessionInfo(DWORD processId, std::string displayName, float peakValue, float volume)
            : ProcessId(processId), DisplayName(displayName), OriginPeakValue(peakValue), Volume(volume) {}

        DWORD ProcessId;
        std::string DisplayName;
        float OriginPeakValue;
        float Volume;
    };

}
