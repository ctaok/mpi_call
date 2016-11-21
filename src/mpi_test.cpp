#include<iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "mpi_test.h"
#include "mpi_ops.h"
#include "mpi_state.h"
#include "common.h"


int master_test_handler(master_info_t * info)
{
	PRINT("master_node");
	return 0;
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
	return 0;
}

int work_entry(void)
{
	msg_head_t head;
	//int myrank = mpi_state_t::get_obj()->get_rank();

	head.type = MSG_TYPE_TEST;
	head.kind = mpi_state_t::get_obj()->get_rank();
	valueType_t* ptr = (valueType_t*)head.data;
	ptr[0] = head.kind;
	ptr[1] = head.kind;
	ptr[2] = head.kind;
	ptr[3] = head.kind;
    MPI_Call(MPI_LOCK_UNLOCK, MPI_Gather, &head, sizeof(head), MPI_CHAR, &head, sizeof(head), MPI_CHAR, 0, MPI_COMM_WORLD);
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
	PRINT("rank: %d, role: %d", mpi_obj->get_rank(), mpi_obj->get_role());
	if (MASTER_NODE == mpi_obj->get_role()) {
		ret = master_entry();
		return ret; 
	}
	sleep(5);
	work_entry();

MPI_ABORT:
	mpi_obj->deinit();
	MPI_Call(MPI_LOCK_UNLOCK, MPI_Finalize);
	return 0;
}
