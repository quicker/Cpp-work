/*
   Srijan Tiwari 201001030
   the code implements linux shell using processes.

   plz give & seperated by space, else program turns to a bit unexpected "hang" eg . firefox &
   */



#include<iostream>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<string>
#include<cstdio>
#include<fstream>
using namespace std;

struct process_list{
	int pid;
	int is_running;
	char name[100];
};


struct process_list process[100];
int proc_count=0;
char home[100];

void shell_name()
{
	char user[100],hostname[100];
	char *pwd;
	char t[] = "/";
	int k,l,m;
	pwd = get_current_dir_name();
//	printf("%s \n",pwd);
	if(getlogin_r(user,100))
	{
		printf("getting login method failed\n");
		exit(1);
	}
	else if(gethostname(hostname,100))
	{
		printf("getting hostname method failed\n");
		exit(1);
	}
	char temp[100];

	int i=0;
	if(strncmp(pwd,home,(strlen(home)-1))==0)
	{
		for(k=(strlen(home));pwd[k]!='\0';k++)
		{
			temp[i] = pwd[k];
			i++;
		}
		temp[i]='\0';
		strcpy(pwd,temp);
	}


	printf("<%s@%s:~%s>",user,hostname,pwd);
	fflush(stdout);
	return;
}


int parse_history(char *cmdlist,char **arg_list)                 	// Parsing the command
{								
	int index=0;
	strcpy(arg_list[index++],strtok(cmdlist,"|"));
	char *tmp;
	while(tmp = strtok(NULL,"|"))
	{
		strcpy(arg_list[index++],tmp);
	}
	return index-1;
}

int parse(char *cmdlist,char **arg_list)                 	// Parsing the command
{								
	int index=0;
	strcpy(arg_list[index++],strtok(cmdlist," \t"));
	char *tmp;
	while(tmp = strtok(NULL," \t"))
	{
		strcpy(arg_list[index++],tmp);
	}
	arg_list[index]=NULL;    // normal execvp, the last arg_list[element] has to be NULL
	return index-1;						// index-1 returned to check if & , arg_list[index-1]=NULL
}
void sigchld_handler_background(int sig){
    pid_t pid;
    pid = wait(NULL);
//    printf("\nPID = %d\n",pid);
    int i;
    for(i=0;process[i].pid!=pid && i<proc_count;i++);
    if(i<proc_count){
        process[i].is_running=0;
        printf("\n%s exited successfully\n",process[i].name);
       	shell_name(); 
    }
    return;
}

void sigchld_handler_foreground(int sig){
    return;
}

void getRedir(char *cmd, char *inp, char *out, int *out1, int *and1)
{
	char *i,*o,*a;
	i = strstr(cmd,"<");					// Redirection thing
	o = strstr(cmd,">");
	a = strstr(cmd,"&");
	if(i)							// input redirection, "i" set to 1
	{
		i++;
		sscanf(i,"%s",inp);
	}
	else
		inp[0]='\0';
	if(o)							// output redirection
	{
		o++;						// double ">" , "o" set to 2
		if(*o == '>')
		{
			*out1 = 2;
			o++;
		}
		else						// single ">", "o" set to 1
		{
			*out1 = 1;
		}
		sscanf(o,"%s",out);
	}
	else
		out[0]='\0';
	if(a)							// single "&", "and1" set to 1, a check for "&" case
	{
		*and1 = 1;
	}
}

void pipe(char *command,char *buff)
{
	FILE *in;
	if (!(in = popen(command, "r")));               
	while (fgets(buff, sizeof(buff), in) != NULL ) 
	{
		cout << buff ;   
	}
	/* close the pipe */
	pclose(in);
}





int main(int argv,char *argc[])
{
	char *command = new char[4096];
	char *com1 = new char[4096];
	char *com2 = new char[4096];
	char *input = new char[1024];
	char *output = new char[1024];
	char **history = new char*[4096];
	string commandn;
	int j,k,l;
	ofstream a("history.txt", ios::trunc);
	ofstream b("temp.txt",ios::trunc);
	for(int i = 0;i < 1024;i++)
		history[i] = new char[1024];
	command[0]='\0';
	int fd = open("history.txt",O_RDWR|O_CREAT);	// history.txt contains list of all my commands / shell
	int fd2 = open("temp.txt",O_RDWR|O_CREAT);
	int out=0,and1=0;
	getcwd(home,100);
	signal(SIGCHLD,sigchld_handler_background);
	signal(SIGINT,SIG_IGN);
	while(1)
	{
		shell_name();
		scanf(" %[^\n]",command);			// scanning the command
kill:
	  	strcpy(com1,command);
		if(strcmp(command,"quit")==0)
			exit(1);

		write(fd,command,strlen(command));
		write(fd,"\n",1);

		char **arg_list = new char*[1024];		// initialising arrays
		for(int i = 0;i < 1024;i++)
			arg_list[i] = new char[1024];

		char **arg_list1 = new char*[1024];
		for(int i = 0;i < 1024;i++)
			arg_list1[i] = new char[1024];

		char **arg_list2 = new char*[1024];
		for(int i = 0;i < 1024;i++)
			arg_list2[i] = new char[1024];

		char **arg_list3 = new char*[1024];
		for(int i = 0;i < 1024;i++)
			arg_list3[i] = new char[1024];
		
		int as = 0;          					// counts no. of pipes
		for(int i=0;i<strlen(command);i++)
		{
			if(command[i] == '|')
				as++;
		}
		int inredir = 0;					// counts no. of <
		for(int i=0;i<strlen(command);i++)
		{
			if(command[i] == '<')
				inredir++;
		}
		int outredir = 0;
		for(int i=0;i<strlen(command);i++)			//counts no. of >
		{
			if(command[i] == '>')
				outredir++;
		}

		if(as > 0)
		{
			char *buffer = new char[1024];
			int ind = 0;
			ind = parse_history(com1,arg_list);
			string c123 = arg_list[1];
			for( int i=2;i<=ind;i++)
			{
				c123 = c123 + "|" + arg_list[i];
			}
			string comm1;
			if((!strncmp(arg_list[0],"hist",4))) 
			{
				
				if(isdigit(arg_list[0][4]))
				{
					cout << "we are dealing with hist command" << endl;
					int n = (arg_list[0][4] -48);
					for(k=5;k<strlen(arg_list[0]);k++)
					{
						n = n*10 + (arg_list[0][k]-48);
					}
					ifstream in ("history.txt");
					ofstream out("temp.txt");
					string l;
					for(k=0;k<n;k++)
					{
						getline(in,l);
						cout << l << endl;
						out << l;
					}
			                 
					comm1 = "cat temp.txt|" + c123;
				}
				else
				{
					comm1 = "cat history.txt|" + c123;
				}
				strcpy(com2,comm1.c_str());	
		//		cout << com2;
				pipe(com2,buffer);
			}
			else
			{
				pipe(command,buffer);
			}
			//call function
		}
		else
		{
			getRedir(command,input,output,&out,&and1);  	// calling Redirection function
			int p = parse(command,arg_list);		// calling parsing function
			
			if(!strcmp(arg_list[p],"&"))			// if we get an ampersand in last, make that NULL
				arg_list[p] = NULL;

			if(!strcmp(command,"quit"))			// exit command breaks
			{
				exit(1);
			}
			else if(!strncmp(arg_list[0],"hist",4))		// runs the history command
			{
			//	printf("hey we are in the history command\n");
				if(arg_list[1] == NULL)			// if no redirection operators in history
				{
					
			//		printf("\n have we already printed all the commands \n");
					if(isdigit(arg_list[0][4]))
					{
			//			cout << "we are dealing with hist command" << endl;
						char *histo[100];
						int count=0;
						int n = (arg_list[0][4] -48);
						for(k=5;k<strlen(arg_list[0]);k++)
						{
							n = n*10 + (arg_list[0][k]-48);
						}
			//			printf(" the value of n is %d\n",n);
						string line;
						ifstream in ("history.txt");
						if(in.is_open())
						{
							for(int g=0;g<n;g++)
							{
								getline(in,line);
								cout << line << endl;
							}
						}
						else
							cout << "could not open file"<< endl;
						in.close();


					}
					else
					{
						string line;
						ifstream in ("history.txt");
						if(in.is_open())
						{
							while(in.good())
							{
								getline(in,line);
								cout << line << endl;
							}
						}
						in.close();
					}

						
					
				}
				else if(!strcmp(arg_list[1],">"))	// if ">" output redirection given
				{
					int fd5;
					if(isdigit(arg_list[0][4]))
					{
						int n = (arg_list[0][4] -48);
						for(k=5;k<strlen(arg_list[0]);k++)
						{
							n = n*10 + (arg_list[0][k]-48);
						}
						ifstream in ("history.txt");
						ofstream out("temp.txt");
						string l;
						for(k=0;k<n;k++)
						{
							getline(in,l);
							cout << l << endl;
							out << l;
						}
						fd5 = open("temp.txt",O_RDONLY);
					}
					else 
						fd5 = open("history.txt",O_RDONLY);
						
					
			//		int fd5 = open("temp.txt",O_RDONLY);
					char *buf = new char[100];

					int sizeOFfile=lseek(fd5,0,SEEK_END);  
					lseek(fd5,0,SEEK_SET);
					int fd6 = creat(output,S_IRWXU);
					for(int i=0;i<sizeOFfile;i++)
					{
						int num=read(fd5,buf,1);	// reads 1 byte and puts it in buffer
						write(fd6,buf,num);	// writes num amount of bytes in fd6(output file)
					}
					//cout << buf << endl;
				}
				else if(!strcmp(arg_list[1],">>"))	// if ">>" output redirection given
				{
					char *buf = new char[100];
					int fd5 = open("history.txt",O_RDONLY);
					int sizeOFfile=lseek(fd5,0,SEEK_END);   
					lseek(fd5,0,SEEK_SET);
					int fd7 = open(output,O_APPEND|O_WRONLY);  // append mode
					for(int i=0;i<sizeOFfile-1;i++)
					{
						int num=read(fd5,buf,1);	// reads 1 byte and puts it in buffer
						write(fd7,buf,num);		// write num amount of bytes in fd7
					}
					//cout << buf << endl;
				}
				else
				{
					cout << "No such file or directory" << endl;
				}
			}
			else if(!strcmp(arg_list[0],"cd"))     // for cd command
			{

				if(arg_list[1]==NULL)
				{
					chdir(home);
				}
				else
					chdir(arg_list[1]);

			}
			else if(!strcmp(arg_list[0],"pid"))
			{
				if(arg_list[1]==NULL)
				{
					printf("command name: %s process_id:%d\n",argc[0],getpid());
				}
				else if(!(strcmp(arg_list[1],"all")))
				{
					printf("list of all the processes spawned by the shell\n");
					for(j=0;j<proc_count;j++)
						printf("command name:%s  process_id:%d\n",process[j].name,process[j].pid);
				}
				else if(!(strcmp(arg_list[1],"current")))
				{
					printf("list of currently executing processes spawned by shell\n");
					for(j=0;j<proc_count;j++)
					{
						if(process[j].is_running)
						{
							printf("command name: %s  process_id: %d",process[j].name,process[j].pid);
						}
					}
				}
			}
			else if(!strncmp(arg_list[0],"!hist",5))
			{
				int n;
				if(isdigit(arg_list[0][4]))
				{
					n = (arg_list[0][4] -48);
					for(k=5;k<strlen(arg_list[0]);k++)
					{
						n = n*10 + (arg_list[0][k]-48);
					}
				}
				ifstream getting("history.txt");
				if(getting.is_open())
				{
					for(int t=1;t<n;t++)
						getline(getting,commandn);
				}
				getline(getting,commandn);
				strcpy(command,commandn.c_str());
//				cout << command << endl;
				goto kill;
			

			}
			else
			{
				int child=fork();		// making a child process
				if(child == 0)
				{
					if(inredir > 1 or outredir > 2 or (out == 1 and outredir == 2))
					{
						//cout << "hi";
						char *buf = new char[1024];
						pipe(com1,buf);
						// function1
					}
					else if(input[0] != '\0' and output[0] == '\0')  // if redirection "<"
					{
						int index1=0;
						while(strcmp(arg_list[index1],"<"))
						{
							strcpy(arg_list1[index1], arg_list[index1]);
							index1++;
						}
						arg_list1[index1] = NULL;
						int fd1 = open(input,O_RDONLY);
						dup2(fd1,0);
						execvp(arg_list[0],arg_list1);
						perror(arg_list[0]);  //errors
					}
					else if(output[0] != '\0' and input[0] == '\0') // if redirection ">"
					{
						if(out == 1)				// specific ">"
						{
							int index2=0;
							while(strcmp(arg_list[index2],">"))
							{
								strcpy(arg_list2[index2], arg_list[index2]);
								index2++;
							}
							arg_list2[index2] = NULL;
							int fd2 = creat(output,S_IRWXU);
							dup2(fd2,1);
							execvp(arg_list[0],arg_list2);
							perror(arg_list[0]);  //errors
						}
						else if(out == 2)			// specific ">>"
						{
							int index2=0;
							while(strcmp(arg_list[index2],">>"))
							{
								strcpy(arg_list2[index2], arg_list[index2]);
								index2++;
							}
							arg_list2[index2] = NULL;
							int fd2 = open(output,O_WRONLY);
							lseek(fd2,0,SEEK_END);
							dup2(fd2,1);
							execvp(arg_list[0],arg_list2);
							perror(arg_list[0]);  //errors
						}

					}
					else if (input[0] != '\0' and output[0] != '\0') // for (< and >) and (< and >>) 
					{
						if(out == 1)				// for (< and >)
						{
							int index2=0;
							int index3=0;
							int flag=0;
							while(strcmp(arg_list[index2],"<"))
							{
								strcpy(arg_list2[index2], arg_list[index2]);
								index2++;
							}
							while(strcmp(arg_list[index3],">"))
							{
								strcpy(arg_list3[index3], arg_list[index3]);
								index3++;
							}
							arg_list2[index2] = NULL;	// getting the arguments before
							arg_list3[index3] = NULL;	// any of the operators come
							if(index2 >= index3)		// followed by NULL in execvp
								flag = 0;
							else 
								flag = 1;
							int fd1 = open(input,O_RDONLY);
							int fd2 = creat(output,S_IRWXU);
							dup2(fd1,0);
							dup2(fd2,1);
							if(flag == 0)
								execvp(arg_list[0],arg_list3);
							else
								execvp(arg_list[0],arg_list2);
							perror(arg_list[0]);  //errors
						}
						else if(out == 2)		// for (< and >>)
						{
							int index2=0;
							int index3=0;
							int flag=0;
							while(strcmp(arg_list[index2],"<"))
							{
								strcpy(arg_list2[index2], arg_list[index2]);
								index2++;
							}
							while(strcmp(arg_list[index3],">>"))
							{
								strcpy(arg_list3[index3], arg_list[index3]);
								index3++;
							}
							arg_list2[index2] = NULL;
							arg_list3[index3] = NULL;
							if(index2 >= index3)	// same as in (< and >)
								flag = 0;	// in this i have to seek to the end of 
							else 			// the file and then write in it
								flag = 1;
							int fd1 = open(input,O_RDONLY);
							int fd2 = open(output,O_WRONLY);
							lseek(fd2,0,SEEK_END);
							dup2(fd1,0);
							dup2(fd2,1);
							if(flag == 0)
								execvp(arg_list[0],arg_list3);
							else
								execvp(arg_list[0],arg_list2);
							perror(arg_list[0]);  //errors
						}
					}
					else if(input[0]=='\0' and output[0]=='\0')	// if no redirection operators
					{						// take the original arg_list and
						execvp(arg_list[0],arg_list);		// and run execvp
						perror(arg_list[0]);  //errors
					}
				}
				else if(child!=0 && child!=-1)
				{

					process[proc_count].pid = child;
					process[proc_count].is_running = 1;
					strcpy(process[proc_count].name,arg_list[0]);
					proc_count++;
//					printf("process count incremented\nproc_count is %d\n",proc_count);
					if(!and1)
					{
						signal(SIGCHLD,sigchld_handler_foreground);
						waitpid(child,NULL,0);
						signal(SIGCHLD,sigchld_handler_background);
						process[proc_count-1].is_running=0;
					}
				}
			/*	else 
				{
					if(!and1)		// to check for &
					{			// if no ampersand present
						int k;		// make the parent process wait normally
						wait(&k);
					}
				}*/
			}
		}
	}
}
