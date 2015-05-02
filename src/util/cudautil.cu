#include "cudautil.hpp"

#include <cuda_runtime.h>

bool CudaUtil::hasCuda() {
  int deviceCount, device;
  int gpuDeviceCount = 0;
  struct cudaDeviceProp properties;
  cudaError_t cudaResultCode = cudaGetDeviceCount(&deviceCount);
  if (cudaResultCode != cudaSuccess) {
    deviceCount = 0;
  }
  // Machines with no GPUs can still report one emulation device
  for (device = 0; device < deviceCount; device++) {
    cudaGetDeviceProperties(&properties, device);
    if (properties.major != 9999) {
      // 9999 means emulation only
      gpuDeviceCount++;
    }
  }
  return gpuDeviceCount > 0;
}
