#ifndef _MPIOPS_H
#define _MPIOPS_H

#include <map>
#include <stdexcept>
#include "mpi.h"
#include "common.h"
//#include "atomic.h"

extern pthread_mutex_t mpi_mutex;
//extern atomic_t gExitError;

struct mpi_func_info {
    std::string name;
    
    ~mpi_func_info() {};
};

extern std::map<long, mpi_func_info> func_infos;

template<typename F> 
mpi_func_info* get_func_info(F f) {
    static mpi_func_info dflt { "<unknown>" }; 
    try {
        return &func_infos.at(reinterpret_cast<long>(f));
    } catch (std::out_of_range r) {
        return &dflt;
    }
}

template<typename F, typename... Args>
void log_mpi_call(F f, Args... args) {
    int rank = -1;
    int flag = 0;
    MPI_Initialized(&flag); 
    if (flag > 0)
    {
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    }
    mpi_func_info* info = get_func_info(f);
    //WRITE_LOG(UL_LOG_WARNING, "Rank[%d] calls %s", rank, info->name.c_str());
}

enum {
    MPI_LOCK_FREE = 0,
    MPI_LOCK = 1,
    MPI_UNLOCK = 2,
    MPI_LOCK_UNLOCK = 3
};

template<typename F, typename...Args>
void MPI_Call(uint32_t lock, F f, Args... args) {
    if (lock & 0x1) pthread_mutex_lock(&mpi_mutex);
    //log_mpi_call(f, args...);
    int e = f(args...);
    if (e != MPI_SUCCESS) {
        //atomic_set(&gExitError, gSendRecvError);
        mpi_func_info* info = get_func_info(f);
        //DEBUG_OUTPUT("MPI_Call %s failed [return:%d]", info->name.c_str(), e);
		printf("MPI_Call %s failed [return:%d]", info->name.c_str(), e);
        exit(-1);
    }
    if (lock & 0x2) pthread_mutex_unlock(&mpi_mutex);
}

#define MPI_MAX_COMM_SIZE (1<<30)

int MPI_Recv_large(void *buf, long long count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm);
int MPI_Send_large(const void *buf, long long count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);

#endif
