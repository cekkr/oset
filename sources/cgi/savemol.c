#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error(char *str);
void cut(char *boundary, char *mark);

char *envtype = "multipart/form-data";
char *boundstr = "boundary=";

main(int argc, char *argv[]) {
    char *content_type;
    char *boundary;

    if(strcmp(getenv("REQUEST_METHOD"),"POST")) 
        error("This script should be referenced with a METHOD of POST.");

    content_type = getenv("CONTENT_TYPE");
    if(strncmp(getenv("CONTENT_TYPE"),envtype,strlen(envtype))) 
        error("This script can only be used with multipart/form-data.");

    if ((boundary = strstr(content_type, boundstr)) == NULL)
        error("No boundary in content_type.");

    boundary += strlen(boundstr);
   
        
    printf("Content-type: application/x-mol%c%c",10,10);
    cut(boundary, "name=\"content\"");
}


void error(char *str)
{
    printf("Content-type: text/html%c%c",10,10);
    printf("%s\n", str);
    exit(1);
}

void cut(char *boundary, char *mark)
{
        char buff[100];
        int state = 0;

        while (fgets(buff, sizeof(buff), stdin)) {
                switch(state) {
                        case 0: /* busca boundary */
                                if (strstr(buff, boundary))
                                        state = 1;
                                break;
                        case 1: /* busca mark */
                                if (strstr(buff, mark))
                                        state = 2;
                                else
                                        state = 0;
                                break;
                        case 2: /* salta enter */
                                state = 3;
                                break;                              
                        case 3: /* copia segmento */
                                if (strstr(buff, boundary))
                                        state = 1;
                                else
                                        printf("%s", buff);
                                break;
                }
        }
}
