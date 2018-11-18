#include <setjmp.h>

jmp_buf mark[10];


int _dotry(void)
{
	return(setjmp(mark[0]));
}

void _dothrow(int ret)
{
	longjmp(mark[0],ret);
}


