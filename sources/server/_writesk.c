#include "server.h"


#define MARGIN	150
#define	MOLMARGIN 10

#define MOLSPERPAGEX 5
#define MOLSPERPAGEY 5

#define	PAGESIZEX	3301
#define PAGESIZEY	2551

#define PAGEMARGIN	150

static int myarrow;

struct rect {
	int x,y,width,height;
};

void writetree(struct sk2info *sk2, struct sock_data *s, struct rect *r, int ry);




void writesk(struct sock_data *s)
{
	char buff[200];
	int depth, width;
	char *aux;
	struct sk2info *sk2;
	struct rect rect;
	int maxx;
	int maxy;
	int rx;
	int ry;
	int pagesx, pagesy;

	memset(&rect, 0, sizeof(struct rect));


	sockgets(s, buff, sizeof(buff));
	logsockfile(s, "> %s", buff);
	if (strnncmp(buff, "NODE")) {
		seterr("Expected NODE");
		return;
	}

	aux = strtok(buff, " ");
	aux = strtok(NULL, " ");
	depth = atoi(aux);
	aux = strtok(NULL, " ");
	width = atoi(aux);

	pagesx = ((depth-1) / MOLSPERPAGEX) + 1;
	pagesy = ((width-1) / MOLSPERPAGEY) + 1;

	rect.width = (PAGESIZEX - 2*PAGEMARGIN) * pagesx ;
	rect.height = (PAGESIZEY - 2*PAGEMARGIN) * pagesy ;

	sk2 = create_sk2("output.sk2");
	setpageinfo_sk2(sk2, PAGESIZEX * pagesx - 2*PAGEMARGIN*(pagesx-1), PAGESIZEY * pagesy -2*PAGEMARGIN*(pagesy-1), PAGESIZEX, PAGESIZEY, pagesx, pagesy, PAGEMARGIN, PAGEMARGIN, PAGEMARGIN, PAGEMARGIN, LANDSCAPE);

	myarrow = addstyle(sk2, 
		CreateArrowStyle(AS_UPDOWN | AS_FILLED, AS_NULL, 
			AS_NULL , AS_NULL, 0, 8, 3),
		ARROWSTYLE);

	maxx = depth * 2 - 1;
	maxy = width;
	rx = (int)(rect.width / maxx);
	ry = (int)(rect.height / maxy);

	rect.x = (maxx-1) * rx;
	rect.width = rx;

	writetree(sk2, s, &rect, ry);

	sockgets(s, buff, sizeof(buff));
	logsockfile(s, "> %s", buff);
	if (strnncmp(buff, "ENDTREE")) {
		seterr("Expected ENDTREE");
		return;
	}

	write_sk2(sk2);
}




void writetree(struct sk2info *sk2, struct sock_data *s, struct rect *r, int ry)
{
	struct rect prect;
	struct mol *mol;
	int y;
	char buff[200];
	char *aux, width, depth;
	int i,numprecs;
	int dx,cy;
 	GroupFormat *g;

	y = r->y + r->height/2 - ry/2;

	if ((mol = trans2mol(s)) != NULL) {
		putmol_sk2(sk2, mol, r->x+MARGIN, y+MARGIN, r->x+r->width+MARGIN, y+ry+MARGIN, MOLMARGIN);
		destroy_mol(mol);
	}

	memmove(&prect, r, sizeof(struct rect));
	prect.x -= 2 * prect.width;

	sockgets(s, buff, sizeof(buff));
	logsockfile(s, "> %s", buff);
	if (!strnncmp(buff, "STARTPRECS")) {
		sockgets(s, buff, sizeof(buff));
		logsockfile(s, "> %s", buff);
		if (strnncmp(buff, "PRECS")) 
			return;
		numprecs = atoi(buff+6);

		cy = r->y + r->height/2;
		dx = r->width / 10;

		addobj_sk2(sk2->obj_list, g = CreateGroup(), GROUP);
		addobj_sk2(g->ObjectsList, CreateLineEx(sk2, -1, myarrow, 2, (double)(r->x - dx * 8 + MARGIN), (double)(cy + MARGIN), (double)(r->x - dx  + MARGIN), (double)(cy + MARGIN)), LINE);

		for (i = 0; i < numprecs; ++i)	{
			sockgets(s, buff, sizeof(buff));
			logsockfile(s, "> %s", buff);
			if (strnncmp(buff, "NODE")) 
				return;

			aux = strtok(buff, " ");
			aux = strtok(NULL, " ");
			depth = atoi(aux);
			aux = strtok(NULL, " ");
			width = atoi(aux);
		
			prect.height = ry*width;
			addobj_sk2(g->ObjectsList, CreateLineEx(sk2, -1, -1, 2, (double)(r->x - dx * 9 + MARGIN), (double)(prect.y + prect.height / 2 + MARGIN), (double)(r->x - dx * 8  + MARGIN), (double)(cy + MARGIN)), LINE);

			writetree(sk2, s, &prect, ry);
			prect.y += prect.height;	
		}

		sockgets(s, buff, sizeof(buff));
		logsockfile(s, "> %s", buff);
		if (strnncmp(buff, "ENDPRECS")) 
			return;
	}
}



