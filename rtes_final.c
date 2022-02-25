#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <sys/time.h>

#include <pthread.h>

#define RUNTIME 25920
#define TOTAL_ADDRESSES 1000
#define MAC_LENGTH 17
#define QUEUESIZE 50
#define P 1
#define Q 1

struct timeval tic(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv;
}

double toc(struct timeval begin){
    struct timeval end;
    gettimeofday(&end, NULL);
    double stime = ((double)(end.tv_sec - begin.tv_sec) * 1000) +
                   ((double)(end.tv_usec - begin.tv_usec) / 1000);
    stime = stime / 1000;
    return (stime);
}

struct timeval start; 
struct timeval hours; 

typedef struct{ 
    struct timeval tv;
    int arguement;
   
} element;

typedef struct{ 
    struct timeval tv;
    char value[MAC_LENGTH];
    int id;
} macaddress;

macaddress closeMacs[3000];
macaddress macsNearME[30000];
macaddress *BTnearMe(char **array);

typedef struct {
    macaddress *(*work)(char **);
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

queue *queueInitialize(void);

int delay[3000];
int counter = 0;
int elementsLeft = (RUNTIME * 10);
int i = 0, j = 0;
int *exists(macaddress *arr, macaddress *target, int limit);

char ** addresses;
char ** mac_produce();

bool testCOVID();

void *producer(void *args);
void *consumer(void *args);
void *timeCounter();

void queueDelete(queue *q);
void queueAdd(queue *q, workFunction *in);
void queueDel(queue *q, workFunction *out);

void writeFile(char **a, char *n, int limit);
void writeBinaryFile(int *array, char *n);

void uploadContacts(macaddress *closeMacs, int limit);
void fun(macaddress *mac);

pthread_cond_t cond;                                                            
pthread_mutex_t mutex;

int main()
{
    pthread_t producers[P], consumers[Q], timeThread;
    addresses =  mac_produce();
    queue *fifo;
    fifo = queueInitialize();

    time_t t;
    srand((unsigned) time(&t));
    start = tic();
    hours = tic();

    if (fifo == NULL)
    {
        fprintf(stderr, "main: Queue Init failed\n");
        exit(1);
    }
    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        perror("pthread_mutex_init() error\n");                                       
        exit(2);                                                                    
    }
    if (pthread_cond_init(&cond, NULL) != 0)
    {                                    
        perror("pthread_cond_init() error\n");                                        
        exit(3);                                                                    
    }

    pthread_create(&timeThread, NULL, timeCounter, NULL);
    for (int k = 0; k < P; k++)
        pthread_create(&producers[k], NULL, producer, fifo);
    for (int k = 0; k < Q; k++)
        pthread_create(&consumers[k], NULL, consumer, fifo);
    for (int k = 0; k < P; k++)
        pthread_join(producers[k], NULL);
    for (int k = 0; k < Q; k++)
        pthread_join(consumers[k], NULL);
    pthread_join(timeThread, NULL);
    queueDelete(fifo);
    
    return 0;
}

macaddress *BTnearMe(char **array)
{
    macaddress *mac;
    mac = (macaddress *)malloc(sizeof(macaddress));
    if (mac == NULL)
        return (NULL);
    int x = (rand() % TOTAL_ADDRESSES);
    for (int k = 0; k < MAC_LENGTH; k++)
        mac->value[k] = array[x][k];
    mac->tv = tic();
    return (mac);
}

int *exists(macaddress *arr, macaddress *target, int limit)
{
    int res[2] = {0, 0};
    int *ptr = res;
    char *b = target->value;
    for (int k = 0; k < limit; k++)
    {
        char *a = arr[k].value;
        if(strncmp(a, b, strlen(b)) == 0)
        {
            res[0] = 1;
            res[1] = k;
        }
    }       
    return ptr;
}

char ** mac_produce()
{

    time_t t;
    srand((unsigned) time(&t));

    char ** addresses = (char **)malloc(TOTAL_ADDRESSES * sizeof(char *));

    for (int i = 0 ; i < TOTAL_ADDRESSES ; i++)
    {

        char str[MAC_LENGTH] = "";
        int ctr = 1;
    
        for (int j = 0 ; j < 12 ; j++)
        {
            int val = (rand() % 16);            //Random integer between 0-15
            char hex[10];  
            sprintf(hex, "%x", val);            //Turning integer into a hex
            char x[10];
            sprintf(x, "%s", hex);              //Turning hex into a char
            strncat(str, x, 1);                 //Adding char to a string
      
            if ( (ctr % 2) == 0 && (ctr != 12))
            {
                const char x = ':';   
                strncat(str, &x, 1);            //Adding ':'
            }   
            ctr++;
        }
        char * address = (char*)malloc(MAC_LENGTH * sizeof(char));
        strcpy(address, str);
        addresses[i] = address;
    }
  writeFile(addresses, "MAC_Addresses.txt", TOTAL_ADDRESSES);
  return addresses;
}

queue *queueInitialize(void)
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

bool testCOVID()
{
    printf("Testing patient for Covid-19 pathogens...\n\n");
    if (rand() % 100 > 85)
        return true;
    else
        return false;
}

void *producer(void *q)
{
    queue *fifo;
    int k;
    int *a = (int *)malloc(RUNTIME * 10 * sizeof(int));
    for (k = 0; k < (RUNTIME * 10); k++){
         a[k] = k;
    }

    workFunction *prodOut;
    prodOut = (workFunction*)malloc(RUNTIME * 10 * sizeof(workFunction));

    element *elem;
    elem = (element *)malloc(RUNTIME * 10 * sizeof(element));

    fifo = (queue *)q;
    
    for (k = 0; ; k++)
    {
        (elem + k)->arguement = a[counter];
        (prodOut + k)->arg = (elem + k);
        (prodOut + k)->work = BTnearMe;
       
        pthread_mutex_lock(fifo->mut);
        
        while (fifo->full)
        {
            pthread_cond_wait(fifo->notFull, fifo->mut);
        }

        queueAdd(fifo, (prodOut + k));
        counter++;
        pthread_mutex_unlock(fifo->mut);
        pthread_cond_signal(fifo->notEmpty);
        double final = toc(start);

        if (final > RUNTIME){
          printf("The time period has finished\n");
        }

        
    }
    return (NULL);
}

//Global integer veriable used by the consumers
int cntr = 0;
int del = 0;

void *consumer(void *q)
{
    queue *fifo;
    fifo = (queue *)q;
    double prevTime = 0.0;
    
    while (elementsLeft > (Q - 1))
    {
        pthread_mutex_lock(fifo->mut);
        while (fifo->empty)
        {
            pthread_cond_wait(fifo->notEmpty, fifo->mut);
            if(elementsLeft < 0){
                exit(0);
            }
        }
       
        workFunction consume;
        element *elem;
        elem = consume.arg;
        
        queueDel(fifo, &consume);
        elementsLeft--;
        
        macaddress *mac = (*consume.work)(addresses);
        double final = toc(start);  
        mac->id = cntr;
        
        fun(mac);

        //Calculating delay time and adding it in an array
        int num = (final - prevTime) * 1000000;
        delay[del] = ((((num / 10) % 10) * 10) + (num % 10)+(((num / 100) % 10) * 100));

        cntr++;
        del++;
        prevTime = final;

        if (del == 3000)
        {
            writeBinaryFile(delay, "Latency.bin");
            del = 0;
        }
        pthread_mutex_unlock(fifo->mut);
        pthread_cond_signal(fifo->notFull);

       //Waiting for the timer to send signal in order to proceed
        if (pthread_cond_wait(&cond, &mutex) != 0)
        {
            perror("pthread_cond_timedwait() error");                                   
            exit(7);
        }
    }

    return (NULL);
}

void *timeCounter()
{
    while(true)
    {
        //Waiting 0.1 seconds to send signal to the consumer
        usleep(100000);
        if (pthread_cond_signal(&cond) != 0)
        {
            perror("pthread_cond_signal() error");
            exit(4);
        }
    }

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

void writeFile(char **array, char *n, int limit){
    FILE *fp;
    char *name = (char *)malloc(100 * sizeof(char));
    sprintf(name,"%s", n);
    fp = fopen(name, "w"); 
    for (int i = 0; i < limit; i++){
      if(array[i]!=NULL)
        fprintf(fp, "%s\n", array[i]); 
    }
    fclose(fp);
}

void writeBinaryFile(int *array, char *n)
{
    FILE *fp;
    char *name = (char *)malloc(100 * sizeof(char));
    sprintf(name,"%s", n);
    fp = fopen(name, "ab");
    for (int k = 0; k < 3000; k++)
        fwrite(&array[k], sizeof(array[k]), 1, fp); 
    printf("Binary file updated\n");
    fclose(fp);
}

void uploadContacts(macaddress *closeMacs, int limit)
{
    printf("Uploading contacts...\n\n");
    char **arr = (char **)malloc(limit * sizeof(char *));
    char *b="none";
    for (int k = 0; k < limit; k++)
    {
        char *a = closeMacs[k].value;
        if (strncmp(a, b, strlen(b)) != 0)
        {
            arr[k] = a;
        }
    }
    char *name = (char *)malloc(100 * sizeof(char));
  
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(name, "Contacts_%d-%d-%d_%d-%d-%d.txt", tm.tm_mday, tm.tm_mon+1, tm.tm_year+1900, tm.tm_hour+1, tm.tm_min, tm.tm_sec);
    writeFile(arr, name, (limit - 1));
    printf("Contacts uploaded successfully\n\n");
}

void fun(macaddress *mac)
{
    //Checking timespan of each mac address in the arrays
    for(int k = 0; k < j; k++)
    {
        struct timeval begin = macsNearME[k].tv;
        double end = toc(begin);
        if(end > 12)
            strcpy(macsNearME[k].value, "none");
    }
    for(int k = 0; k < i; k++)
    {
        struct timeval begin = closeMacs[k].tv;
        double end = toc(begin);
        if(end > 12096)
            strcpy(closeMacs[k].value, "none");
    }

    int *a = exists(macsNearME, mac, (j + 1));
    if (a[0] == 1)
    {
        struct timeval start1 = macsNearME[a[1]].tv;
        double final1 = toc(start1);
        if (final1 > 12)
        {
            strcpy(macsNearME[a[1]].value, "none");
        }else
        {
            int *b = exists(closeMacs, mac, (i+1));
            if (b[0] == 0){
                if (final1 >= 3)
                {
                    strcpy(closeMacs[i].value, mac->value);
                    closeMacs[i].tv = mac->tv;
                    closeMacs[i].id = i;
                    i++;
                }
            }else
            {
                struct timeval start2 = closeMacs[b[1]].tv;
                if (toc(start2) > 12096)
                    strcpy(closeMacs[b[1]].value, "none");
            }
        }
    }
    else
    {
        //Checking for empty space in macsNearME
        int pos = -1;
        char *empty = "none";
        for (int k = 0; k < j; k++)
        {
            char *str = macsNearME[k].value;
            if(strncmp(str, empty, strlen(empty)) == 0)
                pos = k;
        } 
        if (pos == -1)
        {
            strcpy(macsNearME[j].value, mac->value);
            macsNearME[j].tv = mac->tv;
            macsNearME[j].id = j;
            j++;
        }else
        {
            strcpy(macsNearME[pos].value, mac->value);
            macsNearME[pos].tv = mac->tv;
            macsNearME[pos].id = pos;
        }
    }

    if(toc(hours) > 144)
    {
        if (testCOVID())
        {
            printf("The test's result is positive\n\n");
            uploadContacts(closeMacs, (i + 1));
        }else
            printf("The test's result is negative\n\n");
        hours = tic();
    }
}
