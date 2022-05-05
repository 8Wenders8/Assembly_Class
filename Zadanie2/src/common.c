#include "../headers/common.h"

char *OPTIONS[] = {SERVER_ARG, CLIENT_ARG, PORT_ARG, PATH_ARG, HELP_ARG, NULL};
char *PATH = NULL;
char *PORT = NULL;
int sock_fd, conn_fd;
struct sockaddr_un server_addr, client_addr;
socklen_t server_addr_size, client_addr_size;
