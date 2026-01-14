#include "Profiler/Cupti/CuptiCallbacks.h"
#include "Driver/GPU/CuptiApi.h"

#include <algorithm>
#include <array>

namespace proton {

namespace {

constexpr std::array<CUpti_CallbackId, 22> kDriverApiLaunchCallbacks = {
    CUPTI_DRIVER_TRACE_CBID_cuLaunch,
    CUPTI_DRIVER_TRACE_CBID_cuLaunchGrid,
    CUPTI_DRIVER_TRACE_CBID_cuLaunchGridAsync,
    CUPTI_DRIVER_TRACE_CBID_cuLaunchKernel,
    CUPTI_DRIVER_TRACE_CBID_cuLaunchKernel_ptsz,
    CUPTI_DRIVER_TRACE_CBID_cuLaunchKernelEx,
    CUPTI_DRIVER_TRACE_CBID_cuLaunchKernelEx_ptsz,
    CUPTI_DRIVER_TRACE_CBID_cuLaunchCooperativeKernel,
    CUPTI_DRIVER_TRACE_CBID_cuLaunchCooperativeKernel_ptsz,
    CUPTI_DRIVER_TRACE_CBID_cuLaunchCooperativeKernelMultiDevice,
    CUPTI_DRIVER_TRACE_CBID_cuGraphLaunch,
    CUPTI_DRIVER_TRACE_CBID_cuGraphLaunch_ptsz,
    CUPTI_DRIVER_TRACE_CBID_cuStreamBeginCapture,
    CUPTI_DRIVER_TRACE_CBID_cuStreamBeginCapture_ptsz,
    CUPTI_DRIVER_TRACE_CBID_cuStreamEndCapture,
    CUPTI_DRIVER_TRACE_CBID_cuStreamEndCapture_ptsz,
    CUPTI_DRIVER_TRACE_CBID_cuStreamBeginCapture_v2,
    CUPTI_DRIVER_TRACE_CBID_cuStreamBeginCapture_v2_ptsz,
    CUPTI_DRIVER_TRACE_CBID_cuStreamBeginCaptureToGraph,
    CUPTI_DRIVER_TRACE_CBID_cuStreamBeginCaptureToGraph_ptsz,
    CUPTI_DRIVER_TRACE_CBID_cuStreamEndCapture};

constexpr std::array<CUpti_CallbackId, 11> kRuntimeApiLaunchCallbacks = {
    CUPTI_RUNTIME_TRACE_CBID_cudaLaunch_v3020,
    CUPTI_RUNTIME_TRACE_CBID_cudaLaunchKernel_v7000,
    CUPTI_RUNTIME_TRACE_CBID_cudaLaunch_ptsz_v7000,
    CUPTI_RUNTIME_TRACE_CBID_cudaLaunchKernel_ptsz_v7000,
    CUPTI_RUNTIME_TRACE_CBID_cudaLaunchKernelExC_v11060,
    CUPTI_RUNTIME_TRACE_CBID_cudaLaunchKernelExC_ptsz_v11060,
    CUPTI_RUNTIME_TRACE_CBID_cudaLaunchCooperativeKernel_v9000,
    CUPTI_RUNTIME_TRACE_CBID_cudaLaunchCooperativeKernel_ptsz_v9000,
    CUPTI_RUNTIME_TRACE_CBID_cudaLaunchCooperativeKernelMultiDevice_v9000,
    CUPTI_RUNTIME_TRACE_CBID_cudaGraphLaunch_v10000,
    CUPTI_RUNTIME_TRACE_CBID_cudaGraphLaunch_ptsz_v10000,
};

constexpr std::array<CUpti_CallbackId, 6> kGraphResourceCallbacks = {
    CUPTI_CBID_RESOURCE_GRAPHNODE_CREATED,
    CUPTI_CBID_RESOURCE_GRAPHNODE_CLONED,
    CUPTI_CBID_RESOURCE_GRAPHNODE_DESTROY_STARTING,
    CUPTI_CBID_RESOURCE_GRAPHEXEC_CREATED,
    CUPTI_CBID_RESOURCE_GRAPHEXEC_DESTROY_STARTING,
};

constexpr std::array<CUpti_CallbackId, 4> kResourceCallbacks = {
    CUPTI_CBID_RESOURCE_MODULE_LOADED,
    CUPTI_CBID_RESOURCE_MODULE_UNLOAD_STARTING,
    CUPTI_CBID_RESOURCE_CONTEXT_CREATED,
    CUPTI_CBID_RESOURCE_CONTEXT_DESTROY_STARTING,
};

constexpr std::array<CUpti_CallbackId, 2> kNvtxCallbacks = {
    CUPTI_CBID_NVTX_nvtxRangePushA,
    CUPTI_CBID_NVTX_nvtxRangePop,
};

} // namespace

void setRuntimeCallbacks(CUpti_SubscriberHandle subscriber, bool enable) {
  for (auto cbId : kRuntimeApiLaunchCallbacks) {
    cupti::enableCallback<true>(static_cast<uint32_t>(enable), subscriber,
                                CUPTI_CB_DOMAIN_RUNTIME_API, cbId);
  }
}

void setDriverCallbacks(CUpti_SubscriberHandle subscriber, bool enable) {
  for (auto cbId : kDriverApiLaunchCallbacks) {
    cupti::enableCallback<true>(static_cast<uint32_t>(enable), subscriber,
                                CUPTI_CB_DOMAIN_DRIVER_API, cbId);
  }
}

void setGraphCallbacks(CUpti_SubscriberHandle subscriber, bool enable) {
  for (auto cbId : kGraphResourceCallbacks) {
    cupti::enableCallback<true>(static_cast<uint32_t>(enable), subscriber,
                                CUPTI_CB_DOMAIN_RESOURCE, cbId);
  }
}

void setResourceCallbacks(CUpti_SubscriberHandle subscriber, bool enable) {
  for (auto cbId : kResourceCallbacks) {
    cupti::enableCallback<true>(static_cast<uint32_t>(enable), subscriber,
                                CUPTI_CB_DOMAIN_RESOURCE, cbId);
  }
}

void setNvtxCallbacks(CUpti_SubscriberHandle subscriber, bool enable) {
  for (auto cbId : kNvtxCallbacks) {
    cupti::enableCallback<true>(static_cast<uint32_t>(enable), subscriber,
                                CUPTI_CB_DOMAIN_NVTX, cbId);
  }
}

bool isDriverAPILaunch(CUpti_CallbackId cbId) {
  return std::find(kDriverApiLaunchCallbacks.begin(),
                   kDriverApiLaunchCallbacks.end(),
                   cbId) != kDriverApiLaunchCallbacks.end();
}

} // namespace proton
