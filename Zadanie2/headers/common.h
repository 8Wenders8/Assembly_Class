#ifndef COMMON_H_
#define COMMON_H_

#include <arpa/inet.h>
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

#define BUFF_SIZE	10

#define handle_error(msg)  { perror(msg); exit(EXIT_FAILURE); }
#define err_msg(msg) { printf("%s\n", msg); exit(EXIT_FAILURE); } 
#define unknown_error(msg) { printf("%s \"%s\"\n", UNKNOWN_ARG, msg); exit(EXIT_FAILURE); }

extern char *OPTIONS[], *PATH, *PORT;
extern int sock_fd, conn_fd;
extern struct sockaddr_un server_addr, client_addr;
extern socklen_t server_addr_size, client_addr_size;

#endif
