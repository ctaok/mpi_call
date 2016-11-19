#include<iostream>
#include "mpi_ops.h"
#include "mpi_state.h"

#define PRINT(format, args...) fprintf(stdout, format"\n", ##args)

int main(int argc, char * argv[])
{
	int ret;
	mpi_state_t *mpi_obj = NULL;
	mpi_obj = mpi_state_t::get_obj();
	if (0 > (ret = mpi_obj->init(argc, argv))) {
		//WRITE_LOG(UL_LOG_FATAL, "MPI init failed");
		PRINT("");
		goto MPI_ABORT;
	}
MPI_ABORT:
	mpi_obj->deinit();
	return 0;
}
