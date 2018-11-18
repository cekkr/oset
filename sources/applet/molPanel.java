/* molPanel.java
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
import java.net.*;
import java.io.*;
import java.lang.Exception;
import java.util.*;


class molPanel extends Panel
{
	public final static int MODE_DISC = 0;
	public final static int MODE_AUX = 1;
	public final static int MODE_FG0 = 2;

	analysisnode man;
	int pos;
	Scrollbar scroll;
	CAOS CAOS;
	Vector rects;
	molPanelShape sel;
	int mode;

	molPanel(CAOS applet, boolean sc)
	{
		super();

		CAOS = applet;
		man = null;

		if (sc) {		
			setLayout(new BorderLayout());
			add("South", scroll = new Scrollbar(Scrollbar.HORIZONTAL));
			scroll.addAdjustmentListener(new AdjustmentListener() {
				public void adjustmentValueChanged(AdjustmentEvent e) {
					pos = e.getValue();
					scrollmove();
				}
			});
			addMouseListener(new MouseAdapter() {
				public void mousePressed(MouseEvent ev) {
					doMousePressed(ev.getX(), ev.getY());
				}
				public void mouseExited(MouseEvent ev) {
					doMouseExited();
				}
			});
			addMouseMotionListener(new MouseMotionAdapter() {
				public void mouseMoved(MouseEvent ev) {
					doMouseMoved(ev.getX(), ev.getY());
				}
			});
		}
		setBackground(Color.white);
		//reload(null,0);
	}

	public void doMousePressed(int x, int y)
	{
		//System.out.println("clicked "+ev);
		molPanelShape mpr = hittest(x,y);
		if (mpr != null) {
			analysisnode prec;
			setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
			prec = CAOS.processmol(man, pos, mpr.submol, mpr.isorphan, mode, mpr.aux);
			CAOS.reload(prec);
			setCursor(Cursor.getDefaultCursor());
		}
	}
	
	public void exhaustivesearch(int max_depth)
	{
		if (man != null) {
			exhsearch search = new exhsearch(CAOS, max_depth);
			analysisnode node;
			int ret;
			
			setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
			for (node=man; node.target != null; node = node.target)
				;
	
			search.run(node);

			for (int i = 0; i < search.MAXBOOKMARK; ++i)
				if ((search.marks[i] != null) && (search.marks[i].getFitness() < 99999))
					CAOS.addbookmark(search.marks[i], "Exhsearch "+(i+1));
			CAOS.reload(search.marks[0].node, search.marks[0].pos);
			
			setCursor(Cursor.getDefaultCursor());
		}
	}
	
	
	public void doMouseExited()
	{
		if (sel != null) {
			sel.paint(this,false);
			sel = null;
		}
	}

	public void doMouseMoved(int x, int y)
	{
		molPanelShape mpr = hittest(x, y);
		if (mpr != null) {
			if (mpr != sel) {
				if (sel != null) 
					sel.paint(this, false);
				sel = mpr;
				sel.paint(this, true);
			}
		} else if (sel != null) {
			sel.paint(this,false);
			sel = mpr;
		}
	}


	public void setmode(int m)
	{
		mode = m;
	}

	public void reload(analysisnode m)
	{
		reload(m, 0);
	}

	public void reload(molPanel p)
	{
		reload(p.man, p.pos);
	}

	public void reload(analysisnode m, int pos)
	{
		man = m;
		this.pos = pos;

		if (scroll != null) {
			scroll.setValues(pos,1,0,(man!=null ? man.mml.lists.length : 0));
			scroll.setBlockIncrement(1);
		}

		scrollmove();
	}

	public void recalc()
	{
		Rectangle bounds = getBounds();
		Rectangle r,rr;

		rects = new Vector();
		sel = null;
		
		if (scroll != null)
			bounds.height -= scroll.getBounds().height;
		bounds.x = 0;
		bounds.y = 0;

		if ((man != null) && (man.mml.lists.length > pos)) {
			int mols = man.mml.lists.length>0 ? man.mml.lists[pos].mols.length : 0;
			int orphs = man.orphans.size();

			//System.out.println("orphs="+orphs+" mols="+mols);

			if (orphs > 0) {
				r = new Rectangle(bounds);
				r.width /= (mols+1);
				r.x += r.width * mols;
				bounds.width -= r.width;

				//System.out.println("Orphs rect ="+r);

				for (int i = 0; i < orphs; ++i) {
					rr = new Rectangle(r);
					rr.width /= ((orphs+1)/2);
					rr.height /= 2;

					rr.x += (i/2) * rr.width;
					if ((i & 1) != 0)
						rr.y += rr.height;

					//System.out.println("Add rect orphs ="+rr);
					mol mol = man.getelement(pos, i, true);
					rects.addElement(new molPanelRect(rr, true, i, mol));
				}
			}
			if (mols > 0) {
				//System.out.println("Mols rect ="+bounds);
				for (int i = 0; i < mols; ++i) {
					r = new Rectangle(bounds);
					r.width /= mols;
					r.x = i*r.width;

					//System.out.println("Add rect mols ="+r);
					mol mol = man.getelement(pos, i, false);
					mol.normalize(r);
					rects.addElement(new molPanelRect(r, false, i, mol));
				}
			}
		}
	}

	molPanelShape hittest(int x, int y)
	{
		//System.out.println("Hittest x="+x+" y="+y);
		switch (mode) {
			case MODE_DISC:
				if ((rects != null) && (man != null)){
					for (Enumeration e = rects.elements(); e.hasMoreElements(); ) {
						molPanelRect mpr = (molPanelRect)e.nextElement();
						//System.out.println(" Rect "+mpr.r);

						if (mpr.contains(x,y)) 
							return(mpr);
					}
				}
				break;


			case MODE_AUX:
				if ((rects != null) && (man != null)) {
					int p;

					for (Enumeration e = rects.elements(); e.hasMoreElements(); ) {
						molPanelRect mpr = (molPanelRect)e.nextElement();
						//System.out.println(" Rect "+mpr.r);

						if ((p = mpr.mol.hittestatom(x,y)) >= 0) {
							atom atom = mpr.mol.atoms[p];
							Rectangle r = new Rectangle(atom.screenx-6, atom.screeny-6, 12, 12);
							molPanelRect n = new molPanelRect(r, mpr);
							n.setaux(p);

							return(n);
						}
					}
				}
				break;


			case MODE_FG0:
				if ((rects != null) && (man != null)) {
					int p;

					for (Enumeration e = rects.elements(); e.hasMoreElements(); ) {
						molPanelRect mpr = (molPanelRect)e.nextElement();
						//System.out.println(" Rect "+mpr.r);

						if ((p = mpr.mol.hittestbond(x,y)) >= 0) {
							Polygon poly = mpr.mol.getbondpoly(p);
							molPanelPoly n = new molPanelPoly(poly, mpr);
							n.setaux(p);

							return(n);
						}
					}
				}
				break;
		}
		return(null);
	}

	public void scrollmove()
	{
		CAOS.recalc();
	}
	
	public void paint(Graphics g)
	{
		Rectangle r = getBounds();
		FontMetrics fm = g.getFontMetrics();

		if (scroll != null)
			r.height -= scroll.getBounds().height;
		r.x = r.y = 0;

		g.setColor(Color.gray);
		g.draw3DRect(0, 0, r.width-1, r.height-1, false);

		g.setColor(Color.black);

		if (scroll != null) {
			g.drawString("("+(pos+1)+"/"+((man != null) ? man.mml.lists.length : 0)+")", 2, fm.getHeight());
			if (man != null) {
				String s = "D: "+man.depth;
				g.drawString(s, r.width-2-fm.stringWidth(s), fm.getHeight());
			}
		}

		recalc();
		if ((rects != null) && (man != null))
			//System.out.print("paint ");
			for (Enumeration e = rects.elements(); e.hasMoreElements(); ) {
				molPanelRect mpr = (molPanelRect)e.nextElement();
				molDBnode node;
				mol mol;

				mol = man.getelement(pos, mpr.submol, mpr.isorphan);
				node = (molDBnode)analysisnode.moldb.get(mol.smiles);

				if ((scroll != null) && (node != null) && (node.precs != null))
					g.setColor(mpr.isorphan ? Color.red : Color.green.darker());
				else
					g.setColor(mpr.isorphan ? new Color(0xc0,0xc0,0xc0) : Color.black);

				mol.normalize((Rectangle)mpr.r);
				mol.paint(g);
				//System.out.print(mol);
			}

		//System.out.println("");

		if (sel != null) 
			sel.paint(this, true);
	}

	String findcas()
	{
		String ret = "";
		recalc();
		if ((rects != null) && (man != null))
			for (Enumeration e = rects.elements(); e.hasMoreElements(); ) {
				molPanelRect mpr = (molPanelRect)e.nextElement();
				molDBnode node;
				mol mol;

				mol = man.getelement(pos, mpr.submol, mpr.isorphan);
				if (mol != null) 
					ret = ret + CAOS.processcas(mol.smiles);
			}

		return(ret);
	}



	public void info()
	{
		if ((man != null) && (man.mml.lists.length > 0)) 
			CAOS.setinfo(man.mml.lists[pos].rxn, this.toString());
		else
			CAOS.setinfo(0, "NO REACTION AVAILABLE");
	}


	public String toString() 
	{
		if ((man != null) && (man.mml.lists.length > 0)) {
			mollist ml = man.mml.lists[pos];
			if ((ml.rating == 0) && (ml.simplification == 0))
				return("Target mol");
			else
				return("Rating = "+ml.rating+"\nSimplification="+ml.simplification+(((ml.echo != null) && (ml.echo.length() > 0)) ? "\n"+ml.echo : ""));
		} else
			return("NO REACTION AVAILABLE");
	}
}







/* para busqueda en inet

www.chemfinder.com
<form name="SimpleInputForm" action="result.asp" method="post">
	<table width="450" border="0" bgcolor = "FFFFD6">
		<tr>
			<td colspan="2">
				<font size="-1" color="#ff3333">Enter a chemical name, CAS Number, molecular formula, or molecular weight
				<br>Use * for partial names (e.g. ben*)
				</font>
			</td>
		</tr>

		<tr>
			<td width="340"><input type="text" name="polyQuery" size="45"></td>
			<td width="120"><input type="submit" value="Search"></td>
		</tr>
*/




class molPanelShape {
	Shape r;

	boolean isorphan;
	int submol;
	mol mol;

	int aux;

	molPanelShape(Shape r, boolean isorphan, int submol, mol mol) {
		this.r = r; this.isorphan = isorphan; this.submol = submol; this.mol = mol;
	}

	molPanelShape(Shape r, molPanelShape m) {
		this.r = r; this.isorphan = m.isorphan; this.submol = m.submol; this.mol = m.mol;
	}

	public void setaux(int p)
	{
		aux = p;
	}

	void drawShape(Graphics g)
	{
	}
	
	boolean contains(int x, int y)
	{
		return(false);
	}

	void paint(Panel panel, boolean draw) {
		Graphics g = panel.getGraphics();
		Color last = g.getColor();

		g.setColor(Color.white);
		g.setXORMode(Color.blue);
		drawShape(g);
		g.setPaintMode();
		g.setColor(last);

		panel.setCursor(draw ? Cursor.getPredefinedCursor(Cursor.HAND_CURSOR) : Cursor.getDefaultCursor());
	}
}


class molPanelRect extends molPanelShape {
	molPanelRect(Rectangle r, boolean isorphan, int submol, mol mol) {
		super(r, isorphan, submol, mol);
	}

	molPanelRect(Rectangle r, molPanelShape m) {
		super(r, m);
	}
	
	boolean contains(int x, int y)
	{
		return(((Rectangle)r).contains(x,y));
	}

	void drawShape(Graphics g)
	{
		Rectangle rr = (Rectangle)r;
		g.drawRect(rr.x+1, rr.y+1, rr.width-3, rr.height-3);
	}
}


class molPanelPoly extends molPanelShape {
	molPanelPoly(Polygon r, boolean isorphan, int submol, mol mol) {
		super(r, isorphan, submol, mol);
	}

	molPanelPoly(Polygon r, molPanelShape m) {
		super(r, m);
	}

	boolean contains(int x, int y)
	{
		return(((Polygon)r).contains(x,y));
	}

	void drawShape(Graphics g)
	{
		Polygon p = (Polygon)r;
		g.drawPolygon(p);
	}
}

