#include "../headers/common.h"
#include "../headers/args.h"

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
