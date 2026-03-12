#ifndef PROTON_PROFILER_CUPTI_CALLBACKS_H_
#define PROTON_PROFILER_CUPTI_CALLBACKS_H_

#include <cupti.h>

namespace proton {

// Enable/disable Driver API callbacks for kernel launches
void setLaunchCallbacks(CUpti_SubscriberHandle subscriber, bool enable);

// Enable/disable Runtime API callbacks for kernel launches (cudaLaunchKernel, etc.)
void setRuntimeCallbacks(CUpti_SubscriberHandle subscriber, bool enable);

// Enable/disable Resource callbacks (module load/unload, context create/destroy)
void setResourceCallbacks(CUpti_SubscriberHandle subscriber, bool enable);

// Enable/disable Graph callbacks (graph launch, capture, resource)
void setGraphCallbacks(CUpti_SubscriberHandle subscriber, bool enable);

// Enable/disable NVTX callbacks
void setNvtxCallbacks(CUpti_SubscriberHandle subscriber, bool enable);

// Check if a callback ID is a kernel launch
bool isKernel(CUpti_CallbackId cbId);

// Check if a callback ID is a graph launch
bool isGraphLaunch(CUpti_CallbackId cbId);

// Check if a callback ID is any kind of launch (kernel or graph)
bool isLaunch(CUpti_CallbackId cbId);

} // namespace proton

#endif // PROTON_PROFILER_CUPTI_CALLBACKS_H_
