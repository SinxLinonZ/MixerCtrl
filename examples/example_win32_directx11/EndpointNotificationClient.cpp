#include "EndpointNotificationClient.h"

namespace MixerCtrl {
    EndpointNotificationClient::EndpointNotificationClient() : _cRef(1) { };
    EndpointNotificationClient::~EndpointNotificationClient() { };

    ULONG EndpointNotificationClient::AddRef() { return InterlockedIncrement(&_cRef); };
    ULONG EndpointNotificationClient::Release() {
        ULONG ulRef = InterlockedDecrement(&_cRef);
        if (0 == _cRef) {
            delete this;
        }
        return ulRef;
    };

    HRESULT EndpointNotificationClient::QueryInterface(REFIID riid, VOID** ppvInterface) {
        if (riid == IID_IUnknown) {
            AddRef();
            *ppvInterface = (IUnknown*)this;
        }
        else if (riid == __uuidof(IMMNotificationClient)) {
            AddRef();
            *ppvInterface = (IMMNotificationClient*)this;
        }
        else {
            *ppvInterface = NULL;
            return E_NOINTERFACE;
        }
        return S_OK;
    };

    HRESULT EndpointNotificationClient::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId) {
        // Handle the default audio endpoint device for a particular role changing.
        return S_OK;
    };

    HRESULT EndpointNotificationClient::OnDeviceAdded(LPCWSTR pwstrDeviceId) {
        // Handle an audio endpoint device being added to the system.
        return S_OK;
    }
    HRESULT EndpointNotificationClient::OnDeviceRemoved(LPCWSTR pwstrDeviceId) {
        // Handle an audio endpoint device being removed from the system.
        return S_OK;
    }
    HRESULT EndpointNotificationClient::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) {
        // Handle the state of an audio endpoint device changing.
        return S_OK;
    }
    HRESULT EndpointNotificationClient::OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key) {
        // Handle a property value changing on an audio endpoint device.
        return S_OK;
    }
}
