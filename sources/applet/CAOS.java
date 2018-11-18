/* CAOS.java
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



class CAOS extends Frame {
	public int PORT;
	String host;
	boolean isApplet = false;
	
		
//	TextArea text;
	String fname;
	molPanel pd = null;
	treePanel pu = null;
	TextArea text;
	String s1 = "";
	String s2 = "";
	int rxnnum = 0;
	Button upbutt;
	Menu bookmarkmenu;

	public CAOS(String title, String a_host, int a_port, boolean a_applet) 
	{
		super(title);
		
		addWindowListener(new WindowAdapter() {
			public void windowClosing(WindowEvent e) {
				if (isApplet) 
					dispose();
				else
					System.exit(0);
			}
		});
		

		Panel p,q;

		host = a_host;
		PORT = a_port;
		isApplet = a_applet;
		
		GridBagLayout gridbag = new GridBagLayout();
		GridBagConstraints c = new GridBagConstraints();

		setLayout(gridbag);

		c.gridwidth = 1;
		c.gridheight = 1;
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 1.0;
		c.weighty = 0.5;
		c.gridx = 0;
		c.gridy = 0;
		addcomp(pu = new treePanel(this), gridbag, c);

		c.gridwidth = 1;
		c.gridheight = 1;
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 1.0;
		c.weighty = 0.5;
		c.gridx = 0;
		c.gridy = 1;
		addcomp(pd = new molPanel(this,true), gridbag, c);

		p = new Panel();
		p.setLayout(new BorderLayout());

		q = new Panel();
		q.setLayout(new GridLayout(2,1));
		p.add("North", q);
		
		q.add(upbutt = new Button("Up"));
		upbutt.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				treeup();
			}
		});
		upbutt.setEnabled(false);

		/*
		Chemistry:
			Get Info
			Get CAS
			-----------
			Save ChemSketch
			
		Bookmarks
			Add bookmark
			-------------
			1. jsd;flkaj
			
		HELP
			HELP
		
		*/
		
		MenuBar mb = new MenuBar();
		Menu m;
		
		m = new Menu("Chemistry");
		m.add(new MenuItem("Get Info")).addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				findinfo();	
			}
		});
		m.add(new MenuItem("Get CAS")).addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				findcas();	
			}
		});
		m.add(new MenuItem("Save ChemSketch")).addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				savetree();	
			}
		});
		mb.add(m);
		
		bookmarkmenu = new Menu("Bookmarks");
		bookmarkmenu.add(new MenuItem("Add bookmark")).addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				addbookmark();	
			}
		});
		mb.add(bookmarkmenu);

		m = new Menu("Search");
		m.add(new MenuItem("Exhaustive")).addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				exhaustivesearch();	
			}
		});
		mb.add(m);
		
		m = new Menu("Help");
		m.add(new MenuItem("Help")).addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				help();	
			}
		});
		mb.add(m);
		
		setMenuBar(mb);
		

      CheckboxGroup cbg = new CheckboxGroup();
		Checkbox cb;

		q = new Panel();
		q.add(new Label("Mode: "));
		q.add(cb = new Checkbox("Disc.", cbg, true));
		cb.addItemListener(new ItemListener() {
			public void itemStateChanged(ItemEvent e) {
				if (e.getStateChange() == e.SELECTED)
					setmode(molPanel.MODE_DISC);
			}
		});

		q.add(cb = new Checkbox("Aux.", cbg, false));
		cb.addItemListener(new ItemListener() {
			public void itemStateChanged(ItemEvent e) {
				if (e.getStateChange() == e.SELECTED)
					setmode(molPanel.MODE_AUX);
			}
		});

		q.add(cb = new Checkbox("FG0", cbg, false));
		p.add("South", q);
		cb.addItemListener(new ItemListener() {
			public void itemStateChanged(ItemEvent e) {
				if (e.getStateChange() == e.SELECTED)
					setmode(molPanel.MODE_FG0);
			}
		});

		/*
		p.add("South", downbutt = new Button("Down"));
		downbutt.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				treedown();
			}
		});
		downbutt.setEnabled(false);
		*/

		p.add("Center", text = new TextArea("",1,15,TextArea.SCROLLBARS_NONE));
		text.setEditable(false);

		c.gridwidth = GridBagConstraints.REMAINDER;
		c.gridheight = 2;
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 0.1;
		c.weighty = 1.0;
		c.gridx = 1;
		c.gridy = 0;
		c.anchor = GridBagConstraints.EAST;
		addcomp(p, gridbag, c);

/*		
		if (getParameter("start") != null) 
			loadmolfile();
*/			
			
		repaint();
	}
	

	public void start() 
	{
	}

	protected void addcomp(Component comp, GridBagLayout gridbag,	GridBagConstraints c) {
		gridbag.setConstraints(comp, c);
		add(comp);
	}

	public void paint(Graphics g)
	{
		pu.repaint();
		pd.repaint();
		
		_rxninfo rxn = getnames.getrxninfo(rxnnum);

		System.out.println("S1 = "+s1+" S2 = "+s2);
		if ((text != null) && (rxn != null)) 
			text.setText(rxn.name+"\n"+s1+"\n"+s2+"\n\n"+rxn.comments+"\n");
	}

	public void recalc()
	{
		if ((pd != null) && (pu != null)) {
			pd.info();
			pu.setpos(pd.pos);
			repaint();
		}
	}

	public void setinfo(int rxnnum, String txt)
	{
		s1 = txt;
		this.rxnnum = rxnnum;
	}

	public void setinfo(String txt)
	{
		s2 = txt;
	}

	public void setmode(int mode)
	{
		pd.setmode(mode);
	}


	String processcas(String smiles)
	{
		String ret = "\n" + smiles + " ";
		commstream sock = new commstream(host, PORT);

		try {
			String str;

			System.out.println("CAS1");
			if (sock.sendcommand("CAS "+smiles)) {
				System.out.println("CAS2");
				StreamTokenizer st = new StreamTokenizer(sock.r);
				
				st.nextToken();
				System.out.println(st.sval);
				ret = ret + st.sval + " ";
				st.nextToken();
				System.out.println(st.sval);
				ret = ret + "("+st.sval+")";
			} else
				ret = ret + "(unknown)";

			sock.close();
		} catch (Exception e) {
			sock.popup(e);
		}
		System.out.println("CAS3");

		ret = ret + "\n";

		return(ret);
	}


	analysisnode processmol(analysisnode curr, int pos, int element, boolean isorphan, int mode, int param)
	{
		molDBnode mdbn;
		mol mol;
		analysisnode prec = null;
		boolean newprec = false;
		boolean analyze = true;
		analysiskey key = new analysiskey(pos, element,isorphan);

		mol = curr.getelement(key);

		if (mol != null) {
			mdbn = analysisnode.molDBget(mol);
			if (mdbn == null)
				System.out.println("No mdbn!");
			prec = (analysisnode)curr.precs.get(key);


			//System.out.println("curr.precs = "+curr.precs);
			if ((prec == null) && (mdbn != null) && (mdbn.precs != null)) {
				/* mol has been analyzed, but not on this branch */
				prec = new analysisnode(mdbn.precs);
				newprec = true;
			} 

			/* mol has not been analyzed */
			if (!((prec != null) && (mdbn != null) && (mdbn.findform(mode,param)))) {
				commstream sock = new commstream(host, PORT);

				try {
					String str;
					boolean ret = false;

					switch (mode) {
						case molPanel.MODE_DISC:
							ret = sock.sendcommand("ANALYZE");
							break;

						case molPanel.MODE_AUX:
							ret = sock.sendcommand("ANALAUX "+param);
							break;

						case molPanel.MODE_FG0:
							ret = sock.sendcommand("ANALFG0 "+param);
							break;
					}

					if (ret) {
						mol.trans(sock.w);
						str = sock.r.readLine();
						System.out.println("ANWS = "+str);

						if (str.charAt(0) == '+') {
							molmetalist mml = new molmetalist(sock.r);

							if (prec == null) {
								prec = new analysisnode(mml);
								newprec = true;
							} else {
								prec.mml.cat(mml);
							}

							try {
								/* getnames closes the socket */
								getnames t = new getnames(sock, prec);
								new Thread(t).start();
								//sock.close();
							} catch (Exception e) {}
						}
					}
				} catch (Exception e) {
					sock.popup(e);
				}
			}


			if (newprec) {
				mollist mollist = curr.mml.lists[pos];

				curr.addprec(prec, pos, element, isorphan, mode, param);
				//System.out.println("curr.precs = "+curr.precs);
				for (int i = 0; i < mollist.mols.length; ++i) 
					if (isorphan || (i != element))
						prec.addorphan(mollist.mols[i], i);
				
				if (isorphan)
					prec.removeorphan(element);
			}

			if ((mdbn != null) && !mdbn.findform(mode, param))
				mdbn.addform(mode,param);
		}
		
		return(prec);
	}



	void reload(analysisnode prec)
	{
		reload(prec,0);
	}
	
	void reload(analysisnode prec, int pos)
	{
		if (prec != null) {
			pu.reload(prec,pos);
			pd.reload(prec,pos);
			upbutt.setEnabled((prec.target != null));
		}
	}
	


	void treeup()
	{
		if (pd.man != null) {
			analysisnode node = pd.man.target;

			if (node != null) {
				analysisnode nodeup = node.target;

				pu.reload(node, pd.man.targetkey.mollistnum);
				pd.reload(node, pd.man.targetkey.mollistnum);
				/*
				if (nodeup != null) {
					mol mol = nodeup.getelement(node.targetkey);
					pu.reload(new analysisnode(new molmetalist(mol)));
				} else {
					upbutt.setEnabled(false);
					pu.reload(null,0);
				}
				*/
				//downbutt.setEnabled(true);
			}
		}
	}

	void treedown()
	{
	/*
		if (pd.man != null) {
			analysisnode node = pd.man.target;

			if (node != null) {
				analysisnode nodeup = node.target;

				pd.reload(node, pd.man.targetkey.mollistnum);
				if (nodeup != null) 
					pu.reload(new analysisnode(new molmetalist(nodeup.mml.lists[node.targetkey.mollistnum].mols[node.targetkey.molnum])));
				else
					pu.reload(null,0);
			}
		}
	*/
	}


	void findcas()
	{
		if (pd != null) {
			s2 = pd.findcas();
			repaint();
		}
	}

	void openURL(String s)
	{
		if (isApplet && Oset.class.isInstance(getParent())) {
			Oset app = (Oset)getParent();
			
			try {
				app.getAppletContext().showDocument(new URL(app.getDocumentBase(), s), "_blank");
			} catch (MalformedURLException e) {
				e.printStackTrace();
			}
		} else {
			System.out.println("Can't open URL from command line!");
		}
	}
	
	void findinfo()
	{
		System.out.println("findinfo");
		_rxninfo rxn = getnames.getrxninfo(rxnnum);
		if (rxn != null) {
			System.out.println(rxn.link);
			openURL(rxn.link);
		}
	}
	
	void help()
	{
		openURL("help.html");
	}
	
	void savetree()
	{
		if (pu != null) {
			commstream sock = new commstream(host, PORT);
	
			try {
				if (sock.sendcommand("SAVETREE")) 
					pu.trans(sock.w);
				sock.close();
			} catch (Exception e) {
				sock.popup(e);
			}
		}
	}
	
	public void loadmol(String s)
	{
		text.setText(s);

		commstream sock = new commstream(host, PORT);
		if (sock.s != null) {
			try {
				if (sock.sendcommand("START\n"+s+"\nM  END\n")) 
					pd.reload(new analysisnode(new molmetalist(new mol(sock.r)), true));

				sock.close();
			} catch (Exception e) {
				sock.popup(e);
			}

			setinfo(0,"Target mol");
		} else
			setinfo(0,"Can't open communication to server!");
	}
	

	public void loadmolfile(String fname) 
	{
		if (fname != null) {
			System.out.println("FILENAME = "+fname+"\n");

			commstream sock = new commstream(host, PORT);
			if (sock.s != null) {
				try {
					if (sock.sendcommand("LOAD "+fname)) 
						pd.reload(new analysisnode(new molmetalist(new mol(sock.r)), true));
	
					sock.close();
				} catch (Exception e) {
					sock.popup(e);
				}
	
				setinfo(0,"Target mol");
			} else
				setinfo(0,"Can't open communication to server!");
		}
	}

	
	void exhaustivesearch()
	{
		pd.exhaustivesearch(3);
	}


	Vector bookmarks = new Vector();
	
	void addbookmark()
	{
		addbookmark(new bookmark(pd.man, pd.pos), "Bookmark");
	}
	
	
	void addbookmark(bookmark b, String name)
	{
		bookmarks.addElement(b);

		bookmarkmenu.add(new MenuItem(bookmarks.size()+ ". "+name)).addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				int pos = 0;
				bookmark bb;
				try {
					String s = e.getActionCommand();
					pos = Integer.parseInt(s.substring(0,s.indexOf("."))) - 1;
				} catch (Exception ex) {};				
				bb = (bookmark)bookmarks.elementAt(pos);
				reload(bb.node, bb.pos);
			}
		});
	}
}


class commstream {
	Socket s;
	BufferedWriter w;
	BufferedReader r;

	commstream(String host, int port) 
	{
		open(host,port);
	}

	synchronized void open(String host, int port)
	{
		try {
			String str;

			if (host.equals(""))
				host = "127.0.0.1";
			System.out.print("Open comm to "+host+" "+port+"... ");

			s = new Socket(host, port);
			w = new BufferedWriter(new OutputStreamWriter(s.getOutputStream()));
			r = new BufferedReader(new InputStreamReader(s.getInputStream()));

			str = r.readLine();
			System.out.println("HELO = "+str);
		} catch (Exception e) {
			popup(e);
		}
	}

	synchronized boolean sendcommand(String s) {
		boolean ret = false;

		try {
			String str;
			str = new String(s+"\n");
			w.write(str);
			w.flush();

			for (str = ""; str.length() == 0; )
				str = r.readLine();
			
			System.out.println("Command: "+s+", Result: "+((str != null) ? str : "(null)"));

			ret = ((str != null) && (str.length() > 0) && (str.charAt(0) == '+'));
		} catch (Exception e) {
			popup(e);
		}
		return(ret);
	}

	synchronized void close() {
		try {
			sendcommand("QUIT");

			w.close();
			r.close();
			s.close();

			w = null;
			r = null;
			s = null;
			System.out.println("Close comm");
		} catch (Exception e) {
			popup(e);
		}
	}

	void popup(Exception e) {
		StringWriter sw = new StringWriter();
		PrintWriter pw = new PrintWriter(sw);
		e.printStackTrace(pw);
		pw.close();
		System.out.print("Popup Exception:\r\n"+sw.getBuffer());
	}
}

class _rxninfo {
	String name;
	String comments;
	String link;
	_rxninfo() {
		name = "";
		comments = "";
		link = "";
	}
}

class getnames implements Runnable
{
	static Hashtable rxninfo = new Hashtable();
	analysisnode an;
	commstream mysock;

	getnames(commstream sock, analysisnode an) {
		this.an = an;
		this.mysock = sock;
	}

	public void run() 
	{
		if (mysock != null) {
			if (an != null) 
				for (int i = 0; i < an.mml.lists.length; ++i)
					if ((an.mml.lists[i].rxn > 0) && (!rxninfo.containsKey(new Integer(an.mml.lists[i].rxn))))
						getrxninfo(mysock, an.mml.lists[i].rxn);
			mysock.close();
		}
	}

	_rxninfo getrxninfo(commstream sock, int rxnnum)
	{
		_rxninfo rxn = new _rxninfo();
		try {
			String str;

			if (sock.sendcommand("RXNINFO "+rxnnum)) {
				StreamTokenizer st = new StreamTokenizer(sock.r);

				st.nextToken();
				ParseUtil.match(st.sval,"NAME");
				st.nextToken();
				rxn.name = st.sval;

				st.nextToken();
				if (ParseUtil.ismatch(st.sval, "LINK")) {
					st.nextToken();
					rxn.link = st.sval;
					st.nextToken();
				}
				ParseUtil.match(st.sval,"COMMENTS");
				st.nextToken();
				rxn.comments = st.sval;
				
				rxninfo.put(new Integer(rxnnum), rxn);
			}
		} catch (Exception e) {
			sock.popup(e);
		}

		return(rxn);
	}


	public static _rxninfo getrxninfo(int rxnnum)
	{
		_rxninfo tmp = new _rxninfo();

		if (rxnnum > 0) {
			while ((tmp = (_rxninfo)rxninfo.get(new Integer(rxnnum))) == null)
				;
		}
		
		return(tmp);
	}
}



