/* treePanel.java
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

import java.applet.*;
import java.awt.*;
import java.awt.event.*;
import java.text.NumberFormat;
import java.io.*;



class treePanel extends Panel
{
	analysisnode man;
	int pos;
	syntreenode root = null;
	CAOS CAOS;

	treePanel(CAOS ap)
	{
		CAOS = ap;
		setBackground(Color.white);

/*
		addMouseListener(new MouseAdapter() {
			public void mousePressed(MouseEvent ev) {
				doMousePressed(ev.getX(), ev.getY());
			}
		});
		addMouseMotionListener(new MouseMotionAdapter() {
			public void mouseMoved(MouseEvent ev) {
				doMouseMoved(ev.getX(), ev.getY());
			}
		});
*/
	}


	void reload(analysisnode node)
	{
		reload(node, 0);
	}

	void reload(analysisnode node, int newpos)
	{
		man = node;
		pos = newpos;
	}

	void setpos(int newpos)
	{
		pos = newpos;
		if (man != null) {
			NumberFormat form = NumberFormat.getInstance();
			form.setMaximumFractionDigits(2);
			root = new syntreenode(man, pos);

			double fitness = root.fitness();
			double fitness1 = root.fitness1();
			double fitness2 = root.fitness2();

			if (fitness != 0)
				CAOS.setinfo("Fitness: "+form.format(fitness)+" ("+form.format(fitness1)+" "+form.format(fitness2)+")");
			else
				CAOS.setinfo("");
		}
	}	


	void synpaint(Graphics g, syntreenode node, Rectangle r, int ry)
	{
		Rectangle prect;
		syntreenode temp;
		int y;

		y = r.y + r.height/2 - ry / 2;

		prect = new Rectangle(r);
		prect.y = y;
		prect.height = ry;
		node.mol.normalize(prect);
		node.mol.paint(g,false);
		
		//System.out.println(node.mol+" "+prect);

		prect = new Rectangle(r);
		prect.x -= 2*prect.width;

		if (node.precs.size() > 0) {
			int dx,cy;

			cy = r.y + r.height/2;
			dx = r.width / 10;
			

			g.drawLine(r.x - dx * 8, cy, r.x - dx, cy);
			g.drawLine(r.x - dx, cy, r.x - dx * 2, cy - dx);
			g.drawLine(r.x - dx, cy, r.x - dx * 2, cy + dx);


			for (int i = 0; i < node.precs.size(); ++i) {
				temp = (syntreenode)node.precs.elementAt(i);
				prect.height = ry * temp.width();

				g.drawLine(r.x - dx * 9, prect.y + prect.height / 2, r.x - dx * 8, cy);

				synpaint(g, temp, prect, ry);

				prect.y += prect.height;
			}
		}
	}


	public void paint(Graphics g)
	{
		if ((man != null) && (root != null)) {
			//root.print();
			
			int maxx = root.depth() * 2 - 1;
			int maxy = root.width();
			Rectangle r = getBounds();
			int rx = r.width / maxx;
			int ry = r.height / maxy;

			r.x = (maxx-1) * rx;
			r.y = 0;
			r.width = rx;
			
			System.out.println("r.x="+r.x+" r.y="+r.y+" r.width="+r.width+" r.height="+r.height);

			synpaint(g, root, r, ry);
		}
	}
	
	public void trans(Writer w) throws IOException
	{
		w.write("STARTTREE\n");		
		root.trans(w);	
		w.write("ENDTREE\n");
		w.flush();
	}
}
