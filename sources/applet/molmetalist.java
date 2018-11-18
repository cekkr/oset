/* molmetalist.java
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

class molmetalist {
	mollist[] lists;

	molmetalist(mol m) 
	{
		lists = new mollist[1];
		lists[0] = new mollist(m);
	}

	molmetalist(Reader r) throws Exception
	{
		StreamTokenizer st = new StreamTokenizer(r);
		int Nlist;

		st.nextToken();
		ParseUtil.match(st.sval,"STARTTRANSFORMLIST");
		
		/* read transforms */
		st.nextToken();
		ParseUtil.match(st.sval,"TRANSFORMS");

		st.nextToken();
		Nlist = (int)st.nval;
		lists = new mollist[Nlist];
		for (int i = 0; i < Nlist; ++i) 
			lists[i] = new mollist(r);

		/* read end */
		st.nextToken();
		ParseUtil.match(st.sval,"ENDTRANSFORMLIST");
	}

	void cat(molmetalist newmml)
	{
		boolean match;
		mollist[] oldml = this.lists;
		mollist[] newml = newmml.lists;
		Vector aux = new Vector();
		int pos;

		for (int i = 0; i < oldml.length; ++i)
			aux.addElement(oldml[i]);

		for (int i = 0; i < newml.length; ++i)
			if (!aux.contains(newml[i]))
				aux.addElement(newml[i]);

		//System.out.println("AUX="+aux);
		
		this.lists = new mollist[aux.size()];
		aux.copyInto(this.lists);
		
		//this.lists = (mollist[])aux.toArray(new mollist[aux.size()]);
	}
}
