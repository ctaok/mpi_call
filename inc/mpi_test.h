#ifndef _MPI_TRAIN_H
#define _MPI_TRAIN_H

#include <pthread.h>
#include "mpi.h"
#include "mpi_ops.h"
#include "common.h"

typedef enum {
#define CFG(y, f, s) MSG_TYPE_##y,
#include "mpi.def"
} msg_type_t;

typedef struct {
    msg_type_t type;
    int kind;
    char data[128];
} msg_head_t;

typedef struct command {
    // INPUT
    int32_t batch_done; 
    size_t batch_cnt;
    valueType_t * sigma_score_buf;
    int32_t * label_buf;
    valueType_t * label_weight_buf;
    // OUTPUT
    valueType_t * grad_val_buf;
    valueType_t * output_score_buf;
    valueType_t block_lr_loss;
} command_t;

typedef struct {
	msg_head_t * head;
	size_t store_cnt;
} master_info_t;

typedef int (*master_handler)(master_info_t*);

#define CFG(y, f, s) int master_##f##_handler(master_info_t*);
#include "mpi.def"

#endif
