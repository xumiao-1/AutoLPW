package auto.func;

// $Id: oscilloscope.java,v 1.3 2003/10/07 21:46:00 idgay Exp $

/*									tab:4
 * "Copyright (c) 2000-2003 The Regents of the University  of California.  
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice, the following
 * two paragraphs and the author appear in all copies of this software.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Copyright (c) 2002-2003 Intel Corporation
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE     
 * file. If you do not find these files, copies can be found by writing to
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300, Berkeley, CA, 
 * 94704.  Attention:  Intel License Inquiry.
 */

/**
 * File: oscilloscope.java
 *
 * Description:
 * Displays data coming from the apps/oscilloscope application.
 *
 * Requires that the SerialForwarder is already started.
 *
 * @author Jason Hill and Eric Heien
 */

import java.awt.BorderLayout;
import java.awt.Button;
import java.awt.Checkbox;
import java.awt.CheckboxMenuItem;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.FileDialog;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.GridLayout;
import java.awt.Label;
import java.awt.Menu;
import java.awt.MenuBar;
import java.awt.MenuItem;
import java.awt.Panel;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.text.SimpleDateFormat;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Scanner;

import javax.swing.JCheckBox;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JSlider;
import javax.swing.JTextField;
import javax.swing.event.AncestorEvent;
import javax.swing.event.AncestorListener;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import jmetal.experiments.studies.mytest;

public class Oscilloscope extends JPanel implements ActionListener,
		ItemListener, ChangeListener/* , Runnable */{
	/** data format */
	public final static SimpleDateFormat dateFormat = new SimpleDateFormat(
			"yyyy-MM-dd@HH:mm:ss");

	// low power test
	public static LPWTest tblPwr;

	// public static PrintWriter logOut;

	private void myinit() throws IOException {
		FileUtils.setLogFile(new PrintWriter(new FileWriter("log.txt", true)));
		tblPwr = new LPWTest();
	}

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;

	private static IPort.Type PORT_TYPE = IPort.Type.SERIAL_PORT;// IPort.Type.SOCKET_PORT;//

	private MenuBar mb = new MenuBar();
	private Menu portMenu = new Menu("Port");
	private MenuItem openItem = new MenuItem("Open port");
	// private Menu openMenu = new Menu("Open port");
	private MenuItem closeItem = new MenuItem("Close port");
	private MenuItem exitItem = new MenuItem("Exit");

	// send cmd to host via serial port
	private Menu sendMenu = new Menu(TOOL_MENU);
	private MenuItem syncItem = new MenuItem(CFG_MENU);

	// upload menus
	private Menu uploadMenu = new Menu(ALGO_MENU);
	private MenuItem uploadItem = new MenuItem(INIT_MENU);
	private MenuItem uploadtItem = new MenuItem(RUN_MENU);
	// private MenuItem lqiItem = new MenuItem("LQI");

	private MenuItem dumpItem = new MenuItem("Dump flash");
	private MenuItem periodItem = new MenuItem(LOAD_MENU);

	// update menus
	private Menu updateMenu = new Menu("Update");
	private MenuItem hostItem = new MenuItem("Image...");
	// private MenuItem sensorItem = new MenuItem("Sensor...");

	// shutdown
	// private MenuItem shutdownItem = new MenuItem("On/Off");
	private Menu offMenu = new Menu("On/Off");
	private MenuItem shutdownItem = new MenuItem("Shutdown...");
	private MenuItem reprogItem = new MenuItem("Update restart");
	// private MenuItem restartItem = new MenuItem("Restart");

	// view history log
	private Menu viewMenu = new Menu("View");
	private MenuItem histItem = new MenuItem(EXPORT_MENU);
	private Menu graphMenu = new Menu("Graph");
	private CheckboxMenuItem tempItem = new CheckboxMenuItem("Temperature");
	private CheckboxMenuItem elecItem = new CheckboxMenuItem("Electrode");
	private CheckboxMenuItem capaItem = new CheckboxMenuItem("Capacitor");

	Button timeout = new Button("Control Panel");
	Button move_up = new Button("^");
	Button move_down = new Button("v");
	Button move_right = new Button(">");
	Button move_left = new Button("<");
	Button zoom_out_x = new Button("Zoom Out X");
	Button zoom_in_x = new Button("Zoom In X");
	Button zoom_out_y = new Button("Zoom Out Y");
	Button zoom_in_y = new Button("Zoom In Y");
	Button reset = new Button("Reset");
	Button save_data = new Button("Save Data");
	Button load_data = new Button("Load Data");
	Button editLegend = new Button("Edit Legend");
	Button clear_data = new Button("Clear Dataset");
	Checkbox showLegend = new Checkbox("Show Legend", true);
	Checkbox connect_points = new Checkbox("Connect Datapoints", true);
	Checkbox YAxisHex = new Checkbox("hex Y Axis", false);
	Checkbox scrolling = new Checkbox("Scrolling", false);
	JSlider time_location = new JSlider(0, 100, 100);

	public JTextField legendEdit[];
	public JCheckBox legendActive[];

	public JLabel high_pass_val = new JLabel("5");
	public JLabel low_pass_val = new JLabel("2");
	public JLabel cutoff_val = new JLabel("65");
	public JSlider high_pass = new JSlider(0, 30, 5);
	public JSlider low_pass = new JSlider(0, 30, 2);
	public JSlider cutoff = new JSlider(0, 4000, 65);

	GraphPanel panel;
	Panel controlPanel;

	public void init() {
		time_location.addChangeListener(this);

		setLayout(new BorderLayout());

		openItem.addActionListener(this);
		// portMenu.add(openMenu);
		// portMenu.add(openItem);

		closeItem.addActionListener(this);
		closeItem.setEnabled(false);
		// portMenu.add(closeItem);

		exitItem.addActionListener(this);
		portMenu.add(exitItem);

		mb.add(portMenu);

		syncItem.addActionListener(this);
		sendMenu.add(syncItem);
		syncItem.setEnabled(false);

		dumpItem.addActionListener(this);
		sendMenu.add(dumpItem);

		periodItem.addActionListener(this);
		sendMenu.add(periodItem);

		// upload menus
		uploadItem.setEnabled(false);
		uploadItem.addActionListener(this);
		uploadMenu.add(uploadItem);
		uploadtItem.addActionListener(this);
		uploadMenu.add(uploadtItem);
		sendMenu.add(uploadMenu);
		// lqiItem.addActionListener(this);
		// uploadMenu.add(lqiItem);

		// update menus
		hostItem.addActionListener(this);
		// sensorItem.addActionListener(this);
		updateMenu.add(hostItem);
		// updateMenu.add(sensorItem);
		sendMenu.add(updateMenu);
		updateMenu.setEnabled(false);

		// on/off menus
		shutdownItem.addActionListener(this);
		offMenu.add(shutdownItem);
		reprogItem.addActionListener(this);
		offMenu.add(reprogItem);
		sendMenu.add(offMenu);
		offMenu.setEnabled(false);

		// sendMenu.setEnabled(false);
		mb.add(sendMenu);

		// view menus
		graphMenu.add(tempItem);
		tempItem.addItemListener(this);
		tempItem.setState(true);
		graphMenu.add(elecItem);
		elecItem.addItemListener(this);
		graphMenu.add(capaItem);
		capaItem.addItemListener(this);
		viewMenu.add(graphMenu);
		viewMenu.add(histItem);
		histItem.addActionListener(this);

		mb.add(viewMenu);

		panel = new GraphPanel(this);
		add("Center", panel);
		controlPanel = new Panel();
		add("South", controlPanel);

		Panel x_pan = new Panel();
		x_pan.setLayout(new GridLayout(5, 1));
		x_pan.add(zoom_in_x);
		x_pan.add(zoom_out_x);
		x_pan.add(save_data);
		x_pan.add(editLegend);
		x_pan.add(clear_data);
		zoom_out_x.addActionListener(this);
		zoom_in_x.addActionListener(this);
		save_data.addActionListener(this);
		editLegend.addActionListener(this);
		clear_data.addActionListener(this);
		controlPanel.add(x_pan);

		Panel y_pan = new Panel();
		y_pan.setLayout(new GridLayout(5, 1));
		y_pan.add(zoom_in_y);
		y_pan.add(zoom_out_y);
		y_pan.add(load_data);
		y_pan.add(showLegend);
		y_pan.add(connect_points);
		zoom_out_y.addActionListener(this);
		zoom_in_y.addActionListener(this);
		load_data.addActionListener(this);
		showLegend.addItemListener(this);
		connect_points.addItemListener(this);
		controlPanel.add(y_pan);

		Panel scroll_pan = new Panel();
		move_up.addActionListener(this);
		move_down.addActionListener(this);
		move_right.addActionListener(this);
		move_left.addActionListener(this);
		reset.addActionListener(this);
		GridBagLayout g = new GridBagLayout();
		GridBagConstraints c = new GridBagConstraints();
		scroll_pan.setLayout(g);
		c.gridx = 1;
		c.gridy = 0;
		g.setConstraints(move_up, c);
		scroll_pan.add(move_up);
		c.gridx = 0;
		c.gridy = 1;
		g.setConstraints(move_left, c);
		scroll_pan.add(move_left);
		c.gridx = 1;
		c.gridy = 1;
		g.setConstraints(reset, c);
		scroll_pan.add(reset);
		c.gridx = 2;
		c.gridy = 1;
		g.setConstraints(move_right, c);
		scroll_pan.add(move_right);
		c.gridx = 1;
		c.gridy = 2;
		g.setConstraints(move_down, c);
		scroll_pan.add(move_down);
		controlPanel.add(scroll_pan);

		// controlPanel.add(timeout);
		timeout.addActionListener(this);
		Panel p = new Panel();
		p.setLayout(new GridLayout(4, 1));
		p.add(YAxisHex);
		YAxisHex.addItemListener(this);
		p.add(scrolling);
		scrolling.addItemListener(this);
		p.add(time_location);
		p.add(timeout);
		controlPanel.add(p);

		panel.repaint();
		repaint();

		legendEdit = new JTextField[GraphPanel.NUM_CHANNELS];
		for (int i = 0; i < GraphPanel.NUM_CHANNELS; i++)
			legendEdit[i] = new JTextField(128);

		legendActive = new JCheckBox[GraphPanel.NUM_CHANNELS];
		for (int i = 0; i < GraphPanel.NUM_CHANNELS; i++)
			legendActive[i] = new JCheckBox("Plot " + (i + 1));

		// ArrayList<String> ports = myport.getAvailablePorts();
		// if (!ports.isEmpty()) {
		// for (String s : ports) {
		// MenuItem item = new MenuItem(s);
		// item.addActionListener(this);
		// openMenu.add(item);
		// }
		// }

		// mycmd = new Command();

		// for boston deployment
		dumpItem.setEnabled(false);
		// updateMenu.setEnabled(false);
		// uploadMenu.setEnabled(false);

		// get pin configurations
		// ConfigDB.read_cfg_db("pin_config.txt");
		// _pin_config = new ArrayList<String>();
		// read_pin_config("pin_config.txt");
	}

	public void destroy() {
		remove(panel);
		remove(controlPanel);
	}

	public void start() {
		panel.start();
	}

	public void stop() {
		panel.stop();
	}

	public void actionPerformed(ActionEvent e) {
		Object src = e.getSource();
		for (int i = 0; i < GraphPanel.NUM_CHANNELS; i++) {
			if (legendEdit[i] == src) {
				panel.dataLegend[i] = legendEdit[i].getText();
				panel.repaint(100);
			}
		}

		if (src == zoom_out_x) {
			panel.zoom_out_x();
			panel.repaint();
		} else if (src == zoom_in_x) {
			panel.zoom_in_x();
			panel.repaint();
		} else if (src == zoom_out_y) {
			panel.zoom_out_y();
			panel.repaint();
		} else if (src == zoom_in_y) {
			panel.zoom_in_y();
			panel.repaint();
		} else if (src == move_up) {
			panel.move_up();
			panel.repaint();
		} else if (src == move_down) {
			panel.move_down();
			panel.repaint();
		} else if (src == move_right) {
			panel.move_right();
			panel.repaint();
		} else if (src == move_left) {
			panel.move_left();
			panel.repaint();
		} else if (src == reset) {
			panel.reset();
			panel.repaint();
		} else if (src == load_data) {
			panel.load_data();
			panel.repaint();
		} else if (src == clear_data) {
			panel.clear_data();
			panel.repaint();
		} else if (src == save_data) {
			panel.save_data();
			panel.repaint();
		} else if (src == editLegend) {
			JFrame legend = new JFrame("Edit Legend");
			legend.setSize(new Dimension(200, 500));
			legend.setVisible(true);
			Panel slp = new Panel();
			slp.setLayout(new GridLayout(10, 2));
			for (int i = 0; i < GraphPanel.NUM_CHANNELS; i++) {
				legendActive[i].setSelected(panel.legendActive[i]);
				slp.add(legendActive[i]);
				legendActive[i].addChangeListener(this);
				legendEdit[i].setText(panel.dataLegend[i]);
				slp.add(legendEdit[i]);
				legendEdit[i].addActionListener(this);
			}
			legend.getContentPane().add(slp);
			legend.pack();
			legend.repaint();
		} else if (src == timeout) {
			JFrame sliders = new JFrame("Filter Controls");
			sliders.setSize(new Dimension(300, 30));
			sliders.setVisible(true);
			Panel slp = new Panel();
			slp.setLayout(new GridLayout(3, 3));
			slp.add(new Label("high_pass:"));
			slp.add(high_pass);
			slp.add(high_pass_val);
			slp.add(new Label("low_pass:"));
			slp.add(low_pass);
			slp.add(low_pass_val);
			slp.add(new Label("cutoff:"));
			slp.add(cutoff);
			slp.add(cutoff_val);
			high_pass.addChangeListener(this);
			low_pass.addChangeListener(this);
			cutoff.addChangeListener(this);
			sliders.getContentPane().add(slp);
			sliders.pack();
			sliders.repaint();
		} else if (src == exitItem) {
			// if (myport != null)
			// myport.closePort();
			FileUtils.closeLogFile();// logOut.close();
			System.exit(0);
		} else if (src == openItem) {
			// try {
			// if (PORT_TYPE == IPort.Type.SERIAL_PORT) { // serial port
			// myport = new MySerialPort(read_config("MyPlot-config.txt"));
			// } else if (PORT_TYPE == IPort.Type.SOCKET_PORT) { // socket
			// // port
			// myport = new MySocketPort(read_config("MyPlot-config.txt"),
			// 8888);
			// }
			//
			// if (myport.openPort()) {
			// mycmd.setOutputStream(myport.getOutputStream());
			// openItem.setEnabled(false);
			// closeItem.setEnabled(true);
			// // sendMenu.setEnabled(true);
			// } else {
			// JOptionPane.showMessageDialog(null,
			// "Please check your port connection!");
			// }
			// } catch (Exception e1) {
			// JOptionPane.showMessageDialog(null,
			// "Please check your port connection!");
			// }
		} else if (src == closeItem) {
			// if (myport.closePort()) {
			// openItem.setEnabled(true);
			// closeItem.setEnabled(false);
			// // sendMenu.setEnabled(false);
			// }
		} else if (src == syncItem) {
			// // mycmd.sendSync();
			// String pin[] = { "PA:1", "PA:2", "PA:3", "PA:4" };
			// String option[] = { "GP:PP:PU", "GP:PP:PD", "GP:OD:PU",
			// "GP:OD:PD",
			// "IN:FLOAT", "IN:PU", "IN:PD", "AN" };
			// genCFG(pin, option);
		} else if (src == uploadItem) {
			// JFileChooser chooser = new JFileChooser();
			// if (chooser.showOpenDialog(null) == JFileChooser.APPROVE_OPTION)
			// {
			// System.out.println(chooser.getSelectedFile().getName());
			// }
		} else if (src == uploadtItem) {
			if (MyDialog.showConfirmDialog(panel)) {
//				Thread ga = new Thread(new mytest(MyDialog.getInterval()), "GA");
				Thread ga = new Thread(new GreedyTest(MyDialog.getInterval()), "GA");
				ga.start();
			}
		} else if (src == periodItem) {
			// load data from file
			tblPwr = ConfigDB.read_pwr_db("power.txt");
			System.out.println("Load Done: " + tblPwr.getMap().size() + " existing profiles are loaded!");
		} else if (src == histItem) {
			try {
				String s = JOptionPane.showInputDialog("").trim();
				if (s != null && !s.isEmpty()) {
					String[] ss = s.split("\\s+");
					int[] mode = new int[Profile.PIN_TOTAL_NUM];
					for (int i = 0; i < Profile.PIN_TOTAL_NUM; i++) {
						mode[i] = Integer.parseInt(ss[i]);
					}
					System.out.println(new Profile(mode).genCode());
				}

				// FileDialog fd = new FileDialog(Oscilloscope.mainFrame,
				// "Export", FileDialog.LOAD);
				// fd.setVisible(true);
				// String file = fd.getFile();
				// if (file != null) {
				// String dir = fd.getDirectory();
				// MyLog log = new MyLog(dir + file);
				// log.load();
				// }
			} catch (Exception exp) {
			}
		} else if (src == dumpItem) {
			// mycmd.sendDump();
		} else if (src == hostItem) { // host image
			FileDialog fd = new FileDialog(Oscilloscope.mainFrame,
					"Update host", FileDialog.LOAD);
			fd.setVisible(true);
			String fname = fd.getFile();
			if (fname != null) {
				String dname = fd.getDirectory();
				File f = new File(dname + fname);
				sendMenu.setEnabled(false);
				// try { // sending img...
				// mycmd.sendUpdate(f, calc_checksum(f));
				// } catch (IOException e1) {
				// JOptionPane
				// .showMessageDialog(null,
				// "Reprogramming failed! Please check the image file!");
				// }
				sendMenu.setEnabled(true);
			}
		} else if (src == shutdownItem) {
			// String nodeid = JOptionPane
			// .showInputDialog("Node ID (type 0 if you want to close them all)");
			//
			// while (nodeid != null) {
			// nodeid = nodeid.trim();
			// if (nodeid.equals("0"))
			// nodeid = "0000";
			//
			// if (nodeid.length() == 4) { // valid value
			// mycmd.sendShut(nodeid);
			// break;
			// } else { // invalid
			// JOptionPane.showMessageDialog(null, "Invalid node id!");
			// nodeid = JOptionPane
			// .showInputDialog("Node ID (type 0 if you want to close them all)");
			// }
			// }
		} else if (src == reprogItem) {
			// mycmd.sendUpdateRestart("0000");
		} else {
			// if (src instanceof MenuItem) {
			// MenuItem item = (MenuItem) src;
			// if (item.getParent().equals(openMenu)) {
			// if (myport.openPort(item.getActionCommand())) {
			// openMenu.setEnabled(false);
			// closeItem.setEnabled(true);
			// sendMenu.setEnabled(true);
			// }
			// }
			// }
		}
	}

	public void itemStateChanged(ItemEvent e) {
		Object src = e.getSource();
		boolean on = e.getStateChange() == ItemEvent.SELECTED;
		if (src == scrolling) {
			panel.sliding = on;
		} else if (src == showLegend) {
			panel.legend = on;
			panel.repaint(100);
		} else if (src == connect_points) {
			panel.connectPoints = on;
			panel.repaint(100);
		} else if (src == YAxisHex) {
			panel.yaxishex = on;
			panel.repaint(100);
		} else {
			panel.repaint(100);
		}
	}

	public void stateChanged(ChangeEvent e) {
		Object src = e.getSource();
		if (src == time_location) {
			double percent = (time_location.getValue() / 100.0);
			int diff = panel.end - panel.start;
			panel.end = panel.minimum_x
					+ (int) ((panel.maximum_x - panel.minimum_x) * percent);
			panel.start = panel.end - diff;
		}
		high_pass_val.setText("" + high_pass.getValue());
		cutoff_val.setText("" + cutoff.getValue());
		low_pass_val.setText("" + low_pass.getValue());
		for (int i = 0; i < GraphPanel.NUM_CHANNELS; i++)
			if (src == legendActive[i])
				panel.legendActive[i] = legendActive[i].isSelected();

		panel.repaint(100);
	}

	static Oscilloscope app;
	static JFrame mainFrame;

	// If specified as -1, then reset messages will only work properly
	// with the new TOSBase base station
	static int group_id = -1;

	public static void main(String[] args) /*
											 * throws UnknownHostException,
											 * IOException throws IOException
											 */{
		if (args.length == 1) {
			group_id = (byte) Integer.parseInt(args[0]);
			System.err.println("oscilloscope: Using group ID " + group_id);
			System.err
					.println("Note: group id should not be specified if you're using a TOSBase base station");
		}

		// JFrame
		mainFrame = new JFrame("AutoPower - Greedy");
		// mainFrame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		mainFrame.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);

		// JPanel
		app = new Oscilloscope();

		try {
			app.myinit();
		} catch (IOException e) {
			System.out.println("Cannot create log file.");
		}
		app.init();

		app.start();

		mainFrame.setMenuBar(app.mb);
		mainFrame.add("Center", app);

		mainFrame.setSize(app.getSize());
		mainFrame.setVisible(true);

		// // if possible, open port at start
		// try {
		// if (PORT_TYPE == IPort.Type.SERIAL_PORT) { // serial port
		// app.myport = new MySerialPort(read_config("MyPlot-config.txt"));
		// } else if (PORT_TYPE == IPort.Type.SOCKET_PORT) { // socket
		// // port
		// app.myport = new MySocketPort(read_config("MyPlot-config.txt"),
		// 8888);
		// }
		//
		// // if (app.myport.openPort()) {
		// // app.mycmd.setOutputStream(app.myport.getOutputStream());
		// // app.openItem.setEnabled(false);
		// // app.closeItem.setEnabled(true);
		// // // app.sendMenu.setEnabled(true);
		// // } else {
		// // JOptionPane.showMessageDialog(null,
		// // "Please check your port connection!");
		// // }
		// } catch (Exception e1) {
		// JOptionPane.showMessageDialog(null,
		// "Please check your port connection!");
		// }

		// // for boston deployment
		// app.upThread = new Thread(app);
		// app.upThread.start();

		// mainFrame.repaint(1000);
		// app.panel.repaint();

	}

	public Dimension getSize() {
		return new Dimension(600, 600);
	}

	public boolean isDrawTemperature() {
		return tempItem.getState();
	}

	public boolean isDrawElectrode() {
		return elecItem.getState();
	}

	public boolean isDrawCapacitor() {
		return capaItem.getState();
	}

	public void drawData(SPacket_host pkt, int place) {
		panel.drawData(pkt, place);
	}

	// // for boston deployment,
	// // enable continuous query every 30min
	// private static final int UPLOAD_INT = 1 * 10 * 1000;

	// public void run() {
	// while (true) {
	// File f = new File("restart");
	// if (f.exists()) {
	// // delete & restart
	// f.delete();
	// try {
	// Runtime.getRuntime().exec("shutdown -r -t 0");
	// } catch (IOException e) {
	// // TODO Auto-generated catch block
	// e.printStackTrace();
	// }
	// }
	//
	// if (sendMenu.isEnabled()) {
	// mycmd.sendUpload();
	// }
	//
	// try {
	// Thread.sleep(UPLOAD_INT);
	// } catch (Exception exp) {
	// break;
	// }
	// }
	// }

	public static String read_config(String fname) {
		String s = "";

		try {
			Scanner scan = new Scanner(new File(fname));
			s = scan.nextLine().trim().toUpperCase();
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			return s;
		}

		return s;
	}

	private int calc_checksum(File f) throws IOException {
		int checksum = 0x0;

		DataInputStream fis = new DataInputStream(new FileInputStream(f));

		for (int i = 0; i < f.length(); i++) {
			int b = fis.read();
			checksum = (checksum + b);
		}

		fis.close();

		return (checksum) & 0x0000FFFF;
	}

	// // read pin config file
	// public int read_pin_config(String fname) {
	// int n = 0;
	//
	// try {
	// Scanner scan = new Scanner(new File(fname));
	// while (scan.hasNextLine()) {
	// String s = scan.nextLine().trim();
	// _pin_config.add(s);
	// n++;
	// }
	// } catch (FileNotFoundException e) {
	// // TODO Auto-generated catch block
	// e.printStackTrace();
	// }
	//
	// return n;
	// }

	// private Command mycmd;
	// private IPort myport;

	// private Thread upThread;

	// pin configuration
	// static ArrayList<String> _pin_config;
	// public Command get_command_handle() {
	// return mycmd;
	// }

	// menu...
	private final static String TOOL_MENU = "Tools";
	private final static String CFG_MENU = "Gen_cfg";
	private final static String LOAD_MENU = "Open";
	private final static String ALGO_MENU = "GA";
	private final static String INIT_MENU = "Init...";
	private final static String RUN_MENU = "Run...";
	private final static String EXPORT_MENU = "Export...";

	// auto-generate a pin configuration
	// GRP#:PIN#:SPEED:CFG
	public void genCFG(String pin[], String option[]) {
		try {
			PrintWriter cfgfile = new PrintWriter(new FileWriter(new File(
					"pin_config.txt"), false));
			for (int i = 0; i < option.length; i++) {
				re_genCFG(pin[0] + ":400K:" + option[i], pin, option, 1,
						cfgfile);
			}
			cfgfile.close();
		} catch (IOException e) {
			System.out.println("Err: failed to generate config file.");
		}
	}

	private void re_genCFG(String cfg, String pin[], String option[], int i,
			PrintWriter cfgfile) {
		if (i == pin.length - 1) { // 32 pins in total, will change in future
			String prefix = cfg + "|" + pin[i] + ":400K:";
			for (int j = 0; j < option.length; j++) {
				cfg = prefix + option[j];
				System.out.println(cfg);
				cfgfile.println(cfg);
			}
		} else {
			String prefix = cfg + "|" + pin[i] + ":400K:";
			// if (i == 0)
			// prefix += "PA:" + i + ":400K:";
			// else if (i < 16)
			// prefix += "|PA:" + i + ":400K:";
			// else
			// prefix += "|PB:" + i % 16 + ":400K:";
			for (int j = 0; j < option.length; j++) {
				cfg = prefix + option[j];
				re_genCFG(cfg, pin, option, i + 1, cfgfile);
			}
		}
	}
}

/**
 * Embedded class for wake-up time settings
 * 
 * @author Miao
 */
class MyDialog {
	private static int interval = 5;
	private static int duration = 0;

	public static boolean showConfirmDialog(Component arg0) {
		JPanel myPanel = new JPanel(new GridLayout(4, 1));
		JLabel lbl1 = new JLabel("Interval (seconds)");
		JLabel lbl2 = new JLabel("Duration (seconds)");
		JTextField txt1 = new JTextField();
		JTextField txt2 = new JTextField();
		myPanel.add(lbl1);
		myPanel.add(txt1);
		myPanel.add(lbl2);
		myPanel.add(txt2);
		txt1.setText(String.valueOf(interval));
		txt2.setText("0");
		txt2.setEnabled(false);
		txt1.addAncestorListener(new RequestFocusListener());

		while (JOptionPane.showConfirmDialog(null, myPanel, "Wakeup settings",
				JOptionPane.OK_CANCEL_OPTION) == JOptionPane.OK_OPTION) {
			try {
				int val1 = Integer.parseInt(txt1.getText());
				int val2 = Integer.parseInt(txt2.getText());
				/*
				 * if (val2 < 10) { String msg =
				 * "Duration cannot be less than 10s";
				 * JOptionPane.showMessageDialog(myPanel, msg, "Invalid input",
				 * JOptionPane.ERROR_MESSAGE); } else
				 */if (val1 < val2 + interval) {
					String msg = "Measurement time cannot be less than "
							+ (val2 + interval) + "s.";
					JOptionPane.showMessageDialog(myPanel, msg,
							"Invalid input", JOptionPane.ERROR_MESSAGE);
				} else {
					interval = val1;
					duration = val2;
					return true;
				}
			} catch (NumberFormatException exp) {
				String msg = "Input must be valid integers";
				JOptionPane.showMessageDialog(myPanel, msg, "Invalid input",
						JOptionPane.ERROR_MESSAGE);
			}
		}

		return false;
	}

	public static int getInterval() {
		return interval;
	}

	public static int getDuration() {
		return duration;
	}
}

/**
 * Convenience class to request focus on a component.
 * 
 * When the component is added to a realized Window then component will request
 * focus immediately, since the ancestorAdded event is fired immediately.
 * 
 * When the component is added to a non realized Window, then the focus request
 * will be made once the window is realized, since the ancestorAdded event will
 * not be fired until then.
 * 
 * Using the default constructor will cause the listener to be removed from the
 * component once the AncestorEvent is generated. A second constructor allows
 * you to specify a boolean value of false to prevent the AncestorListener from
 * being removed when the event is generated. This will allow you to reuse the
 * listener each time the event is generated.
 */
/* public */class RequestFocusListener implements AncestorListener {
	private boolean removeListener;

	/*
	 * Convenience constructor. The listener is only used once and then it is
	 * removed from the component.
	 */
	public RequestFocusListener() {
		this(true);
	}

	/*
	 * Constructor that controls whether this listen can be used once or
	 * multiple times.
	 * 
	 * @param removeListener when true this listener is only invoked once
	 * otherwise it can be invoked multiple times.
	 */
	public RequestFocusListener(boolean removeListener) {
		this.removeListener = removeListener;
	}

	@Override
	public void ancestorAdded(AncestorEvent e) {
		JComponent component = e.getComponent();
		component.requestFocusInWindow();

		if (removeListener)
			component.removeAncestorListener(this);
	}

	@Override
	public void ancestorMoved(AncestorEvent e) {
	}

	@Override
	public void ancestorRemoved(AncestorEvent e) {
	}
}
