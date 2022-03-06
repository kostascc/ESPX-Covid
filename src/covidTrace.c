/**
 * Real Time Embedded Systems
 * 
 * Ⓒ 2022 K. Chatzis
 * kachatzis <at> ece.auth.gr
 **/

#include "covidTrace.h"

int tests = 0;


void tick(double* time_enqueued){
    pthread_mutex_lock(&tick_mutex);

    // Scan for bluetooth
    macaddress* mac = BTnearMe();

    double time_BTnear_ran = getRealClockSeconds();
    fprintf(fout_tickLatency,"%lf %lf %lf\n", *time_enqueued, time_BTnear_ran, time_BTnear_ran - *time_enqueued);
     

    if( !find_previous_temp_no_close_contact(mac) ){
        // Did not find no-close contact (0-3 minutes)

        record* record = find_previous_temp_close_contact(mac);
        if(record != NULL){
            // Close Contact Found

            // It has already been removed from the temporary records
            // Add it to the close contacts list
            queuenode* new_queuenode = malloc(sizeof(queuenode));

            pthread_mutex_lock(&contact_mutex);
            new_queuenode->prev_node = last_contact_queuenode;
            new_queuenode->record    = record;
            last_contact_queuenode   = new_queuenode;
            // printf("Close Contact: %#012lx\n", record->mac->value);
            pthread_mutex_unlock(&contact_mutex);

        }else{
            
            // Found No no-close or close contacts at all

            // Create no-close contact record
            record = malloc(sizeof(record));
            record->mac = mac;
            record->time = time_BTnear_ran * SPEED_MULTIPLIER;

            queuenode* new_temp_queuenode = malloc(sizeof(queuenode));
            
            pthread_mutex_lock(&temp_find_mutex);
            new_temp_queuenode->prev_node = last_temp_queuenode;
            new_temp_queuenode->record    = record;
            last_temp_queuenode = new_temp_queuenode;
            pthread_mutex_unlock(&temp_find_mutex);

        }
    }else{
        // Found no-close Contact (0+3 minutes ago)
        // Do Nothing

    }

    if( fabs(remainder(getRealClockSeconds()*SPEED_MULTIPLIER, COVID_TEST_INTERVAL)) < 0.7 ){
        tests ++;
        printf("TEST %d\n", tests);
        // Test
        if(testCOVID()){
            printf(" - Positive!\n");
            // Log positive test
            pthread_mutex_lock(&covidTest_file_mutex);
            fprintf(fout_CovidTest,"%f %d\n", getRealClockSeconds() * SPEED_MULTIPLIER, 1);
            pthread_mutex_unlock(&covidTest_file_mutex);

            // Upload near Contacts of last 14 Days
            scan_and_upload_near_contacts();

        }else{
            // Log Negative Test
            pthread_mutex_lock(&covidTest_file_mutex);
            fprintf(fout_CovidTest,"%f %d\n", getRealClockSeconds() * SPEED_MULTIPLIER, 0);
            pthread_mutex_unlock(&covidTest_file_mutex);

        }

    }

    clean_records();

    pthread_mutex_unlock(&tick_mutex);
}


void scan_and_upload_near_contacts(){

    pthread_mutex_lock(&upload_file_mutex);

    int count = 0;
    if(last_contact_queuenode != NULL){
        queuenode* q = last_contact_queuenode;
        while(true){
            count++;
            if(q->prev_node != NULL){
                q = q->prev_node;
            }else{
                break;
            }
        }
    }

    printf("count: %d\n", count);
    
    if(count > 0){

        macaddress** macs = (macaddress**)malloc(count*sizeof(macaddress));
        
        queuenode* q = last_contact_queuenode;

        for(int i=0; i<count; i++){

            if(q->record != NULL){
                
                macs[i] = q->record->mac;

            }else{
                macs[i] = NULL;
            }
            q = q->prev_node;
        }
        
        uploadContacts(macs, count);

        free(macs);
        macs = NULL;

        destroy_queuenode(last_contact_queuenode);
        last_contact_queuenode = NULL;

    }

    pthread_mutex_unlock(&upload_file_mutex);

    return;
}


void destroy_queuenode(queuenode* queuenode){
    
    if(queuenode->prev_node != NULL)
        destroy_queuenode(queuenode->prev_node);

    if(queuenode->record != NULL){

        if(queuenode->record->mac != NULL){
            free(queuenode->record->mac);
            queuenode->record->mac = NULL;
        }

        free(queuenode->record);
        queuenode->record = NULL;
    }
    free(queuenode);

    return;
}


// record* create_record(double time, macaddress* mac){

//     record* record = malloc(sizeof(record));

//     record->time = time;
//     record->mac = mac;

//     return record;
// }


void clean_records(){

    if(last_temp_queuenode != NULL){
        pthread_mutex_lock(&temp_find_mutex);

        queuenode* parent_node = NULL;
        queuenode* current_node = last_temp_queuenode;
        
        while(true){

            if(current_node->record != NULL){
                // Check and skip every record from 4 minutes ago
                if( getRealClockSeconds()*SPEED_MULTIPLIER - current_node->record->time > MAX_TEMP_NEAR_CONTACT_DURATION ){
                    if(parent_node!=NULL){
                        parent_node->prev_node = NULL;
                    }
                    // printf("Destroy Temp @%f: %f\n", getRealClockSeconds()*SPEED_MULTIPLIER, current_node->record->time);
                    destroy_queuenode(current_node);
                    break;
                }
            }

            parent_node = current_node;
            current_node = current_node->prev_node;
            if(current_node == NULL){
                break;
            }

        }

        pthread_mutex_unlock(&temp_find_mutex);
    }

    if(last_contact_queuenode != NULL){

        pthread_mutex_lock(&contact_mutex);
        
        queuenode* parent_node = NULL;
        queuenode* current_node = last_contact_queuenode;
        
        while(true){

            if(current_node->record != NULL){
                // Check and skip every record from 4 minutes ago
                if( getRealClockSeconds()*SPEED_MULTIPLIER - current_node->record->time > MAX_NEAR_CONTACT_DURATION ){
                    if(parent_node!=NULL){
                        parent_node->prev_node = NULL;
                    }
                    // printf("Destroy Contact @%f: %f\n", getRealClockSeconds()*SPEED_MULTIPLIER, current_node->record->time);
                    destroy_queuenode(current_node);
                    break;
                }
            }
 
            parent_node = current_node;
            current_node = current_node->prev_node;
            if(current_node == NULL){
                break;
            }

        }

        pthread_mutex_unlock(&contact_mutex);
        
    }

    return;

}


record* find_previous_temp_close_contact(macaddress* mac){

    if(last_temp_queuenode == NULL){
        return NULL;
    }

    queuenode* current_node = last_temp_queuenode;
    
    while(true){

        if(current_node->record != NULL){
            // If mac addresses are equal
            if(current_node->record->mac->value == mac->value){
                // If last scan of address was 4 minutes ago
                if( getRealClockSeconds()*SPEED_MULTIPLIER - current_node->record->time >= (double)MIN_TEMP_NEAR_CONTACT_DURATION){
                    
                    pthread_mutex_lock(&temp_find_mutex);
                    // Delete Record from node
                    record* record = current_node->record;
                    current_node->record = NULL;
                    pthread_mutex_unlock(&temp_find_mutex);

                    return record;
                }
            }
        }

        current_node = current_node->prev_node;
        if(current_node == NULL){
            return NULL;
        }

    }

    return NULL;
}


bool find_previous_temp_no_close_contact(macaddress* mac){

    if(last_temp_queuenode == NULL){
        return false;
    }

    queuenode* current_node = last_temp_queuenode;
    
    while(true){

        if(current_node->record != NULL){
            // Check and skip every record from 4 minutes ago
            if( getRealClockSeconds()*SPEED_MULTIPLIER - current_node->record->time >= (double)(MIN_TEMP_NEAR_CONTACT_DURATION) ){
                return false;
            }

            // If mac addresses are equal
            if(current_node->record->mac->value == mac->value){
                // If last scan of address was 4 minutes ago
                return true;
            }
        }

        current_node = current_node->prev_node;
        if(current_node == NULL){
            return false;
        }

    }

    return false;
}


macaddress* BTnearMe(){

    macaddress* ret = malloc(sizeof( macaddress));
    ret->value = (uint64_t)MAC_ADDRESS_MIN + ((uint64_t)rand()) % ((uint64_t)(MAC_ADDRESS_MAX - MAC_ADDRESS_MIN));

    pthread_mutex_lock(&btnear_file_mutex);
    fprintf(fout_BTnear,"%f %#012lx\n", getRealClockSeconds() * SPEED_MULTIPLIER, (int64_t)ret->value);
    pthread_mutex_unlock(&btnear_file_mutex);

    return ret;
}

double clock_diff_to_seconds(clock_t start, clock_t end){
    return (double) ( ((double)(end - start)) / CLOCKS_PER_SEC);
}

bool testCOVID(){
    return rand()/(RAND_MAX+1.0) < POSITIVE_TEST_PROBABILITY;
}

void uploadContacts(macaddress** mac, int count){

    upload_idx ++;

    for(int i=0; i<count; i++){
        if(mac[i] == NULL)
            continue;

        fprintf(fout_upload,"%d %f %#012lx\n", upload_idx, getRealClockSeconds() * SPEED_MULTIPLIER, mac[i]->value);
        
    }

}

int translate_time_int(int time){
    return (int)(time / SPEED_MULTIPLIER);
}

float translate_time_float(float time){
    return (float)((float)time / SPEED_MULTIPLIER);
}

double translate_time_double(double time){
    return (double)((double)time / SPEED_MULTIPLIER);
}

void covidTraceInit(){

    if (pthread_mutex_init(&tick_mutex, NULL) != 0) {
        printf("\n[ERROR] Mutex init failed\n");
        exit(1);
    }
    if (pthread_mutex_init(&contact_mutex, NULL) != 0) {
        printf("\n[ERROR] Mutex init failed\n");
        exit(1);
    }
    if (pthread_mutex_init(&temp_find_mutex, NULL) != 0) {
        printf("\n[ERROR] Mutex init failed\n");
        exit(1);
    }
    if (pthread_mutex_init(&upload_file_mutex, NULL) != 0) {
        printf("\n[ERROR] Mutex init failed\n");
        exit(1);
    }
    if (pthread_mutex_init(&btnear_file_mutex, NULL) != 0) {
        printf("\n[ERROR] Mutex init failed\n");
        exit(1);
    }
    if (pthread_mutex_init(&covidTest_file_mutex, NULL) != 0) {
        printf("\n[ERROR] Mutex init failed\n");
        exit(1);
    }

    // Clean outputs
    fout_BTnear = fopen("BTnear.txt", "w");
    fprintf(fout_BTnear,"");
    fout_upload = fopen("Upload.txt", "w");
    fprintf(fout_upload,"");
    fout_CovidTest = fopen("CovidTest.txt", "w");
    fprintf(fout_CovidTest,"");
    fout_tickLatency = fopen("TickLatency.txt", "w");
    fprintf(fout_tickLatency,"");

    // Set Queues
    last_contact_queuenode = NULL;
    last_temp_queuenode    = NULL;

    upload_idx = 0;

    // Start Clock
    clock_gettime(CLOCK_REALTIME, &system_clock_start);
}

void covidTraceDestroy(){
    pthread_mutex_destroy(&tick_mutex);
    pthread_mutex_destroy(&contact_mutex);
    pthread_mutex_destroy(&temp_find_mutex);
    pthread_mutex_destroy(&upload_file_mutex);
    pthread_mutex_destroy(&btnear_file_mutex);

    fclose(fout_BTnear);
    fclose(fout_upload);
}

double getRealClockSeconds(){
    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);
    double delta_s = ((double) ((end.tv_sec - system_clock_start.tv_sec) * 1000000 + (end.tv_nsec - system_clock_start.tv_nsec) / 1000))/1000000;
    return delta_s;
}
