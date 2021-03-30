#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>

#define QUEUESIZE 10
#define LOOP 200

#define P 3
#define Q 10

int check = (int)P * (int)LOOP;
int elements_to_be_consumed = (int)P * (int)LOOP;
int elements_consumed = 0;
float avarage_time = 0;

void *producer(void *args);
void *consumer(void *args);

typedef struct
{
    void *(*work)(void *);
    void *arg;
} workFunction;

typedef struct
{
    workFunction *buf[QUEUESIZE];
    long head, tail;
    int full, empty;
    pthread_mutex_t *mut;
    pthread_cond_t *notFull, *notEmpty;
} queue;

struct timeval tic()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv;
}

double toc(struct timeval begin)
{
    struct timeval end;
    gettimeofday(&end, NULL);
    double stime = ((double)(end.tv_sec - begin.tv_sec) * 1000) +
                   ((double)(end.tv_usec - begin.tv_usec) / 1000);
    stime = stime / 1000;
    return (stime);
}

typedef struct
{
    int arguments;
    struct timeval timer;
}argument_for_doWork;

queue *queueInit(void);
void queueDelete(queue *q);
void queueAdd(queue *q, workFunction *in);
void queueDel(queue *q, workFunction *out);
void *doWork(void *arg);

int main()
{
    queue *fifo;
    pthread_t pro[P], con[Q];

    fifo = queueInit();
    if (fifo == NULL)
    {
        fprintf(stderr, "Main: Queue Init failed.\n");
        exit(1);
    }

    for (int i = 0; i < P; i++)
    {
        printf("Producer %d created.\n", i);
        pthread_create(&pro[i], NULL, producer, fifo);
    }
    for (int i = 0; i < Q; i++)
    {
        printf("Consumer %d created.\n", i);
        pthread_create(&con[i], NULL, consumer, fifo);
    }
    for (int i = 0; i < P; i++)
        pthread_join(pro[i], NULL);
    for (int i = 0; i < Q; i++)
        pthread_join(con[i], NULL);
    queueDelete(fifo);

    printf("In total, %d packages were created and consumed in this session.\n", elements_consumed);
    printf("The avarage time of this session was: %f.\n", (avarage_time / elements_consumed));

    return 0;
}

void *producer(void *q)
{
    queue *fifo;
    int i;
    fifo = (queue *)q;

    workFunction *producer_struct;
    producer_struct = (workFunction *)malloc(LOOP * sizeof(workFunction));
    argument_for_doWork *argument_struct;
    argument_struct = (argument_for_doWork *)malloc(LOOP * sizeof(argument_for_doWork));

    int *argument_array = (int *)malloc(LOOP * sizeof(workFunction));
    for (i = 0; i < LOOP; i++)
    {
        argument_array[i] = i;
    }

    for (i = 0; i < LOOP; i++)
    {
        (argument_struct+i)->arguments = argument_array[i];
        (argument_struct+i)->timer = tic();
        (producer_struct+i)->work = doWork;
        (producer_struct+i)->arg = (argument_struct + i);
        
        pthread_mutex_lock(fifo->mut);
        while (fifo->full)
        {
            printf("Producer: queue FULL.\n");
            pthread_cond_wait(fifo->notFull, fifo->mut);
        }
        
        //printf("Producer: loop %d\n", i);  //Was used to determine whether the loop was functioning properly
        queueAdd(fifo, producer_struct+i);
        pthread_mutex_unlock(fifo->mut);
        pthread_cond_signal(fifo->notEmpty);
    }
    return (NULL);
}

void *consumer(void *q)
{
    queue *fifo;
    int i;
    fifo = (queue *)q;

    for (;;)
    {
        check--;
        if (check > -1)
        {
            pthread_mutex_lock(fifo->mut);
            while (fifo->empty)
            {
                printf("Consumer: queue EMPTY.\n");
                pthread_cond_wait(fifo->notEmpty, fifo->mut);
            }

            workFunction consumer_struct;
            argument_for_doWork *argument_struct;

            queueDel(fifo, &consumer_struct);

            argument_struct = consumer_struct.arg;
            int argument = argument_struct->arguments;
            struct timeval start = argument_struct->timer;
            double time_passed = toc(start);
            avarage_time += time_passed;

            //printf("Package was delivered after %f seconds\n", time_passed); // It displays the time that the package was in the queue.

            (*consumer_struct.work)(&argument);

            //printf("Consumer consumed\n");

            pthread_mutex_unlock(fifo->mut);
            pthread_cond_signal(fifo->notFull);

            elements_consumed++;
        }
        if (check <= -1)
        {
            break;
        }
    }

    return (NULL);
}

queue *queueInit(void)
{
    queue *q;

    q = (queue *)malloc(sizeof(queue));
    if (q == NULL)
        return (NULL);

    q->empty = 1;
    q->full = 0;
    q->head = 0;
    q->tail = 0;
    q->mut = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(q->mut, NULL);
    q->notFull = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
    pthread_cond_init(q->notFull, NULL);
    q->notEmpty = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
    pthread_cond_init(q->notEmpty, NULL);

    return (q);
}

void queueDelete(queue *q)
{
    pthread_mutex_destroy(q->mut);
    free(q->mut);
    pthread_cond_destroy(q->notFull);
    free(q->notFull);
    pthread_cond_destroy(q->notEmpty);
    free(q->notEmpty);
    free(q);
}

void queueAdd(queue *q, workFunction *in)
{
    q->buf[q->tail] = in;
    q->tail++;
    if (q->tail == QUEUESIZE)
        q->tail = 0;
    if (q->tail == q->head)
        q->full = 1;
    q->empty = 0;

    return;
}

void queueDel(queue *q, workFunction *out)
{
    *out = *q->buf[q->head];

    q->head++;
    if (q->head == QUEUESIZE)
        q->head = 0;
    if (q->head == q->tail)
        q->empty = 1;
    q->full = 0;

    return;
}

void *doWork(void *arg)
{
    int argument = *(int *)arg;
    //printf("Argument = %d.\n", argument); //Was used to determine whether the arguments recieved were the ones expected.
    double k = 0;
    for (int i = 0; i < 1000; i++)
    {
        k += sin(argument) + cos(argument);
    }
    //printf("Result = %f.\n", k); //Was used to determine whether the function worked properly.
}
