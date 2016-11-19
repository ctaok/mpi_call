/**
 * mpi_ops.cpp
 * Author: wangguibin (wangguibin@conew.com)
 * Created on: 2016-06-01
 * Copyright (c) .com, Inc. All Rights Reserved
 */

#include <map>
#include "mpi_ops.h"

#define P(f) {reinterpret_cast<long>(&f), {#f}}
#define PR(f, r) {reinterpret_cast<long>(&f), {#f}}

std::map<long, mpi_func_info> func_infos = {
    P(MPI_Init),
    P(MPI_Init_thread),
    P(MPI_Initialized),
    P(MPI_Finalized),
    P(MPI_Finalize),
    P(MPI_Isend),
    P(MPI_Irecv),
    P(MPI_Gather),
    P(MPI_Reduce),
    P(MPI_Allreduce),
    P(MPI_Bcast),
    P(MPI_Ibcast),
    P(MPI_Wait),
    PR(MPI_Test, 0.5),
    P(MPI_Get_count),
    P(MPI_Comm_rank),
    P(MPI_Comm_size),
    P(MPI_Barrier),
    P(MPI_Comm_set_errhandler),
    P(MPI_Comm_split),
    P(MPI_Comm_split_type),
    P(MPI_Comm_create),
    P(MPI_Comm_dup),
    P(MPI_Comm_free),
    P(MPI_Abort)
};

int MPI_Send_large(const void *buf, long long count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
    int tosend = 0;
    int dsize = 0;
    MPI_Type_size(datatype, &dsize);
    long long bytesize = count * dsize;
    char * sendbuf = (char*) buf;
    while(bytesize > 0) {
        if(bytesize >= MPI_MAX_COMM_SIZE) {
            tosend = MPI_MAX_COMM_SIZE;
        } else {
            tosend = bytesize; 
        }
        MPI_Call(MPI_LOCK_UNLOCK, MPI_Send, sendbuf, tosend, MPI_BYTE, dest, tag, comm); 
        sendbuf += tosend;
        bytesize -= tosend;
    }
    return 0;
}

int MPI_Recv_large(void *buf, long long count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm)
{
    MPI_Status status;
    int torecv = 0;
    int recved = 0;
    int dsize = 0;
    MPI_Type_size(datatype, &dsize);
    long long bytesize = count * dsize;
    char * recvbuf = (char*) buf;
    while(bytesize > 0) {
        if(bytesize >= MPI_MAX_COMM_SIZE) {
            torecv = MPI_MAX_COMM_SIZE;
        } else {
            torecv = bytesize; 
        }
        MPI_Call(MPI_LOCK_UNLOCK, MPI_Recv, recvbuf, torecv, MPI_BYTE, source, tag, comm, &status); 
        MPI_Call(MPI_LOCK_UNLOCK, MPI_Get_count, &status, MPI_BYTE, &recved);
        if (recved != torecv) 
        {
            //WRITE_LOG(UL_LOG_FATAL, "MPI_Recv %d bytes, but expect %d", recved, torecv);
        }
        recvbuf += torecv;
        bytesize -= torecv;
    }
    return 0;   
}
