//  __  ____     __  
// |  \/  \ \   / /   Author: Matej Volansky
// | |\/| |\ \ / /    AIS ID: 103180
// | |  | | \ V /     SPAASM 2nd assignment: Interactive shell
// |_|  |_|  \_/      Project name: Wenders's shell (wsh) 
//--------------------------------------------------------------------
//	Description:
//	Just for clarification, 'Wenders' is the authors go to username.
//	The goal of the assignment is to design interactive shell in C 
//	using various syscalls. 
//
//	Platform: Intel 386/Linux/C or Intel 386/FreeBSD/C
//-------------------------------------------------------------------

#include "../headers/common.h"
#include "../headers/args.h"
#include "../headers/conn.h"

void wsh_help(){
	printf("%s\n", HELP_MSG); 
	exit(EXIT_SUCCESS);
}

void wsh_prompt(){
	printf(">>> ");
}

void wsh_server_loop(){
	wsh_server();
	while(1){
		wsh_accept();
		while(1){
			/* Read. */
			char BUFF[1024] = { '\0' };
			int n = read(conn_fd, BUFF, sizeof(BUFF));
			if(n) printf("%.*s\n", n, BUFF);
			int res;
			if(!(res = strcmp(BUFF, "quit"))){
				printf("Quitting..\n");
				break;
			}
			/* Parse. */
			/* Execute. */
		}
	}
}


void wsh_read_line(char** buffer, int* buffer_size){
	int index = 0, c;
	while((c = getchar()) != EOF && c != '\n'){
		if(index >= *buffer_size){
			*buffer_size += BUFF_SIZE;
			if(!(*buffer = (char*) realloc(*buffer, *buffer_size * sizeof(char)))) 
				err_msg("Error at reallocating space");
		}
		*(*buffer + index++) = c;
	}
	*(*buffer + index) = '\0';
}


void wsh_client_loop(){
	wsh_client();
	int buffer_size = BUFF_SIZE;
	char *buffer;
   	if(!(buffer = (char*) malloc(buffer_size * sizeof(char))))
		err_msg("Error at allocating space");

	while(1){
		wsh_prompt();
		wsh_read_line(&buffer, &buffer_size);
		write(sock_fd, buffer, buffer_size);
	}
}

void wsh_mode(short int mode){
	switch(mode){
		case 0: wsh_server_loop(); break;
		case 1: wsh_client_loop(); break;
		default: err_msg("Error at choosing mode"); break;
	}
}

void wsh_init(int argc, char **argv, short int *mode){
	if(--argc){
		/* Check for unknown arguments */
		wsh_arg_check(argv);
		switch(argv[1][1]){
			case 'c': {*mode = 1; wsh_conn_check(argc, argv, 0);}; break;
			case 's': {*mode = 0; wsh_conn_check(argc, argv, 0);}; break;
			case 'h': wsh_help(); break; 
			default:  {*mode = 0; wsh_conn_check(argc, argv, 1);}; break;
		}
	}
}

int main(int argc, char **argv){
	short int mode = 0;			// Server(0) or Client(1)
	//printf("Args: %d\n", argc);
	//for(int i = 0; i < argc; i++) printf("Arg no.%d: %s\n", i, argv[i]);
	wsh_init(argc, argv, &mode);
	wsh_mode(mode);
	return 0;	
}

//TODO: -p port
