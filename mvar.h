#ifndef MVAR_H
#define MVAR_H

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef struct mvar_implementation mvar;
mvar * new_empty_mvar();
void delete_mvar(mvar * mv);
void put_mvar(mvar * mv, void * val);
void * take_mvar(mvar * mv);

#endif