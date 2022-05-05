#ifndef ARGS_H_
#define ARGS_H_

int cmp_list_list(char **src, char **dst);
void wsh_arg_check(char **argv);
void wsh_conn_check(int argc, char **argv, int short_arg);

#endif
