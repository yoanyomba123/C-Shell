#ifndef _builtin_h_
#define _builtin_h_

#include "parse.h"

int is_builtin (char* cmd);
void builtin_execute (Task T, char* file);
int builtin_which (Task T);
#endif /* _builtin_h_ */
