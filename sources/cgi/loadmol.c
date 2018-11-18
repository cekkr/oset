#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error(char *str);
int cut(char *boundary, char *mark);

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
         
    printf("Content-type: text/html%c%c",10,10);
    if (!cut(boundary, "name=\"fname\"")) 
        printf("File not found!");
}


void error(char *str)
{
    printf("Content-type: text/html%c%c",10,10);
    printf("%s\n", str);
    exit(1);
}
#ifdef xyz
int cut(char *boundary, char *mark)
{
char buff[200];
while (fgets(buff, sizeof(buff), stdin)) {
	puts(buff);
}
}
#endif

int cut(char *boundary, char *mark)
{
        char buff[1000];
        int state = 0;
        int count = 0;
        int ok = 0;

        while (fgets(buff, sizeof(buff), stdin)) {
                switch(state) {
                        case 0: /* busca boundary */
                                if (strstr(buff, boundary))
                                        state = 1;
                                break;
                        case 1: /* busca mark */
                                if (strstr(buff, mark)) {
                                        count = 0;
                                        state = 2;
                                } else
                                        state = 0;
                                break;
                        case 2: /* salta enter */
                                if (strstr(buff,boundary))
                                        state = 1;
                                if (++count > 1)
                                        state = 3;
                                break;                              
                        case 3: /* copia segmento */
                                if (strstr(buff, boundary)) {
                                        if (ok)
                                                printf("</textarea></form></body></html>\n");
                                        state = 1;
                                } else {
                                        if (!ok) {
                                                printf("<html><head>\n");
                                                printf("<script language='Javascript'>\n");
                                                printf("function postmol() { opener.document.SDA.setMolContent(document.form.content.value); window.close(); }\n");
                                                printf("</script>\n");
                                                printf("</head><body onload=postmol();>\n");
                                                printf("<form name=form><textarea name=content>\n");
                                                ok = 1;
                                        }
                                        printf("%s", buff);
                                }
                                break;
                }
        }

        return(ok);
}


