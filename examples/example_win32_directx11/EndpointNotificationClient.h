#pragma once
#include <Windows.h>
#include <mmdeviceapi.h>

namespace MixerCtrl {

    class EndpointNotificationClient : public IMMNotificationClient {
    public:
        EndpointNotificationClient();
        ~EndpointNotificationClient();

        // IUnknown methods -- AddRef, Release, and QueryInterface
        ULONG STDMETHODCALLTYPE AddRef();
        ULONG STDMETHODCALLTYPE Release();
        HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID riid,
            VOID** ppvInterface
        );
        // Callback methods for device-event notifications.
        HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(
            EDataFlow flow,
            ERole role,
            LPCWSTR pwstrDeviceId
        );
        HRESULT STDMETHODCALLTYPE OnDeviceAdded(
            LPCWSTR pwstrDeviceId
        );
        HRESULT STDMETHODCALLTYPE OnDeviceRemoved(
            LPCWSTR pwstrDeviceId
        );
        HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(
            LPCWSTR pwstrDeviceId,
            DWORD dwNewState
        );
        HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(
            LPCWSTR pwstrDeviceId,
            const PROPERTYKEY key
        );
    private:
        ULONG _cRef;
    };

}
