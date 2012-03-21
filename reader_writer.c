/* ################code implements solution to famous reader writer synchronization problem#########################
   
   low is priority 2 which is higher than that of writer but lower than high , ie 10.
   if given total 3 readers only three will enter.. no cycling possible..
 */



#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>


int reader_count = 0;
char filename[100];
sem_t x,wrt;
int reader_freq,writer_freq;
void *reader(void *arg)
{
	int number;
	FILE *fp;
	number = ((int)arg);
	sem_wait(&x);
	reader_count++;
	if(reader_count==1)
		sem_wait(&wrt);
	sem_post(&x);
        char c='0';
	fp = fopen(filename,"r"); 
	while(1)
	{
		c=getc(fp);
		if(c!=EOF)
		{
		putchar(c);
		fflush(stdout);
		}
		else
			break;
	}
	fclose(fp);
	sem_wait(&x);
	reader_count--;
	if(reader_count==0)
		sem_post(&wrt);
	sem_post(&x);
}

void *writer(void *arg)
{
	int number,i;
	FILE *fp;
	number = ((int) arg);
	sem_wait(&wrt);
	fp = fopen(filename,"a+");
        fprintf(fp,"%s","A random statement written by the writer. ");
	fclose(fp);
	sem_post(&wrt);
}

struct sched_param sch[100];
pthread_t readid[100],writeid[100],read_init,write_init;
int l,h,w;

void *create_reader()
 {
       int i,j;
        
        j=1;
         for(i=1;i<=l;i++)
        {
                pthread_create(&readid[i],NULL,reader,(void *)i);
                sch[i].sched_priority=2;
                pthread_setschedparam(readid[i],SCHED_RR, &sch[i]);
                sleep(reader_freq);
        }
        while(j<=h)
        {
                pthread_create(&readid[i],NULL,reader,(void *)i);
                sch[i].sched_priority=10;
                pthread_setschedparam(readid[i],SCHED_RR, &sch[i]);
                j++;
                i++;
                sleep(reader_freq);
        }

	for(i=1;i<=l+h;i++)
	{
		pthread_join(readid[i],NULL);
	}
}

void *create_writer()
{
 	int i=1;

	while(i<=w)
        {
		pthread_create(&writeid[i],NULL,writer,(void*)i);
                sleep(writer_freq);
		i++;
        }

	
	for(i=1;i<=w;i++)
		pthread_join(writeid[i],NULL);

}

void main()
{
	int i,n,j; 
	sem_init(&x,0,1);
	sem_init(&wrt,0,1);
	printf("Enter the filename:\n");
	scanf("%s",filename);
	printf("Enter the number of low priority readers:\n");
	scanf("%d",&l);
	printf("Enter the number of high priority readers:\n");
	scanf("%d",&h);
	printf("Enter the number of writers:\n");
	scanf("%d",&w);
	printf("Enter the frequency of readers:\n");
	scanf("%d",&reader_freq);
	printf("Enter the frequency of writers:\n");
	scanf("%d",&writer_freq);

        pthread_create(&read_init,NULL,create_reader,NULL);
        pthread_create(&write_init,NULL,create_writer,NULL);
        pthread_join(read_init,NULL);
        pthread_join(write_init,NULL);

	
}


