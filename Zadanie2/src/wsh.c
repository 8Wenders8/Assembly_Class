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
	uid_t uid = geteuid();
	struct passwd *pwuid = getpwuid(uid);
	char hostname[BUFF_SIZE];
	gethostname(hostname, BUFF_SIZE - 1);
	time_t act_time;
	struct tm *time_info;

	time(&act_time);
	time_info = localtime(&act_time);

	printf("\x1B[33m(%02d:%02d)[%s@%s]$ \x1B[0m", time_info->tm_hour, time_info->tm_min, pwuid ? pwuid->pw_name : "", hostname);
	fflush(stdout);
}


int wsh_read_socket(int sock, char **buffer, uint32_t *buffer_size){
	uint32_t recv_size;
	int ret;
	char *temp;
	if((ret = read(sock, &recv_size, sizeof(uint32_t))) == -1)
		handle_error("read");
	
	if(!ret) return 0;
	*buffer_size = ntohl(recv_size);

	if(*buffer_size > BUFF_SIZE){
		if(!(temp = realloc(*buffer, *buffer_size)))
			err_msg("Error at reallocating space");
		*buffer = temp;	
	}
	
	if((ret = read(sock, *buffer, *buffer_size)) == -1)
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
			switch(*token){
				case '#': {args[*index] = malloc(2); strcpy(args[(*index)++], "");}; break;
				default: args[(*index)++] = token; break;
			}	
			token = strtok(NULL, " \t\r");
		}
	}

	args[*index] = NULL;
	return args;
}


void wsh_execute(char **args){
	pid_t pid, wpid;
	int status, stdout_fd[2], stderr_fd[2];

	if(pipe(stdout_fd) || pipe(stderr_fd))
		handle_error("pipe");
	
	if(!(pid = fork())){
		// Child procces
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		dup2(stdout_fd[1], STDOUT_FILENO);
		dup2(stderr_fd[1], STDERR_FILENO);

		close(stdout_fd[0]);
		close(stdout_fd[0]);
		close(stderr_fd[1]);
		close(stderr_fd[1]);

		if(execvp(args[0], args) == -1)
			handle_error("exec");
	} else if (pid < 0){
		perror("fork");
	} else {
		// Parent procces
		char *buffer, *temp;
		int read_len = 0, buffer_size = BUFF_SIZE;

		if(!(buffer = (char*) malloc(BUFF_SIZE * sizeof(char))))
			err_msg("Error at allocating space");

		close(stdout_fd[1]);
		close(stderr_fd[1]);

		do{
			wpid = waitpid(pid, &status, WUNTRACED);
		} while(!WIFEXITED(status) && !WIFSIGNALED(status));

		while((read_len += read(stdout_fd[0], buffer + read_len, BUFF_SIZE))){
			if(read_len == -1)
				handle_error("read");
			if(read_len == buffer_size){
				buffer_size += BUFF_SIZE;
				if(!(temp = realloc(buffer, buffer_size)))
					err_msg("Error at reallocating space");
				buffer = temp;
				continue;
			}
			break;
		}

		while((read_len += read(stderr_fd[0], buffer + read_len, BUFF_SIZE))){
			if(read_len == -1)
				handle_error("read");
			if(read_len == buffer_size){
				buffer_size += BUFF_SIZE;
				if(!(temp = realloc(buffer, buffer_size)))
					err_msg("Error at reallocating space");
				buffer = temp;
				continue;
			}
			break;
		}

		close(stdout_fd[0]);	
		close(stderr_fd[0]);	

		uint32_t send_size = htonl(buffer_size);
		write(conn_fd, &send_size, sizeof(uint32_t));
		write(conn_fd, buffer, buffer_size);
	}
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
			if(!wsh_read_socket(conn_fd, &buffer, &buffer_size)){
				printf("Client disconnected..\n");
				break;
			}

			int semi_count = 0, quote = 0;
			for(int i = 0; i < buffer_size; i++){
				if(buffer[i] == '"')
					quote = !quote;
				if(quote) continue;
				semi_count += (buffer[i] == ';');
			}
			printf("%d", semi_count);

			/* Parse. */
			args = wsh_parse(buffer, buffer_size, &argc);

			if(!strcmp(buffer, "quit")){
				printf("Quitting..\n");
				break;
			}
			if(!strcmp(buffer, "halt")){
				free(buffer);
				// TODO FREE ARGS:for(i
				exit(EXIT_SUCCESS);
			}

			/* Execute. */
			uint32_t send_size = htonl(semi_count);
			write(conn_fd, &send_size, sizeof(uint32_t));

			char **arg_cpy = (char**) malloc(buffer_size * sizeof(char*));
			int semi_num = 0, offset = 0, start_semi = 1;
			do{
				int cpy_index = 0;
				for(int i = offset; i < argc; i++){
					start_semi = 1;
					if(**(args + i) == ';'){
						if(strlen(*(args + i)) > 1){
							*(args + i) = *(args + i) + 1;
							start_semi = 0;
						}
						semi_num++;
						break;
					}
					if(*(*(args + i) + strlen(*(args + i))) == ';' ){
						semi_num++;
						break;
					}
					arg_cpy[cpy_index] = malloc(strlen(*(args + i)) + 1);
					strcpy(arg_cpy[cpy_index++], *(args + i));
				} 
				arg_cpy[cpy_index] = NULL;
				wsh_execute(arg_cpy);
				offset = cpy_index + start_semi;
			} while(semi_num-- > 0);

			memset(buffer, '\0', buffer_size);
			if(!(buffer = (char*) realloc(buffer, BUFF_SIZE)))
				err_msg("Error at reallocating space");
		}
		close(sock_fd);
		close(conn_fd);
		unlink(PATH);
	}
}


void wsh_read_line(char** buffer, uint32_t* buffer_size){
	int index = 0, c, nl_limit = 0, backslash = 0;
	while((c = getchar()) != EOF){
		if(c == '\n'){
			if(!nl_limit) break;
			printf("> ");
			continue;
		}

		if(backslash && c != '\n'){
			backslash = 0;
			nl_limit = 0;
		}

		if(!nl_limit && c == '\\') {
			nl_limit = 1; 
			backslash = 1;
			continue;
		}

		if(index >= *buffer_size - 1){
			*buffer_size += BUFF_SIZE;
			if(!(*buffer = (char*) realloc(*buffer, *buffer_size * sizeof(char)))) 
				err_msg("Error at reallocating space");
		}
		*(*buffer + index++) = c;
	}
	*(*buffer + index) = '\0';
}


int wsh_client_write(char* buffer, uint32_t buffer_size){
	if(*buffer == '\0' || *buffer == '#') return 1;
	if(!strcmp(buffer, "help")) {printf("%s\n", HELP_MSG); return 1;}
	uint32_t send_size = htonl(buffer_size);
	write(sock_fd, &send_size, sizeof(uint32_t));
	write(sock_fd, buffer, buffer_size);
	return 0;
}

void wsh_client_read(char* buffer, uint32_t buffer_size){
	uint32_t recv_size;
	if((read(sock_fd, &recv_size, sizeof(uint32_t))) == -1)
		handle_error("read");
	for(int i = 0; i < ntohl(recv_size) + 1; i++){
		wsh_read_socket(sock_fd, &buffer, &buffer_size);
		printf("%s", buffer);
	}
}

void wsh_client_loop(){
	wsh_client();
	uint32_t buffer_size = BUFF_SIZE;
	char *buffer;
   	if(!(buffer = (char*) malloc(buffer_size * sizeof(char))))
		err_msg("Error at allocating space");

	while(1){
		wsh_prompt();
		wsh_read_line(&buffer, &buffer_size);
		if(wsh_client_write(buffer, buffer_size)) continue;
		if(!strcmp(buffer, "quit") || !strcmp(buffer, "halt")) break;
		wsh_client_read(buffer, buffer_size);
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
