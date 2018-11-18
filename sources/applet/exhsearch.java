/* exhsearch.java
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


import java.util.*;

class exhsearch {
	int max_depth;
	CAOS CAOS;
	static final int MAXBOOKMARK = 5;
	bookmark marks[] = new bookmark[MAXBOOKMARK+1];

	exhsearch(CAOS C, int max_d) 
	{
		CAOS = C;
		max_depth = max_d;
	}

	public int run(analysisnode node)
	{
		int ret = 0;
		mollist ml;
		
		for (int i = 0; i < node.mml.lists.length; ++i) {
			ml = node.mml.lists[i];
			
			for (int j = 0; j < ml.mols.length; ++j) {
				CAOS.processmol(node, i, j, false, molPanel.MODE_DISC, 0);
				++ret;
			}

			System.out.println("MARK");
			for (Enumeration e = node.precs.elements(); e.hasMoreElements(); ) {
				int l;
				analysisnode n = (analysisnode)e.nextElement();

				for (int k = 0; k < n.mml.lists.length; ++k) {
					bookmark b = new bookmark(n,k);
					System.out.println("MARK"+ b.getFitness()+" "+b.node+" "+b.pos);
					
					for (l = MAXBOOKMARK-1; l >= 0; --l) {
						if ((marks[l] == null) || (b.getFitness() < marks[l].getFitness())) 
							marks[l+1] = marks[l];
						else 
							break;
					}
					marks[l+1] = b;
				}
/*				
				System.out.println("");
				for (l = 0; l < MAXBOOKMARK; ++l)
					if (marks[l] != null)
						System.out.println(marks[l].getFitness()+" "+marks[l].node+" "+marks[l].pos);
*/					
				if (node.depth < max_depth) 
					ret += run(n);
			}
		}
		
		return(ret);
	}
}





