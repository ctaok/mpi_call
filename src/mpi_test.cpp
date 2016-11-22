#include<iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "mpi_test.h"
#include "mpi_ops.h"
#include "mpi_state.h"
#include "common.h"


int master_recv_handler(master_info_t * info)
{
	int comm_size = 0;
	int i = 0;
	mpi_state_t * mpi_obj = mpi_state_t::get_obj();
	comm_size = mpi_obj->get_size();
	valueType_t tmp[100];
	for (int k=0; k<100; k++)
		tmp[k] = k + 100.0f;
	const int revSize = 100/(comm_size-1);
	for (i = 1; i < comm_size; i++) {
		if (100 <= MPI_MAX_COMM_SIZE)
			MPI_Call(MPI_LOCK_UNLOCK, MPI_Send, tmp+revSize*(i-1), revSize, MPI_ValueType, i, i, MPI_COMM_WORLD);
		else
			MPI_Send_large(tmp+revSize*(i-1), revSize, MPI_ValueType, i, i, MPI_COMM_WORLD);
	}

	return 0;
}

int master_send_handler(master_info_t * info)
{
	int comm_size = 0;
	mpi_state_t * mpi_obj = mpi_state_t::get_obj();
	comm_size = mpi_obj->get_size();
	const int revSize = 100/(comm_size-1);
	valueType_t tmp2[revSize];
	for (int k=0; k<revSize; k++)
		tmp2[k] = 0;
	//tmp2 must initialize for MPI_Reduce
	MPI_Call(MPI_LOCK_UNLOCK, MPI_Reduce, MPI_IN_PLACE, tmp2, revSize, MPI_ValueType, MPI_SUM, 0, MPI_COMM_WORLD);
	PRINT("server: %d", mpi_obj->get_rank());
	for (int k=0; k<revSize; k++)
		printf("%lf, ", tmp2[k]);
	PRINT("\n");
	//fflush(stdout);

	return 0;
}

int master_finish_handler(master_info_t * info)
{
	return -1;
}

int master_invalid_handler(master_info_t * info)
{
	PRINT("Rank[0] receive invalid msg");
	return -1;
}

msg_type_t master_check_header(msg_head_t * head, int size)
{
    int i = 0;

    msg_head_t * first_msg = head + 1;   
    msg_type_t first_type = first_msg->type;
    int first_kind = first_msg->kind;

    /* check header consistent */    
    for (i = 2; i < size; i++)
    {
        msg_head_t * msg = head + i;      
        msg_type_t type = msg->type;
        int kind = msg->kind;

        if (first_type != type || first_kind != kind) 
        {
			PRINT("Server received two different msg [%d/%d]", first_type, type);
            return MSG_TYPE_INVALID;  
        }
    }

    return first_type;
}

int master_entry(void)
{
	mpi_state_t * mpi_obj = mpi_state_t::get_obj();
	int comm_size = mpi_obj->get_size();
	msg_head_t* head = new msg_head_t[comm_size];
	int ret = 0;
	master_info_t local_info;

	master_handler mapper[] = {
#define CFG(y, f, s) master_##f##_handler,
#include "mpi.def"
	};
	while(true) {
		MPI_Call(MPI_LOCK_UNLOCK, MPI_Gather, head, sizeof(msg_head_t), MPI_CHAR, head, sizeof(msg_head_t), MPI_CHAR, 0, MPI_COMM_WORLD);
		msg_type_t type = master_check_header(head, comm_size);
		master_handler handler = mapper[type];
		ret = handler(&local_info);
		if (ret < 0)
			break;
	}
	mpi_obj->deinit();
	MPI_Call(MPI_LOCK_UNLOCK, MPI_Finalize);
	PRINT("server: %d finished!", mpi_obj->get_rank());
	local_info.head = NULL;
	delete head;
	return 0;
}

int work_entry(void)
{
	msg_head_t head;

	//recv
	head.type = MSG_TYPE_RECV;
	head.kind = 3;
	valueType_t* ptr = (valueType_t*)head.data;
	ptr[0] = head.kind;
	ptr[1] = head.kind;
	ptr[2] = head.kind;
	ptr[3] = head.kind;
	mpi_state_t * mpi_obj = mpi_state_t::get_obj();
	int comm_size = mpi_obj->get_size();
	const int revSize = 100/(comm_size-1);
	valueType_t tmp[revSize];
	int myrank = mpi_state_t::get_obj()->get_rank();
	MPI_Status status;
	int count = 0;
    MPI_Call(MPI_LOCK_UNLOCK, MPI_Gather, &head, sizeof(head), MPI_CHAR, &head, sizeof(head), MPI_CHAR, 0, MPI_COMM_WORLD);
	if (100 <= MPI_MAX_COMM_SIZE) {
		MPI_Call(MPI_LOCK_UNLOCK, MPI_Recv, tmp, revSize, MPI_ValueType, 0, myrank, MPI_COMM_WORLD, &status);
		MPI_Call(MPI_LOCK_UNLOCK, MPI_Get_count, &status, MPI_ValueType, &count);
		if (count != revSize) {
			PRINT("MPI_Recv %d elements, but expect %d", count, revSize);
		}
	} else
		MPI_Recv_large(tmp, revSize, MPI_ValueType, 0, myrank, MPI_COMM_WORLD);
	PRINT("worker: %d", mpi_obj->get_rank());
	for (int k=0; k<revSize; k++)
		printf("%lf, ", tmp[k]);
	PRINT("\n");

	//send
	head.type = MSG_TYPE_SEND;
	head.kind = 1;
	ptr[0] = head.kind;
	ptr[1] = head.kind;
	ptr[2] = head.kind;
	ptr[3] = head.kind;
	for (int k=0; k<revSize; k++)
		tmp[k] -= k;
	MPI_Call(MPI_LOCK_UNLOCK, MPI_Gather, &head, sizeof(head), MPI_CHAR, &head, sizeof(head), MPI_CHAR, 0, MPI_COMM_WORLD);
	MPI_Call(MPI_LOCK_UNLOCK, MPI_Reduce, tmp, tmp, revSize, MPI_ValueType, MPI_SUM, 0, MPI_COMM_WORLD);
	return 0;
}

int main(int argc, char * argv[])
{
	int ret;
	mpi_state_t *mpi_obj = NULL;
	mpi_obj = mpi_state_t::get_obj();
	if (0 > (ret = mpi_obj->init(argc, argv))) {
		PRINT("fail to init");
		goto MPI_ABORT;
	}
	//PRINT("rank: %d, role: %d", mpi_obj->get_rank(), mpi_obj->get_role());
	if (MASTER_NODE == mpi_obj->get_role()) {
		ret = master_entry();
		return ret; 
	}
	work_entry();

MPI_ABORT:
	mpi_obj->deinit();
	msg_head_t head;
	head.type = MSG_TYPE_FINISH;
	head.kind = 1;
	MPI_Call(MPI_LOCK_UNLOCK, MPI_Gather, &head, sizeof(head), MPI_CHAR, &head, sizeof(head), MPI_CHAR, 0, MPI_COMM_WORLD);
	MPI_Call(MPI_LOCK_UNLOCK, MPI_Finalize);
	PRINT("worker: %d finished!", mpi_obj->get_rank());
	return 0;
}

