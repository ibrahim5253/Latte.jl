/*
Copyright (c) 2015, Intel Corporation

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "comm.h"
#include <vector>
#include <mpi.h>
#include <cmath>

std::vector<MPI_Request *> requests;
void init() {
    MPI_Init(NULL, NULL);
}

int init_request() {
    MPI_Request *request = (MPI_Request *) malloc(sizeof(MPI_Request));
    int id = requests.size();
    requests.push_back(request);
    return id;
}

uint64_t zero_grads = 0;
uint64_t all_grads = 0;

void sync_gradients(float *data, float* values, int count, int request_id) {
//void sync_gradients(float *data, int count, int request_id) {
    int rank = get_rank();
    all_grads += count;
    for(int i=0; i<count; i++)
        if(std::abs(data[i])<std::abs(values[i])/100.0) {
          //  zero_grads++;
          //  data[i] = 0.0;
        }

    MPI_Request *request = requests[request_id];
    MPI_Iallreduce(MPI_IN_PLACE, data, count, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD, request);
    // int size;
    // MPI_Comm_size(MPI_COMM_WORLD, &size);
    // Scale for gradient accumulation normalization
    // #pragma omp parallel for simd
    // for (int i=0; i < count; i++) {
    //     data[i] /= (float) size;
    // }
}

int print_stat() {
   // printf("rank %d zeros %lld all %lld rate:%lf \n", get_rank(), zero_grads, all_grads, zero_grads/(double)all_grads);
    return 1;
}

void wait(int request_id) {
    MPI_Request *request = requests[request_id];
    MPI_Status stat;
    MPI_Wait(request, &stat);
}

float reduce_accuracy(float acc) {
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  
    float total_acc = 0.0f;
    MPI_Reduce(&acc, &total_acc, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        return total_acc / size;
    } else {
        return -1.0f;
    }
}

void broadcast(float* value, int length) {
    MPI_Bcast(value, length, MPI_FLOAT, 0, MPI_COMM_WORLD);
}

int get_rank() {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    return rank;
}
