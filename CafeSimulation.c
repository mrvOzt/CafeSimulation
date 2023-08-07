//--- INCLUDES ---
#include <pthread.h>         // have to include pthread.h library to use threads and threads function
#include <stdio.h>           // have to include stdio.h library to use printf(),scanf() etc.
#include <stdlib.h>          // have to include stdlib.h library --> information of memory allocation/freeing functions
#include <unistd.h>          // have to include unistd.h to access to the POSIX operating system API
#include <semaphore.h>       // have to include semophore.h to perform semaphore operations.
#include <sys/types.h>       // have to include sys/types.h to defines data types 
#include <sys/time.h>        // have to include sys/time.h to define the timeval structure

//--- PROJECT CONSTANTS ---
#define CUSTOMER_NUMBER 25              // customer number of project
#define CUSTOMER_ARRIVAL_TIME_MIN 1     // customer minimum arrive time of project
#define CUSTOMER_ARRIVAL_TIME_MAX 3     // customer minimum arrive time of project
#define REGISTER_NUMBER 5               // register number of project
#define COFFEE_TIME_MIN 2               // minimum making coffee time of project
#define COFFEE_TIME_MAX 5               // maximum making coffee time of project

//--- STRUCT ---
struct thread_data{ // this struct holds register id and customer id 
   int  th_id;      // customer thread id
   int  reg_id;     // register id
};
struct thread_data th_arg[CUSTOMER_NUMBER];  // array of thread_data struct for every customer's buying operation

//--- SEMOPHORES --- 
sem_t customer_ready[CUSTOMER_NUMBER];      // this semophore controls customer ready or not
sem_t modify_register[REGISTER_NUMBER];     // this semophore controls register empty or not

//--- THREADS ---
pthread_t main_customer_thread;             //  main customer thread
pthread_t register_thread;                  //  register thread
int err;                                    //  err is a control variable for pthread_creates


void *register_routine(void *th_arg){        
    struct thread_data *my_data;                    //this line create my_data
    my_data = (struct thread_data *) th_arg;        //this line holds struct arguments in my_data
    int id = my_data->th_id;                        // id is customer id
    int reg_id = my_data->reg_id;                   // reg_id is register id
    int counter = 0;                                // to control have customer or not
    while(1){
        sem_wait(&customer_ready[id]);              // waits for the customer
        int making_coffee_time = COFFEE_TIME_MIN + rand() % (COFFEE_TIME_MAX - COFFEE_TIME_MIN + 1);  // finds random second for make coffee
        sleep(making_coffee_time);                  // sleep until random making time
        printf("CUSTOMER %d FINISHED BUYING FROM REGISTER %d AFTER %d SECONDS.\n" , id,reg_id,making_coffee_time);     // print customer,register and time information
        sem_post(&modify_register[reg_id]);         //customer gone, now register empty , register ready for new customer
        counter++;                                  // increments counter
        if(counter == CUSTOMER_NUMBER){             // control counter equals or not to CUSTOMER NUMBER
            break;                                  // if equals each other exit while     
        }

    }    
}
void *customer_routine(int *id){
    int ran;                                            //random variable for random register
    int control = 1;                                    // while control variable
    int value;                                          // semophore value variable
    while(control){
        ran = rand() % 5;                               // random integer between [0,4]
        sem_getvalue(&modify_register[ran],&value);     // gets semophore value and records this value in value variable
        if(value == 1){                                 // if register is empty
            control = 0;                                // control = 0
            break;                                      // exit from while
        }  
    }
    sem_wait(&modify_register[ran]);                    // wait register
    sem_post(&customer_ready[*id]);                     //customer is ready now
    th_arg[*id].reg_id = ran;                           // assign values to struct variables
    th_arg[*id].th_id = *id;                            // assign values to struct variables
    printf("CUSTOMER %d GOES TO REGISTER %d.\n" , *id,ran);         // print which customer goes which register
    err = pthread_create(&register_thread, NULL, (void *)register_routine,&th_arg[*id]);  // create thread for register routine
    if(err){                                                    // if any error err = -1 
        printf("Failed to create main customer thread!");       // print error message
    }   
}
void *make_customer(){
    int err;                                                    // error variable
    int customer_id = 0;                                        // initialize customer id
    while(customer_id < CUSTOMER_NUMBER){                       // if customer_id smaller than total customer number 
        pthread_t customer_thread;                              // customer threads
        err = pthread_create(&customer_thread,NULL,(void *)customer_routine,&customer_id);    // create customer_thread in customer routine
        if(err){                                                        // if any error err = -1 
            printf("Failed to create customer %d \n!", customer_id);    // print error message
        }
        int wait_another_customer = CUSTOMER_ARRIVAL_TIME_MIN +rand() % (CUSTOMER_ARRIVAL_TIME_MAX - CUSTOMER_ARRIVAL_TIME_MIN + 1);    // find random second for customer arrive time 
        if(customer_id != -1){
            printf("CUSTOMER %d IS CREATED AFTER %d SECONDS.\n",customer_id,wait_another_customer);       // print customer created second
        }
        sleep(wait_another_customer);       // sleep until wait_customer_time 
        customer_id++;                      // increments customer_id
    }
}

int main(){
    srand(time(NULL));                          // for rand() function work better
    int err;                                    // error variable
    int i;                                      // initialize i
    //--- INITIALIZE SEMOPHORES ---
    for(i = 0 ; i < CUSTOMER_NUMBER;i++){       
        sem_init(&customer_ready[i], 0, 0);     // customer semophores values are 0 because customers are not ready now
    }
    for(i = 0 ; i < REGISTER_NUMBER;i++){
        sem_init(&modify_register[i], 0, 1);    // register semophores values are 1 because registers are empty and ready at the begining
    }
    err = pthread_create(&main_customer_thread, NULL, (void *)make_customer,NULL);      // main_customer_thread is created in make_customer function 
    if(err){
        printf("Failed to create main customer thread!");   // if any error,prints error message
    }
    pthread_join(register_thread,NULL);      //join register thread
    pthread_join(main_customer_thread, NULL);   // join customer thread
    pthread_exit(NULL);
}