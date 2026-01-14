#ifndef PROTON_PROFILER_CUPTI_CALLBACKS_H_
#define PROTON_PROFILER_CUPTI_CALLBACKS_H_

#include <cupti.h>

namespace proton {

// Enable/disable Runtime API callbacks for kernel launches
void setRuntimeCallbacks(CUpti_SubscriberHandle subscriber, bool enable);

// Enable/disable Driver API callbacks for kernel launches
void setDriverCallbacks(CUpti_SubscriberHandle subscriber, bool enable);

// Enable/disable Resource callbacks (module load/unload, context create/destroy)
void setResourceCallbacks(CUpti_SubscriberHandle subscriber, bool enable);

// Enable/disable Graph resource callbacks
void setGraphCallbacks(CUpti_SubscriberHandle subscriber, bool enable);

// Enable/disable NVTX callbacks
void setNvtxCallbacks(CUpti_SubscriberHandle subscriber, bool enable);

// Check if a callback ID is a Driver API launch
bool isDriverAPILaunch(CUpti_CallbackId cbId);

} // namespace proton

#endif // PROTON_PROFILER_CUPTI_CALLBACKS_H_
