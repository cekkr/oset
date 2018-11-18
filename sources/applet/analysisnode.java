/* analysisnode.java
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

import java.util.Hashtable;
import java.util.Vector;




class analysisnode 
{
	molmetalist mml;		// current mml 
	analysisnode target;	// parent 
	analysiskey targetkey;	// pos in parent
	Hashtable precs;		// children 
	int depth;			
	Vector orphans;			// unanalyzed mols

	static Hashtable moldb = new Hashtable();
							// mol data base

	analysisnode(molmetalist curr) {
		mml = curr;

		target = null;
		targetkey = null;
		depth = 1;
		precs = new Hashtable();
		orphans = new Vector();
	}

	analysisnode(molmetalist curr, boolean add) {
		this(curr);

		mol mol = curr.lists[0].mols[0];
		moldb.put(mol.smiles, new molDBnode(mol));
	}

	void addprec(analysisnode prec, int mollistnum, int molnum, boolean isorphan, int mode, int aux) {
		mol mol;
		molDBnode mdb;

		analysiskey key = new analysiskey(mollistnum, molnum, isorphan);
		prec.targetkey = key;
		prec.target = this;
		prec.depth = depth+1;
		prec.orphans = (Vector)orphans.clone();
		precs.put(key, prec);

		if (!isorphan) {
			mol = mml.lists[mollistnum].mols[molnum];
		} else {
			System.out.println("Orphs: "+orphans);
			System.out.println("mollistnum: "+mollistnum);
			analysisorphan orph = (analysisorphan)orphans.elementAt(molnum);
			mol = orph.mol;
		}

		if ((mol!=null) && ((mdb = molDBget(mol)) != null)) {
			mdb.precs = prec.mml;
			//mdb.addform(mode, aux);
		}

		for (int i = 0; i < prec.mml.lists.length; ++i) {
			for (int j = 0; j < prec.mml.lists[i].mols.length; ++j) {
				mol = prec.mml.lists[i].mols[j];

				if (!moldb.containsKey(mol.smiles))
					moldb.put(mol.smiles, new molDBnode(mol));
			}
		}
		//System.out.println("MOLDB="+moldb);
	}

	static molDBnode molDBget(mol mol)
	{
		return((molDBnode)moldb.get(mol.smiles));
	}

	mol getelement(int mollistnum,int molnum,boolean isorphan)
	{
		if (!isorphan) {
			return(mml.lists[mollistnum].mols[molnum]);
		} else {
			return(((analysisorphan)orphans.elementAt(molnum)).mol);
		}
	}

	mol getelement(analysiskey key)
	{
		return(getelement(key.mollistnum, key.molnum, key.isorphan));
	}

	void addorphan(mol mol, int pos) {
		System.out.println("+Orphan D"+(depth-1)+"/"+pos+" "+mol);
		orphans.addElement(new analysisorphan(depth-1, pos, mol));
		System.out.println("Orphans = "+orphans);
	}

	void removeorphan(int pos) {
		orphans.removeElementAt(pos);
	}
	
	
	public static String info()
	{
		return("moldb = "+moldb.size());
	}
}



class analysisorphan
{
	int depth;
	int pos;
	mol mol;

	analysisorphan(int depth, int pos, mol mol)
	{
		this.depth = depth;
		this.pos = pos;
		this.mol = mol;
	}

	public String toString() {
		return("D"+depth+"/"+pos+" "+mol);
	}
}



