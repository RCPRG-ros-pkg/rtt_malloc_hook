/*
 Copyright (c) 2014, Robot Control and Pattern Recognition Group, Warsaw University of Technology
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the Warsaw University of Technology nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL <COPYright HOLDER> BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <malloc.h>
#include <sys/resource.h>
#include <pthread.h>

char tmpbuff[1024000];
unsigned long tmppos = 0;
unsigned long tmpallocs = 0;

void *memset(void*,int,size_t);
void *memmove(void *to, const void *from, size_t size);

typedef void * (*t_calloc)(size_t, size_t);
static t_calloc myfn_calloc;

typedef void * (*t_malloc)(size_t);
static t_malloc myfn_malloc;

typedef void   (*t_free)(void *);
static t_free myfn_free;

typedef void * (*t_realloc)(void *, size_t);
static t_realloc myfn_realloc;

typedef void * (*t_memalign)(size_t, size_t);
static t_memalign myfn_memalign;

//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void init()
{
    myfn_malloc     = (t_malloc) dlsym(RTLD_NEXT, "malloc");
    myfn_free       = (t_free) dlsym(RTLD_NEXT, "free");
//    myfn_calloc     = (t_calloc) dlsym(RTLD_NEXT, "calloc");
//    myfn_realloc    = (t_realloc) dlsym(RTLD_NEXT, "realloc");
//    myfn_memalign   = (t_memalign) dlsym(RTLD_NEXT, "memalign");

    if (!myfn_malloc || !myfn_free)// || !myfn_calloc || !myfn_realloc || !myfn_memalign)
    {
        fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        exit(1);
    }
}

int counter = 0;
void *malloc(size_t size)
{

//    pthread_mutex_lock(&mutex);


    int policy;
    struct sched_param param;
//    pthread_attr_t attr;
//    pthread_getattr_np(pthread_self(), &attr);
//    pthread_attr_getschedpolicy(&attr, &policy);
//    pthread_attr_getschedparam(&attr, &param);
    pthread_getschedparam(pthread_self(), &policy, &param);

    if (counter > 2000 && policy != 0) {
        *(int*)0 = 1;
    }

    static int initializing = 0;
    if (myfn_malloc == NULL)
    {
        if (!initializing)
        {
            initializing = 1;
            init();
            initializing = 0;

//            fprintf(stdout, "jcheck: allocated %lu bytes of temp memory in %lu chunks during initialization\n", tmppos, tmpallocs);
        }
        else
        {
            if (tmppos + size < sizeof(tmpbuff))
            {
                void *retptr = tmpbuff + tmppos;
                tmppos += size;
                ++tmpallocs;
//                pthread_mutex_unlock(&mutex);
                return retptr;
            }
            else
            {
                fprintf(stdout, "jcheck: too much memory requested during initialisation - increase tmpbuff size\n");
                exit(1);
            }
        }
    }

    void *ptr = myfn_malloc(size);

//    if (counter == 2000) {
//        fprintf(stdout, "malloc: %lu\n", size);
//    }
    counter++;

//    pthread_mutex_unlock(&mutex);

    return ptr;
}

void free(void *ptr)
{
//    pthread_mutex_lock(&mutex);

    // something wrong if we call free before one of the allocators!
//  if (myfn_malloc == NULL)
//      init();

    if (ptr >= (void*) tmpbuff && ptr <= (void*)(tmpbuff + tmppos))
        fprintf(stdout, "freeing temp memory\n");
    else
        myfn_free(ptr);
//    pthread_mutex_unlock(&mutex);
}
/*
void *realloc(void *ptr, size_t size)
{
//    pthread_mutex_lock(&mutex);
    if (myfn_malloc == NULL)
    {
        void *nptr = malloc(size);
        if (nptr && ptr)
        {
            memmove(nptr, ptr, size);
            free(ptr);
        }
//        pthread_mutex_unlock(&mutex);
        return nptr;
    }

    void *nptr = myfn_realloc(ptr, size);
//    pthread_mutex_unlock(&mutex);
    return nptr;
}

void *calloc(size_t nmemb, size_t size)
{
//    pthread_mutex_lock(&mutex);
    if (myfn_malloc == NULL)
    {
        void *ptr = malloc(nmemb*size);
        if (ptr)
            memset(ptr, 0, nmemb*size);
//        pthread_mutex_unlock(&mutex);
        return ptr;
    }

    void *ptr = myfn_calloc(nmemb, size);
//    pthread_mutex_unlock(&mutex);
    return ptr;
}

void *memalign(size_t blocksize, size_t bytes)
{
//    pthread_mutex_lock(&mutex);
    void *ptr = myfn_memalign(blocksize, bytes);
//    pthread_mutex_unlock(&mutex);
    return ptr;
}
*/

