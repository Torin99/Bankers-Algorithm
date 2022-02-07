/*
-------------------------------------
File:   BankersAlgorithm.c
Project: CP386 Assignment 4
Creates threads in a function and with the main and prints which is active, each runs 5 times

Link to GitHub Repository: https://github.com/Royce-33/BankersAlgorithm
-------------------------------------
Author:  Riley Adams & Torin Borton-McCallum
ID:      190416070 & 190824620
Email:   adam6070@mylaurier.ca & bort4620@mylaurier.ca
Version  2021-07-30
-------------------------------------
 */

/**
 * List of functions/functionality:
 *  - Banker should keep track of the available, maximum, currently allocated, and remaining need amounts of each resource
 *  - Implements the function for requesting resources, returns 0 if successful, -1 otherwise
 *  - Implements the function for releasing resources
 *  - Includes a safety algorithm to grant a request if it leaves the system in a safe state, denies it if otherwise
 *  - Allows the user to interactively enter a request for resources, to release resources, or to output the values of the different data structures
 *  - Should be invoked by passing the number of resources of each type via CLI to initialize the available array by said values.
 *    For example: "./q4 10 5 7 8" would set available to 10 for type A, 5 for type B, 7 for type C, and 8 for type D
 *  - Read in the sample input file, it contains the maximum number of requests for each customer (five customers, four resources)
 *    Where each line represents the maximum request of each resource type for each customer, the maximum data structure is initialized to these values.
 *  - Code will take in user entered commands regarding the requesting, releasing, and displaying of resources.
 *    Commands are as follows:
 *      * "RQ" -> requests resources and fills the allocation array, remember that a customer cannot request anything more than its maximum
 *      * "RL" -> releases resources entered from the the given customer 
 *      * "Status" -> Display the current state of available, maximum, allocation, and need arrays
 *      * "Run" -> Executes the safe sequence based on the current state and all the threads run the same function code and prints (see assign page for example output)
 */

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<pthread.h>
#include<stdbool.h>

/* BEGIN STRUCTURE FUNCTION AND GLOBAL VARIABLE DEFINITION */
#define NUM_RESOURCES 4 //Change me if you want to change the number resources available
const char *FILE_NAME = "sample4_in.txt";
const int BUFFER_SIZE = 32;
typedef struct customer {

    int *maximum;
    int *allocated;
    int *need;
    int id;
    bool finished;

} customer;

int main(int argc, char *args[]);
int read_file();
customer *customer_init(int *maximum_resources);
void *request_resources(customer *customer, int *requested_resources);
void *release_resources(customer *customer, int *release_request);
void *print_status();
void *run_safety_algorithm();
void *request_resources_safety(customer *customer, int *request_resources);
void *release_resources_safety(customer *customer, int *release_request);
void *thread_run();
int command_handler();

int num_customers = 0;
int available_resources[NUM_RESOURCES];
customer *customers = NULL; //pointer to a list of customers
/* END DEFINITIONS */

int main (int argc, char *args[]) {
    
    if (argc < 2) {

        printf("main -> Not enough available resources inputted, please try again!\n");
        return -1;
    }

    else {

        for (int i = 0; i < argc; i++) { //setting up available resources as defined with input

            //printf("Current argument: %s\n", args[i]);
            if ( i != 0 ) {
                //printf("current arg: %s\n", args[i]);
                int curr_arg = atoi(args[i]);
                //printf("integer version: %d\n", curr_arg);
                available_resources[i] = curr_arg;
                //printf("array element %d: %d\n", i, availabe_resources[i]);
            }
                

               
        }
        
        
        read_file(); //read in the contents of the file and initialize customer objects
        //printf("contents of the global customer list: %s\n", customers);

        printf("Number of customers: %d\n", num_customers);

        printf("Currently available resources:");

        for (int i = 1; i < NUM_RESOURCES + 1; i++){
            if (i == (NUM_RESOURCES + 1) - 1)
                printf(" %d\n", available_resources[i]);

            else    
                printf(" %d", available_resources[i]);
        }

        printf("Maximum resources from file:\n");

        for (int i = 1; i < num_customers + 1; i++) { //realloc adds empty space at the start of the list, so we need to start from second element and go 1 past num_customers to see all of them
            
            //printf("outside for loop iteration %d\n", i);

            for (int j = 0; j < NUM_RESOURCES; j++) {

                if (j == NUM_RESOURCES - 1) {
                    printf(" %d\n", customers[i].maximum[j]);
                }

                else if (j == 0) {
                    
                    printf("%d: %d", customers[i].id, customers[i].maximum[j]);
                }
                    

                else
                    printf(" %d", customers[i].maximum[j]);
                
            }

        }
    }

    command_handler();

    return 0;
}

/**
 * Reads the given input file and passes each line as an array of ints to the customer_init function
 */
int read_file() {

    FILE *in_file = fopen(FILE_NAME, "r"); //open the file in read mode
    
    char line[BUFFER_SIZE];
    
    customer *incoming_customers = NULL; //works similairly to the global list, collects each customer as it comes in

    if (in_file != NULL) {

        incoming_customers = (customer *) malloc(sizeof(customer) * 5); //intial setup so it can take a customer, and will resize to fit more
        int customer_counter = 1; //helps to realloc the size of the pointer list properly
        int k = 0;
        while (fgets(line, BUFFER_SIZE, in_file) != NULL) {
            char *line_token = line; //
            //printf("line_token value: %s\n", line_token);
            
            int curr_max[NUM_RESOURCES];
            char *char_token = strtok(line_token, ",");
            int i = 0;
            while (char_token != NULL) {
                
                curr_max[i] = atoi(char_token);
                //printf("curr_max[i] value: %d\n", curr_max[i]);
                char_token = strtok(NULL, ",");
                i++;


            }

            k++;
            num_customers++;
            customer_counter++;
            // printf("calling customer_init with:\n");

            // for (int i = 0; i < NUM_RESOURCES; i++) {

            //     if (i == NUM_RESOURCES - 1) {
            //         printf(" %d\n", curr_max[i]);

            //     }

            //     else
            //         printf(" %d", curr_max[i]);

            // }

            incoming_customers[k] = *customer_init(curr_max);
            customer_counter++;
            incoming_customers = realloc(incoming_customers, sizeof(customer) * customer_counter);

        }

        customers = incoming_customers;

    }

    return 0;
}
                          

/**
 * Helper function used in read_file(), takes the given array of ints and initializes a new customer object
 */
customer *customer_init(int *maximum_resources) {

    customer *new_customer = (customer *) malloc(sizeof(customer));
    
    new_customer->maximum = malloc(NUM_RESOURCES);
    new_customer->need = malloc(NUM_RESOURCES);
    new_customer->allocated = malloc(NUM_RESOURCES);
    new_customer->finished = false;

    for (int i = 0; i < NUM_RESOURCES; i++) {
        new_customer->maximum[i] = maximum_resources[i];
    }

    for (int i = 0; i < NUM_RESOURCES; i++) {
        new_customer->need[i] = maximum_resources[i];
    }
    
    for (int i = 0; i < NUM_RESOURCES; i++) {
        new_customer->allocated[i] = 0;
    }

    new_customer->id = num_customers - 1; //takes the non-updated num_customers so first customer id is zero and so on

    //Testing print statements
    // printf("%d: Setting max resources to:", new_customer->id);

    // for (int i = 0; i < NUM_RESOURCES; i++) {

    //     if (i == NUM_RESOURCES - 1) {
    //         printf(" %d\n", new_customer->maximum[i]);
    //     }
    //     else
    //         printf(" %d", new_customer->maximum[i]);
    // }

    // printf("%d: Setting needed resources to:", new_customer->id);

    // for (int i = 0; i < NUM_RESOURCES; i++) {

    //     if (i == NUM_RESOURCES - 1) {
    //         printf(" %d\n", new_customer->need[i]);
    //     }
    //     else
    //         printf(" %d", new_customer->need[i]);
    // }
    // printf("%d: Setting allocated resources to:", new_customer->id);

    // for (int i = 0; i < NUM_RESOURCES; i++) {

    //     if (i == NUM_RESOURCES - 1) {
    //         printf(" %d\n", new_customer->allocated[i]);
    //     }
    //     else
    //         printf(" %d", new_customer->allocated[i]);
    // }

    return new_customer;
}

/**
 * Function used to request resources from the banker
 */
void *request_resources(customer *customer, int *request_resources) {


    int *customer_max = customer->maximum; 
    int *customer_need = customer->need; 
    int *customer_allocated = customer->allocated;
    int customer_id = customer->id;
    
    //printf("request resources called with customer id: %d\n", customer->id);

    // for (int i = 0; i < NUM_RESOURCES; i++) {

    //     printf("request resources array element %d value: %d\n", i, request_resources[i]);
    // }

    bool safe = true; //assures the requested resources is less than or equal to the available resources
    bool have_max = true; //assures customer has not exceed their max
    for (int i = 0; i < NUM_RESOURCES; i++) { //checking each type of resource to make sure it can be safely allocated

        //printf("%dth value of available resources: %d\n", i, available_resources[i]);

        if (request_resources[i] > available_resources[i+1]) { //available needs to be +1 because of weird behaviour when available is being set
            //request is bigger than available
            //printf("%d: comparing requested: %d and available: %d set safe to false\n", i, request_resources[i], available_resources[i]);
            safe = false;
        }

        //printf("current value of customer allocated: %d\n", customer_allocated[i]);
        //printf("current value of customer max: %d\n", customer_max[i]);
        if (customer_allocated[i] != customer_max[i]) {
            //requesting customer does not have its max resources
            have_max = false; 
        }

    }

    //printf("values of safe:%d\nand have_max:%d\n", safe, have_max);

    if (safe && !(have_max)) { //if the request is safe and the customer does not have its max resources

        printf("Request is safe, granting resources.\n");

        for (int i = 0; i < NUM_RESOURCES; i++) {

            customer_allocated[i] += request_resources[i];
            customer_need[i] -= request_resources[i];
            available_resources[i+1] -= request_resources[i];
        }

    }
    
    else {
        printf("Request is not safe, resources will not be granted.\n");

    }

    command_handler();
    
    pthread_exit(0);

    return 0;
}

void *release_resources(customer *customer, int *release_request) {

    int *customer_max = customer->maximum; 
    int *customer_need = customer->need; 
    int *customer_allocated = customer->allocated;
    int customer_id = customer->id;
    
    bool safe = true;
    bool have_none = false;
    for (int i = 0; i < NUM_RESOURCES; i++) { //checking each type of resource to make sure it can be safely allocated

        //printf("%dth value of available resources: %d\n", i, available_resources[i]);

        if (release_request[i] > customer_allocated[i]) { 
            //request is bigger than allocated
            //printf("%d: comparing requested: %d and available: %d set safe to false\n", i, request_resources[i], available_resources[i]);
            safe = false;
        }

        //printf("current value of customer allocated: %d\n", customer_allocated[i]);
        //printf("current value of customer max: %d\n", customer_max[i]);
        if ( (release_request[i] != 0) && (customer_allocated[i] == 0) ) { //if we are trying to release a resource that the customer does not have
            //requesting customer does not have its max resources
            have_none = true; 
        }

    }

    //printf("values of safe:%d\nand have_max:%d\n", safe, have_max);

    if (safe && !(have_none)) { //if the request is safe and the customer does not have its max resources

        printf("The resources have been released successfully.\n");

        for (int i = 0; i < NUM_RESOURCES; i++) {

            customer_allocated[i] -= release_request[i];
            //customer_need[i] -= request_resources[i];
            available_resources[i+1] += customer_allocated[i];
        }

    }
    
    else {
        printf("Request is not safe, resources will not be released.\n");

    }

    //thread has finished its job, go back to command handler and exit the thread
    command_handler();

    pthread_exit(0);
    
    return 0;
}

/**
 * Function that prints the current status of the system
 */
void *print_status() {

    //prints Available resources
    printf("Available Resources:\n");
    for (int i = 1; i < NUM_RESOURCES + 1; i++) {

        if (i == 1) {
            printf("%d", available_resources[i]);
        }

        else if (i == (NUM_RESOURCES + 1) - 1) {
            printf(" %d\n", available_resources[i]);
        }

        else {
            printf(" %d", available_resources[i]);
        }

    }

    //prints Maximum resources
    printf("Maximum Resources:\n");
    for (int i = 1; i < num_customers + 1; i++) { //need it starting at 1 and going over num_customers by one because empty space from realloc is at the start of the list

        for (int j = 0; j < NUM_RESOURCES; j++) {

            if (j == 0) {
                printf("%d: %d", customers[i].id, customers[i].maximum[j]);
            }

            else if (j == NUM_RESOURCES - 1) {
                printf(" %d\n", customers[i].maximum[j]);
            }

            else {
                printf(" %d", customers[i].maximum[j]);
            }

        }

    }

    //prints Allocated resources
    printf("Allocated Resources:\n");
    for (int i = 1; i < num_customers + 1; i++) {

        for (int j = 0; j < NUM_RESOURCES; j++) {

            if (j == 0) {
                printf("%d: %d", customers[i].id, customers[i].allocated[j]);
            }

            else if (j == NUM_RESOURCES - 1) {
                printf(" %d\n", customers[i].allocated[j]);
            }

            else {
                printf(" %d", customers[i].allocated[j]);
            }

        }
    }

    //prints Needed resources
    printf("Needed Resources:\n");
    for (int i = 1; i < num_customers + 1; i++) {

        for (int j = 0; j < NUM_RESOURCES; j++) {

            if (j == 0) {
                printf("%d: %d", customers[i].id, customers[i].need[j]);
            }

            else if (j == NUM_RESOURCES - 1) {
                printf(" %d\n", customers[i].need[j]);
            }

            else {
                printf(" %d", customers[i].need[j]);
            }
        }

    }

    command_handler();
    pthread_exit(0);
    
    return 0;
}

/**
 * Function that takes the system in its current state and searches for safe sequence
 */
void *run_safety_algorithm() { //in current state it requests and releases resources properly, but gets stuck in a loop on the first thread because after the first call to request and release they are not safe
                               //and does not move onto the next customer

    bool no_sequence = false;
    bool done = false;
    int safe_sequence[num_customers];
    int counter = 0; //keeps track of the next open position in safe_sequence
    int run_counter = 0; //keeps track of how many passes the safety algorithm has made

    while ( (!done) || (!(no_sequence)) ) { //while we have not finished checking all customers or there is no safe sequence

        for (int i = 1; i < num_customers + 1; i++) { //starts at one so we dont try to use the empty space

            if (customers[i].finished == false) { //if the current customer has not been given its needed resources

                bool not_safe = false;
                bool full = true;
                for (int j = 0; j < NUM_RESOURCES; j++) {

                    if (customers[i].need[j] > available_resources[j+1]) { //j+1 so we don't get test against the zero at the start
                        not_safe = true;
                    }

                    else if (customers[i].allocated[j] != customers[i].maximum[j]) {
                        full = false;
                    }

                }


                if ((not_safe == false) && (full == false)) {

                    //printing info for current thread
                    printf(">>> Customer/Thread %d\n", customers[i].id);
                    
                    printf("Allocated Resources: ");
                    for (int j = 0; j < NUM_RESOURCES; j++) {

                        if (j == 0) {
                            printf("%d", customers[i].allocated[j]);
                        }

                        else if (j == NUM_RESOURCES - 1) {
                            printf(" %d\n", customers[i].allocated[j]);
                        }

                        else {
                            printf(" %d", customers[i].allocated[j]);
                        }
                    }

                    printf("Needed: ");
                    for (int j = 0; j < NUM_RESOURCES; j++) {

                        if (j == 0) {
                            printf("%d", customers[i].need[j]);
                        }

                        else if (j == NUM_RESOURCES - 1) {
                            printf(" %d\n", customers[i].need[j]);
                        }

                        else {
                            printf(" %d", customers[i].need[j]);
                        }
                    }

                    printf("Available: ");
                    for (int j = 0; j < NUM_RESOURCES; j++) {

                        if (j == 0) {
                            printf("%d", available_resources[j+1]);
                        }

                        else if (j == NUM_RESOURCES - 1) {
                            printf(" %d\n", available_resources[j+1]);
                        }

                        else {
                            printf(" %d", available_resources[j+1]);
                        }
                    }

                    //call request resources for the full amount
                    //printf("Now calling request_resources\n");
                    request_resources_safety(&customers[i], customers[i].need);
                    //printf("After request_resources\n");
                    //create and run thread now that it has its resources
                    pthread_t thread;
                    pthread_attr_t thread_attr;
                    int status;

                    status = pthread_attr_init(&thread_attr);

                    if (status != 0) {
                        printf("Error while creating thread attributes object for execution!\n");
                        exit(-1);
                    }

                    //printf("Before pthread_create\n");
                    status = pthread_create(&thread, &thread_attr, thread_run, NULL); //run the current thread through its critical section

                    if (status != 0) {
                        printf("Error while creating thread for execution!\n");
                        exit(-1);
                    }

                    pthread_join(thread, NULL); //so main thread waits for new one to finish, so that prints are structured properly

                    //then call release resources for full amount
                    release_resources_safety(&customers[i], customers[i].allocated);
                    
                    //print new available
                    printf("New Available: ");
                    for (int j = 0; j < NUM_RESOURCES; j++) {

                        if (j == 0) {
                            printf("%d", available_resources[j+1]);
                        }

                        else if (j == NUM_RESOURCES - 1) {
                            printf(" %d\n", available_resources[j+1]);
                        }

                        else {
                            printf(" %d", available_resources[j+1]);
                        }
                    }

                    //finally add the customer_id to safe sequence at position indicated by counter
                    safe_sequence[counter] = customers[i].id;
                    counter++;
                    customers[i].finished = true;

                }

            }

        }

        bool check_done = true;
        for (int i = 1; i < num_customers + 1; i++) {

            if (customers[i].finished == false) {

                check_done = false;
            }

        }

        if (run_counter > 2) {

            bool check_no_sequence = false;
            for (int i = 0; i < num_customers + 1; i++) {

                if (customers[i].finished == false) {

                    check_no_sequence = true;
                }
            }

            if (check_no_sequence == true) {
                no_sequence = true;
            }

        }

        //printf("Value of check_done: %d\n", check_done);
        if (check_done == true) {
            done = true;
            
        }

        run_counter++;
    }

    //printf("Outside of while loop\n");
    if (done == true) { //if the sequence is found print safe sequence

        printf("Safe Sequence is: ");
        for (int i = 0; i < num_customers; i++) {

            if (i == 0) {
                printf("%d", safe_sequence[i]);
            }

            else if (i == num_customers - 1) {
                printf(" %d\n", safe_sequence[i]);
            }

            else {
                printf(" %d", safe_sequence[i]);
            }

        }
    }

    else {
        printf("No possible safe sequence!\n");
    }

    command_handler();

    pthread_exit(0);
    
    return 0;
}

void *request_resources_safety(customer *customer, int *request_resources) {

    int *customer_max = customer->maximum;
    int *customer_need = customer->need; 
    int *customer_allocated = customer->allocated;
    int customer_id = customer->id;

    bool safe = true; //assures the requested resources is less than or equal to the available resources
    bool have_max = true; //assures customer has not exceed their max
    for (int i = 0; i < NUM_RESOURCES; i++) { //checking each type of resource to make sure it can be safely allocated

        if (request_resources[i] > available_resources[i+1]) { //available needs to be +1 because of weird behaviour when available is being set
            //request is bigger than available
            //printf("%d: comparing requested: %d and available: %d set safe to false\n", i, request_resources[i], available_resources[i]);
            safe = false;
        }

        //printf("current value of customer allocated: %d\n", customer_allocated[i]);
        //printf("current value of customer max: %d\n", customer_max[i]);
        if (customer_allocated[i] != customer_max[i]) {
            //requesting customer does not have its max resources
            have_max = false; 
        }

    }

    //printf("values of safe:%d\nand have_max:%d\n", safe, have_max);

    if (safe && !(have_max)) { //if the request is safe and the customer does not have its max resources
        //nothing happens within the safety_sequence version of this so that the math can be done properly within the release function

        //printf("Request is safe, granting resources.\n");
        

    }
    else {
        printf("Request is not safe, resources will not be granted.\n");
        
    }
    
    return 0;
}

void *release_resources_safety(customer *customer, int *release_request) {

    int *customer_max = customer->maximum;
    int *customer_need = customer->need; 
    int *customer_allocated = customer->allocated;
    int customer_id = customer->id;

    bool safe = true;
    bool have_none = false;
    for (int i = 0; i < NUM_RESOURCES; i++) { //checking each type of resource to make sure it can be safely allocated

        //printf("%dth value of available resources: %d\n", i, available_resources[i]);

        if (release_request[i] > customer_allocated[i]) { //available needs to be +1 because of weird behaviour when available is being set
            //request is bigger than allocated
            //printf("%d: comparing requested: %d and available: %d set safe to false\n", i, request_resources[i], available_resources[i]);
            safe = false;
        }

        //printf("current value of customer allocated: %d\n", customer_allocated[i]);
        //printf("current value of customer max: %d\n", customer_max[i]);
        if ( (release_request[i] != 0) && (customer_allocated[i] == 0) ) { //if we are trying to release a resource that the customer does not have
            //requesting customer does not have its max resources
            have_none = true; 
        }

    }

    //printf("values of safe:%d\nand have_max:%d\n", safe, have_max);

    if (safe && !(have_none)) { //if the request is safe and the customer does not have its max resources

        printf("Thread is releasing resources.\n");

        for (int i = 0; i < NUM_RESOURCES; i++) {
            available_resources[i+1] += customer_allocated[i];
            //printf("available resource of type %d is increasing by %d\n", i, customer_allocated[i]);
            customer_allocated[i] -= release_request[i];
            customer_need[i] -= release_request[i];

        }

    }
    
    else {
        printf("Request is not safe, resources will not be released.\n");

    }
    
    return 0;
}

/**
 * Function used in run_safety_algorithm to run customer thread
 */
void *thread_run() {

    printf("Thread has started\n");
    sleep(1);
    printf("Thread has finished\n");

    pthread_exit(0);

    return 0;
}

/**
 * Function used to handle user inputted commands once customer object setup is complete
 */
int command_handler() {

    char command_line[BUFFER_SIZE];

    printf("Enter a command: ");
    fgets(command_line, BUFFER_SIZE, stdin); //gets the inputted command 

    char *input = strtok(command_line, "\r\n"); //remove the newline character so we can compare strings properly
    
    //printf("results of strncmp against %s and quit: %d\n", command, strncmp(command, "quit", BUFFER_SIZE));

    while (strcmp(input, "Exit") != 0) {

        char *command = strtok(input, " ");

        //if the first set of characters is RQ, then get the requested resources and send them to a thread that calls request_resources
        if (strcmp(command, "RQ") == 0) {

            int request[NUM_RESOURCES];
            int i = 0;
            command = strtok(NULL, " ");
            int customer_id = atoi(command);
            while(command != NULL) {
                
                if (i < NUM_RESOURCES) {
                    //printf("Current value of command: %s\n", command);
                    command = strtok(NULL, " ");
                    request[i] = atoi(command);
                    //printf("value of request[i]: %d\n", request[i]);
                    i++;
                }

                else
                    break;
                
            }

            //printf("Now beginning thread creation\n");
            //now that we have our parameters pass the input and the request function call to a new thread
            customer *requesting_customer = &customers[customer_id + 1]; //needs to be plus one because the extra space from realloc sits at the start of the list
            
            pthread_t thread_id;
            pthread_attr_t thread_attributes;
            int status;

            status = pthread_attr_init(&thread_attributes);

            if (status != 0) {
                printf("Error creating thread attributes for request command!\n");
                exit(-1);
            }
            
            //printf("Thread attributes created\n");
            status = pthread_create(&thread_id, &thread_attributes, request_resources(requesting_customer, request), (requesting_customer, request));
            //printf("After thread creation\n");
            
            //printf("status value: %d\n", status);
            
            if (status != 0) {
                printf("Error creating thread for request command!\n");
                exit(-1);

            }

            //printf("Before pthread_join\n");
            pthread_join(thread_id, NULL);
            //printf("After pthread_join\n");

        }


        //if the first set of characters is RL, then get the released resources and send them to a thread that calls release_resources
        else if (strcmp(command, "RL") == 0) {

            int request[NUM_RESOURCES];
            int i = 0;
            command = strtok(NULL, " ");
            int customer_id = atoi(command);
            while(command != NULL) {
                
                if (i < NUM_RESOURCES) {
                    //printf("Current value of command: %s\n", command);
                    command = strtok(NULL, " ");
                    request[i] = atoi(command);
                    //printf("value of request[i]: %d\n", request[i]);
                    i++;
                }

                else
                    break;
                
            }

            customer *requesting_customer = &customers[customer_id + 1]; //needs to be plus 1 so we don't hit the empty space at the start of the list

            pthread_t thread_id;
            pthread_attr_t thread_attributes;
            int status;

            status = pthread_attr_init(&thread_attributes);

            if (status != 0) {
                printf("Error creating thread attributes for release command!\n");
                exit(-1);
            }

            status = pthread_create(&thread_id, &thread_attributes, release_resources(requesting_customer, request), (requesting_customer, request));

            if (status != 0) {
                printf("Error creating thread for release command!\n");
                exit(-1);

            }

            pthread_join(thread_id, NULL);

        }

        //if the first set of characters is Run, then create a thread that calls the safety algorithm
        else if (strcmp(command, "Run") == 0) {

            pthread_t thread_id;
            pthread_attr_t thread_attributes;
            int status;

            status = pthread_attr_init(&thread_attributes);

            if (status != 0) {
                printf("Error creating thread attributes for run command!\n");
                exit(-1);
            }

            status = pthread_create(&thread_id, &thread_attributes, run_safety_algorithm, NULL);

            if (status != 0) {
                printf("Error creating thread for run command!\n");
                exit(-1);
            }

            pthread_join(thread_id, NULL);
        }

        //if the first set of characters is Status, then create a thread that calls the print_status function
        else if (strcmp(command, "Status") == 0) {

            pthread_t thread_id;
            pthread_attr_t thread_attributes;
            int status;

            status = pthread_attr_init(&thread_attributes);

            if (status != 0) {
                printf("Error creating thread attributes for status command!\n");
                exit(-1);
            }

            status = pthread_create(&thread_id, &thread_attributes, print_status, NULL);

            if (status != 0) {
                printf("Error creating thread for status command!\n");
                exit(-1);
            }

            pthread_join(thread_id, NULL);


        }

        else {
            printf("Command not recognized, please try again!\n");
            command_handler();

        }

    }

    printf(">>> Program Terminated.\n");
    exit(0);
}