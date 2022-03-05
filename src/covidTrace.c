/**
 * Real Time Embedded Systems
 * 
 * Ⓒ 2022 K. Chatzis
 * kachatzis <at> ece.auth.gr
 **/

#include "covidTrace.h"


void tick(){
    pthread_mutex_lock(&tick_mutex);

    // Scan for bluetooth
    macaddress* mac = BTnearMe();

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
            pthread_mutex_unlock(&contact_mutex);

        }else{
            
            // Found No no-close or close contacts at all

            // Create no-close contact record
            record = malloc(sizeof(record));
            record->mac = mac;
            record->time = clock();

            queuenode* new_temp_queuenode = malloc(sizeof( queuenode));
            
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

    pthread_mutex_unlock(&tick_mutex);
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


// void add_record(record* record){

//     queuenode* queuenode = malloc(sizeof(queuenode));
//     queuenode->record = record;

//     // add_queuenode(queuenode); 


// }


record* create_record(clock_t time, macaddress* mac){

    record* record = malloc(sizeof(record));

    record->time = time;
    record->mac = mac;

    return record;
}


void clean_records(){

    if(last_temp_queuenode != NULL){

        queuenode* current_node = last_temp_queuenode;
        
        while(true){

            // Check and skip every record from 4 minutes ago
            if( clock_diff_to_seconds(current_node->record->time, clock()) > translate_time_double(MAX_TEMP_NEAR_CONTACT_DURATION) ){
                
                pthread_mutex_lock(&temp_find_mutex);
                destroy_queuenode(current_node);
                pthread_mutex_unlock(&temp_find_mutex);

                break;
            }

            current_node = current_node->prev_node;
            if(current_node == NULL){
                break;
            }

        }

    }

    if(last_contact_queuenode != NULL){

        queuenode* current_node = last_contact_queuenode;
        
        while(true){

            // Check and skip every record from 4 minutes ago
            if( clock_diff_to_seconds(current_node->record->time, clock()) > translate_time_double(MAX_NEAR_CONTACT_DURATION) ){
                
                pthread_mutex_lock(&contact_mutex);
                destroy_queuenode(current_node);
                pthread_mutex_unlock(&contact_mutex);

                break;
            }
 
            current_node = current_node->prev_node;
            if(current_node == NULL){
                break;
            }

        }
        
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
                if( clock_diff_to_seconds(current_node->record->time, clock()) >= translate_time_double(MIN_TEMP_NEAR_CONTACT_DURATION)){
                    
                    pthread_mutex_lock(&temp_find_mutex);
                    // Delete Record from node
                    record* record = current_node->record;
                    current_node->record = NULL;
                    pthread_mutex_unlock(&temp_find_mutex);

                    return current_node->record;
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
            if( clock_diff_to_seconds(current_node->record->time, clock()) >= translate_time_double(MIN_TEMP_NEAR_CONTACT_DURATION) ){
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
    return ret;
}

double clock_diff_to_seconds(clock_t start, clock_t end){
    return (double) ( ((double)(end - start)) / CLOCKS_PER_SEC);
}

bool testCOVID(){
    return rand()/(RAND_MAX+1.0) < POSITIVE_TEST_PROBABILITY;
}

void uploadContacts(macaddress** mac, int count){

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

void init(){

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


}

void destroy(){
    pthread_mutex_destroy(&tick_mutex);
    pthread_mutex_destroy(&contact_mutex);
    pthread_mutex_destroy(&temp_find_mutex);
}