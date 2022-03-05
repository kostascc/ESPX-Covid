/**
 * Real Time Embedded Systems
 * 
 * Ⓒ 2022 K. Chatzis
 * kachatzis <at> ece.auth.gr
 **/

#include <stdio.h>
#include "pthread_fifo.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "covidTrace.h"


#define DEFAULT_PRODUCERS 2     // Default Number of Producer Threads
#define DEFAULT_CONSUMERS 2     // Default Number of Consumer Threads
#define DEFAULT_QUEUE_SIZE 1    // Default Size of Function Queue
#define DEFAULT_FUNCTIONS_T 3   // Default Functions per Thread

// #define PRINT_TIMER
#define PRINT_FILE

// #define DEBUG_QUEUE_INFO

FILE *fp;

/**
 * Producer Thread Function
 **/
void *_f_producer (void *q);

/**
 * Consumer Thread Function
 **/
void *_f_consumer (void *q);

/** 
 * Delay millisecods 
 **/
void delay_ms(int ms)
{
    // Storing start time
    clock_t start_time = clock();
  
    // looping till required time is achieved
    while (clock() < start_time + ms*1000)
        ;
}


/** 
 * Delay nanoseconds 
 **/
void delay_ns(int ns)
{
    // Storing start time
    clock_t start_time = clock();
  
    // looping till required time is achieved
    while (clock() < start_time + ns)
        ;
}


/**
 * Work Function
 **/
void work(void * arg){ 
    // Get Variable from arguments 
    struct timespec start = *((struct timespec *)arg);
    struct timespec end;

    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    float delta_us = (float) ((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000);
    #ifdef PRINT_TIMER
        printf(" > took %d us\n", delta_us);
    #endif
    #ifdef PRINT_FILE
        fprintf(fp, "%f\n", delta_us);
        // printf("%f us\n", delta_us);
    #endif

    // for(int i=0; i<100; i++) ;

    // Return
    return NULL; 
}


/**
 * Print Statistics
 **/
void print_stats(char* fname){

    FILE *in = fopen(fname, "r");
    
    if (in != NULL) {
        double sum = 0, sum_squares = 0, n = 0;
        double val;
        
        while (fscanf(in, "%lf", &val) == 1) {
            sum += val;
            sum_squares += val * val;
            ++n;
        }

        fclose(in);
        
        if (n > 0) {
            double mean = (double)sum / n;
            double variance = (double)sum_squares / n - (mean * mean);
            double std_dev = sqrt(variance);
            
            printf("Mean: %f\nStdDev: %f\n", mean, std_dev);
        }
    }
    
    return 0;
}


/**
 * Thread Function Arguments
 * 
 * @param fifo Function Queue
 * @param loop Function Created per Thread
 * @param p Number of Producers
 * @param q Number of Consumers
 * @param idx Thread Index
 **/
typedef struct t_args {
    queue* fifo;
    int loop;
    int p;
    int q;
    int idx;
} t_args;


int main(int argc, char** argv)
{

    /***********************
     * Read Arrguments
     ***********************/

    // If no arguments
    if (argc <= 1)
	{
        // Show usage info
		printf( "Usage: \n" );
		printf( " -p <int>  : Producers\n" );
		printf( " -q <int>  : Consumers\n" );
        printf( " -s <int>  : Queue Size\n" );
        printf( " -l <int>  : Functions per Producer (Thread)\n" );
        // exit
        return 0;
	}


    srand(time(NULL));

    



}

int fifo_main(int argc, char** argv)
{


    /***********************
     * Read Arrguments
     ***********************/

    // If no arguments
    if (argc <= 1)
	{
        // Show usage info
		printf( "Usage: \n" );
		printf( " -p <int>  : Producers\n" );
		printf( " -q <int>  : Consumers\n" );
        printf( " -s <int>  : Queue Size\n" );
        printf( " -l <int>  : Functions per Producer (Thread)\n" );
        // exit
        return 0;
	}


    int p = DEFAULT_PRODUCERS;  // Producers
    int q = DEFAULT_CONSUMERS;  // Consumers
    int s = DEFAULT_QUEUE_SIZE; // Queue Size
    int l = DEFAULT_FUNCTIONS_T;// Functions per Thread

    // Read Arguments
	for (int i = 1; i < argc; i++)
	{
		if ( strcmp(argv[i], "-p") == 0 ) {
			i++;
			p = (i < argc) ? atoi(argv[i]) : DEFAULT_PRODUCERS;
		}

        else if ( strcmp(argv[i], "-q") == 0 ) {
			i++;
			q = (i < argc) ? atoi(argv[i]) : DEFAULT_CONSUMERS;
		}

        else if ( strcmp(argv[i], "-s") == 0 ) {
			i++;
			s = (i < argc) ? atoi(argv[i]) : DEFAULT_QUEUE_SIZE;
		}

        else if ( strcmp(argv[i], "-l") == 0 ) {
			i++;
			l = (i < argc) ? atoi(argv[i]) : DEFAULT_FUNCTIONS_T;
		}

	}

    printf( "Producers: %d\nConsumers: %d\nQueue Size: %d\nFunctions per Producer: %d\n", p, q, s, l );

    /* Initialize Output File */
    #ifdef PRINT_FILE
        char fname[80];
        sprintf(fname, "out_p-%d_q-%d_s-%d.txt\0", p, q, s);
        // sprintf(fname, "out.txt\0");
        fp = fopen (fname, "w+");
    #endif


    /***********************
     * Create & Run Queue
     ***********************/

    queue *fifo;
    // Initialize Producers and Consumers
    pthread_t* pro = (pthread_t*)malloc(p * sizeof(pthread_t));
    pthread_t* con = (pthread_t*)malloc(q * sizeof(pthread_t));

    // Producer And Consumer FUnctions
    void *producer (void *args);
    void *consumer (void *args);

    // Initialize Queue
    fifo = queueInit (s);
    if (fifo ==  NULL) {
        fprintf (stderr, "main: Queue Init failed.\n");
        exit (1);
    }

    // Start Consumer Threads
    for(int i=0; i<q; i++)
    {    
        t_args* _t_args = (t_args*) malloc(sizeof(t_args));
        _t_args->fifo = fifo;
        _t_args->loop = l;
        _t_args->p = p;
        _t_args->q = q;
        _t_args->idx = i;
        pthread_create (&(con[i]), NULL, _f_consumer, (void*)_t_args);
    }    

    // Start Producer Threads
    for(int i=0; i<p; i++)
    {
        t_args* _t_args = (t_args*) malloc(sizeof(t_args));
        _t_args->fifo = fifo;
        _t_args->loop = l;
        _t_args->p = p;
        _t_args->q = q;
        _t_args->idx = i;
        pthread_create (&(pro[i]), NULL, _f_producer, (void*)_t_args);
    }

    // Join Threads
    for(int i=0; i<p; i++)
        pthread_join (pro[i], NULL);

    for(int i=0; i<q; i++)  
        pthread_join (con[i], NULL);

    // Clean Up
    queueDelete(fifo);
    free(pro);
    free(con); 

    #ifdef PRINT_FILE
        fclose(fp);
    #endif

    print_stats(fname);

    return 0;
}


void *_f_producer (void *inArgs)
{
    queue *fifo;
    int loop;
    int i;
    int idx;

    fifo = (queue*) ((struct t_args*)inArgs)-> fifo;
    loop = (int)    ((struct t_args*)inArgs)-> loop;
    idx  = (int)    ((struct t_args*)inArgs)-> idx;

    for (i = 0; i < loop; i++) {
        pthread_mutex_lock (fifo->mut);

        while (fifo->full) {
            #ifdef DEBUG_QUEUE_INFO
                printf ("producer %d: queue FULL.\n", idx);
            #endif

            pthread_cond_wait (fifo->notFull, fifo->mut);
        }

        workFunction* wf = (workFunction*)malloc(sizeof(workFunction));

        /* Start Timer */
        // struct timespec start;
        // clock_gettime(CLOCK_MONOTONIC_RAW, &start);

        // Set Arguments
        struct timespec * argi = (struct timespec *)malloc(sizeof(struct timespec));
        // argi[0] = start;
        clock_gettime(CLOCK_MONOTONIC_RAW, argi);
        wf->arg  = (argi);

        // Set Function
        wf->work = (&work);
        
        enqueue (fifo, (workFunction*)wf);

        #ifdef DEBUG_QUEUE_INFO
            printf ("> producer %d: enqueued.\n", idx);
        #endif
        pthread_mutex_unlock (fifo->mut);
        pthread_cond_signal (fifo->notEmpty);
    }

    return (NULL);
}


void *_f_consumer (void *inArgs)
{
    queue *fifo;
    int loop;
    int i;
    int p, q;
    int idx;
    workFunction* d /*= (workFunction**)malloc(sizeof(workFunction*)) */ ;

    fifo = (queue*) ((struct t_args*)inArgs)-> fifo;
    loop = (int)    ((struct t_args*)inArgs)-> loop;
    p    = (int)    ((struct t_args*)inArgs)-> p;
    q    = (int)    ((struct t_args*)inArgs)-> q;
    idx  = (int)    ((struct t_args*)inArgs)-> idx;

    int rec_count = (idx<q-1)? (loop*p)/q : ((loop*p)/q) + (loop*p)%q ;

    // printf ("consumer %d: rec count %d.\n", idx, rec_count);

    for(i=0; i<rec_count; i++) {
        pthread_mutex_lock (fifo->mut);

        while (fifo->empty) {

            #ifdef DEBUG_QUEUE_INFO
                printf ("consumer %d:     queue EMPTY.\n", idx);
            #endif

            pthread_cond_wait (fifo->notEmpty, fifo->mut);
        }

        dequeue (fifo, &d);
        pthread_mutex_unlock (fifo->mut);
        pthread_cond_signal (fifo->notFull);

        //Call Function from pointer
        (*(d->work))(d->arg);

    }

    return (NULL);
}
