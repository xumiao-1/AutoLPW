package auto.func;


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
 * File: GraphPanel.java
 *
 * Description:
 * Communicates with SerialForward, receiving packets and displaying
 * the received data graphically.
 *
 * @author Jason Hill and Eric Heien
 */

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.Panel;
import java.awt.Point;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Date;
import java.util.Vector;

import javax.swing.JFileChooser;

public class GraphPanel extends Panel implements MouseListener,
		MouseMotionListener {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;

	// If true, verbosely report received packet contents
//	private static final boolean VERBOSE = false;

	// // If true, log data to a file called LOG_FILENAME
	// private static final boolean LOG = false;
	// private static final String LOG_FILENAME = "log";

	// Set the number of channels to display and the number of readings
	// per packet. 2 channels and 4 reading means 2 readings per packet
	// per channel.
	static final int NUM_CHANNELS = 10;
	static int NUM_READINGS = 10;

	private static final double DEFAULT_BOTTOM = 0.0;
	private static final double DEFAULT_TOP = 50.0;
	private static final int DEFAULT_START = -10;
	private static final int DEFAULT_END = 150;
	private static final double X_AXIS_POSITION = 0.1;
	private static final double Y_AXIS_POSITION = 0.1;

	boolean sliding = false;
	boolean legend = true;
	boolean connectPoints = true;
	boolean yaxishex = false;
	boolean valueTest = false;
	int testChannel = -1, valueX, valueY;
	Oscilloscope graph;
	// MoteIF mote;

	// output stream for logging the data to.
	// PrintWriter log_os;

	double[] bottom, top;
	// double bottom2, top2; // added by Miao@Apr. 2012, for electrode/capacitor
	int start, end;
	int maximum_x = 0, minimum_x = Integer.MAX_VALUE;
	Vector cutoff;
	Vector2[] data;
	String dataLegend[];
	boolean legendActive[];
	Color plotColors[];
	int moteNum[]; // Maps channel # to mote #
	int streamNum[]; // Maps channel # to mote stream #
	int lastPacketNum[]; // Last packet # for channel
	Point highlight_start, highlight_end;

	// add by Miao @Apr. 2012
	// for more info
	Vector2[] electrode;
	Vector2[] capacitor;

	GraphPanel(Oscilloscope graph) {
		setBackground(Color.white);
		addMouseListener(this);
		addMouseMotionListener(this);
		cutoff = new Vector();

		// create an array to hold the data sets.
		data = new Vector2[NUM_CHANNELS];
		electrode = new Vector2[NUM_CHANNELS];
		capacitor = new Vector2[NUM_CHANNELS];

		for (int i = 0; i < NUM_CHANNELS; i++) {
			data[i] = new Vector2();
			electrode[i] = new Vector2();
			capacitor[i] = new Vector2();
		}

		dataLegend = new String[NUM_CHANNELS];
		legendActive = new boolean[NUM_CHANNELS];
		lastPacketNum = new int[NUM_CHANNELS];
		streamNum = new int[NUM_CHANNELS];
		moteNum = new int[NUM_CHANNELS];
		plotColors = new Color[NUM_CHANNELS];

		for (int i = 0; i < NUM_CHANNELS; i++) {
			lastPacketNum[i] = -1;
			streamNum[i] = -1;
			moteNum[i] = -1;
			dataLegend[i] = "";
			legendActive[i] = false;
		}
		plotColors[0] = Color.green;
		plotColors[1] = Color.red;
		plotColors[2] = Color.blue;
		plotColors[3] = Color.magenta;
		plotColors[4] = Color.orange;
		plotColors[5] = Color.yellow;
		plotColors[6] = Color.cyan;
		plotColors[7] = Color.pink;
		plotColors[8] = Color.green;
		plotColors[9] = Color.white;

		try {
			// create a file for logging data to.
			// FileOutputStream f = new FileOutputStream(LOG_FILENAME);
			// log_os = new PrintWriter(f);
		} catch (Exception e) {
			e.printStackTrace();
		}
		this.graph = graph;
		bottom = new double[2];
		top = new double[2];
		bottom[0] = DEFAULT_BOTTOM;
		top[0] = DEFAULT_TOP;
		bottom[1] = 0x0;
		top[1] = 0x0FFF;
		start = DEFAULT_START;
		end = DEFAULT_END;

		// OK, connect to the serial forwarder and start receiving data
		// mote = new MoteIF(PrintStreamMessenger.err, oscilloscope.group_id);
		// mote.registerListener(new OscopeMsg(), this);
	}

	int big_filter;
	int sm_filter;
	int db_filter;

	/**
	 * Add a point to the graph
	 * 
	 * @param dataset
	 *            a set that the point to be added to
	 * @param point
	 *            the point to be added
	 * @param place
	 *            which channel to be added
	 */
	void add_point(Vector2[] dataset, Point2D point, int place) {
		if (place >= dataset.length)
			return;
		dataset[place].add(point);
		if (point != null) {
			if ((int) point.x < minimum_x)
				minimum_x = (int) point.x;
			if ((int) point.x > maximum_x)
				maximum_x = (int) point.x;
		}

		if (point != null && sliding
				&& ((point.getX() > (end - 20)) || (point.getX() < start)) /*
																			 * &&
																			 * place
																			 * ==
																			 * 0
																			 */) {
			int diff = end - start;
			end = (int) point.getX() + 20;
			start = end - diff;
		}
		int max_length = 0x3fff;
		for (int i = 0; i < NUM_CHANNELS; i++) {
			if (dataset[i].size() > max_length) {
				synchronized (dataset[i]) {
					dataset[i].chop(max_length / 10);
				}
			}
		}

		// if( LOG && val != null )
		// log_os.println(""+val.toString() + ", " + place);

		repaint(100);
	}

	/**
	 * This is the handler invoked when a msg is received from SerialForward.
	 */
	public void drawData(SPacket_host packet, int place) {
		if (BASE_X == 0) {
			BASE_X = packet.getTimestamp().getTime();
		}/*
		 * else if ( (packet.getTimestamp().getTime() - BASE_X) > 24*3600*1000 )
		 * { BASE_X = packet.getTimestamp().getTime(); }
		 */

		double x = (packet.getTimestamp().getTime() - BASE_X) / 1000;

		Point2D aPoint = new Point2D(x, packet.getTmpr());
		add_point(data, aPoint, place);

		// Point2D bPoint = new Point2D(x, packet.getVoltage_E());
		Point2D bPoint = new Point2D(x, packet.isUnderWater_E() ? 1 : 0);
		add_point(electrode, bPoint, place);

//		Point2D cPoint = new Point2D(x, packet.getVoltage_C());
		Point2D cPoint = new Point2D(x, packet.isUnderWater_C() ? 1 : 0);
		add_point(capacitor, cPoint, place);
	}

	// public void messageReceived(int dest_addr, Message msg) {
	// if (msg instanceof OscopeMsg) {
	// oscopeReceived( dest_addr, (OscopeMsg)msg);
	// } else {
	// throw new
	// RuntimeException("messageReceived: Got bad message type: "+msg);
	// }
	// }

	// public void oscopeReceived(int dest_addr, OscopeMsg omsg) {
	// boolean foundPlot = false;
	// int moteID, packetNum, channelID, channel = -1, i;
	//
	// moteID = omsg.get_sourceMoteID();
	// channelID = omsg.get_channel();
	// packetNum = omsg.get_lastSampleNumber();
	//
	// for(i = 0; i < NUM_CHANNELS; i++) {
	// if(moteNum[i] == moteID && streamNum[i] == channelID) {
	// foundPlot = true;
	// legendActive[i] = true;
	// channel = i;
	// i = NUM_CHANNELS+1;
	// }
	// }
	//
	// if (!foundPlot) {
	// for(i = 0; i < NUM_CHANNELS && moteNum[i] != -1; i++);
	// if (i >= NUM_CHANNELS) {
	// System.err.println("\nWARNING: Cannot find empty channel for packet from mote ID "+moteID+" channelID "+channelID+" - recompile GraphViz.java with a larger setting for NUM_CHANNELS (currently "+NUM_CHANNELS+")");
	// return;
	// }
	//
	// channel = i;
	// moteNum[i] = moteID;
	// streamNum[i] = channelID;
	// lastPacketNum[i] = packetNum;
	// legendActive[i] = true;
	// dataLegend[i] = "Mote "+moteID+" Chan "+channelID;
	// }
	// if(channel < 0) {
	// System.err.println("All data streams full.  Please clear data set.");
	// return;
	// }
	// if(lastPacketNum[channel] == -1) lastPacketNum[channel] = packetNum;
	//
	// int packetLoss = packetNum - lastPacketNum[channel] - NUM_READINGS;
	//
	// for(int j = 0; j < packetLoss; j++) {
	// // Add "NUM_READINGS" blank points for each lost packet
	// for(i = 0; i < NUM_READINGS; i++)
	// add_point(null, channel);
	// }
	// lastPacketNum[channel] = packetNum;
	// int limit = omsg.numElements_data();
	// for (i = 0; i < limit; i++) {
	// Point2D newPoint;
	// int val = omsg.getElement_data(i);
	//
	// if (VERBOSE)
	// System.err.println("val: "+val+" (0x"+Integer.toHexString(val)+")");
	// newPoint = new Point2D( ((double)(packetNum+i)), val );
	// add_point( newPoint, channel);
	// }
	// }

	public void mouseEntered(MouseEvent e) {
	}

	public void mouseExited(MouseEvent e) {
	}

	/* Select a view rectangle. */
	public void mouseDragged(MouseEvent e) {
//		Dimension d = getSize();

		if (valueTest) {
			Point2D virt_drag = screenToVirtual(new Point2D(e.getX(), e.getY()));
			Point2D dp = findNearestX(data[testChannel], virt_drag);
			if (dp != null) {
				valueX = (int) dp.x;
				valueY = (int) dp.y;
			}

		} else if (highlight_start != null) {
			highlight_end.x = e.getX();
			highlight_end.y = e.getY();
		}
		repaint(100);
		e.consume();
	}

	public void mouseMoved(MouseEvent e) {
	}

	public void mouseClicked(MouseEvent e) {
	}

	/* Set zoom to selected rectangle. */
	public void mouseReleased(MouseEvent e) {
		removeMouseMotionListener(this);
		if (highlight_start != null)
			set_zoom();
		valueTest = false;
		testChannel = -1;
		highlight_start = null;
		highlight_end = null;
		e.consume();
		repaint(100);
	}

	public void mousePressed(MouseEvent e) {
		addMouseMotionListener(this);

		// Check to see if mouse clicked near plot
//		Dimension d = getSize();
//		double xVal, yVal;
		Point2D virt_click = screenToVirtual(new Point2D(e.getX(), e.getY()));
		for (int i = 0; i < NUM_CHANNELS; i++) {
			Point2D dp = findNearestX(data[i], virt_click);
			if (dp != null) {
				if (Math.abs(dp.y - virt_click.y) <= (top[0] - bottom[0]) / 10) {
					valueTest = true;
					testChannel = i;
					valueX = (int) dp.x;
					valueY = (int) dp.y;
				}
			}
		}

		if (!valueTest) {
			highlight_start = new Point();
			highlight_end = new Point();
			highlight_start.x = e.getX();
			highlight_start.y = e.getY();
			highlight_end.x = e.getX();
			highlight_end.y = e.getY();
		}
		repaint(100);
		e.consume();
	}

	public void start() {
	}

	public void stop() {
	}

	// double buffer the graphics.
	Image offscreen;
	Dimension offscreensize;
	Graphics offgraphics;

	public synchronized void update(Graphics g) {
		// get the size of the window.
		Dimension d = getSize();
		// get the end value of the window.
		int end = this.end;

		// graph.time_location.setValue((int)(end / ((maximum_x -
		// minimum_x)*1.0)));
		// create the offscreen image if necessary (only done once)
		if ((offscreen == null) || (d.width != offscreensize.width)
				|| (d.height != offscreensize.height)) {
			offscreen = createImage(d.width, d.height);
			offscreensize = d;
			if (offgraphics != null) {
				offgraphics.dispose();
			}
			offgraphics = offscreen.getGraphics();
			offgraphics.setFont(getFont());
		}
		// blank the screen.
		offgraphics.setColor(Color.black);
		offgraphics.fillRect(0, 0, d.width, d.height);

		// Draw axes
		Point2D[] origin = new Point2D[2];

		double xTicSpacing = (end - start) / 25.03;
		double[] yTicSpacing = new double[2];
		yTicSpacing[0] = (top[0] - bottom[0]) / 13.7;
		yTicSpacing[1] = (top[1] - bottom[1]) / 13.7;

		origin[0] = new Point2D(0, 0);
		origin[0].x = start + ((end - start) * X_AXIS_POSITION);
		origin[0].y = bottom[0] + ((top[0] - bottom[0]) * Y_AXIS_POSITION);
		origin[1] = new Point2D(0, 0);
		origin[1].x = end - ((end - start) * X_AXIS_POSITION);
		origin[1].y = bottom[1] + ((top[1] - bottom[1]) * Y_AXIS_POSITION);

		if (yaxishex) {
			// Round origin to integer
			for (int i = 0; i < 2; i++) {
				if ((origin[i].x % 1.0) != 0)
					origin[i].x -= (origin[i].x % 1.0);
				if ((origin[i].y % 1.0) != 0)
					origin[i].y -= (origin[i].y % 1.0);
			}
		} else {
			// Round origin to integer
			for (int i = 0; i < 2; i++) {
				if ((origin[i].x % 1.0) != 0)
					origin[i].x -= (origin[i].x % 1.0);
				if ((origin[i].y % 1.0) != 0)
					origin[i].y -= (origin[i].y % 1.0);
			}
		}

		// Prevent tics from being too small
		if (yTicSpacing[0] < 1.0)
			yTicSpacing[0] = 1.0;
		if ((yTicSpacing[0] % 1.0) != 0)
			yTicSpacing[0] += (1.0 - (yTicSpacing[0] % 1.0));
		if (yTicSpacing[1] < 1.0)
			yTicSpacing[1] = 1.0;
		if ((yTicSpacing[1] % 1.0) != 0)
			yTicSpacing[1] += (1.0 - (yTicSpacing[1] % 1.0));
		if (xTicSpacing < 1.0)
			xTicSpacing = 1.0;
		if ((xTicSpacing % 1.0) != 0)
			xTicSpacing += (1.0 - (xTicSpacing % 1.0));

		Color xColor, yColor;
		xColor = Color.white;
		yColor = Color.white;

		drawGridLines(offgraphics, origin[0], xTicSpacing, yTicSpacing[0]);
		drawAxisAndTics(offgraphics, origin, start, end, top, bottom,
				xTicSpacing, yTicSpacing, xColor, yColor);

		// draw the highlight box if there is one.
		draw_highlight(offgraphics);

		// draw the input channels.
		for (int i = 0; i < NUM_CHANNELS; i++) {
			offgraphics.setColor(plotColors[i]);

			// draw temperature
			if (graph.isDrawTemperature() && legendActive[i])
				draw_data(offgraphics, data[i], start, end);

			// draw electrode
			if (graph.isDrawElectrode() && legendActive[i])
				draw_voltage(offgraphics, electrode[i], start, end);

			// draw capacitor
			if (graph.isDrawCapacitor() && legendActive[i])
				draw_voltage(offgraphics, capacitor[i], start, end);

		}

		// Draw the value tester line if needed
		if (valueTest) {
			offgraphics.setFont(new Font("Default", Font.PLAIN, 12));
			offgraphics.setColor(new Color((float) 0.9, (float) 0.9,
					(float) 1.0));
			Point2D vt = virtualToScreen(new Point2D(valueX, valueY), YAXIS.L);
			offgraphics.drawLine((int) vt.x, 0, (int) vt.x, d.height);
			offgraphics.drawRect((int) vt.x - 3, (int) vt.y - 3, 6, 6);
			if (yaxishex) {
				offgraphics.drawString("[" + valueX + ",0x"
						+ Integer.toHexString(valueY).toUpperCase() + "]",
						(int) vt.x + 15, (int) vt.y - 15);
			} else {
				offgraphics.drawString("[" + valueX + "," + valueY + "]",
						(int) vt.x + 15, (int) vt.y - 15);
			}
		}

		drawLegend(offgraphics);

		// transfer the constructed image to the screen.
		g.drawImage(offscreen, 0, 0, null);
	}

	// Draw the grid lines
	void drawGridLines(Graphics offgraphics, Point2D origin,
			double xTicSpacing, double yTicSpacing) {

		offgraphics.setColor(new Color((float) 0.2, (float) 0.6, (float) 0.2));

		Point2D virt, screen;

		virt = new Point2D(origin.x, origin.y);
		screen = virtualToScreen(virt, YAXIS.L);
		while (screen.x < getSize().width) {
			offgraphics.drawLine((int) screen.x, 0, (int) screen.x,
					getSize().height);
			virt.x += xTicSpacing;
			screen = virtualToScreen(virt, YAXIS.L);
		}
		virt = new Point2D(origin.x, origin.y);
		screen = virtualToScreen(virt, YAXIS.L);
		while (screen.x >= 0) {
			offgraphics.drawLine((int) screen.x, 0, (int) screen.x,
					getSize().height);
			virt.x -= xTicSpacing;
			screen = virtualToScreen(virt, YAXIS.L);
		}

		virt = new Point2D(origin.x, origin.y);
		screen = virtualToScreen(virt, YAXIS.L);
		while (screen.y < getSize().height) {
			offgraphics.drawLine(0, (int) screen.y, getSize().width,
					(int) screen.y);
			virt.y -= yTicSpacing;
			screen = virtualToScreen(virt, YAXIS.L);
		}
		virt = new Point2D(origin.x, origin.y);
		screen = virtualToScreen(virt, YAXIS.L);
		while (screen.y >= 0) {
			offgraphics.drawLine(0, (int) screen.y, getSize().width,
					(int) screen.y);
			virt.y += yTicSpacing;
			screen = virtualToScreen(virt, YAXIS.L);
		}
	}

	void drawAxisAndTics(Graphics offgraphics, Point2D[] origin, int start,
			int end, double[] top, double[] bottom, double xTicSpacing,
			double[] yTicSpacing, Color xColor, Color yColor) {

		// Draw axis lines
		Point2D[] origin_screen = new Point2D[2];
		origin_screen[0] = virtualToScreen(origin[0], YAXIS.L);
		origin_screen[1] = virtualToScreen(origin[1], YAXIS.R);

		// x axis
		offgraphics.setColor(xColor);
		offgraphics.drawLine(0, (int) origin_screen[0].y, getSize().width,
				(int) origin_screen[0].y);
		// y axis
		offgraphics.setColor(yColor);
		for (int i = 0; i < 2; i++) {
			offgraphics.drawLine((int) origin_screen[i].x, 0,
					(int) origin_screen[i].x, getSize().height);
		}

		// Draw the tic marks and numbers
		offgraphics.setFont(new Font("Default", Font.PLAIN, 10));
		offgraphics.setColor(yColor);

		Point2D[] virt = new Point2D[2];
		Point2D[] screen = new Point2D[2];
		boolean label;

		// Y axis
		label = true;
		virt[0] = new Point2D(origin[0].x, origin[0].y);
		virt[1] = new Point2D(origin[1].x, origin[1].y);
		screen[0] = virtualToScreen(virt[0], YAXIS.L);
		screen[1] = virtualToScreen(virt[1], YAXIS.R);
		for (int i = 0; i < 2; i++) {
			while (screen[i].y < getSize().height) {
				offgraphics.drawLine((int) screen[i].x - 5, (int) screen[i].y,
						(int) screen[i].x + 5, (int) screen[i].y);
				if (label) {
					String tickstr;
					int xsub;
					if (yaxishex || i == 1) {
						int tmp = (int) (virt[i].y);
						tickstr = "0x" + Integer.toHexString(tmp).toUpperCase();
						xsub = 40;
					} else {
						tickstr = new Double(virt[i].y).toString();
						xsub = 25;
					}
					offgraphics.drawString(tickstr, (int) screen[i].x - xsub,
							(int) screen[i].y - 2);
					label = false;
				} else {
					label = true;
				}
				virt[i].y -= yTicSpacing[i];
				if (i == 0)
					screen[i] = virtualToScreen(virt[i], YAXIS.L);
				else
					screen[i] = virtualToScreen(virt[i], YAXIS.R);
			}
		}

		label = false;
		virt[0] = new Point2D(origin[0].x, origin[0].y + yTicSpacing[0]);
		virt[1] = new Point2D(origin[1].x, origin[1].y + yTicSpacing[1]);
		screen[0] = virtualToScreen(virt[0], YAXIS.L);
		screen[1] = virtualToScreen(virt[1], YAXIS.R);
		for (int i = 0; i < 2; i++) {
			while (screen[i].y >= 0) {
				offgraphics.drawLine((int) screen[i].x - 5, (int) screen[i].y,
						(int) screen[i].x + 5, (int) screen[i].y);
				if (label) {
					String tickstr;
					int xsub;
					if (yaxishex || i == 1) {
						int tmp = (int) (virt[i].y);
						tickstr = "0x" + Integer.toHexString(tmp).toUpperCase();
						xsub = 40;
					} else {
						tickstr = new Double(virt[i].y).toString();
						xsub = 25;
					}
					offgraphics.drawString(tickstr, (int) screen[i].x - xsub,
							(int) screen[i].y - 2);
					label = false;
				} else {
					label = true;
				}
				virt[i].y += yTicSpacing[i];
				if (i == 0)
					screen[i] = virtualToScreen(virt[i], YAXIS.L);
				else
					screen[i] = virtualToScreen(virt[i], YAXIS.R);
			}
		}

		// X axis
		label = true;
		virt[0] = new Point2D(origin[0].x, origin[0].y);
		screen[0] = virtualToScreen(virt[0], YAXIS.L);
		while (screen[0].x < getSize().width) {
			offgraphics.drawLine((int) screen[0].x, (int) screen[0].y - 5,
					(int) screen[0].x, (int) screen[0].y + 5);
			if (label) {
				String tickstr = new Double(virt[0].x).toString();
				offgraphics.drawString(tickstr, (int) screen[0].x - 15,
						(int) screen[0].y - 15);
				label = false;
			} else {
				label = true;
			}
			virt[0].x += xTicSpacing;
			screen[0] = virtualToScreen(virt[0], YAXIS.L);
		}

		label = false;
		virt[0] = new Point2D(origin[0].x - xTicSpacing, origin[0].y);
		screen[0] = virtualToScreen(virt[0], YAXIS.L);
		while (screen[0].x >= 0) {
			offgraphics.drawLine((int) screen[0].x, (int) screen[0].y - 5,
					(int) screen[0].x, (int) screen[0].y + 5);
			if (label) {
				String tickstr = new Double(virt[0].x).toString();
				offgraphics.drawString(tickstr, (int) screen[0].x - 15,
						(int) screen[0].y - 15);
				label = false;
			} else {
				label = true;
			}
			virt[0].x -= xTicSpacing;
			screen[0] = virtualToScreen(virt[0], YAXIS.L);
		}
	}

	void drawLegend(Graphics offgraphics) {
		int i;

		// Draw the legend
		if (legend) {
			int activeChannels = 0, curChan = 0;
			for (i = 0; i < NUM_CHANNELS; i++)
				if (legendActive[i])
					activeChannels++;

			if (activeChannels == 0)
				return;

			offgraphics.setColor(Color.black);
			offgraphics.fillRect(getSize().width - 20 - 130, getSize().height
					- 20 - 20 * activeChannels, 130, 20 * activeChannels);
			offgraphics.setColor(Color.white);
			offgraphics.drawRect(getSize().width - 20 - 130, getSize().height
					- 20 - 20 * activeChannels, 130, 20 * activeChannels);

			for (i = NUM_CHANNELS - 1; i >= 0; i--) {
				if (legendActive[i]) {
					offgraphics.setColor(Color.white);
					offgraphics.drawString(dataLegend[i],
							getSize().width - 20 - 100, getSize().height - 30
									- 17 * curChan);
					offgraphics.setColor(plotColors[i]);
					offgraphics.fillRect(getSize().width - 20 - 120,
							getSize().height - 38 - 17 * curChan, 10, 10);
					curChan++;
				}
			}
		}
	}

	// //return the difference between the two input vectors.
	// Vector<Double> diff(Iterator a, Iterator b){
	// Vector<Double> vals = new Vector<Double>();
	// while(a.hasNext() && b.hasNext()){
	// vals.add(new Double((((Double)b.next()).doubleValue() -
	// ((Double)a.next()).doubleValue())));
	// }
	// return vals;
	// }

	// draw the highlight box.
	void draw_highlight(Graphics g) {
		if (highlight_start == null)
			return;
		int x, y, h, l;
		x = Math.min(highlight_start.x, highlight_end.x);
		y = Math.min(highlight_start.y, highlight_end.y);
		l = Math.abs(highlight_start.x - highlight_end.x);
		h = Math.abs(highlight_start.y - highlight_end.y);
		g.setColor(Color.white);
		g.fillRect(x, y, l, h);
	}

	void draw_data(Graphics g, Vector data, int start, int end) {
		draw_data(g, data, start, end, 1);
	}

	// scale multiplies a signal by a constant factor.
	void draw_data(Graphics g, Vector data, int start, int end, int scale) {
		Point2D screen = null, screen2 = null;
		boolean noplot = true; // Used for line plotting

		for (int i = 0; i < data.size(); i++) {
			Point2D virt;
			// map each point to a x,y position on the screen.
			if ((virt = (Point2D) data.get(i)) != null) {
				screen = virtualToScreen(virt, YAXIS.L);
				if (screen.x >= 0 && screen.x < getSize().width) {
					if (connectPoints && !noplot)
						g.drawLine((int) screen2.x, (int) screen2.y,
								(int) screen.x, (int) screen.y);
					else if (!connectPoints)
						g.drawRect((int) screen.x, (int) screen.y, 1, 1);
					if (noplot)
						noplot = false;
				} else {
					noplot = true;
				}
			}
			screen2 = screen;
		}
	}

	void draw_voltage(Graphics g, Vector data, int start, int end) {
		draw_voltage(g, data, start, end, 1);
	}

	// scale multiplies a signal by a constant factor.
	void draw_voltage(Graphics g, Vector data, int start, int end, int scale) {
		Point2D screen = null, screen2 = null;
		boolean noplot = true; // Used for line plotting

		for (int i = 0; i < data.size(); i++) {
			Point2D virt;
			// map each point to a x,y position on the screen.
			if ((virt = (Point2D) data.get(i)) != null) {
				screen = virtualToScreen(virt, YAXIS.R);
				if (screen.x >= 0 && screen.x < getSize().width) {
					if (connectPoints && !noplot)
						g.drawLine((int) screen2.x, (int) screen2.y,
								(int) screen.x, (int) screen.y);
					else if (!connectPoints)
						g.drawRect((int) screen.x, (int) screen.y, 1, 1);
					if (noplot)
						noplot = false;
				} else {
					noplot = true;
				}
			}
			screen2 = screen;
		}
	}

	// functions for controlling zooming.
	void move_up() {
		double height = top[0] - bottom[0];
		bottom[0] += height / 4;
		top[0] += height / 4;

	}

	void move_down() {
		double height = top[0] - bottom[0];
		bottom[0] -= height / 4;
		top[0] -= height / 4;

	}

	void move_right() {
		int width = end - start;
		start += width / 4;
		end += width / 4;

	}

	void move_left() {
		int width = end - start;
		start -= width / 4;
		end -= width / 4;

	}

	void zoom_out_x() {
		int width = end - start;
		start -= width / 2;
		end += width / 2;
	}

	void zoom_out_y() {
		double height = top[0] - bottom[0];
		bottom[0] -= height / 2;
		top[0] += height / 2;
	}

	void zoom_in_x() {
		int width = end - start;
		start += width / 4;
		end -= width / 4;
	}

	void zoom_in_y() {
		double height = top[0] - bottom[0];
		bottom[0] += height / 4;
		top[0] -= height / 4;
	}

	void reset() {
		bottom[0] = DEFAULT_BOTTOM;
		top[0] = DEFAULT_TOP;
		start = DEFAULT_START;
		end = DEFAULT_END;
	}

	void clear_data() {
		// Reset all motes

		// try {
		// System.err.println("SENDING OscopeResetmsg\n");
		// mote.send(MoteIF.TOS_BCAST_ADDR, new OscopeResetMsg());
		// } catch (IOException ioe) {
		// System.err.println("Warning: Got IOException sending reset message: "+ioe);
		// ioe.printStackTrace();
		// }

		int i;
		data = new Vector2[NUM_CHANNELS];
		for (i = 0; i < NUM_CHANNELS; i++)
			data[i] = new Vector2();
		for (i = 0; i < NUM_CHANNELS; i++)
			dataLegend[i] = "";
		for (i = 0; i < NUM_CHANNELS; i++)
			legendActive[i] = false;
		for (i = 0; i < NUM_CHANNELS; i++)
			lastPacketNum[i] = -1;
		for (i = 0; i < NUM_CHANNELS; i++)
			streamNum[i] = -1;
		for (i = 0; i < NUM_CHANNELS; i++)
			moteNum[i] = -1;
	}

	// Currently non-functional b/c of switch to 2D point data
	void load_data() {
		JFileChooser file_chooser = new JFileChooser();
		File loadedFile;
		FileReader dataIn;
		String lineIn;
		int retval, chanNum, numSamples;
		boolean keepReading;

		retval = file_chooser.showOpenDialog(null);
		if (retval == JFileChooser.APPROVE_OPTION) {
			try {
				loadedFile = file_chooser.getSelectedFile();
				System.out.println("Opened file: " + loadedFile.getName());
				dataIn = new FileReader(loadedFile);
				keepReading = true;
				chanNum = numSamples = -1;
				while (keepReading) {
					lineIn = read_line(dataIn);
					if (lineIn == null)
						keepReading = false;
					else if (!lineIn.startsWith("#")) {
						if (chanNum == -1) {
							try {
								chanNum = Integer.parseInt(lineIn.substring(0,
										lineIn.indexOf(" ")));
								numSamples = Integer.parseInt(lineIn.substring(
										lineIn.indexOf(" ") + 1,
										lineIn.length()));
								data[chanNum] = new Vector2();
								System.out.println("" + chanNum + " "
										+ numSamples + "\n");
							} catch (NumberFormatException e) {
								System.out.println("File is invalid.");
								System.out.println(e);
							}
						} else {
							try {
								numSamples--;
								if (numSamples <= 0)
									numSamples = chanNum = -1;
							} catch (NumberFormatException e) {
								System.out.println("File is invalid.");
								System.out.println(e);
							}
						}
					}
				}
				dataIn.close();
			} catch (IOException e) {
				System.out.println(e);
			}
		}

	}

	String read_line(FileReader dataIn) {
		StringBuffer lineIn = new StringBuffer();
		int c/*, readOne*/;

		try {
			while (true) {
				c = dataIn.read();
				if (c == -1 || c == '\n') {
					if (lineIn.toString().length() > 0)
						return lineIn.toString();
					else
						return null;
				} else
					lineIn.append((char) c);
			}
		} catch (IOException e) {
		}
		return lineIn.toString();
	}

	void save_data() {
		JFileChooser file_chooser = new JFileChooser();
		File savedFile;
		FileWriter dataOut;
		int retval, i, n;

		retval = file_chooser.showSaveDialog(null);
		if (retval == JFileChooser.APPROVE_OPTION) {
			try {
				savedFile = file_chooser.getSelectedFile();
				System.out.println("Saved file: " + savedFile.getName());
				dataOut = new FileWriter(savedFile);
				dataOut.write("# Test Data File\n");
				dataOut.write("# " + (new Date()) + "\n");
				for (i = 0; i < 10; i++) {
					if (data[i].size() > 0) {
						dataOut.write("# Channel " + i + "\n");
						dataOut.write("# " + data[i].size() + " samples\n");
						dataOut.write("" + i + " " + data[i].size() + "\n");
						for (n = 0; n < data[i].size(); n++) {
							Point2D sample;
							sample = (Point2D) data[i].get(n);
							if (sample != null) {
								dataOut.write("" + sample.getX() + " "
										+ sample.getY());
								dataOut.write("\n");
							}
						}
					}
				}
				dataOut.close();
			} catch (IOException e) {
				System.out.println(e);
			}
		}

	}

	void set_zoom() {
//		int base = getSize().height;
		int x_start = Math.min(highlight_start.x, highlight_end.x);
		int x_end = Math.max(highlight_start.x, highlight_end.x);
		int y_start = Math.min(highlight_start.y, highlight_end.y);
		int y_end = Math.max(highlight_start.y, highlight_end.y);

		if (Math.abs(x_start - x_end) < 10)
			return;
		if (Math.abs(y_start - y_end) < 10)
			return;

		Point2D topleft = screenToVirtual(new Point2D(x_start, y_start));
		Point2D botright = screenToVirtual(new Point2D(x_end, y_end));

		start = (int) topleft.x;
		end = (int) botright.x;
		top[0] = topleft.y;
		bottom[0] = botright.y;
	}

	/** Convert from virtual coordinates to screen coordinates. */
	// Point2D virtualToScreen(Point2D virt) {
	// double xoff = virt.getX() - start;
	// double xpos = xoff / (end * 1.0 - start * 1.0);
	// double screen_xpos = xpos * getSize().width;
	//
	// double yoff = virt.getY() - bottom;
	// double ypos = yoff / (top * 1.0 - bottom * 1.0);
	// double screen_ypos = getSize().height - (ypos * getSize().height);
	//
	// return new Point2D(screen_xpos, screen_ypos);
	// }

	enum YAXIS {
		L, R
	};

	Point2D virtualToScreen(Point2D virt, YAXIS side) {
		double xoff = virt.getX() - start;
		double xpos = xoff / (end * 1.0 - start * 1.0);
		double screen_xpos = xpos * getSize().width;

		double t, b;
		if (side == YAXIS.L) {
			t = top[0];
			b = bottom[0];
		} else {
			t = top[1];
			b = bottom[1];
		}

		double yoff = virt.getY() - b;
		double ypos = yoff / (t * 1.0 - b * 1.0);
		double screen_ypos = getSize().height - (ypos * getSize().height);

		return new Point2D(screen_xpos, screen_ypos);
	}

	/** Convert from screen coordinates to virtual coordinates. */
	Point2D screenToVirtual(Point2D screen) {
		double xoff = screen.getX();
		double xpos = xoff / (getSize().width * 1.0);
		double virt_xpos = start + (xpos * (end * 1.0 - start * 1.0));

		double yoff = screen.getY();
		double ypos = yoff / (getSize().height * 1.0);
		double virt_ypos = top[0] - (ypos * (top[0] * 1.0 - bottom[0] * 1.0));

		return new Point2D(virt_xpos, virt_ypos);
	}

	/** Find nearest point in 'data' to x-coordinate of given point. */
	Point2D findNearestX(Vector2 data, Point2D test) {
		try {
			float xval = Math.round(test.x);
			for (int i = 0; i < data.size(); i++) {
				Point2D pt = (Point2D) data.get(i);
				if (pt == null)
					continue;
				if (Math.round(pt.x) == xval) {
					return pt;
				}
			}
			return null;
		} catch (Exception e) {
			return null;
		}
	}

	/** A simple inner class representing a 2D point. */
	class Point2D {
		double x, y;

		Point2D(double newX, double newY) {
			x = newX;
			y = newY;
		}

		double getX() {
			return x;
		}

		double getY() {
			return y;
		}

		public String toString() {
			return x + "," + y;
		}
	}

	/** An extension to Vector supporting chop(). */
	class Vector2 extends Vector<Point2D> {
		/**
		 * 
		 */
		private static final long serialVersionUID = 1L;

		void chop(int index) {
			removeRange(0, index);
		}
	}

	static long BASE_X = 0;
	// static long BASE_Y = 0;

}
