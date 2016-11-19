#ifndef _MPISTATE_H
#define _MPISTATE_H

#include "mpi.h"
#include "mpi_ops.h"

typedef float valueType_t;
#define MPI_ValueType OMPI_PREDEFINED_GLOBAL(MPI_Datatype, ompi_mpi_float)

typedef enum {
    MASTER_NODE = 1,
    WORKER_NODE,
    INVALID_NODE
} role_t;

class mpi_state_t
{
	int _rank;
	int _size;
    role_t _role;
	MPI_Status _status;
	MPI_Request * _reqs;
    MPI_Comm _worker_comm;
    int _worker_rank;
    int _worker_size;

	int64_t _all_buf_mem_size;

public:
	static mpi_state_t * get_obj()
	{
		static mpi_state_t mpi_state_obj;
		return &mpi_state_obj;
	}

	int64_t init( int argc, char ** argv );

	void deinit();

    role_t get_role() {return _role; }
	int get_rank() { return _rank; }
	int get_size() { return _size; }
	int get_worker_rank() { return _worker_rank; }
	int get_worker_size() { return _worker_size; }
	MPI_Comm get_worker_comm() { return _worker_comm; }
	MPI_Status * get_status() { return &_status; }
	MPI_Request * get_reqs() { return _reqs; }
	int64_t get_all_buf_mem_size() { return _all_buf_mem_size; }

private:
	mpi_state_t(){};
	~mpi_state_t(){};

};

/**把每一个MPI节点的node_val值累加，并直接覆盖原地址，正确返回0，否则返回-1*/
inline int64_t combine_global_cluster_info(valueType_t & node_val)
{
	valueType_t local_node_val = node_val;
	int64_t ret = 0;
	//如果只有1个节点，特殊处理
	if (1 != mpi_state_t::get_obj()->get_size())
	{
		MPI_Call(MPI_LOCK_UNLOCK, MPI_Allreduce, &local_node_val, &node_val, 1, MPI_ValueType, MPI_SUM, MPI_COMM_WORLD);
	}
	return ret;
}

inline int64_t combine_local_cluster_info(valueType_t & node_val)
{
	valueType_t local_node_val = node_val;
	int64_t ret = 0;
    mpi_state_t * mpi_obj = mpi_state_t::get_obj();
	if (1 != mpi_state_t::get_obj()->get_worker_size())
	{
		MPI_Call(MPI_LOCK_UNLOCK, MPI_Allreduce, &local_node_val, &node_val, 1, MPI_ValueType, MPI_SUM, mpi_obj->get_worker_comm());
	}
	return ret;
}

#endif

