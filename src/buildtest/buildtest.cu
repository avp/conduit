#include "buildtest.hpp"

#include <cstdio>
#include <iostream>

namespace BuildTest {
  __global__ void computeIndex(int* arr, int N) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < N) {
      arr[i] = i;
    }
  }

  #define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
  inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort=true)
  {
     if (code != cudaSuccess) 
     {
        fprintf(stderr,"GPUassert: \"%s\", at %s, line %d\n", cudaGetErrorString(code), file, line);
        if (abort) exit(code);
     }
  }

  void printCudaInfo()
  {
    // for fun, just print out some stats on the machine

    int deviceCount = 0;
    gpuErrchk(cudaGetDeviceCount(&deviceCount));

    printf("---------------------------------------------------------\n");
    printf("Found %d CUDA devices\n", deviceCount);

    for (int i=0; i<deviceCount; i++)
    {
      cudaDeviceProp deviceProps;
      gpuErrchk(cudaGetDeviceProperties(&deviceProps, i));
      printf("Device %d: %s\n", i, deviceProps.name);
      printf("   SMs:        %d\n", deviceProps.multiProcessorCount);
      printf("   Global mem: %.0f MB\n",
          static_cast<float>(deviceProps.totalGlobalMem) / (1024 * 1024));
      printf("   CUDA Cap:   %d.%d\n", deviceProps.major, deviceProps.minor);
    }
    printf("---------------------------------------------------------\n");
  }

  int runBuildTest() {
    printCudaInfo();

    std::cout << "Running the tests..." << std::endl;

    const int N = 1 << 12;
    const int THREADS_PER_BLK = 64;

    int* arr;
    gpuErrchk(cudaMalloc((void**)&arr, N * sizeof(int)));

    computeIndex<<<N/THREADS_PER_BLK, THREADS_PER_BLK>>>(arr, N);

    int localArr[N];

    gpuErrchk(cudaMemcpy(localArr, arr, N * sizeof(int), cudaMemcpyDeviceToHost));

    int numErrors = 0;
    for (int i = 0; i < N; i++) {
      if (i != localArr[i]) {
        numErrors++;
      }
    }

    if (numErrors == 0) {
      std::cout << "+++ CUDA build test passed! +++" << std::endl;
    } else {
      std::cerr << "--- FAILED CUDA BUILD TEST ---" << std::endl;
    }

    return numErrors;
  }
}
