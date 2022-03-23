#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 80

/* background process (id, command) */
struct back_process{
	int id;
	char command[100];
};

int main(void){
	char *args[MAX_LINE/2 + 1];
	char *args2[MAX_LINE/2 + 1];
	int should_run = 1;

	struct back_process pro_info[100];
	int back_num = 0; /* number of background process */

	while (should_run) {
		printf("osh<");
		fflush(stdout);
		/* input */
		int i=0,j=0;
		int sign = 0;/* ">" -> 1, "<" -> 2, "|" -> 3 */
		int amp = 0;/* ampersand "&" */
		int cont = 0;/* check continue situation */
		do{
			char *tmp = malloc(sizeof(char)*20);
			scanf("%s",tmp);
			/* exit */
			if(strcmp(tmp,"exit") == 0){
				should_run = 0;
				cont++;
			}
			/* confirm background process */
			if(strcmp(tmp,"jobs") == 0){
				if(back_num == 0)
					printf("Not a background process\n");
				else{
					for(int k=0;k<back_num; k++){
						/* print background info */
						printf("%d\t%s\n",pro_info[k].id, pro_info[k].command);
					}
				}
				cont++;
			}
			/* confirm ampersand */
			if(strcmp(tmp,"&") == 0){
				amp++;
				break;
			}
			/* confirm sign */
			if(strcmp(tmp,">") == 0)
				sign = 1;
			else if(strcmp(tmp,"<") == 0)
				sign = 2;
			else if(strcmp(tmp,"|") == 0)
				sign = 3;
			else if(sign > 0){
				args2[j++] = tmp;
			}
			else
				args[i++] = tmp;
		} while(getc(stdin) == ' ');

		/* continue situation, ex) exit, jobs */
		if(cont != 0)
			continue;

		args[i] = NULL;
		args2[j] = NULL;
		/* fork */
		pid_t pid, pid2;
		int fd[2];
		pid = fork();
		if(pid < 0){ /* error occurred */
			fprintf(stderr,"Fork Failed");
			return 1;
		}
		else if(pid == 0){ /* child process */
			if(sign == 0){
				execvp(args[0],args);
			}
			else if(sign == 1){ /* command > file */
				/* create txt file */	
				int file = open(args2[0], O_CREAT | O_RDWR | S_IROTH, 0644);
				/* exception handling */
				if(file < 0){
					perror("error");
					exit(-1);
				}
				dup2(file, STDOUT_FILENO);
				close(file);
				execvp(args[0],args);
			}
			else if(sign == 2){ /* command < file */
				int file = open(args2[0], O_RDONLY, 0644);
				if(file < 0){
					perror("error");
					exit(-1);
				}
				dup2(file, STDIN_FILENO);
				close(file);
				execvp(args[0],args);
			}
			else if(sign == 3){ /* command | command */
				if(pipe(fd) == -1){
					fprintf(stderr,"pipe error");
					exit(1);
				}
				pid2 = fork();
				if(pid2 < 0){ /* error occurred */
					fprintf(stderr,"Fork2 Failed");
					return 1;
				}
				else if(pid2 == 0){ /* grandson process */
					dup2(fd[1],1);
					close(fd[0]);
					execvp(args[0],args);
					exit(1);
				}
				else{/* child process*/
					waitpid(pid2, NULL, 0);
					dup2(fd[0],0);
					close(fd[1]);
					execvp(args2[0],args2);
				}

			}
			exit(0);
		}
		else{ /* parent process */
			if(amp == 0)
				waitpid(pid,NULL,0);
			else{ /* &, background process */
				waitpid(WNOHANG, NULL, 0);
				printf("[%d] %d\n",back_num+1,pid);
				strcpy(pro_info[back_num].command , args[0]);
				pro_info[back_num++].id = pid;
			}
		}
	}
	return 0;
}
