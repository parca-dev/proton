#include "Profiler/Cupti/CuptiCallbacks.h"
#include "Driver/GPU/CuptiApi.h"

#include <array>

namespace proton {

namespace {

constexpr std::array<CUpti_CallbackId, 11> kGraphCallbacks = {
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

#define PROTON_KERNEL_CALLBACK_LIST(X)                                         \
  X(CUPTI_DRIVER_TRACE_CBID_cuLaunch)                                          \
  X(CUPTI_DRIVER_TRACE_CBID_cuLaunchGrid)                                      \
  X(CUPTI_DRIVER_TRACE_CBID_cuLaunchGridAsync)                                 \
  X(CUPTI_DRIVER_TRACE_CBID_cuLaunchKernel)                                    \
  X(CUPTI_DRIVER_TRACE_CBID_cuLaunchKernel_ptsz)                               \
  X(CUPTI_DRIVER_TRACE_CBID_cuLaunchKernelEx)                                  \
  X(CUPTI_DRIVER_TRACE_CBID_cuLaunchKernelEx_ptsz)                             \
  X(CUPTI_DRIVER_TRACE_CBID_cuLaunchCooperativeKernel)                         \
  X(CUPTI_DRIVER_TRACE_CBID_cuLaunchCooperativeKernel_ptsz)                    \
  X(CUPTI_DRIVER_TRACE_CBID_cuLaunchCooperativeKernelMultiDevice)

#define PROTON_KERNEL_CB_AS_ID(cbId) cbId,
constexpr std::array<CUpti_CallbackId, 10> kKernelCallbacks = {
    PROTON_KERNEL_CALLBACK_LIST(PROTON_KERNEL_CB_AS_ID)};
#undef PROTON_KERNEL_CB_AS_ID

constexpr std::array<CUpti_CallbackId, 5> kGraphResourceCallbacks = {
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

} // namespace

void setLaunchCallbacks(CUpti_SubscriberHandle subscriber, bool enable) {
  for (auto cbId : kKernelCallbacks) {
    cupti::enableCallback<true>(static_cast<uint32_t>(enable), subscriber,
                                CUPTI_CB_DOMAIN_DRIVER_API, cbId);
  }
}

void setRuntimeCallbacks(CUpti_SubscriberHandle subscriber, bool enable) {
  for (auto cbId : kRuntimeApiLaunchCallbacks) {
    cupti::enableCallback<true>(static_cast<uint32_t>(enable), subscriber,
                                CUPTI_CB_DOMAIN_RUNTIME_API, cbId);
  }
}

void setGraphCallbacks(CUpti_SubscriberHandle subscriber, bool enable) {
  for (auto cbId : kGraphCallbacks) {
    cupti::enableCallback<true>(static_cast<uint32_t>(enable), subscriber,
                                CUPTI_CB_DOMAIN_DRIVER_API, cbId);
  }
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

bool isKernel(CUpti_CallbackId cbId) {
  switch (cbId) {
#define PROTON_KERNEL_CB_AS_CASE(cbId)                                         \
  case cbId:                                                                   \
    return true;
    PROTON_KERNEL_CALLBACK_LIST(PROTON_KERNEL_CB_AS_CASE)
#undef PROTON_KERNEL_CB_AS_CASE
  default:
    return false;
  }
}

bool isGraphLaunch(CUpti_CallbackId cbId) {
  return cbId == CUPTI_DRIVER_TRACE_CBID_cuGraphLaunch ||
         cbId == CUPTI_DRIVER_TRACE_CBID_cuGraphLaunch_ptsz;
}

bool isLaunch(CUpti_CallbackId cbId) {
  return isKernel(cbId) || isGraphLaunch(cbId);
}

#undef PROTON_KERNEL_CALLBACK_LIST

} // namespace proton
