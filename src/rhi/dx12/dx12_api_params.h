#ifndef __dx12_api_params_h__
#define __dx12_api_params_h__

// TODO: READ FORM FILE
constexpr int kDeviceMaxRTVHeapDescriptors = 3;
constexpr int kSwapChainBufferCount = std::min(3, kDeviceMaxRTVHeapDescriptors);

#endif