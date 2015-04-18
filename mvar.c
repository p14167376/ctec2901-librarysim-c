/* Library: mvar
 * Author:  drs
 */

#include <stdio.h>
#include "mvar.h"

struct mvar_implementation
{
    pthread_mutex_t mvar_mutex;
    pthread_cond_t  mvar_full;
    pthread_cond_t  mvar_empty;
    int full;
    void * value;
};

mvar * new_empty_mvar()
{
    mvar * mv = (mvar *) malloc(sizeof(mvar));
    if (mv==NULL)
    {
        printf("mvar_new failed to allocate memory\n");
        exit(1);
    }
    pthread_mutex_init(&(mv->mvar_mutex), NULL);
    pthread_cond_init(&(mv->mvar_empty), NULL);
    pthread_cond_init(&(mv->mvar_full), NULL);
    mv->full = 0;
    mv->value = NULL;
    return mv;
}

void delete_mvar(mvar * mv)
{
    pthread_mutex_destroy(&(mv->mvar_mutex));
    pthread_cond_destroy(&(mv->mvar_empty));
    pthread_cond_destroy(&(mv->mvar_full));
    free(mv);
}

void put_mvar(mvar * mv, void * val)
{
    pthread_mutex_lock(&(mv->mvar_mutex));
    while (mv->full)
    {
        pthread_cond_wait(&(mv->mvar_empty), &(mv->mvar_mutex));
    }
    mv->value = val;
    mv->full = 1;
    pthread_cond_broadcast(&(mv->mvar_full));
    pthread_mutex_unlock(&(mv->mvar_mutex));
}

void * take_mvar(mvar * mv)
{
    void * val;
    pthread_mutex_lock(&(mv->mvar_mutex));
    while (!(mv->full))
        pthread_cond_wait(&(mv->mvar_full), &(mv->mvar_mutex));
    val = mv->value;
    mv->full = 0;
    pthread_cond_broadcast(&(mv->mvar_empty));
    pthread_mutex_unlock(&(mv->mvar_mutex));
    return val;
}

