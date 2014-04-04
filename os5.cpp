#include <pthread.h>	 //library function
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include "time_functions.h"


typedef struct	 //common structure to update and read
{
      int seconds;
      int milliseconds;
}common;

common db;

typedef struct	 //structure to hold the reader and writer number
{
int id;
}coun;

void *reader(void* );	
void *writer(void*);
void read_time(int i);
void write_time(int i);

int cread,cwrite,rdelay,wdelay,nt,ar=0,ww=0,aw=0,wr=0;
pthread_mutex_t monitor;	
pthread_cond_t canread,canwrite;
int i;

int main(int argc, char* argv[])
{
int i;
pthread_mutex_init(&monitor,NULL);	 //initializing the mutex
pthread_cond_init(&canread,NULL);	 //initializing the condition variables
pthread_cond_init(&canwrite,NULL);

      pthread_t reader_id[100];	 //variable to store the thread id to join it
pthread_t writer_id[100];

if(argc!=5)
 	{
          printf("enter the readers and writers with their delays:\n");
            exit(0);
    }
      
  cread = atoi(argv[1]);	 //getting reader, writer and their delays from command line
       cwrite = atoi(argv[2]);
       rdelay=atoi(argv[3]);
wdelay=atoi(argv[4]);
       
nt = cread+ cwrite;	 //total number of threads
coun *wid=(coun*)malloc(sizeof(coun)*cwrite);        	 //structure declaration to store the reader and writer number
       coun *rid=(coun*)malloc(sizeof(coun)*cread);

for(i=0;i<cwrite;i++)
{
wid[i].id = i;
}
for(i=0;i<cread;i++)
{
rid[i].id = i;
}

       int t1;
       for(i=0;i < cwrite;i++)
       { 
  t1=pthread_create(&writer_id[i],NULL,writer,(void *)&wid[i].id);     //creating the writer thread
              if(t1!=0)
              {
                      printf("Error in creating the thread: \n");
                      exit(0);
              }
      }
for(i=0;i < cread;i++)
      {
              t1=pthread_create(&reader_id[i], NULL,reader, (void *)&rid[i].id);	//creating the reader thread

              if(t1!=0)
              {
                      printf("Error in creating the thread: \n");
                      exit(0);
              }
      }
      for(i=0;i<cwrite;i++)
      {
pthread_join(writer_id[i],NULL);                   //joining the writer thread
      }
for(i=0;i<cread;i++)
{
pthread_join(reader_id[i],NULL);	 //joining the reader thread
}
      return 0;
}

void *reader(void *i)
{
int *j=(int*)i;
int r_access = 10;
while(r_access>0)	 //each thread access 10 times
{	
pthread_mutex_lock(&monitor);	

if(aw+ww>0)	 //checks for the writer availability. If no writer, then proceed with reader
{
wr++;	 //counting waiting reader
pthread_cond_wait(&canread,&monitor);
wr--;
}
       	ar++;	 //counting active writer

       	pthread_mutex_unlock(&monitor);
       	printf(">>> DB value read =: %d:%d by reader number: %d\n", db.seconds,db.milliseconds,*j);	//reading the common storage
       	pthread_mutex_lock(&monitor);
       	ar--;	
       	if (ar==0&&ww>0)	 //checking for the writer. If there is a writer, signal the writer or continue with reader
{
           	 pthread_cond_signal(&canwrite);
}
else
{
pthread_cond_broadcast(&canread);
}
      pthread_mutex_unlock(&monitor);
      r_access--;
sleep(rdelay);	 //sleep to avoid starvation
}	
pthread_exit(0);	
}

void *writer(void *i)
{
int* j=(int*)i;
int w_access=10;
while(w_access>0)	
{
pthread_mutex_lock(&monitor);

     	 if(aw+ar>0)	 //to check for any active writer or reader. if so, put this in waiting queue
{
ww++;	 //waiting writer
pthread_cond_wait(&canwrite,&monitor);
ww--;
}
aw++;	 //active writer

    pthread_mutex_unlock(&monitor);

 	 pthread_mutex_lock(&monitor);
get_wall_time_ints(&db.seconds,&db.milliseconds);	 //updating the current wall clock time
               printf("*** DB value set to: %d:%d by writer number: %d\n", db.seconds,db.milliseconds,*j);
    pthread_mutex_unlock(&monitor);
    pthread_mutex_lock(&monitor);
    aw--;
      if (ww==0)	 //checks for writer. If no writer, signal the reader	
{
           	 pthread_cond_signal(&canread);
}
else
{
pthread_cond_broadcast(&canwrite);
}
w_access--;
    pthread_mutex_unlock(&monitor);
sleep(wdelay);	
}
pthread_exit(0);
}
