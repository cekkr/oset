/* syntreenode.java
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
//import java.applet.*;
//import java.awt.*;
//import java.awt.event.*;
//import java.net.*;
import java.io.*;
//import java.lang.Exception;


class syntreenode
{
	mol mol;
	Vector precs;
	static final double FITNESS_K = 0.1;
	static final double FITNESS_K2 = 5500.0;
	boolean last = false;
	int	depth = 0;
	int 	rating, simplification;

	syntreenode(mol tgt)
	{
		mol = tgt;
		precs = new Vector();
	}

	syntreenode(mol tgt, mollist precs)
	{
		this(tgt);
		for (int i = 0; i < precs.mols.length; ++i) 
			addprec(precs.mols[i]);
	}

	syntreenode(analysisnode node, int pos)
	{
		syntreenode root = null;
		mollist ml;
		boolean match;
		Vector pending;

		pending = new Vector();

		if ((node.mml.lists == null) || (node.mml.lists.length == 0)) 
			last = true;
		depth = node.depth;
		
		if (node.depth > 1) {
			if ((node.mml.lists != null) && (node.mml.lists.length > 0)) {
				root = new syntreenode(node.target.getelement(node.targetkey), node.mml.lists[pos]);
				root.rating = node.mml.lists[pos].rating;
				root.simplification = node.mml.lists[pos].simplification;
				pending.addElement(root);
			}

			while (node.depth > 2) {
				pos = node.targetkey.mollistnum;
				node = node.target;
				ml = node.mml.lists[pos];
				root = new syntreenode(node.target.getelement(node.targetkey));
				root.rating = ml.rating;
				root.simplification = ml.simplification;

				for (int i = 0; i < ml.mols.length; ++i) {
					match = false;
					for (int j = 0; (j < pending.size()) && !match; ++j) {
						if (ml.mols[i] == ((syntreenode)pending.elementAt(j)).mol) {
							root.addprec((syntreenode)pending.elementAt(j));
							pending.removeElementAt(j);
							match = true;
						}
					}
					if (!match)
						root.addprec(ml.mols[i]);
				}
				pending.addElement(root);
			}
		} else
			root = new syntreenode(node.mml.lists[0].mols[0]);

		mol = root.mol;
		precs = root.precs;
		rating = root.rating;
		simplification = root.simplification;
	}

	void addprec(mol m)
	{
		precs.addElement(new syntreenode(m));
	}

	void addprec(syntreenode m)
	{
		precs.addElement(m);
	}

	double fitness()
	{
		return((fitness1() + fitness2()) / 2.0);
	}

	double fitness1(int level)
	{
		double ret = 0;

		if (mol != null) {
			if (precs.size() == 0) {
				ret = mol.complexity * Math.exp(FITNESS_K * level);
			} else {
				for (int i = 0; i < precs.size(); ++i) 
					ret = Math.max(ret, ((syntreenode)precs.elementAt(i)).fitness1(level + 1));
			}
		}

		return(ret);
	}

	double fitness1()
	{
		double ret = 0;
		if (mol != null) {
			ret = fitness1(0) * 100.0 / mol.complexity;
			ret = Math.floor(ret * 100.0) / 100.0 + depth - 1;
			if (last)
				ret -= 1;
		}
			
		return(ret);
	}

	double fitness2(int level)
	{
		double ret = 1.0;
		double ret2;
		int nprec;

		if (mol != null) {
			if (precs.size() > 0) {
				nprec = 0;
				for (int i = 0; i < precs.size(); ++i)  {
					ret2 = ((syntreenode)precs.elementAt(i)).fitness2(level + 1);
					if (ret2 != 0.0) {
						ret *= ret2;
						++nprec;
					}
				}
				
				if (nprec > 1) {
					ret = Math.pow(ret, 1.0/nprec)*2.0/3.0;
					//System.out.println("root"+precs.size()+" ret = "+ret);
				}
			}
			if ((rating > 0) && (simplification > 0)) {
				//System.out.println("fitness2 "+mol+" "+rating+" "+simplification+" = "+FITNESS_K2 / ((double)rating * (double)simplification));
				ret *= FITNESS_K2 / ((double)rating * (double)simplification);
				//System.out.println("*ret = "+ret);
			} else
				ret = 0.0;
		}

		//System.out.println("ret = "+ret);
		return(ret);
	}


	double fitness2()
	{
		double ret = 0;
		if (mol != null)  {
			ret = fitness2(0);
			if (ret == 0) 
				ret = 1.0;
			ret = Math.floor(ret * 10000.0) / 100.0;
		}
			
		return(ret);
	}

/*	
	double fitness2(int level)
	{
		double ret = 0.0;
		double ret2;
		int nprec;

		if (mol != null) {
			if (precs.size() > 0) {
				for (int i = 0; i < precs.size(); ++i)  
					ret += ((syntreenode)precs.elementAt(i)).fitness2(level + 1);
			} else {
				//System.out.println("ret 1 = "+mol.complexity);				
				ret = mol.complexity; 
			}

			if (rating > 0)
				ret /= 1-Math.exp(-rating/25.0);
		}

		//stem.out.println("ret = "+ret);
		return(ret);
	}
	

	double fitness2()
	{
		return(fitness2(0) * 100.0 / mol.complexity);
	}
*/

	void print(int level)
	{
		for (int i = 0; i < level; ++i)
			System.out.print(" ");
		if (mol != null)
			System.out.println(mol + " " + rating + " " + simplification);
		else
			System.out.println("null");
		for (int i = 0; i < precs.size(); ++i) 
			((syntreenode)precs.elementAt(i)).print(level+1);
	}

	void print()
	{
		System.out.println("SYNTREE:");
		print(0);
		System.out.println("Depth = "+depth()+" Width = "+width());
	}

	int depth()
	{
		int max = 0;
		int x;

		for (int i = 0; i < precs.size(); ++i) {
			x = ((syntreenode)precs.elementAt(i)).depth();
			if (x > max)
				max = x;
		}

		return(max + 1);
	}

	int width()
	{
		int max = 0;
		int x = 0;

		for (int i = 0; i < precs.size(); ++i) 
			max += ((syntreenode)precs.elementAt(i)).width();

		if (max == 0)
			max = 1;

		return(max);
	}
	
	
	void trans(Writer w) throws IOException
	{
		if (mol != null) {
			w.write("NODE "+depth()+" "+width()+" "+mol.smiles+"\n");
			mol.trans(w);
			if (precs.size() != 0) {
				w.write("STARTPRECS\n");
				w.write("PRECS "+precs.size()+"\n");
				for (int i = 0; i < precs.size(); ++i) 
					((syntreenode)precs.elementAt(i)).trans(w);
				w.write("ENDPRECS\n");
			} else
				w.write("\n");
		}
	}
}

