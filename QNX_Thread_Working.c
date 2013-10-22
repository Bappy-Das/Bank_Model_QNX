#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <unistd.h>

#define MY_PULSE_CODE   _PULSE_CODE_MINAVAIL

static int global_time = 60;

typedef union {
        struct _pulse   pulse;
} my_message_t;  //This union is for timer module.

void *time_update( void *ptr );
void *teller_1( void *ptr );
void *teller_2( void *ptr );
void *teller_3( void *ptr );

struct Customer
 {
        int time_in;
        int time_out;
        struct Customer* next;
 }*rear, *front;

void delQueue()
{
      struct Customer *temp, *var=rear;
      if(var==rear)
      {
             rear = rear->next;
             free(var);
      }
      else
      printf("\nQueue Empty");
}

void push(int value)
{
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
}

void display()
{
     struct Customer *var=rear;
     if(var!=NULL)
     {
           printf("\nElements are as:  ");
           while(var!=NULL)
           {
                printf("\t%d",var->time_in);
                var=var->next;
           }
     printf("\n");
     }
     else
     printf("\nQueue is Empty");
}


int main(int *argc, char ***argv)
{
    pthread_t thread1, thread2, thread3;
    const char *message1 = "Teller 1";
    const char *message2 = "Teller 2";
    const char *message3 = "Teller 3";
    int next_customer_in_time;
    int  timerThread, tellerThread1, tellerThread2, tellerThread3;

    /* independent threads for tellers */

    timerThread = pthread_create( &thread1, NULL, time_update, (void*) message1);
   /* tellerThread1 = pthread_create( &thread1, NULL, teller_1, (void*) message1);
    tellerThread2 = pthread_create( &thread2, NULL, teller_2, (void*) message2);
    tellerThread3 = pthread_create( &thread3, NULL, teller_3, (void*) message3);
*/
     pthread_join( time_update, NULL);
    /* pthread_join( thread1, NULL);
     pthread_join( thread2, NULL);
     pthread_join( thread3, NULL);*/
     while (global_time > 0){
         next_customer_in_time = rand() % 8 + 1;
         //printf("next_customer_in_time = %d, global_time = %d\n", next_customer_in_time, global_time);
         usleep( 100000 * next_customer_in_time );
         push(global_time);
     }
     tellerThread1 = pthread_create( &thread1, NULL, teller_1, (void*) message1);
     pthread_join( thread1, NULL);
     printf("Teller 1 returns: %d\n",timerThread);
     printf("Teller 1 returns: %d\n",tellerThread1);
     printf("Teller 2 returns: %d\n",tellerThread2);
     printf("Teller 3 returns: %d\n",tellerThread3);

     exit(0);
}

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
	          	if (global_time > 0)
	        	  global_time--;
	            //printf("we got a pulse from our timer and time = %d\n", global_time);
	          } /* else other pulses ... */
	     } /* else other messages ... */
    }
}

void *teller_1( void *ptr )
{
     while(global_time > 0){
    	 int customer_time;    //individual customer time.
    	 customer_time = rand() % 12 + 1;
    	 //printf("teller_1 : global_time = %d and customer_time = %d min\n", global_time, ((customer_time / 2) + (customer_time %2)) );
    	 usleep( 50000 * customer_time );
    	 delQueue();
     }
}

void *teller_2( void *ptr )
{
     while(global_time > 0){
    	 int customer_time;    //individual customer time.
    	 customer_time = rand() % 12 + 1;
    	 printf("teller_2 : global_time = %d and customer_time = %d min\n", global_time, ((customer_time / 2) + (customer_time %2)) );
    	 usleep( 50000 * customer_time);
     }
}

void *teller_3( void *ptr )
{
     while(global_time > 0){
    	 int customer_time;     //individual customer time.
    	 customer_time = rand() % 12 + 1;
    	 printf("teller_3 : global_time = %d  and customer_time = %d min\n", global_time, ((customer_time / 2) + (customer_time %2)) );
    	 usleep( 50000 * customer_time );
     }
}

