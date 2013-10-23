/**************
 * 
 * This is bak simulation model on QNX using threads.
 * 
 * Author : Rajeev Verma
 * 
 * 
 * ***********************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <unistd.h>

#define MY_PULSE_CODE   _PULSE_CODE_MINAVAIL

static int global_time = 420;
static int total_customer = 0;
static int max_customer_queue_wait = 0;
static int current_queue_legth = 0;
static int max_length_queue = 0;
static int max_transaction_time_1 = 0;
static int max_transaction_time_2 = 0;
static int max_transaction_time_3 = 0;
static int max_transaction_time = 0;
static int total_queue_time = 0;
static int total_service_time = 0;

int customer_queue_flag = 0;

pthread_mutex_t lock;
pthread_mutex_t max_value_lock;

typedef union {
        struct _pulse   pulse;
} my_message_t;  //This union is for timer module.

void *time_update( void *ptr );
void *teller_1( void *ptr );
void *teller_2( void *ptr );
void *teller_3( void *ptr );

/**************
 *
 * This structure defines the individual customer.
 *
 **************/
struct Customer
 {
        int time_in;
        struct Customer* next;
 }*rear, *front;

 /**************
  *
  * This function is used to add a customer into the queue.
  * Every Customer will add-up with his/her in-time to the system.
  *
  **************/

int customer_move_out()
{
	  //printf("Customer_move_out!\n");
	  pthread_mutex_lock(&lock);
	  if (current_queue_legth > 0)
	  	  current_queue_legth--;
      struct Customer *temp, *var=rear;
      //printf("Customer_move_out: var->time_in = %d rear = %d\n", var->time_in, rear->time_in);
      if(var==rear)
      {
             rear = rear->next;
             int current_customer_queuetime = var->time_in-global_time;
             total_queue_time += (current_customer_queuetime);
             if (current_customer_queuetime > max_customer_queue_wait)
            	 max_customer_queue_wait = current_customer_queuetime;
             //printf("Customer_move_out:Customer intime= %d outtime = %d wait time in queue = %d\n", var->time_in, global_time, var->time_in-global_time);
             free(var);
             if(rear == NULL)
            	 customer_queue_flag = 0;
             pthread_mutex_unlock(&lock);
             return 1;
      }
      else{
    	  printf("\nCustomer_move_out: Queue Empty");
    	  customer_queue_flag = 0;
    	  pthread_mutex_unlock(&lock);
    	  return 0;
      }
}
/**************
 *
 * This function is used to add a customer into the queue.
 * Every Customer will add-up with his/her in-time to the system.
 *
 **************/
void customer_move_in(int value)
{
	 //printf("Customer_come_in\n");
	 display_queue_status();
	 pthread_mutex_lock(&lock);
	 customer_queue_flag = 1;
	 current_queue_legth++;
	 if(current_queue_legth > max_length_queue)
		 max_length_queue = current_queue_legth;
	 struct Customer *temp;
     temp=(struct Customer *)malloc(sizeof(struct Customer));
     temp->time_in=value;

     if (front == NULL)
     {
           front=temp;
           front->next=NULL;
           rear=front;
     }
     else
     {
           front->next=temp;
           front=temp;
           front->next=NULL;
     }
     pthread_mutex_unlock(&lock);
}

void display_queue_status()
{
	 //printf("Display\n");
	 pthread_mutex_lock(&lock);
     struct Customer *var=rear;
     if(var!=NULL)
     {
           printf("\nCustomers in queue:  ");
           while(var!=NULL)
           {
                printf("\t%d",var->time_in);
                var=var->next;
           }
     printf("\n");
     }
     else{
    	 printf("display_queue_status: Queue is Empty\n");
         customer_queue_flag = 0;
     }
     pthread_mutex_unlock(&lock);
}


int main(int argc, char ***argv)
{
    printf("Bank is open.. Customer can come in now..:)\n");
	pthread_t thread0, thread1, thread2, thread3;
    const char *message1 = "Teller 1";
    const char *message2 = "Teller 2";
    const char *message3 = "Teller 3";
    int next_customer_in_time;
    int  timerThread, tellerThread1, tellerThread2, tellerThread3;

    // This is mutex for saving queue from Parallel access.
    if (pthread_mutex_init(&lock, NULL) != 0)
        {
            printf("\n mutex init failed\n");
            return 1;
        }
    if (pthread_mutex_init(&max_value_lock, NULL) != 0)
        {
            printf("\n Mutex init failed for max_value_lock\n");
            return 1;
        }

    /* independent threads for tellers */

    timerThread = pthread_create( &thread0, NULL, time_update, (void*) message1);
    tellerThread1 = pthread_create( &thread1, NULL, teller_1, (void*) message1);
    tellerThread2 = pthread_create( &thread2, NULL, teller_2, (void*) message2);
    tellerThread3 = pthread_create( &thread3, NULL, teller_3, (void*) message3);

     while (global_time > 0){
    	 srand(1);
    	 customer_move_in(global_time);
    	 total_customer ++;
         //next_customer_in_time = rand() % 8 + 1;
    	 next_customer_in_time = (rand() % 4 + 1);
         //printf("Main: next_customer_in_time = %d, global_time = %d\n", next_customer_in_time, global_time);
         usleep( 50000 * next_customer_in_time );
     }

     pthread_join( thread1, NULL);
     pthread_join( thread2, NULL);
     pthread_join( thread3, NULL);

     pthread_mutex_destroy(&max_value_lock);   // This is destroying the mutex.
     pthread_mutex_destroy(&lock);

     printf("\nBank is closed for the day.. Sorry for the inconvenience. \n");
     printf("Total customer serviced today : %d\n", total_customer);
     printf("Maximum wait time for customer in queue= %d Min\n", max_customer_queue_wait);
     printf("Maximum depth of the customer queue = %d customers\n", max_length_queue);
     printf("Max transaction time for teller-1 : %d\nMax transaction time for teller-2 : %d\nMax transaction time for teller-3 : %d\n", max_transaction_time_1, max_transaction_time_2, max_transaction_time_3);
     printf("Max transaction time for all the tellers : %d\n", max_transaction_time);
     printf("Avg time a customer spend in queue = %d min\n", total_queue_time/total_customer);
     printf("Avg time each customer spends with the teller = %d min\n", total_service_time/total_customer);

     exit(0);
}
/**************
 *
 * This is thread to update my system_time.
 *
 *
 **************/
void *time_update( void *ptr )
{
	 struct sigevent         event;
	 struct itimerspec       itime;
	 timer_t                 timer_id;
	 int                     chid, rcvid;
	 my_message_t            msg;

	 chid = ChannelCreate(0);

	 event.sigev_notify = SIGEV_PULSE;
	 event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0,
	                                    chid,
	                                    _NTO_SIDE_CHANNEL, 0);
	 event.sigev_priority = getprio(0);
	 event.sigev_code = MY_PULSE_CODE;
	 timer_create(CLOCK_REALTIME, &event, &timer_id);

	 itime.it_value.tv_sec = 0;
	 /* 100 ms = .1 secs */
	 itime.it_value.tv_nsec = 100000000;
	 itime.it_interval.tv_sec = 0;
	 /* 100 ms = .1 secs */
	 itime.it_interval.tv_nsec = 100000000;
	 timer_settime(timer_id, 0, &itime, NULL);
	 // This for loop will update the global_time for every 100 ms which is 1 minute in simulation time.
	 for (;;) {
	     rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
	     if (rcvid == 0) { /* we got a pulse */
	          if (msg.pulse.code == MY_PULSE_CODE) {
	        	  global_time--;
	            //printf("we got a pulse from our timer and time = %d\n", global_time);
	          } /* else other pulses ... */
	     } /* else other messages ... */
    }
}

/***********************
 *
 * This is thread for teller-1.
 *
 *
 ***********************/
void *teller_1( void *ptr )
{
	 usleep(200000);
	 int random_break = 0;
	 random_break = global_time - (rand()%30 + 30);
	 printf("This is from Thread_1.\n");
     while(customer_queue_flag != 0 || global_time > 0/*global_time > 0 || front!=0 && rear != 0*/){
    	 if (customer_move_out() == 0) {
    		 printf("Teller_1 sleeping\n");
    		 usleep(50000);
    	 }
         else {
        	 //display_queue_status();
        	 srand(1);
        	 int customer_time;    //individual customer time.
        	 display_queue_status();
        	 //customer_time = rand() % 12 + 1;
				 customer_time = rand() % 11 + 1;
	    		 pthread_mutex_lock(&max_value_lock);
	    		 total_service_time += customer_time;   //update the total service time for the Bank database.
	    		 pthread_mutex_unlock(&max_value_lock);
			 if(customer_time > max_transaction_time_1)
				 max_transaction_time_1 = customer_time;
			 if(customer_time > max_transaction_time){
				 pthread_mutex_lock(&max_value_lock);
				 max_transaction_time = customer_time;   //update the max transaction time for the Bank database.
				 pthread_mutex_unlock(&max_value_lock);
			 }
			 //printf("teller_1 : global_time = %d and customer_time = %d min\n", global_time, ((customer_time / 2) + (customer_time %2)) );
			 usleep( 50000 * customer_time );
         }
    	 //printf("This is thread1\n");
    	 if( global_time <= random_break && global_time >0 )
    	 {
    		int break_time = (rand()%3+1);
    		printf("Teller_1:I am on break for %d.\n", break_time);
    		usleep(50000 * break_time);
    		random_break = global_time - (rand()%30 + 30);
    	 }
     }
     printf("Teller_1 goes offline.\n");
}

/***********************
 *
 * This is thread for teller-2.
 *
 *
 ***********************/
void *teller_2( void *ptr )
{
	 usleep(400000);
	 int random_break = 0;
	 random_break = global_time - (rand()%30 + 30);
     while(customer_queue_flag != 0 || global_time > 0/* || front!=0 && rear != 0*/){
    	 if (customer_move_out() == 0) {
    	     		 printf("Teller_2 sleeping\n");
    	     		 usleep(50000);
    	 }
    	 else {
    		 srand(1);
    		 //display_queue_status();
    		 int customer_time;    //individual customer time.
    		 //customer_time = rand() % 12 + 1;
    		 customer_time = rand() % 11 + 1;

    		 pthread_mutex_lock(&max_value_lock);
    		 total_service_time += customer_time;  //update the total service time for the Bank database.
    		 pthread_mutex_unlock(&max_value_lock);

    		 if(customer_time > max_transaction_time_2)
    			 max_transaction_time_2 = customer_time;
    		 if(customer_time > max_transaction_time){
    			 pthread_mutex_lock(&max_value_lock);
    			 max_transaction_time = customer_time;   //update the max transaction time for the Bank database.
    			 pthread_mutex_unlock(&max_value_lock);
    		 }
    		 usleep( 50000 * customer_time);
    		 //printf("teller_2 : global_time = %d and customer_time = %d min\n", global_time, ((customer_time / 2) + (customer_time %2)) );
    	 }
    	 //printf("Teller_2: Lock_3\n");
     	 if(global_time <= random_break && global_time >0 )
     	 {
     		int break_time = (rand()%3+1);
     		printf("Teller_2:I am on break for %d.\n", break_time);
     		usleep(50000 * break_time);
     		random_break = global_time - (rand()%30+ 30);
     	 }
     }
     printf("Teller_2 going offline.\n");
}

/***********************
 *
 * This is thread for teller-4.
 *
 *
***********************/

void *teller_3( void *ptr )
{
     usleep(600000);
	 int random_break = 0;
	 random_break = global_time - (rand()%30 + 30);
     while( customer_queue_flag != 0 || global_time > 0/*global_time > 0 || front!=0 && rear != 0*/){
    	 if (customer_move_out() == 0) {
    	     		 printf("Teller_3 sleeping\n");
    	     		 usleep(50000);
    	 }
    	 else{
    		 srand(1);
    		 int customer_time;     //individual customer time.
    		 customer_time = rand() % 20 + 1;
    		 pthread_mutex_lock(&max_value_lock);
    		 total_service_time += customer_time;  //update the total service time for the Bank database.
    		 pthread_mutex_unlock(&max_value_lock);
    		 if(customer_time > max_transaction_time_3)
    			 max_transaction_time_3 = customer_time;
    		 if(customer_time > max_transaction_time){
    			 pthread_mutex_lock(&max_value_lock);
    			 max_transaction_time = customer_time;    //update the max transaction time for the Bank database.
    			 pthread_mutex_unlock(&max_value_lock);
    		 }
    		 //printf("teller_3 : global_time = %d  and customer_time = %d min\n", global_time, ((customer_time / 2) + (customer_time %2)) );
    		 usleep( 50000 * customer_time );
    		 //printf("This is from Thread_3.\n");
    	 }
     	if(global_time <= random_break)
     	{
     		int break_time = (rand()%3+1);
     		printf("Teller_3:I am on break for %d.\n", break_time);
     		usleep(50000 * break_time);
     		random_break = global_time - (rand()%30 + 30);
     	}
     }
     printf("Teller_3 going offline.\n");
}

