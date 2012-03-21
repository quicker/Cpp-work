/*#######################the code copies multiple files using multitheads#####################
  the code takes two command line argments as input.
 1. path/name of the source file , only name should ensure that the file is in same directory as the code
 2. path/name of the destination file. to copy file with same name make both names same.
 path can be absolute as well as relative but should be of 'pwd' form , should not contain '~' and all
Srijan Tiwari
201001030
 */


#include<pthread.h>
#include<semaphore.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>


sem_t x,wsem;


FILE *file_read;
FILE *file_write;
int counter_threads=0;
int part_rest;
int file_pointer_count=0;
char file_name[100];
char file_dest[100];
int number_parts;

void func(char file_n[],char file_d[]);
void write(char *buff,int n,int loc);
void *read()
{
//	printf("thread has started\n");
	while(1)
	{
	FILE *read;
	int i;
	int id;
	int last=0;
	int file_loc=0;
	read = fopen(file_name,"r");	
	char buffer[512];
	sem_wait(&x);
	counter_threads++;
	if(counter_threads==number_parts)
		last=1;
	fseek(read,file_pointer_count,SEEK_CUR);
	file_loc = file_pointer_count;
	file_pointer_count=file_pointer_count + 512;
	sem_post(&x);
 	if(last==1)
	{
		for(i=0;i<part_rest;i++)
		{
			buffer[i] = getc(read);
		}
//		printf("write function is being called part rest\n");
		write(&buffer[0],part_rest,file_loc);
	
	}
	else
	{
		for(i=0;i<512;i++)
			buffer[i]=getc(read);
//		printf("write function is being called\n");
		write(&buffer[0],512,file_loc);
	}
	}
}

void write(char *buff,int n,int loc)
{
	int i;
	
//	sem_wait(&wsem);
//	printf("\nwriting is under process with n = %d\n",n);
	FILE *write;
	write = fopen(file_dest,"r+");
	fseek(write,loc,SEEK_CUR);
	for(i=0;i<n;i++)
	{
		fputc(*buff,write);
		buff++;
	}
	fclose(write);
	if(n!=512)
		exit(0);
//	sem_post(&wsem);
}


int main(int argc, char *argv[])
{
	int i;
//	int number_thread=10;
	char dest[100];
	strcpy(dest,argv[argc-1]);	
//	printf("this is the destination folder::: %s\n",dest);
	sem_init(&x,0,1);
	sem_init(&wsem,0,1);
	
	for(i=1;i<argc-1;i++)
	{
		func(argv[i],dest);
		counter_threads=0;
		strcpy(dest,argv[argc-1]);
	}
	return 0;
}


void func(char file_n[],char file_d[])
{
	int i;
	FILE *fp;
	int number_thread=10;
//	strcat(file_d,file_n);	
	fp = fopen(file_n,"r");
	strcpy(file_name,file_n);
	strcpy(file_dest,file_d);
	fseek(fp, 0L, SEEK_END);
	int sz = ftell(fp);
//	printf("file size is : %d\n",sz);
//	fseek(fp, 0L, SEEK_SET);
	fclose(fp);
	pthread_t threads[number_thread];
	number_parts = (sz/512) + 1;
//	printf("number parts = %d\n",number_parts);

	part_rest = sz%512;
//	printf("parts rest = %d\n",part_rest);
	fp = fopen(file_dest,"w");
	for(i=0;i<sz;i++)
		fputc('0',fp);
	fclose(fp);


//	while(1)

	for(i=0;i<number_thread;i++)
	{
		pthread_create(&threads[i],NULL,read,(void *)i);
	}

	for(i=0;i<number_thread;i++)
	{
		pthread_join(threads[i],NULL);
	}

//	printf("the number of threads entered semaphores are %d\n",counter_threads);
}
	


