/* mollist.java
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

class mollist {
	mol[] mols;
	int rxn, rating, simplification;
	String echo;

	mollist(mol m) 
	{
		mols = new mol[1];
		mols[0] = m;
	}

	mollist(Reader r) throws Exception
	{
		StreamTokenizer st = new StreamTokenizer(r);
		int Nmol;

		st.nextToken();
		ParseUtil.match(st.sval,"STARTTRANSFORM");
		
		/* read transform */
		st.nextToken();
		ParseUtil.match(st.sval,"HEADER");

		/* ignore header */
		st.nextToken();
		ParseUtil.match(st.sval,"RXN");
		st.nextToken();
		rxn = (int)st.nval;

		st.nextToken();
		ParseUtil.match(st.sval,"RATING");
		st.nextToken();
		rating = (int)st.nval;

		st.nextToken();
		ParseUtil.match(st.sval,"SIMPLIFICATION");
		st.nextToken();
		simplification = (int)st.nval;
		
		st.nextToken();
		if (ParseUtil.ismatch(st.sval, "ECHO")) {
			st.nextToken();
			echo = st.sval;
			System.out.println("ECHO = "+echo);
			st.nextToken();
		}

		ParseUtil.match(st.sval,"MOLS");

		/* read mols */
		st.nextToken();
		Nmol = (int)st.nval;
		mols = new mol[Nmol];
		for (int i = 0; i < Nmol; ++i) 
			mols[i] = new mol(r);

		/* read end */
		st.nextToken();
		ParseUtil.match(st.sval,"ENDTRANSFORM");
	}
/*
	public void paint(Graphics g, Rectangle bounds)
	{
		int nummols = mols.length;
		Rectangle r;

		if (mols != null) {
			for (int i = 0; i < nummols; ++i) {
				r = new Rectangle(bounds);
				r.width /= nummols;
				r.x = i*r.width;
				mols[i].paint(g, r);
			}
		}
	}
  */


	public boolean equals(Object obj) {
		boolean ret = true;
		mollist ml = (mollist)obj;

		if (this.mols.length == ml.mols.length) {
			for (int i = 0; (i < this.mols.length) && ret; ++i)
				ret = this.mols[i].smiles.equals(ml.mols[i].smiles);
		} else
			ret = false;

		return(ret);
	}

	public int hashCode() {
		if ((mols != null) && (mols[0] != null) && (mols[0].smiles != null))
			return(mols[0].smiles.hashCode());
		else
			return(0);
	}
}

