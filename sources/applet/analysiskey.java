/* analysiskey.java
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

import java.util.Vector;



class analysiskey
{
	int mollistnum;
	int molnum;
	boolean isorphan;
	Vector forms;

	
	analysiskey(int mollistnum, int molnum, boolean isorphan)
	{
		this.mollistnum = mollistnum;
		this.molnum = molnum;
		this.isorphan = isorphan;

		forms = new Vector();
	}

	analysiskey(int mollistnum, int molnum) {
		this(mollistnum, molnum, false);
	}

	public boolean equals(Object obj) {
		boolean ret = false;

		analysiskey key = (analysiskey)obj;
		ret = ((mollistnum == key.mollistnum) && (molnum == key.molnum) && (isorphan == key.isorphan));

		return(ret);
	}

	public int hashCode() {
		return(mollistnum * 10 + molnum);
	}

	public String toString() {
		return("K("+mollistnum+","+molnum+")");
	}
}
