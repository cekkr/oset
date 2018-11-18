/* startan.c
 *
 * Copyright (C) 2000 Ivan Tubert and Eduardo Tubert
 * 
 * Contact: tubert@eros.pquim.unam.mx
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * All I ask is that proper credit is given for my work, which includes
 * - but is not limited to - adding the above copyright notice to the beginning
 * of your source code files, and to any copyright notice that you may distribute
 * with programs based on this work.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. 
 * 
 */

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
   
      
    printf("Content-type: text/html%c%c",10,10);
    cut(boundary, "name=\"content\"");

	return(0);
}


void error(char *str)
{
    printf("Content-type: text/html%c%c",10,10);
    printf("%s\n", str);
    exit(1);
}

void cut(char *boundary, char *mark)
{
        char buff[100], s[20];
        int state = 0, port;
        FILE *f, *f2;

		char *name = tmpnam(NULL);

        if (((f = fopen(name, "w")) != NULL) && ((f2 = fopen("port", "r")) != NULL)) {
                while (fgets(buff, sizeof(buff), stdin)) {
                        switch(state) {
                                case 0: /* find boundary */
                                        if (strstr(buff, boundary))
                                                state = 1;
                                        break;
                                case 1: /* find mark */
                                        if (strstr(buff, mark))
                                                state = 2;
                                        else
                                                state = 0;
                                        break;
                                case 2: /* skip enter */
                                        state = 3;
                                        break;                              
                                case 3: /* copy segment */
                                        if (strstr(buff, boundary)) 
                                                state = 1;
                                        else 
                                            fprintf(f, "%s", buff);
                                        break;
                        }
                }

                fclose(f);

				printf("<html>\n<head>\n<BASE HREF=\"http://litio.pquim.unam.mx/caos/\">\n<title>OSET Applet</title>\n</head>\n<body bgcolor=\"#005000\">\n");
				printf("<APPLET codebase=\"/caos/\" code=\"CAOS.class\" archive=\"CAOS.jar\"width=\"100%\" height=\"100%\">\n");
				printf("<PARAM name=fname value=\"%s\">\n", name);
				if(fgets(s, 10, f2) != NULL) {
					port = atoi(s);
					if(port > 0) {
						printf("<PARAM name=\"port\" value=\"%i\">\n", port);
					}
				} 
				fclose(f2);
				printf("</APPLET>\n</body>\n</html>\n");
        } else
                printf("File open error. Can't read or write on server.<br>");
}
