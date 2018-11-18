import java.applet.*;
import java.awt.*;
import java.awt.event.*;
import java.net.*;
import java.io.*;
import java.lang.Exception;
import java.util.*;


public class testmenu extends Applet {
        boolean loaded = false;

        public void init() {
                Frame fr = new testmenuframe("testmenu");
                fr.setSize(400,200);
                fr.setVisible(true);
                loaded = true;
	}

        public boolean is_loaded() {
                return(loaded);
        }
}


class testmenuframe extends Frame {
        public testmenuframe(String title) {
                super(title);

		MenuBar mb = new MenuBar();
		Menu m = new Menu("Commands");
		mb.add(m);
		m.add("Info");
		m.add("Next");
                setMenuBar(mb);
        }

        public boolean handleEvent(Event e) {
                switch(e.id) {
                        case e.WINDOW_DESTROY:
                                dispose();
                                return(true);
                }

                return(false);
        }
}


