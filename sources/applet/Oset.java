/* CAOSApplet.java
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


public class Oset extends Applet {
	static final int DEFAULTPORT = 8000;
	int PORT = DEFAULTPORT;		/* default port */

	public void init() 
	{
		try {
			PORT = Integer.parseInt(getParameter("port"));
		} catch (Exception e) { }

		String fname = getParameter("fname");
		if (fname != null)
			loadmolfile(fname);		
	}

	public CAOS newCAOS(boolean isApplet)
	{
		CAOS c = new CAOS("CAOS", getDocumentBase().getHost(), PORT, isApplet);
		c.setSize(640,480);
		c.setVisible(true);
		return(c);
	}

	public void loadmol(String s)
	{
		newCAOS(true).loadmol(s);
	}
	
	public void loadmolfile(String fname)
	{
		newCAOS(true).loadmolfile(fname);
	}	
	
	
	public static void main(String args[])
	{
		if (args.length < 3) {
			System.out.println("Oset [host] [port] [local mol file name]");
		} else {
			File f = new File(args[2]);
			if (f.exists()) {
				String s = new String();
				int port = DEFAULTPORT;
				
				try {
					BufferedReader r = new BufferedReader(new FileReader(f));
					while (r.ready()) 
						s = s+r.readLine()+"\n";
					r.close();
				} catch (Exception e) {
					System.out.println("Can't read file "+args[2]+"!");
					e.printStackTrace();
					System.exit(0);
				}

				try {
					port = Integer.parseInt(args[1]);
				} catch (Exception e) { }

				System.out.println("s="+s);

				CAOS c = new CAOS("CAOS", args[0], port, false);
				c.setSize(640,480);
				c.setVisible(true);
				c.loadmol(s);
			} else
				System.out.println("File "+args[2]+" not found!");
		}
	}
}



