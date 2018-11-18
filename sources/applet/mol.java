/* mol.java
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

import java.io.*;
import java.awt.*;




class mol
{
	atom[] atoms;
	bond[] bonds;
	String smiles;
	String name;
	int complexity;

	double enlacepromedio;
	double scale;
	Rectangle bounds;

	mol(Reader r) throws Exception
	{
		StreamTokenizer st = new StreamTokenizer(r);
		int Natom, Nbond;

		st.nextToken();
		ParseUtil.match(st.sval,"STARTMOL");
		
		/* read atoms */
		st.nextToken();
		ParseUtil.match(st.sval,"ATOMS");

		st.nextToken();
		Natom = (int)st.nval;
		atoms = new atom[Natom];
		for (int i = 0; i < Natom; ++i) 
			atoms[i] = new atom(r);

		/* read bonds */
		st.nextToken();
		ParseUtil.match(st.sval,"BONDS");

		st.nextToken();
		Nbond = (int)st.nval;
		bonds = new bond[Nbond];
		for (int i = 0; i < Nbond; ++i) 
			bonds[i] = new bond(r);

		/* read smiles */
		st.nextToken();
		ParseUtil.match(st.sval,"SMILES");
		st.nextToken();
		smiles = st.sval;

		/* read complexity */
		st.nextToken();
		ParseUtil.match(st.sval,"CMPLX");
		st.nextToken();
		complexity = (int)st.nval;

		/* read name? */
		st.nextToken();
		if (ParseUtil.ismatch(st.sval, "NAME")) {
			st.nextToken();
			name = st.sval;
			st.nextToken();
		}

		ParseUtil.match(st.sval,"ENDMOL");

		System.out.println(this);
	}

	
	void trans(Writer w) throws IOException
	{
		w.write("STARTMOL\n");		
		w.write("ATOMS " + atoms.length + "\n");
		for (int i = 0; i < atoms.length; ++i) 
			w.write("\t"+atoms[i].x+" "+atoms[i].y+" "+atoms[i].sym+" "+atoms[i].charge+"\n");
		w.write("BONDS " + bonds.length + "\n");
		for (int i = 0; i < bonds.length; ++i) 
			w.write("\t"+bonds[i].a1+" "+bonds[i].a2+" "+bonds[i].type+"\n");
		w.write("ENDMOL\n");		

		w.flush();
	}

	public String toString()
	{
		String s = "";

		if (smiles != null)
			s += smiles + " ";
		if (name != null)
			s += name + " ";
/*
		int i;
		s = "A"+atoms.length+" ";
		for (i = 0; i < atoms.length; ++i)
			s += atoms[i].sym;

		s += " B"+bonds.length+" ";
		for (i = 0; i < bonds.length; ++i)
			s += "("+bonds[i].a1+(bonds[i].type==2 ? '=' : (bonds[i].type==3 ? '#' : '-'))+bonds[i].a2+")";
*/		
		return(s);		
	}

	void normalize(Rectangle bounds)
	{
		double minx=10000,maxx=-10000,miny=10000,maxy=-10000;
		double deltax,deltay,length;

		enlacepromedio = 0;

		for (int i = 0; i < atoms.length; ++i) {
			if (atoms[i].x < minx)
				minx = atoms[i].x;
			if (atoms[i].x > maxx)
				maxx = atoms[i].x;
			if (atoms[i].y < miny)
				miny = atoms[i].y;
			if (atoms[i].y > maxy)
				maxy = atoms[i].y;
		}

		for (int i = 0; i < bonds.length; ++i) {
			deltay = atoms[bonds[i].a2].y - atoms[bonds[i].a1].y;
			deltax = atoms[bonds[i].a2].x - atoms[bonds[i].a1].x;
			length = Math.sqrt(deltax*deltax + deltay*deltay);
			enlacepromedio += length;
		}

		if (bonds.length > 0)
			enlacepromedio /= bonds.length;
		else
			enlacepromedio = 1;

		double scalex, scaley;
		double margin = enlacepromedio / 4.0;
		int    ofsx, ofsy;

		if ((minx != maxx) || (miny != maxy)) {
			scalex = (bounds.width) / (maxx-minx+margin*2);
			scaley = (bounds.height) / (maxy-miny+margin*2);

			//System.out.println("minx="+minx+" miny="+miny+" maxx="+maxx+" maxy="+maxy+" width="+bounds.width+" height="+bounds.height);
			//System.out.println("scalex="+scalex+" scaley="+scaley);

			if (scalex < scaley)
				scale = scalex;
			else
				scale = scaley;

			ofsx = (int)((bounds.width - (maxx-minx+margin*2)*scale) / 2 + margin*scale) + bounds.x;
			ofsy = (int)((bounds.height - (maxy-miny+margin*2)*scale) / 2 + margin*scale) - bounds.y;
		} else {
			scale = 80;
			ofsx = (int)(bounds.width/2) + bounds.x;
			ofsy = (int)(bounds.height/2) - bounds.y;
		}


		for (int i = 0; i < atoms.length; ++i) {
			atoms[i].screenx = (int)(ofsx + (atoms[i].x-minx) * scale);
			atoms[i].screeny = (int)(bounds.height - ofsy - (atoms[i].y-miny) * scale);
		}

		this.bounds = bounds;
	}


	public void paint(Graphics g)
	{
		paint(g, true);
	}

	public void paint(Graphics g, boolean pintarnombre)
	{
		//System.out.print("paintmol "+this);
		if (bounds != null) {
			for (int i = 0; i < bonds.length; ++i) {
				int x1,x2,y1,y2;
				int dx=0,dy=0;

				x1 = atoms[bonds[i].a1].screenx;
				y1 = atoms[bonds[i].a1].screeny;
				x2 = atoms[bonds[i].a2].screenx;
				y2 = atoms[bonds[i].a2].screeny;

				if (bonds[i].type > 1) {
					double factor = 10.0;
					do {
						dx = (int)((y2 - y1) / factor); 
						dy = (int)((x2 - x1) / factor); 
						factor /= 2;
					} while ((dx/2 == 0) && (dy/2 == 0) && (factor > 0.01));
					//System.out.println("Dx = "+dx+" Dy = "+dy+" factor = "+factor);

				}

				switch(bonds[i].type) {
					case 3:
						// fall through
						g.drawLine(x1 + dx, y1 - dy, x2 + dx, y2 - dy);
						g.drawLine(x1 - dx, y1 + dy, x2 - dx, y2 + dy);

					case 1:
						g.drawLine(x1,y1,x2,y2);
						break;

					case 2:
						g.drawLine(x1 + dx/2, y1 - dy/2, x2 + dx/2, y2 - dy/2);
						g.drawLine(x1 - dx/2, y1 + dy/2, x2 - dx/2, y2 + dy/2);
						break;
				}
			}

			int mysize = (int)(enlacepromedio / 4.0 * scale);
			boolean usecolors = false;
			
			if(mysize < 10) 
				if (mysize > 4) {
					mysize = 10;
				} else {
					usecolors = true;
				}

			//System.out.println("Font size: "+ mysize);
			g.setFont(new Font("Sansserif", Font.BOLD, mysize));

			for (int i = 0; i < atoms.length; ++i) {
				//System.out.print("("+atoms[i].sym+","+atoms[i].screenx+","+atoms[i].screeny+")");
					
				if (!atoms[i].sym.equals("C")) {
					FontMetrics fm = g.getFontMetrics();
					int height, width;
					Color last = g.getColor();
					int x = atoms[i].screenx;
					int y = atoms[i].screeny;
					String s = atoms[i].sym;

					height = fm.getHeight();
					width = fm.stringWidth(s);
					//System.out.println("Font height: "+height);
					//System.out.println("Font width: "+ width);

					if(s.equals("N"))
						g.setColor(Color.blue);
					else if (s.equals("O"))
						g.setColor(Color.red);
					else if(s.equals("Cl"))
						g.setColor(Color.green);
					else if(s.equals("Br"))
						g.setColor(Color.orange);
					else if(s.equals("I"))
						g.setColor(Color.magenta);
					else if(s.equals("S"))
						g.setColor(Color.yellow);

					if(!usecolors) {
						Color use = g.getColor();

						x -= width / 2;
						y += height / 2;
						g.setColor(Color.white);
						g.fillOval((int)(x - width/2.0), (int)y - height + fm.getDescent(), width * 2, height);
						g.setColor(use);
						g.drawString(s, (int)x,(int)y);
					} else {
						g.fillOval((int)x-1, (int)y-1, 3, 3);
					}
					g.setColor(last);
				}
			}

			if (pintarnombre && (name != null)) {
				g.setFont(new Font("Sansserif", Font.BOLD, 10));
				g.drawString(name, bounds.x+5, bounds.y+bounds.height-5);
				//System.out.print(name);
			}
			//System.out.println("");
		}
	}

	public Polygon getbondpoly(int b)
	{
		int x1,x2,y1,y2;
		int dx=0,dy=0;

		x1 = atoms[bonds[b].a1].screenx;
		y1 = atoms[bonds[b].a1].screeny;
		x2 = atoms[bonds[b].a2].screenx;
		y2 = atoms[bonds[b].a2].screeny;

		dx = (y2 - y1) / 5;
		dy = (x2 - x1) / 5;

		Polygon ret = new Polygon();

		ret.addPoint(x1 + dx, y1 - dy);
		ret.addPoint(x2 + dx, y2 - dy);
		ret.addPoint(x2 - dx, y2 + dy);
		ret.addPoint(x1 - dx, y1 + dy);

		return(ret);
	}


	public int hittestatom(int x, int y)
	{
		for (int i = 0; i < atoms.length; ++i) {
			if ((Math.abs(atoms[i].screenx - x) < 6) && (Math.abs(atoms[i].screeny - y) < 6))
				return(i);
		}
		return(-1);
	}


	public int hittestbond(int x, int y)
	{
		Polygon p;
		for (int i = 0; i < bonds.length; ++i) {
			p = getbondpoly(i);
			if (p.contains(x,y))
				return(i);
		}
		return(-1);
	}
}


