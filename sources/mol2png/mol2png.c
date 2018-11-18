/* mol2png.c
 *
 * Copyright (C) 2000 Ivan Tubert and Eduardo Tubert
 * 
 * Contact: tubert@eros.pquim.unam.mx
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * All we ask is that proper credit is given for our work, which includes
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

#include <math.h>
#include "../mollib/mollib.h"
#include "gd.h"
#include "gdfontg.h"
#include "gdfonts.h"
#include "gdfontl.h"
#include "gdfontt.h"
#include "gdfontmb.h"


void mol2png(struct mol *mol, char *fname, int x, int y, int margin);

extern char *symbol[];

int main(int argc, char *argv[])
{
	struct mol *mol;
	
	printf("Giant: %i * %i\n", gdFontGiant->w, gdFontGiant->h);
	printf("Large: %i * %i\n", gdFontLarge->w, gdFontLarge->h);
	printf("Medium: %i * %i\n", gdFontMediumBold->w, gdFontMediumBold->h);
	printf("Small: %i * %i\n", gdFontSmall->w, gdFontSmall->h);
	printf("Tiny: %i * %i\n", gdFontTiny->w, gdFontTiny->h);
	if(argc > 1) {
		mol = readmolfile(argv[1]);
		if(mol == NULL){
			printf("Error reading molfile: %s\n\t", argv[1]);
			printf("%s\n", geterr());	
			exit(1);
		} else {
			printf("Creating PNGs...\n");
			mol2png(mol, "mol.png",  50, 50, 10);
			mol2png(mol, "mol2.png", 75, 75, 10);
			mol2png(mol, "mol3.png", 100, 100, 10);
			mol2png(mol, "mol4.png", 125, 125, 10);
			mol2png(mol, "mol5.png", 150, 150, 10);
			mol2png(mol, "mol6.png", 200, 200, 10);
			mol2png(mol, "mol7.png", 300, 300, 10);
			mol2png(mol, "mol8.png", 40, 40, 10);
			printf("Done.\n");
		}
	}
	

	return(0);
}




void mol2png(struct mol *mol, char *fname, int x, int y, int margin)
{
	FILE *out;
//	FILE *jpegout;
	gdImagePtr im_out;
	int white, blue, red, green, black, brown, gray, color;
	gdFontPtr font;
	struct drect r;
	double scale;
	int i, avglength, bondspace;

	im_out = gdImageCreate(x, y);

	/* First color allocated is background. */
	white = gdImageColorAllocate(im_out, 255, 255, 255);
	gdImageColorTransparent(im_out, white); 	/* Set as transparent color. */

	red = gdImageColorAllocate(im_out, 255, 0, 0);
	green = gdImageColorAllocate(im_out, 0, 255, 0);
	blue = gdImageColorAllocate(im_out, 0, 0, 255);
	black = gdImageColorAllocate(im_out, 0, 0, 0);
	brown = gdImageColorAllocate(im_out, 99, 99, 0);
	gray = gdImageColorAllocate(im_out, 99, 99, 99);

	r = getmolrect(mol);
	scale = min((x - 2 * margin) / (r.right - r.left), (y - 2 * margin) / (r.top - r.bottom));
	avglength = (int)(scale * getmeanbondlength(mol));
	bondspace = avglength * 2 / 10;
        if((bondspace < 3) && (avglength > 3))
		bondspace = 3;

	if(avglength <= 8)
		font = 0;
	if(avglength > 8)
		font = gdFontTiny;
	if(avglength > 14)
		font = gdFontSmall;
	if(avglength > 20)
		font = gdFontLarge;
	if(avglength > 26)
		font = gdFontGiant;







	for(i = 0; i < mol->Nbond; ++i) {
		int x1, x2, y1, y2, dx, dy;
		double deltax, deltay, length;
		
		x1 = (int)((mol->atoms[mol->bonds[i].a1].x - r.left) * scale) + margin;
		y1 = (int)((mol->atoms[mol->bonds[i].a1].y - r.top) * -scale) + margin;
		x2 = (int)((mol->atoms[mol->bonds[i].a2].x - r.left) * scale) + margin;
		y2 = (int)((mol->atoms[mol->bonds[i].a2].y - r.top) * -scale) + margin;

		if(mol->bonds[i].order > 1)
		{
			deltay = y2 - y1;
			deltax = x2 - x1;
			length = sqrt(deltax*deltax + deltay*deltay);
			dx = (int)(deltay * bondspace / length );
			dy = (int)(deltax * bondspace / length );
		}
		if(mol->bonds[i].order == 2)
		{
			gdImageLine(im_out, x1 + dx/2, y1 - dy/2, x2 + dx/2, y2 - dy/2, black);
			gdImageLine(im_out, x1 - dx/2, y1 + dy/2, x2 - dx/2, y2 + dy/2, black);
		} else 	{
			gdImageLine(im_out, x1, y1, x2, y2, black);
			if(mol->bonds[i].order == 3)
			{
				gdImageLine(im_out, x1 + dx, y1 - dy, x2 + dx, y2 - dy, black);
				gdImageLine(im_out, x1 - dx, y1 + dy, x2 - dx, y2 + dy, black);
			}
		}

	}


	for(i = 0; i < mol->Natom; ++i) {
		struct atom *atom = &mol->atoms[i];
		if(atom->Z != C) {
			switch(atom->Z){
				case O:
					color = red;
					break;

				case Cl:
					color = green;
					break;

				case N:
					color = blue;
					break;

				case Br:
					color = brown;
					break;

				default:
					color = gray;
					break;
			}
			if(font) {
				gdImageFilledRectangle(im_out, (int)((atom->x - r.left) * scale) + margin - font->w / 2,
					(int)((atom->y - r.top) * -scale) + margin - font->h / 2,
					(int)((atom->x - r.left) * scale) + margin + (font->w / 2) * (strlen(symbol[atom->Z]) * 2 -1), 
					(int)((atom->y - r.top) * -scale) + margin + font->h / 2, white);
				gdImageString(im_out, font, (int)((atom->x - r.left) * scale) + margin - font->w / 2, (int)((atom->y - r.top) * -scale) + margin - font->h / 2, 
					(unsigned char *) symbol[atom->Z], color);
			} else {
				gdImageFilledRectangle(im_out, (int)((atom->x - r.left) * scale) + margin - (avglength > 3),
					(int)((atom->y - r.top) * -scale) + margin - (avglength > 3),
					(int)((atom->x - r.left) * scale) + margin + (avglength > 3), 
					(int)((atom->y - r.top) * -scale) + margin + (avglength > 3), color);
			}
		}

	}



	/* Write PNG */
	out = fopen(fname, "wb");
//	jpegout = fopen("mol.jpg", "wb");
	gdImagePng(im_out, out);
//	gdImageJpeg(im_out, jpegout, -1);
	fclose(out);
//	fclose(jpegout);

	gdImageDestroy(im_out);

}
