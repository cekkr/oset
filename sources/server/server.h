#include "../mollib/mollib.h"

#undef BOOL

#include "sock.h"


int strnncmp(char *buff, char *patt);
void mol2trans(struct struct_info *info, struct sock_data *s,struct mol *mol);
struct mol *trans2mol(struct sock_data *s);
void meta2trans(struct struct_info *info, struct sock_data *s, struct mol_metalist *mol_metalist);

void writesk(struct sock_data *s);
