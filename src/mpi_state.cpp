/**
 * mpi_state.cpp
 * Author: wangguibin (wangguibin@conew.com)
 * Created on: 2016-06-01
 * Copyright (c) .com, Inc. All Rights Reserved
 */

#include "mpi_state.h"

pthread_mutex_t mpi_mutex = PTHREAD_MUTEX_INITIALIZER;

int64_t mpi_state_t::init( int argc, char ** argv )
{
    int provided = 0;

    MPI_Call(MPI_LOCK_UNLOCK, MPI_Init_thread, &argc, &argv, MPI_THREAD_SERIALIZED, &provided);
    if (provided < MPI_THREAD_SERIALIZED)
    {
        throw std::runtime_error("MPI does not support MPI_THREAD_SERIALIZED.");
    }
    MPI_Call(MPI_LOCK_UNLOCK, MPI_Comm_set_errhandler, MPI_COMM_WORLD, MPI_ERRORS_RETURN);
    MPI_Call(MPI_LOCK_UNLOCK, MPI_Comm_rank, MPI_COMM_WORLD, &_rank);
    MPI_Call(MPI_LOCK_UNLOCK, MPI_Comm_size, MPI_COMM_WORLD, &_size);

    /* Construct new communicator for  all the rank!=0 processes */
    if (_rank == 0) 
    {
        MPI_Call(MPI_LOCK_UNLOCK, MPI_Comm_split, MPI_COMM_WORLD, MPI_UNDEFINED, 0, &_worker_comm);
    }
    else 
    {
        MPI_Call(MPI_LOCK_UNLOCK, MPI_Comm_split, MPI_COMM_WORLD, 1, 0, &_worker_comm);
        MPI_Call(MPI_LOCK_UNLOCK, MPI_Comm_size, _worker_comm, &_worker_size);
        MPI_Call(MPI_LOCK_UNLOCK, MPI_Comm_rank, _worker_comm, &_worker_rank);
    }

    if (_rank == 0) 
    {
        _role = MASTER_NODE;
    }
    else 
    {
        _role = WORKER_NODE;
    }

    //config_t * config_obj = config_t::get_obj();

    if (_rank == 0)
    {
        //_all_buf_mem_size = config_obj->gTotal_feature_num * config_obj->lr_out_dim * sizeof(valueType_t);
    }
    else
    {
        //int64_t avg_feature_num = get_avg(config_obj->gTotal_feature_num, _worker_size); 
        //_all_buf_mem_size = avg_feature_num * config_obj->lr_out_dim * sizeof(valueType_t);
    }

    _reqs = new(std::nothrow) MPI_Request[_size];

    if ( _reqs == NULL) {
        delete[] _reqs;
        _reqs = NULL;
        return -1;
    }

    return 0;
}

void mpi_state_t::deinit()
{
    delete[] _reqs;
    _reqs = NULL;
}


