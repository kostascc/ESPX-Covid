/**
 * Real Time Embedded Systems
 * 
 * Ⓒ 2021 K. Chatzis
 * kachatzis <at> ece.auth.gr
 **/

#ifndef PTHREAD_FIFO_H
#define PTHREAD_FIFO_H

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/**
 * A placeholder for functions in the Queue Structure
 * 
 * @param work Work Function
 * @param arg Arguments for Work Function
 **/
typedef struct {
  void * (*work)(void *);
  void * arg;
} workFunction;

/**
 * FIFO Queue Structure
 * 
 * @param buf Buffer of Functionns
 * @param head Head of Queue
 * @param tail Tail of Queue
 * @param full Is Queue Full
 * @param empty Is Queue Empty
 * @param queueSize Queue Size
 * @param mut Queue R/W Mutex (POSIX)
 * @param notFull Queue Not Full Condition (POSIX)
 * @param notEmpty Queue Not Empty Condition (POSIX)
 **/
typedef struct {
  workFunction** buf;
  long head, tail;
  int full, empty;
  int queueSize;
  pthread_mutex_t *mut;
  pthread_cond_t *notFull, *notEmpty;
} queue;

/**
 * Initiate an empty Queue
 * 
 * @param size Size of Queue
 **/ 
queue *queueInit (int size);

/**
 * Delete Queue and its contents
 **/
void queueDelete (queue *q);

/**
 * Add (Enqueue) function to Queue
 * 
 * @param q
 * @param in
 **/
void enqueue (queue *q, workFunction* in);

/**
 * Remove (Dequeue) Function from Queue
 * 
 * @param q
 * @param out
 **/
void dequeue (queue *q, workFunction **out);


#endif