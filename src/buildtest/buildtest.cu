#include "buildtest.h"

#include <iostream>

namespace BuildTest {
  __global__ void computeIndex(int* arr, int N) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < N) {
      arr[i] = i;
    }
  }

  int runBuildTest() {
    std::cout << "Running the tests..." << std::endl;

    const int N = 1 << 12;
    const int THREADS_PER_BLK = 64;

    int* arr;
    cudaMalloc((void**)&arr, N * sizeof(int));

    computeIndex<<<N/THREADS_PER_BLK, THREADS_PER_BLK>>>(arr, N);

    int localArr[N];

    cudaMemcpy(localArr, arr, N * sizeof(int), cudaMemcpyDeviceToHost);

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
