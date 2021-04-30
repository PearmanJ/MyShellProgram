#include <stdio.h>
#include <unistd.h>  
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <sys/wait.h>

//set text colors
#define GREEN "\x1B[32m"
#define BLUE "\x1B[34m"
#define BGREEN "\x1B[1m\033[32m"
#define BBLUE "\x1B[1m\033[34m" 
#define BCYAN "\x1B[1m\033[36m"
#define BBLACK "\x1B[1m\033[30m"
#define DEFAULT "\x1B[0m"
#define BDEFAULT "\x1B[1m\033[37m"

#define BRED     "\x1B[1m\033[31m"        
#define BYELLOW  "\x1B[1m\033[33m"        
#define BMAGENTA "\x1B[1m\033[35m"      


//Author: Jarrod Pearman
//PID: 3668653
//Course: Advanced Operating Systems

//global history buffer and global linebuffer for each user input
char inputBuffer[512];
char input2Buffer[512];
char input3Buffer[512];
char lineBuffer[512];
char History[10][512];

//initialize some functions for easy use
void addHistory();
int CheckHowManyPipes();

void printWelcome(){
	printf(BBLACK "--------------------------------------------------\n" DEFAULT);
	printf(BBLACK "--------------------------------------------------\n" DEFAULT);
	printf(BBLACK "-----------" DEFAULT);
	printf(BCYAN " Welcome to my Custom Shell " DEFAULT);
	printf(BBLACK "-----------\n" DEFAULT);
	printf(BBLACK "--------------------------------------------------\n" DEFAULT);
	printf(BBLACK "--------------------------------------------------\n" DEFAULT);
}

//builtin  help command
void printHelp(){
	printf("-------------------- Built In commands -------------------\n");
	printf("> exit  	 -closes the shell\n");
	printf("> help  	 -shows useful commands\n");
	printf("> |  		 -allows the usage of pipes\n");	
	printf("> history  	 -list the most recent commands(up to 10)\n");	
	printf("> history  <num> -executes command @ <num> in the history \n");
	printf("> time <command> -execute command and its execution time \n");		
	printf("----------------------------------------------------------\n");	
}
//readLine and trim leading adn trailing blank space
void trimString(){
	fgets(inputBuffer,512,stdin);	
	inputBuffer[strcspn(inputBuffer,"\n")]=0;
	if(inputBuffer[0]==' '){
		int i = 0, j=0;
		while(inputBuffer[i] != '\0'){
			if(i==0){
				
			}
			else if(!(inputBuffer[i]==' ' && inputBuffer[i+1]==' ')){
				input2Buffer[j]=inputBuffer[i];
				j++;
			}			
			i++;
		}
		if(input2Buffer[0]==' '){
			int x = 0;
			while(input2Buffer[x] != '\0'){
				input3Buffer[x]=input2Buffer[x+1];
				x++;
			}
		}
		else{
			strcpy(input3Buffer,input2Buffer);
		}
	}
	else{
		strcpy(input3Buffer,inputBuffer);
	}
	int len=strlen(input3Buffer);
	int i;
	for(i = len-1; i >= 0;i--){
		if(input3Buffer[i]!=' '){
			break;	
		}
		input3Buffer[i]='\0';
	}
	strcpy(lineBuffer,input3Buffer);	
}
//read user commands
void readLine(){
	trimString();	
}

//tokenize a single command(no pipes)
char** dealWithLine(char* tempBuffer){		
	char** cmd_tokens;
	char* token;
	int index=0, word_buff_size = 128;

	cmd_tokens = malloc( word_buff_size * sizeof(char*));
	if(!cmd_tokens){
		printf("ERROR WITH BUFFER ALLOCATION");
		exit(0);
	}		

	token = strtok(tempBuffer," ");
	while(token != NULL){
		cmd_tokens[index++] = token;

		if(index > word_buff_size){ //if for some reason we need more memory, allocate more
			cmd_tokens=realloc(cmd_tokens,(word_buff_size*2)*sizeof(char*));
			if(!cmd_tokens){
				printf("ERROR WITH BUFFER ALLOCATION");
				exit(0);
			}

		}
		token = strtok(NULL," ");				
	}
	cmd_tokens[index]=NULL;
	
	return cmd_tokens;
}

//excecute a single command(no pipes)
void execute_command(char** command){
	pid_t pid;
	
	pid=fork();
	if(pid==0){
		if(execvp(command[0],command)==-1){
			printf("Command Does Not Exist\n");	
		}		
		exit(0);
	}
	else if(pid<0){
		printf("Error Forking Command\n");
	}
	else{ 		
		wait(NULL);		
	}	
}

//function to execute a single piped command
//creates two child processes for each command and connects
//output and input accordingly between each child process
//then parent waits and we get our result
//function is uses since DoPipes2 seg faulted with only 1
//pipe and debugging/time cruntch couldnt find as to why.
void DoPipes(char* tempBuffer){		
	char** cmd1;
	char** cmd2;
	char* token;
	int index=0,index2=0, word_buff_size = 128, flag=0;
	
	//Generate our 2 commands
	cmd1 = malloc( word_buff_size * sizeof(char*));
	if(!cmd1){
		printf("ERROR WITH BUFFER ALLOCATION");
		exit(0);
	}	
	cmd2 = malloc( word_buff_size * sizeof(char*));
	if(!cmd2){
		printf("ERROR WITH BUFFER ALLOCATION");
		exit(0);
	}	
	
	token = strtok(tempBuffer," ");
	while(token != NULL){
		if(strcmp(token,"|")==0){
			flag=1;
			token = strtok(NULL," ");
		}
		if(flag==0){
			cmd1[index++] = token;
			
			//generally not needed
			if(index > word_buff_size){ //if for some reason we need more memory, allocate more
				cmd1=realloc(cmd1,(word_buff_size*2)*sizeof(char*));
				if(!cmd1){
					printf("ERROR WITH BUFFER ALLOCATION");
					exit(0);
				}

			}
			token = strtok(NULL," ");
		}
		else if(flag==1){
			cmd2[index2++] = token;
			
			//generally not needed
			if(index2 > word_buff_size){ //if for some reason we need more memory, allocate more
				cmd2=realloc(cmd2,(word_buff_size*2)*sizeof(char*));
				if(!cmd2){
					printf("ERROR WITH BUFFER ALLOCATION");
					exit(0);
				}

			}
			token = strtok(NULL," ");
		}
	}
	cmd1[index]=NULL;
	cmd2[index2]=NULL;
	
	//create and execute pipes
	int cfd[2];	
	pid_t pid1, pid2;
	
	if(pipe(cfd)==-1){
		printf("Error Creating Pipe\n");
		exit(0);
	}
	
	//command 1
	pid1=fork();
	if(pid1<0){
		printf("Error Forking Command 1\n");
	}
	else if(pid1==0){		
		//connect cmd1 STDOUT to cmd2 STDIN		
		dup2(cfd[1],1);	
		
		//close pipe
		close(cfd[0]);		
		
		//execute cmd1
		execvp(cmd1[0],cmd1);			
		exit(0);
	}
	
	//command 2
	pid2=fork();
	if(pid2<0){
		printf("Error Forking Command 2\n");
	}
	else if(pid2==0){		
		//connect cmd1 STDOUT to cmd2 STDIN		
		dup2(cfd[0],0);
		
		//close pipe
		close(cfd[1]);				
		
		//execute cmd2
		execvp(cmd2[0],cmd2);			
		exit(0);
	}	
	
	//close pipe
	close(cfd[0]);
	close(cfd[1]);	
	
	waitpid(pid1,NULL,0);
	waitpid(pid2,NULL,0);	
}

//function to handle more than 1 pipes
//it first parses the seperate commands into a 3d array
//then loops until it executes each command, passin the 
//output of the prev to the input of the next loops command
void DoPipes2(char* tempBuffer){	
	char*** listCMD;	
	char* token;
	int index=0,index2=0, word_buff_size = 128;
	int levels = CheckHowManyPipes(tempBuffer) + 1;	
	
	listCMD = (char***)malloc(16*sizeof(char**));
	for(int i=0;i<16;i++){
		listCMD[i] = (char**)malloc(word_buff_size*sizeof(char*));
		
	}	
	
	token = strtok(tempBuffer," ");
	while(token != NULL){
		if(strcmp(token,"|")==0){			
			listCMD[index][index2]=NULL;
			index++;
			index2=0;
			token = strtok(NULL," ");			
		}
			
		listCMD[index][index2++] = token;		

		if(index2 > word_buff_size){ //if for some reason we need more memory, allocate more
			listCMD[index]=realloc(listCMD[index],(word_buff_size*2)*sizeof(char*));
			if(!listCMD[index]){
				printf("ERROR WITH BUFFER ALLOCATION");
				exit(0);
			}
		}
		
		token = strtok(NULL," ");		
	}
	listCMD[index+1]=NULL;	
	
	int cfd[2];
	int prev = 0;
	pid_t pid;
	
	
	int i=0;
	for(i=0;i<levels;i++){		
		if(pipe(cfd)==-1){
			printf("Error Creating Pipe\n");
			exit(0);
		}
		pid=fork();
		if(pid<0){
			printf("Error Forking Command 1\n");
		}
		else if(pid==0){		
			//connect prev output to current input for the exec
			dup2(prev,0);// 0 is STDINPUT
			
			//If not the last command we save exec output for the next
			//if last command we do nothing and only read the prev input
			//as called before
			if(i < levels-1){
				dup2(cfd[1],1);//write the output of exec to pipe, 1 is STDOUTPUT
			}
			
			//close pipe
			close(cfd[0]);
						
			//execute cmd1
			execvp(listCMD[i][0],listCMD[i]);			
			exit(0);
		}
		else{
			waitpid(pid,NULL,0);
			close(cfd[1]);
			prev = cfd[0]; //save the what exec wrote to the pipe for the next command				
		}
	}
	free(listCMD);
}

int CheckHowManyPipes(char* buf){
	int z=0,index;		
	for(index=0;index<strlen(buf);index++){
		if(buf[index]=='|'){
			z++;
		}
	}
	return z;
}

int checkForBuiltIn(char* tempBuffer){	
	if(strcmp(tempBuffer,"help")==0){		
		printHelp();		
		return 0;
	}
	else if(strcmp(tempBuffer,"exit")==0){
		exit(0);
		return 0;
	}
	else if(strstr(tempBuffer,"history")!=NULL){		
		return 3;
	}
	else if(strstr(tempBuffer," | ")!=NULL){
		//DO PIPES	
		return 2;
	}
	else if(strcmp(tempBuffer,"")==0){
		//empty string
		return 0;
	}
	else{
		return 1;	
	}
}

void shiftHistory(char* tempBuffer){	
	for(int i=0;i<9;i++){		
		strcpy(History[9-i],History[8-i]);	
	}	
	strcpy(History[0],tempBuffer);
}


void printHistory(){
	for(int i=0;i<10;i++){
		if(strcmp(History[i],"")==0)
			break;
		printf("%d: %s\n",(i+1),History[i]);	
	}	
}

void addHistory(char* tempBuffer){	
	if(strcmp(tempBuffer,"history")!=0){
		shiftHistory(tempBuffer);	
	}	
}

void executeHistory(char* tempBuffer){	
	char** cmd;	
	
	if(checkForBuiltIn(tempBuffer)==0){
		addHistory(tempBuffer);		
	}
	else if(checkForBuiltIn(tempBuffer)==2){ //PIPES	
		addHistory(tempBuffer);
		int temp = CheckHowManyPipes(tempBuffer);
		if(temp==1)
			DoPipes(tempBuffer);
		else if(temp>1)
			DoPipes2(tempBuffer);
		else if(temp<=0){
			printf("Non Supported Command\n");
		}
	}
	else if(checkForBuiltIn(tempBuffer)==3){	
		printf("Command error\n");	
	}
	else{//basic function		
		addHistory(tempBuffer);
		cmd = dealWithLine(tempBuffer);		
		execute_command(cmd);			
	}	
}

int checkNum(char *word){
	int length = strlen(word);
	for(int i = 0; i < length; i++){
		if(isdigit(word[i])==0){
			return 0;	
		}		
	}
	return 1;
}

void handleHistory(char *tempBuffer){
	if(strcmp(tempBuffer,"history")==0){
		printf("--- Command History ---\n");
		printHistory();
	}
	else{		
		char* token;
		char** history_tokens;		

		history_tokens = malloc( 16 * sizeof(char*));
		if(!history_tokens){
			printf("ERROR WITH BUFFER ALLOCATION");
			exit(0);
		}	

		token = strtok(tempBuffer," ");		
		history_tokens[0]=token;
		token = strtok(NULL," ");
		history_tokens[1]=token;
		
		if(strcmp(history_tokens[0],"history")==0){
			if(checkNum(history_tokens[1])==1){
				int z =(int) strtol(history_tokens[1],NULL,10);
				char newBuffer[64];
				strcpy(newBuffer,History[z-1]);
				executeHistory(newBuffer);
			}
			else{
				printf("Invalid history argument\n");	
			}
		}			
	}
}

int checkForTime(char* tempBuffer){	
	char* token;	

	token = strtok(tempBuffer," ");
	if(strcmp(token,"time")==0){		
		return 0;
	}
	return -1;
}

char newBuffer[512];
void removeTime(char* tempBuffer){		
	char* token;		

	token = strtok(tempBuffer," ");
	token = strtok(NULL," ");
	strcpy(newBuffer,token);
	token = strtok(NULL," ");
	while(token != NULL){
		strcat(newBuffer," ");
		strcat(newBuffer,token);		
				
		token = strtok(NULL," ");
	}
	strcat(newBuffer,"\0");	
	strcpy(lineBuffer,newBuffer);
	
}

//Main
int main( int argc, char *argv[] )
{	
	char** cmd;	
	int timeFlag = 0;
	clock_t t,t2;	
	
	printWelcome();
		   
	//Main Shell Loop
	while(1){		
		printf(BGREEN "<>" DEFAULT);
		printf(BBLUE "$: " DEFAULT);		
		
		//Read input
		readLine();	
		
		//Handle input
		if(strstr(lineBuffer,"time ")!=NULL){	
			if(strlen(lineBuffer)<6)
				continue;	
			timeFlag=1;
			removeTime(lineBuffer);
			t = clock();
		}
		if(strstr(lineBuffer,"time")!=NULL){
			if(strlen(lineBuffer)<6)
				continue;			
		}
		if(strstr(lineBuffer,"time  ")!=NULL){			
			continue;			
		}
		if(checkForBuiltIn(lineBuffer)==0){//BUILT IN Function
			addHistory(lineBuffer);			
		}
		else if(checkForBuiltIn(lineBuffer)==2){ //PIPES	
			addHistory(lineBuffer);
			int temp = CheckHowManyPipes(lineBuffer);
			if(temp==1)
				DoPipes(lineBuffer);
			else if(temp>1) 
				DoPipes2(lineBuffer);
			else if(temp<=0){
				printf("Non Supported Command\n");
			}
		}
		else if(checkForBuiltIn(lineBuffer)==3){ //History Command
			handleHistory(lineBuffer);	
		}
		else{//basic function			
			addHistory(lineBuffer);
			cmd = dealWithLine(lineBuffer);		
			execute_command(cmd);			
		}					
		
		//clear buffer
		strcpy(lineBuffer,"");
		
		//If time flag used, print time
		if(timeFlag==1){
			t2 = clock();
			double exec_time = (double)(t2-t)/CLOCKS_PER_SEC;
			printf(BMAGENTA "\nExecution Time:" DEFAULT);
			printf(BYELLOW " %f seconds\n" DEFAULT,exec_time);
			timeFlag=0;
		}
	}
	//Free cmd memory
	free(cmd);
}
