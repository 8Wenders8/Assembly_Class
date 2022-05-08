#ifndef COMMON_H_
#define COMMON_H_

#include <arpa/inet.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define SERVER_ARG	"-s"
#define CLIENT_ARG	"-c"
#define PORT_ARG	"-p"
#define	PATH_ARG	"-u"
#define HELP_ARG	"-h"
#define UNKNOWN_ARG  "Error: Unknown argument"

#define MY_SOCK_PATH "/tmp/sckt"
#define LISTEN_BACKLOG 50
#define HELP_MSG	"NAME\n\twsh - SPAASM Wenders SHell\nSYNOPSIS\n\twsh [OPTIONS]\nDESCRIPTION\n\tInteractive shell by Matej Volansky. Running wsh without any arguments leads to running as server listening to default socket (/tmp/sckt).\nOPTIONS\n\t-s\t\tStart shell as server. Also default without any arguments.\n\t-c\t\tStart shell as client.\n\t-u [PATH]\tSpecify socket path.\n\t-p [PORT]\tSpecify TCP port.\n\t-h\t\tPrint out help message.\nCOMMANDS\n\thelp\t\tPrint out help message.\n\tquit\t\tDisconnect from the server.\n\thalt\t\tQuit both the server and all the clients."

#define BUFF_SIZE	256

#define handle_error(msg)  { perror(msg); exit(EXIT_FAILURE); }
#define err_msg(msg) { printf("%s\n", msg); exit(EXIT_FAILURE); } 
#define unknown_error(msg) { printf("%s \"%s\"\n", UNKNOWN_ARG, msg); exit(EXIT_FAILURE); }

extern char *OPTIONS[], *PATH, *PORT;
extern int sock_fd, conn_fd;
extern struct sockaddr_un server_addr, client_addr;
extern socklen_t server_addr_size, client_addr_size;

#endif
