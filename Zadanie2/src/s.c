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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>

#define SERVER_ARG	"-s"
#define CLIENT_ARG	"-c"
#define PORT_ARG	"-p"
#define	PATH_ARG	"-u"
#define HELP_ARG	"-h"
#define UNKNOWN_ARG  "Error: Unknown argument"
#define MY_SOCK_PATH "/tmp/sckt"
#define LISTEN_BACKLOG 50
#define HELP_MSG	"Interactive shell by Matej Volansky.\nFormat: wsh [OPTIONS]"

#define handle_error(msg)  { perror(msg); exit(EXIT_FAILURE); }
#define unknown_error(msg) { printf("%s \"%s\"\n", UNKNOWN_ARG, msg); exit(EXIT_FAILURE); }
#define err_msg(msg) { printf("%s\n", msg); exit(EXIT_FAILURE); } 

/* Arguments variables */
char *OPTIONS[] = {SERVER_ARG, CLIENT_ARG, PORT_ARG, PATH_ARG, HELP_ARG, NULL};
/* Conectivity variables */
char *PATH = NULL;
char *PORT = NULL;
int sock_fd, conn_fd;
struct sockaddr_un server_addr, client_addr;
socklen_t server_addr_size, client_addr_size;



/*  Generic function for comparing two lists of strings
 *	Use: Finding if there are any matching strings in the user provided arguments and shell intern arg options
 *	Returns: 
 *		0   - If atleast one match was found,
 *		1>= - Unknown arguments error, returns index of the unknown arguemnt
 *  */
int cmp_list_list(char **src, char **dst){
	short int match = 1, result = 0, curr_match;
	for(char **s = src; *s != NULL; s++){
		curr_match = 1;
		/* If the first string doesn't contain '-', exit */
		if(s == src && **s != '-') 
			return 1;
		/* If the current string isn't the first compared string and it doesn't contain '-',
		 * and the previous string was either '-p' or '-u' ( input data for the port or path arguments ) skip the string
		 * otherwise it's unknown argument error and return the index of that string */	
		if(s != src && **s != '-'){
			if(!strcmp(*(s - 1), "-p") || !strcmp(*(s - 1), "-u")) continue;
			else return (s - src) + 1;
		}
		/* Compare each string from src to each string from dst */	
		for(char **d = dst ; *d != NULL; d++){
			result = strcmp(*s, *d);
			/* If we already found atleast one match (strcmp return value 0) don't change the match flag */
			match = !match ? match : result;
			curr_match = !curr_match ? curr_match : result;
		}
		/* If none match, it's a unknown arg */
		if(curr_match != 0) return (s - src) + 1;
	}
	return match;
}

/* Function for checking if any unknown arguments were given
 * Return: void, exits with error with error message if any unknown args found
 *  */
void wsh_arg_check(char **argv){
	int err_no;
	if((err_no = cmp_list_list(argv + 1, OPTIONS)))
		unknown_error(argv[err_no]);
}

/* Function for checking if the options '-p' or '-u' and the input data were given  */
void wsh_conn_check(int argc, char **argv, int short_arg ){
	if((argc = argc - 1 - !short_arg) > 0){
		int index = short_arg ? 1 : 2;
		switch(argv[index][1]){
			case 'p':{ 
				if(argv[index + 1] == NULL || *argv[index + 1] == '-')
			   		{printf("Error: -p option without port\n"); exit(EXIT_FAILURE); }
				PORT = (char*) malloc(sizeof(argv[index + 1]));
				strcpy(PORT, argv[index + 1]); }; break; 
			case 'u':{
				if(argv[index + 1] == NULL || *argv[index + 1] == '-')
			   		{printf("Error: -u option without path to the socket\n"); exit(EXIT_FAILURE); }
				PATH = (char*) malloc(sizeof(argv[index + 1]));	
				strcpy(PATH, argv[index + 1]); }; break; 
			default: break; 
		}

		if(argc > 1)
			printf("Warning: %s ommited because of %s argument!\n", argv[index + 2], argv[index]);
	}
}


void wsh_prep_socket(){
	/* Create a socket endpoint, if successful returns file descriptor. */
	if((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) 
			handle_error("socket");		
	/* Init my_addr structure with zeors. */
    memset(&server_addr, 0, sizeof(server_addr));	
	/* Set family localhost AF_UNIX. */
    server_addr.sun_family = AF_UNIX;
	/* Copy MY_SOCK_PATH to my_addr_sun_path. */
    strncpy(server_addr.sun_path, PATH, sizeof(server_addr.sun_path) - 1);
}


void wsh_server_socket(){
	/* Bind socket. */
    if (bind(sock_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
        handle_error("bind");
	/* Listen at the socket */
    if (listen(sock_fd, LISTEN_BACKLOG) == -1)
        handle_error("listen");
}

void wsh_set_connection(){
	if(!PORT){
		if(!PATH){
			PATH = (char*) malloc(sizeof(MY_SOCK_PATH));	
			strcpy(PATH, MY_SOCK_PATH); 
			printf("No socket path given. Setting up default socket path..\n");
		}
		printf("Socket path: %s\n", PATH);
		wsh_prep_socket();
	} else { printf("Port: %s\n", PORT); }
}

void wsh_help(){
	printf("%s\n", HELP_MSG); 
	exit(EXIT_SUCCESS);
}

void wsh_prompt(){
	printf(">>> ");
}

void wsh_server_loop(){
	printf("[Server] ");
	wsh_set_connection();
	wsh_server_socket();
	while(1){
		/* Client addr. */
    	client_addr_size = sizeof(client_addr);
		/* Connection. */
    	if((conn_fd = accept(sock_fd, (struct sockaddr *) &client_addr, &client_addr_size)) == -1)
        	handle_error("accept");

		while(1){
			char BUFF[1024] = { '\0' };
			int n = read(conn_fd, BUFF, sizeof(BUFF));
			printf("Buffer: %s\n", BUFF);
			if(n) printf("%.*s\n", n, BUFF);
			int res;
			if(!(res = strcmp(BUFF, "quit"))){
				printf("Quitting..\n");
				break;
			}
			printf("Res: %d\n", res);
		}
	}
	//while(1){
	//	wsh_read();
	//	wsh_parse();
	//	wsh_execute();
}


void wsh_client_loop(){
	printf("Client\n");
	wsh_set_connection();
	server_addr_size = sizeof(server_addr);
    if((conn_fd = connect(sock_fd, (struct sockaddr *) &server_addr, server_addr_size)) == -1)
        handle_error("connect");

	while(1){
		wsh_prompt();

		char BUFF[1024] = { '\0' };
		scanf("%s", BUFF);
		write(sock_fd, BUFF, sizeof(BUFF));
	}
}

void wsh_client(){

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
