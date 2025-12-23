#ifndef PROTON_PROFILER_CUPTI_PC_SAMPLING_UTILS_H_
#define PROTON_PROFILER_CUPTI_PC_SAMPLING_UTILS_H_

#include "Driver/GPU/CuptiApi.h"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <tuple>

namespace proton {
namespace cupti {
namespace pcsampling {

// Get CRC checksum of a CUBIN binary
inline uint64_t getCubinCrc(const char *cubin, size_t size) {
  CUpti_GetCubinCrcParams cubinCrcParams = {
      /*size=*/CUpti_GetCubinCrcParamsSize,
      /*cubinSize=*/size,
      /*cubin=*/cubin,
      /*cubinCrc=*/0,
  };
  cupti::getCubinCrc<true>(&cubinCrcParams);
  return cubinCrcParams.cubinCrc;
}

// Get the number of stall reasons supported by the GPU
inline size_t getNumStallReasons(CUcontext context) {
  size_t numStallReasons = 0;
  CUpti_PCSamplingGetNumStallReasonsParams numStallReasonsParams = {
      /*size=*/CUpti_PCSamplingGetNumStallReasonsParamsSize,
      /*pPriv=*/NULL,
      /*ctx=*/context,
      /*numStallReasons=*/&numStallReasons};
  cupti::pcSamplingGetNumStallReasons<true>(&numStallReasonsParams);
  return numStallReasons;
}

// Get SASS to source correlation (line number and file name) for a PC offset
// Note: This function frees the CUPTI-allocated fileName and dirName after
// copying them to std::string, so the caller doesn't need to free anything.
// Set checkErrors=false if line mapping may not be available in the cubin.
template <bool checkErrors = true>
inline std::tuple<uint32_t, std::string, std::string>
getSassToSourceCorrelation(const char *functionName, uint64_t pcOffset,
                           const char *cubin, size_t cubinSize) {
  CUpti_GetSassToSourceCorrelationParams sassToSourceParams = {
      /*size=*/CUpti_GetSassToSourceCorrelationParamsSize,
      /*cubin=*/cubin,
      /*functionName=*/functionName,
      /*cubinSize=*/cubinSize,
      /*lineNumber=*/0,
      /*pcOffset=*/pcOffset,
      /*fileName=*/NULL,
      /*dirName=*/NULL,
  };
  cupti::getSassToSourceCorrelation<checkErrors>(&sassToSourceParams);
  std::string fileName =
      sassToSourceParams.fileName ? sassToSourceParams.fileName : "";
  std::string dirName =
      sassToSourceParams.dirName ? sassToSourceParams.dirName : "";
  // Free CUPTI-allocated memory
  if (sassToSourceParams.fileName)
    std::free(sassToSourceParams.fileName);
  if (sassToSourceParams.dirName)
    std::free(sassToSourceParams.dirName);
  return {sassToSourceParams.lineNumber, fileName, dirName};
}

// Allocate PC sampling data buffer
inline CUpti_PCSamplingData allocPCSamplingData(size_t collectNumPCs,
                                                size_t numValidStallReasons) {
  CUpti_PCSamplingData pcSamplingData{
      /*size=*/sizeof(CUpti_PCSamplingData),
      /*collectNumPcs=*/collectNumPCs,
      /*totalSamples=*/0,
      /*droppedSamples=*/0,
      /*totalNumPcs=*/0,
      /*remainingNumPcs=*/0,
      /*rangeId=*/0,
      /*pPcData=*/
      static_cast<CUpti_PCSamplingPCData *>(
          std::calloc(collectNumPCs, sizeof(CUpti_PCSamplingPCData)))};
  for (size_t i = 0; i < collectNumPCs; ++i) {
    pcSamplingData.pPcData[i].stallReason =
        static_cast<CUpti_PCSamplingStallReason *>(std::calloc(
            numValidStallReasons, sizeof(CUpti_PCSamplingStallReason)));
  }
  return pcSamplingData;
}

// Enable PC sampling for a context
inline void enablePCSampling(CUcontext context) {
  CUpti_PCSamplingEnableParams params = {
      /*size=*/CUpti_PCSamplingEnableParamsSize,
      /*pPriv=*/NULL,
      /*ctx=*/context,
  };
  cupti::pcSamplingEnable<true>(&params);
}

// Start PC sampling
inline void startPCSampling(CUcontext context) {
  CUpti_PCSamplingStartParams params = {
      /*size=*/CUpti_PCSamplingStartParamsSize,
      /*pPriv=*/NULL,
      /*ctx=*/context,
  };
  cupti::pcSamplingStart<true>(&params);
}

// Stop PC sampling
inline void stopPCSampling(CUcontext context) {
  CUpti_PCSamplingStopParams params = {
      /*size=*/CUpti_PCSamplingStopParamsSize,
      /*pPriv=*/NULL,
      /*ctx=*/context,
  };
  cupti::pcSamplingStop<true>(&params);
}

// Get PC sampling data
inline void getPCSamplingData(CUcontext context,
                              CUpti_PCSamplingData *pcSamplingData) {
  CUpti_PCSamplingGetDataParams params = {
      /*size=*/CUpti_PCSamplingGetDataParamsSize,
      /*pPriv=*/NULL,
      /*ctx=*/context,
      /*pcSamplingData=*/pcSamplingData,
  };
  cupti::pcSamplingGetData<true>(&params);
}

// Set configuration attributes
inline void setConfigurationAttribute(
    CUcontext context,
    std::vector<CUpti_PCSamplingConfigurationInfo> &configurationInfos) {
  CUpti_PCSamplingConfigurationInfoParams infoParams = {
      /*size=*/CUpti_PCSamplingConfigurationInfoParamsSize,
      /*pPriv=*/NULL,
      /*ctx=*/context,
      /*numAttributes=*/configurationInfos.size(),
      /*pPCSamplingConfigurationInfo=*/configurationInfos.data(),
  };
  cupti::pcSamplingSetConfigurationAttribute<true>(&infoParams);
}

} // namespace pcsampling
} // namespace cupti
} // namespace proton

#endif // PROTON_PROFILER_CUPTI_PC_SAMPLING_UTILS_H_
