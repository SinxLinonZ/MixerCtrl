#pragma once

#include <Windows.h>

namespace MixerCtrl {

    class AudioSessionInfo {
    public:
        DWORD ProcessId;
        std::string DisplayName;
        float PeakValue;

        AudioSessionInfo(DWORD processId, std::string displayName, float peakValue)
            : ProcessId(processId), DisplayName(displayName), PeakValue(peakValue){}
    };

}
