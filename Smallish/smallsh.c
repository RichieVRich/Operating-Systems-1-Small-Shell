#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <limits.h>

void  print_start();
char* read_input();
char** segment_input(char*, int*);
int execute(char**,int );
int process(char**, int);
/*
 * Signal Handlers Utili 
 *
 */
void signa_intu(int);
void signa_sus(int);
int global_check = 0;
int gv = 0;
char HOME_STA[100];
/*
 *	Background Check utillity 
 *
 *
 */
int childExitStatus = -5;
void check_background();
int pid_array[100];
int pid_counter = 0;
int start = 0;

/*
 *	Builted in shell commands
 *
 */ 
int bi_cd( char** arg);
int bi_exit(char** arg);
int bi_status(char** arg);
char *bi_str[] = {"cd", "exit","status" }; // Array * holds the names for the buildins 
int (*bi_fun[])(char**) = {&bi_cd, &bi_exit, &bi_status}; // Function pointer points to the builtins
/*
 * Home_Directory:
 * Gets Current User Name and copys string to HOME_STA
 * HOME_STA is a global variable that hold the Home directory 
 *
 *
 */
char*  home_directory(){
	char cwd[PATH_MAX];
	char * token;
	register uid_t uid;
	register struct passwd *pw;
	char home[] = "/nfs/stak/users/";
	char* test  ;
	uid = geteuid();
	pw = getpwuid(uid);
	test = pw->pw_name;
	strcpy(test, (char*)pw->pw_name);
	strcat(home,test);
	strcpy(HOME_STA,home);
	strcat(HOME_STA,"/");
}

void main(){
	char* buffer;	
	char** split;
	char *arr;
	size_t len = 0;
	int set = 1;
	int val;
 	int i = 0;
	char cwd[PATH_MAX];
	home_directory();
	do{
		check_background();
		signal(SIGINT,signa_intu);
		signal(SIGTSTP,signa_sus);
		val = 0; // Must be zero'd out
		print_start();
		buffer = read_input();
		if( buffer != 0){
			split = segment_input(buffer, &val);
			set = execute(split, val);
		}
	}while(set != 0 );
		for( i = 0; i < val ; i++){
			free(split[i]);
		}
		free(split);	
		free(buffer);
		fflush(stdout);
	fflush(stdout);
}
/*
 *	An array that holds background pid and checks on them
 *	displays before returing promt to user.
 *
 */
void check_background(){
	int i = start;
	int exit_st = -5 ;
	for( i ; i < pid_counter; i++){ 
		pid_t actualPid = waitpid(pid_array[i],&exit_st,WNOHANG);
		if( WIFEXITED(exit_st) ){
			int exit_status = WEXITSTATUS(exit_st);
			
			printf("background pid %d is done:", actualPid, exit_status, exit_st);
			if( actualPid == -1 && start == i){
				start++	;
			}
			else if( exit_st == 1 || exit_st == 0){
				printf("exit vaule %d\n",exit_st );
				pid_array[i] = -1;
				break;
			}
			
		}else if(exit_st  >= 0){
				printf("background proccess %d terminated by signal %d\n",actualPid, exit_st);
				pid_array[i] = -1;
				break;
		}
	}
	if( exit_st != -5)
		childExitStatus = exit_st;

}
/*
 * print_start 
 * outputs: to screen :
 * input: no 
 * simply just outputs to stdout and fflushs stdout
 */

void print_start(){
	printf(": ");
	fflush(stdout);

}
/*
 *	read_input
 *	input: yes 
 *	output: returns char* buffer
 *
 */
char* read_input(){
	char* buffer;	
	size_t bufsize = 1024;
	char *arr;
	size_t len = 0;
	size_t read; 
	int set = 0;
	int i = 0;
	int dollar = 0;
	char p_id[64];
	int pid = getpid();
		buffer = (char *)malloc(bufsize * sizeof(char));
		if( buffer == NULL){
			perror("an error occured");
			exit(1);
		}
		if((read  = getline( &buffer, &bufsize, stdin)) != -1){
			;
		}
		len = bufsize - 1;
		if( buffer[0] ==  '\n'){
			return 0;
		}
		for( i = 0; i < len; i++){
			if( buffer[i] == '\n'){
				buffer[i] = '\0';	
				break;	
			}
			if( buffer[i] == '$'){
				dollar++;
					if(dollar == 2){
						buffer[i] = '\0';
						buffer[i-1] = '\0';
						sprintf( p_id, "%d", pid);
						strcat(buffer,p_id);
					}
			}

		if( buffer[0] == '#'){
			printf("%c", buffer[i]);
			set = 1;
		}
		fflush(stdout);
		}
		if (set == 1){
			printf("\n");
			return 0;
		}
		return buffer;
}
/*
 *	segment_input
 *	recieves char*, int*
 *	output: returns char** containing segmented buffer
 *	input: no
 *
 */ 
char** segment_input(char* buffer, int* v ){
	char* arr;
	char command[100][100];
	char temp[200] = "";
	char argument;
	size_t len;
	int max = 512, max_len = 2048; 
	int val = 0;
	int i = 0, j, r = 0, x = 0, x_temp = 0;
	arr = &buffer[0];
	len = strlen(arr);
	if( len > 2048){
		return 0;
	}
		for( j = 0; j < len; j++){ 	
			if( arr[j] == ' '){
				
				if( i > 0){
					// track how many spaces were found
					val++;  
					strcpy( command[r], temp);	
					r++;
					memset(temp,0, strlen(buffer));
				}
				
				
				i = 0;
			}else{
			 	temp[i] = arr[j];
			 	i++;
				x_temp = i;
			}	
			if( j + 1 == len){
				strcpy( command[r], temp);
				r++;
	
				if( i > 0){
					val++;
				}
			
			}
			if( x <  x_temp){
				x = x_temp;
			}
			fflush(stdout);
		}
		val++;
		if( val > max){
			return 0;
		}
		char** word;
		char *null = NULL;
		word = (char**)malloc(val*sizeof(char*));
		for(i = 0; i < val; i++){
			word[i] = (char*)malloc( x * sizeof(char));
		}
		x = val - 1;
		// The array needs to be NULL terminated.
		for( i = 0; i < val; i++){
			if( i == x){
				word[i] = NULL;
			}else{
				strcpy( word[i], command[i]);
			}
			
		}
		*v = val;
	return word;
}
/*
 * Signal Handlers
 * Returns the signal to default 
 *
 */
void sig_defau(int sig){
	signal(SIGINT, SIG_DFL);
}
void signa_intu(int sig){
	fflush(stdout);
}
void signa_sus(int sig){
	global_check++;
	if(global_check> 1){
		printf("Exiting foreground-only mode\n");
		fflush(stdout);
		global_check= 0;
	}else{
		printf("Entering foreground-only mode (& is now ignored)\n");
		fflush(stdout);
	}
}
/*
 *	Returns the number of elemements 
 *	+ Usefulk for future additional build-ins
 */
int bi_num(){
	return (sizeof(bi_str)/sizeof(bi_str[0]));

}
/*
 *	This function executes and returns signa_intud on 0 1
 *	if 0 exit was called and returns 0 
 *	else returns 1
 *
 *	Checks for buildin function first
 *	else calls process 
 *
 *
 */ 
int execute(char** arg,int val){
	int i = 0;
	
	for( i = 0; i < bi_num(); i++){
//		printf(" arg[%d] = %s, bi_str[%d] = %s\n", i,arg[0],i, bi_str[i]);
			if(strcmp(arg[0], bi_str[i]) == 0){
				return((*bi_fun[i])(arg));
			}

	}
//	printf("going to direct call \n");
	return process(arg, val);

	return 0;
}
/*
 *	redirect handles the calls with < || >
 *
 */
void redirect(char ** arg, int val){
	char* update[100];
	int sourceFD, targetFD, result, source_t = 0, i =0;
	pid_t pid, wpid, p ;
	signal(SIGINT, sig_defau);
	signal(SIGTSTP,SIG_IGN);
	pid = fork();
	    switch(pid){
		case -1: {
			 	perror("Hull Breached\n");
				exit(3);
				break;
			 }
		case 0: {
				for( i = 0; i < val-1; i++){
				// Source Fille 
					if( strcmp(arg[i], "<") == 0 ){
						sourceFD = open(arg[i+1], O_RDONLY);
						source_t = 1;
							if( sourceFD == -1 ){printf("%s: no such file or directory\n", arg[i+1]);exit(1);
						}
					}else if( strcmp(arg[i], ">") == 0){
				// Target File		
						
						targetFD = open(arg[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
						if( targetFD == -1){ printf("Something went wrongd\n");exit(1);}
					}
				}
				if( source_t == 1){
					result = dup2(sourceFD, 0);
				}
				result = dup2(targetFD, 1);
				execlp( arg[0], arg[0], NULL);
				perror("Command was not reconized\n");
				exit(2);
				break;
			}
		default: {
				fflush(stdout);
				pid_t actualPid = waitpid(pid,&childExitStatus,0);
				break;	
			 }
		}
}

/*
 *	process 
 *	takes in char** and int 
 *	forks and exec here 
 *	  for all non built ins
 *
 */
int process( char** arg, int val){
	int i, status;
	pid_t pid, wpid, p ;
	int test = 1;
	int proc_id = getpid();
	char p_id[64] ; 
	int run_background = 0;
//	Change $$ to the process id 
	for( i = 0; i < val - 1 ; i++){
		test = strcmp(arg[i], "$$");
		if((strcmp( arg[i],"$$") == 0 )) {
			sprintf( p_id, "%d", proc_id);
			strcpy(arg[i],p_id );
		}
		if(strcmp(arg[i],">")==0 || strcmp(arg[i], "<") == 0 ){
			redirect(arg,val);
			return 1;
		}
		fflush(stdout);
	}
	if( strcmp( arg[val - 2], "&") == 0) {
		arg[val-2] = NULL;
		run_background = 1;
	}
	fflush(stdout);

	if( run_background == 1 && global_check == 0){
				signal(SIGINT, SIG_IGN);
				signal(SIGTSTP,SIG_IGN);

	    pid = fork();
	    switch(pid){
		case -1: {
			 	perror("Hull Breached\n");
				exit(3);
				break;
			 }
		case 0: {
				execvp( arg[0], arg);
				printf("%s",arg[0]);
				exit(2);
				break;
				
			}
		default: {
				fflush(stdout);
				pid_t actualPid = waitpid(pid,&childExitStatus,WNOHANG);
				pid_array[pid_counter] = pid;	
				pid_counter++;
				printf(" background pid is %d\n", pid);
				break;	
			 }
		}
	}else{
				signal(SIGINT, sig_defau);
				signal(SIGTSTP,SIG_IGN);
		// 
	    pid = fork();
	    switch(pid){
		case -1: {
			 	perror("Hull Breached\n");
				exit(3);
				break;
			 }
		case 0: {
				execvp( arg[0], arg);
				printf("%s: no such file or directory\n",arg[0]);
				exit(2);
				break;
				
			}
		default: {
				fflush(stdout);
				pid_t actualPid = waitpid(pid,&childExitStatus,0);
				break;	
			 }
		}
	
	
	}
		return 1;	


}
/*
 *	Built-in implementations
 *	Returns 1 to keep going
 *	Returns 0 to exit program
 *
 */
int bi_cd(char** arg){
	char directory[50] = "$HOME";
	if( arg[1] == NULL || strcmp(arg[1],directory)== 0   ){
		if( chdir(HOME_STA) != 0 ){
			perror("Unable to go home\n");
		}
	}
	else{
		if( chdir( arg[1]) != 0 ){
			perror("Unable to change directory");
		}
	}
	return 1;
}

int bi_exit(char** arg){
	int i = 0;
	for( i ; i < pid_counter; i++){
		if( pid_array[i] != -1){
			kill(pid_array[i], 15);
		
		}
	}
	printf("exit called\n");
	return 0;
}
int bi_status(char** arg){
	// Might be incorrect to say everything greater than 15 is probably a failed 
	if( childExitStatus > 15){
		childExitStatus = 1;
	}
	printf( " exit value %d" , childExitStatus);
	return 1;
}
