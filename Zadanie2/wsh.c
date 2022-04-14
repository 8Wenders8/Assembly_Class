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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>


#define ARGS		5
#define SERVER_ARG	"-s"
#define CLIENT_ARG	"-c"
#define PORT_ARG	"-p"
#define	PATH_ARG	"-u"
#define HELP_ARG	"-h"
#define UNKNOWN_ARG  "Error: Unknown argument"
#define MY_SOCK_PATH "/tmp/sckt"
#define LISTEN_BACKLOG 50
#define HELP_MSG	"Interactive shell by Matej Volansky.\nFormat: wsh [OPTIONS]"

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

char *OPTIONS[] = {SERVER_ARG, CLIENT_ARG, PORT_ARG, PATH_ARG, HELP_ARG, NULL};
char *PATH = NULL;
char *PORT = NULL;

int cmp_list_list(char **src, char **dst){
	short int match = 1;
	for(char **s = src; *s != NULL; s++){
		if(s == src && **s != '-')
			return 1;
		
		if(s != src)
			if(**s != '-' && (!strcmp(*(s - 1), "-p") || !strcmp(*(s - 1), "-u"))) continue;
			else return (s - src) / sizeof(char**);

		for(char **d = dst ; *d != NULL; d++){
			int result = strcmp(*s, *d);
			match = !match ? match :result;
		}
	}
	return match;
}

void wsh_arg_check(char **argv){
	int err_no;
	if((err_no = cmp_list_list(argv + 1, OPTIONS))){
		printf("%s %s\n", UNKNOWN_ARG, argv[err_no]);
		exit(EXIT_FAILURE);
	}
}

void wsh_conn_check(int argc, char **argv){
	if(--argc)
		switch(argv[2][1]){
			case 'p':{ 
				if(argv[3] == NULL) {printf("Error: -p option without port\n"); exit(EXIT_FAILURE); }
				PORT = (char*) malloc(sizeof(argv[3]));
				strcpy(PORT, argv[3]); }; break; 
			case 'u':{
				if(argv[3] == NULL) {printf("Error: -u option without path\n"); exit(EXIT_FAILURE); }
				PATH = (char*) malloc(sizeof(argv[3]));	
				strcpy(PATH, argv[3]); }; break; 
			default: break; 
		}
}

void wsh_server_socket(){
    int sock_fd, conn_fd;
    struct sockaddr_un server_addr, client_addr;
    socklen_t client_addr_size;

	if((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) // Create a socket endpoint, if successful returns file descriptor.
			handle_error("socket");		

    memset(&server_addr, 0, sizeof(server_addr));	// Init my_addr ( structure ) with zeros
    server_addr.sun_family = AF_UNIX;			// set family localost AF_UNIX

    strncpy(server_addr.sun_path, MY_SOCK_PATH, sizeof(server_addr.sun_path) - 1); // Copy MY_SOCK_PATH to my_addr_sun_path

    if (bind(sock_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
        handle_error("bind");

    if (listen(sock_fd, LISTEN_BACKLOG) == -1)
        handle_error("listen");

    client_addr_size = sizeof(client_addr);

    if((conn_fd = accept(sock_fd, (struct sockaddr *) &client_addr, &client_addr_size)) == -1)
        handle_error("accept");
}

void wsh_server_loop(){
	printf("Server\n");
	printf("Port: %s\n", PORT == NULL ? "Undefined" : PORT);
	//wsh_server_socket();
	//while(1){
	//	wsh_read();
	//	wsh_parse();
	//	wsh_execute();
}


void wsh_client_loop(){
	printf("Client\n");
}

void wsh_loop(short int type){
	switch(type){
		case 0: wsh_server_loop(); break;
		case 1: wsh_client_loop(); break;
		default: printf("Error at choosing mode\n"); break;
	}
}

void wsh_init(int argc, char **argv, short int *type){
	if(--argc){
		/* Check for unknown arguments */
		wsh_arg_check(argv);
		switch(argv[1][1]){
			case 'c': {*type = 1; wsh_conn_check(argc, argv);}; break;
			case 's': {*type = 0; wsh_conn_check(argc, argv);}; break;
			case 'h': { printf("%s\n", HELP_MSG); exit(EXIT_SUCCESS); } break; 
			default: *type = -1; break;
		}
	}
}

int main(int argc, char **argv){
	short int type = 0;			// Server(0) or Client(1)
	wsh_init(argc, argv, &type);
	wsh_loop(type);
	return 0;	
}

//TODO: -p port
