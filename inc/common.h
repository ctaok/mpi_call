#include<iostream>
using namespace std;

typedef float valueType_t;
#define MPI_ValueType OMPI_PREDEFINED_GLOBAL(MPI_Datatype, ompi_mpi_float)
#define PRINT(format, args...) fprintf(stdout, format"\n", ##args)
