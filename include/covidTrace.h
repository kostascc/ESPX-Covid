/**
 * Real Time Embedded Systems
 * 
 * Ⓒ 2022 K. Chatzis
 * kachatzis <at> ece.auth.gr
 **/

#ifndef COVIDTRACE_H
#define COVIDTRACE_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>	
#include <time.h> 
#include "pthread_fifo.h"

#define SPEED_MULTIPLIER 5000

#define POSITIVE_TEST_PROBABILITY 0.1

/**
 * Mac Address limits
 **/
#define MAC_ADDRESS_MIN 0x123456789F00
#define MAC_ADDRESS_MAX 0x123456789FFF

/**
 * Timing Configuration
 **/
#define MAX_TEMP_NEAR_CONTACT_DURATION       20*60  // 20 Minutes (in secs)
#define MIN_TEMP_NEAR_CONTACT_DURATION        4*60  //  4 Minutes (in secs)
#define MAX_NEAR_CONTACT_DURATION      14*24*60*60  // 14 Days    (in secs)
#define TICK_INTERVAL                           10  // 10 Seconds
#define COVID_TEST_INTERVAL                4*60*60  // 4 Hours    (in secs)
#define RUNTIME_SECS                   30*24*60*60  // 30 Days    (in secs)

// Tick Mutex
pthread_mutex_t tick_mutex;
// Contact Queue access mutex
pthread_mutex_t contact_mutex;
// Temporary Contact Queue mutex
pthread_mutex_t temp_find_mutex;
// Contact File mutex
pthread_mutex_t upload_file_mutex;
// BTnear file mutex
pthread_mutex_t btnear_file_mutex;
// CovidTest file mutex
pthread_mutex_t covidTest_file_mutex;

int upload_idx;

// Bluetooth near finds
FILE* fout_BTnear;

// Uploaded Contacts
FILE* fout_upload;

// Covid Tests
FILE* fout_CovidTest;

// Tick Latency
FILE* fout_tickLatency;

/**
 * uint48_t Integer containing the value of a MAC address
 **/
typedef struct { 
    uint64_t value: 48;
} macaddress ;

/**
 * MAC Address Struct
 * 
 * @param time clock_t
 * @param mac 48-bit mac address 
 **/
typedef struct {
    double time;
    macaddress* mac;
} record ;

/**
 * Record Queue Node
 * 
 * @param record Current node record
 * @param prev_node Previous (time-based) Node
 **/
typedef struct quenenode_{
    record* record;
    struct queuenode_* prev_node;
} queuenode ;

// Latest contact record in queue
queuenode* last_contact_queuenode;

// Temporary finds of queuenodes, that were seen 0-20 minutes ago.
queuenode* last_temp_queuenode;

// Running Clock
struct timespec system_clock_start;

/**
 * Timer tick function
 **/
void tick();

/**
 * Find mac address record in close contacts 4-20 minutes ago,
 * and deletes that record from the temporary contacts list.
 **/
record* find_previous_temp_close_contact(macaddress* mac);

/** 
 * Initialize Tracing
 **/
void init();

/**
 * Destroy Tracing
 **/
void destroy();

/**
 * Find mac address record in no close contacts 0-4 minutes ago
 **/
bool find_previous_temp_no_close_contact(macaddress* mac);

/**
 * Translate actual integer time to algorithmic
 **/
int translate_time_int(int time);

/**
 * Translate actual float time to algorithmic
 **/
float translate_time_float(float time);

/**
 * Translate actual double time to algorithmic
 **/
double translate_time_double(double time);

/**
 * Destroy a single queuenode and its' contents
 **/
void destroy_queuenode(queuenode* queuenode);

/**
 * Create dynamically allocated record
 **/
// record* create_record(clock_t time, macaddress* mac);

/**
 * Scan for close-by bluetooth device
 **/
macaddress* BTnearMe();

/**
 * Converts Clock difference to real-time seconds
 **/
double clock_diff_to_seconds(clock_t start, clock_t end);

/**
 * Test device owner for Covid.
 * Returns true if tested positive.
 * Edit POSITIVE_TEST_PROBABILITY to configure positive test probability.
 **/
bool testCOVID();

/**
 * Notify close contacts of a Positive Test.
 * Close Contact Records will be removed from queue after the upload.
 **/
void uploadContacts(macaddress** mac, int count);

double getRealClockSeconds();

#endif
