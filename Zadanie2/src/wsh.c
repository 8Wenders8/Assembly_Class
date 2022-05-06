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

int wsh_read_socket(char **buffer, uint32_t *buffer_size){
	uint32_t recv_size;
	int ret;
	char *temp;
	if((ret = read(conn_fd, &recv_size, sizeof(uint32_t))) == -1)
		handle_error("read");
	
	if(!ret) return 0;
	*buffer_size = ntohl(recv_size);

	if(*buffer_size > BUFF_SIZE){
		if(!(temp = realloc(*buffer, *buffer_size)))
			err_msg("Error at reallocating space");
		*buffer = temp;	
	}
	
	if((ret = read(conn_fd, *buffer, *buffer_size)) == -1)
		handle_error("read");

	return ret;
}


char** wsh_parse(char *buffer, uint32_t buffer_size, int *index){
	char *token, **args, **temp;
	int temp_index = 0;
	if(!(args = (char**) malloc(buffer_size * sizeof(char*))))
		err_msg("Error at allocating space");

	if(!(temp = (char**) malloc(buffer_size * sizeof(char*))))
		err_msg("Error at allocating space");

	token = strtok(buffer, "\"");
	while(token){
		temp[temp_index++] = token;
		token = strtok(NULL, "\"");
	}

	for(int i = 0; i < temp_index; i++){
		if(!((i + 1) % 2)){
			args[(*index)++] = temp[i];
			continue;
		}

		token = strtok(temp[i], " \t\r");
		while(token){
			args[(*index)++] = token;
			token = strtok(NULL, " \t\r");
		}
	}

	args[*index] = NULL;
	return args;
}

void wsh_server_loop(){
	while(1){
		wsh_server();
		wsh_accept();
		char *buffer, **args;
		if(!(buffer = (char*) malloc(BUFF_SIZE * sizeof(char))))
			err_msg("Error at allocating space");

		while(1){
			uint32_t buffer_size = BUFF_SIZE;
			int argc = 0;
			/* Read. */
			if(!wsh_read_socket(&buffer, &buffer_size)){
				printf("Client disconnected..\n");
				break;
			}

			args = wsh_parse(buffer, buffer_size, &argc);
			for(int i=0;i<argc;i++) printf("%s\n",args[i]);

			if(!strcmp(buffer, "quit")){
				printf("Quitting..\n");
				break;
			}
			if(!strcmp(buffer, "halt")){
				free(buffer);
				// TODO FREE ARGS:for(i
				exit(EXIT_SUCCESS);
			}
			/* Parse. */
			/* Execute. */

			memset(buffer, '\0', buffer_size);
			if(!(buffer = (char*) realloc(buffer, BUFF_SIZE)))
				err_msg("Error at reallocating space");
		}
		close(sock_fd);
		close(conn_fd);
		unlink(PATH);
	}
}


void wsh_read_line(char** buffer, int* buffer_size){
	int index = 0, c;
	while((c = getchar()) != EOF && c != '\n'){
		if(index >= *buffer_size - 1){
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

		uint32_t send_size = htonl(buffer_size);
		write(sock_fd, &send_size, sizeof(uint32_t));

		write(sock_fd, buffer, buffer_size);
		if(!strcmp(buffer, "quit") || !strcmp(buffer, "halt")) break;
	}
	free(buffer);
	close(sock_fd);
	close(conn_fd);
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
