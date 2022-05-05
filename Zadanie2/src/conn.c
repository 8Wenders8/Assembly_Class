#include "../headers/common.h"
#include "../headers/conn.h"

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


void wsh_accept(){
    client_addr_size = sizeof(client_addr);
    if((conn_fd = accept(sock_fd, (struct sockaddr *) &client_addr, &client_addr_size)) == -1)
    	handle_error("accept");
}

void wsh_connect(){
	server_addr_size = sizeof(server_addr);
    if((conn_fd = connect(sock_fd, (struct sockaddr *) &server_addr, server_addr_size)) == -1)
        handle_error("connect");
}

void wsh_server(){
	wsh_set_connection();
	/* Bind socket. */
    if (bind(sock_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
        handle_error("bind");
	/* Listen at the socket */
    if (listen(sock_fd, LISTEN_BACKLOG) == -1)
        handle_error("listen");

	printf("[Server] ");
}

void wsh_client(){
	printf("[Client] \n");
	wsh_set_connection();
	wsh_connect();
}
