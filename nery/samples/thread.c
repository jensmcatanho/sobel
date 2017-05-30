#include <pthread.h>
#include <stdio.h>
#define NUM_THREADS 5

void *print_hello(void *threadId)
{
    long tid;
    tid = (long) threadId;
    printf("Hello World! Sou eu, thread %ld!\n",tid);
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    pthread_t threads[NUM_THREADS];
    int rc;
    long t;

    for(t = 0 ; t < NUM_THREADS ; t++)
    {
        printf("Main: criando thread %ld\n",t);
        rc = pthread_create(&threads[t],NULL,print_hello, (void*) t);
        if(rc)
        {
            fprintf(stderr,"erro %d ao criar thread\n",rc);
            return -1;
        }
    }

    pthread_exit(NULL);

}
