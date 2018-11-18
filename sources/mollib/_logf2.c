#include <stdio.h>
#include <stdarg.h>
#include <time.h>

void logfile(char *s, ...)
{
	FILE *f;
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	va_list args;

	va_start(args, s);

	vprintf(s, args);

	if ((f = fopen("oset.log", "at")) != NULL) {
		fprintf(f, "%02i/%02i/%02i %02i:%02i ", tm->tm_mon+1, tm->tm_mday, tm->tm_year % 100, tm->tm_hour, tm->tm_min);
		vfprintf(f, s, args);
		fclose(f);
	}

	va_end(args);
}



