package auto.func;

import gnu.io.CommPortIdentifier;
import gnu.io.SerialPort;
import gnu.io.SerialPortEvent;
import gnu.io.SerialPortEventListener;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Enumeration;

public class DAQMeasure implements SerialPortEventListener {

	// private static IPort myport;
	// private static Command mycmd;

	private static DAQMeasure inst_ = null;

	protected DAQMeasure() /* throws Exception */{
		// command from PC to nodes
		mycmd = new Command();

		// // open the port
		// openPort();
	}

	public static DAQMeasure getInstance() /* throws Exception */{
		if (inst_ == null) {
			inst_ = new DAQMeasure();
		}

		return inst_;
	}

	// measure
	public Double measure(Profile pro) throws Exception {
		// // power on the sensor
		// String[] exec_cmd = { "powertoolcmd", "/vout=3.2",
		// "/trigger=DTD" + String.valueOf(2 * _mtime) + "A",
		// "/savefile=datafile.pt4", "/noexitwait" };
		// Process p = Runtime.getRuntime().exec(exec_cmd);

		_pro = pro;

		// 1. open port
		// 2. talk to sensor
		// 3. close port
		// 4. get power
		mycmd.sendSync(2 * _mtime); // activate sensor for measurement
		_finish = false;
		while (!_finish) {
		}
		Thread.sleep(1000);

		// // keep the power on
		// int ret = p.waitFor();
		// p.destroy();
		//
		// Thread.sleep(5000);
		//
		// if (ret != 0) {
		// FileUtils.logFile.println("**********" + ret);
		// }

		// collect the measurement
		if (_result != null) {
			FileUtils.logFile.println(_pro.toString() + " " + _result);
			FileUtils.logFile.flush();
			System.out.println(_result + " ("
					+ new java.util.Date().toString().substring(11, 19) + ")");
			String[] ss = _result.split("\\s+");
			_result = null;
			return Double.parseDouble(ss[1]);
		} else { // for test purpose
			Double rand = Math.random();
			FileUtils.logFile.println(rand);
			FileUtils.logFile.flush();
			System.out.println(rand + " (rand "
					+ new java.util.Date().toString().substring(11, 19) + ")");
			return rand;
		}

		// return null;
	}

	private CommPortIdentifier portId;
	private Command mycmd;
	private SerialPort serialPort;

	private InputStream inputStream; // input stream from serial port
	private OutputStream outputStream; // output stream to serial port

	// private PrintWriter logOut;
	// private PrintWriter cmdOut;
	// private PrintWriter debugOut;

	// private FileM fileM;
	private String fname = null;

	public ArrayList<String> getAvailablePorts() {
		ArrayList<String> list = new ArrayList<String>();

		Enumeration<CommPortIdentifier> portEnum = CommPortIdentifier
				.getPortIdentifiers();

		while (portEnum.hasMoreElements()) {
			CommPortIdentifier portIdentifier = portEnum.nextElement();
			if (portIdentifier.getPortType() == CommPortIdentifier.PORT_SERIAL)
				list.add(portIdentifier.getName());
		}

		return list;
	}

	public boolean openPort() {
		try {
			// associate the port
			portId = CommPortIdentifier.getPortIdentifier(Oscilloscope
					.read_config("AUTOSVR-config.txt"));

			serialPort = (SerialPort) portId.open("MySerial", 2000);
			inputStream = serialPort.getInputStream();
			outputStream = serialPort.getOutputStream();

			// logOut = new PrintWriter(new FileWriter(new File("log.txt"),
			// true));
			// debugOut = new PrintWriter(new FileWriter(new File("debug.txt"),
			// true));

			// logOut.println("####" + new java.util.Date());

			serialPort.addEventListener(this);

			serialPort.notifyOnDataAvailable(true);

			serialPort.setSerialPortParams(115200, SerialPort.DATABITS_8,
					SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);

			mycmd.setOutputStream(outputStream);

			// // Add ownership listener to allow ownership event handling.
			// portId.addPortOwnershipListener(this);
		} catch (Exception exp) {
			System.out.println("Please check your port connection!");
			System.exit(-1);
			return false;
		}

		return true;
	}

	public boolean closePort() {
		try {
			// close files
			// fileM.closeFiles();

			inputStream.close();
			outputStream.close();

			// logOut.close();
			// debugOut.close();

			// serialPort.removeEventListener();
			serialPort.close();
		} catch (Exception exp) {
		}
		// portId.removePortOwnershipListener(this);
		return true;
	}

	public void serialEvent(SerialPortEvent event) {
		switch (event.getEventType()) {
		case SerialPortEvent.BI:
		case SerialPortEvent.OE:
		case SerialPortEvent.FE:
		case SerialPortEvent.PE:
		case SerialPortEvent.CD:
		case SerialPortEvent.CTS:
		case SerialPortEvent.DSR:
		case SerialPortEvent.RI:
		case SerialPortEvent.OUTPUT_BUFFER_EMPTY:
			break;
		case SerialPortEvent.DATA_AVAILABLE:

			try {
				boolean bflag = false;
				while (inputStream.available() > 0 && !bflag) {
					int pre = inputStream.read();

					if (pre == 0) { // data info
						// System.out.println(inputStream.read());
						// System.out.println(inputStream.read());
						byte[] cmd = new byte[4];
						int n = inputStream.read(cmd);
						// if (n != 2)
						// continue;

						// logOut.println("Read a packet: " + cmd[0]);
						System.out.println("[ data] "
								+ cmd[0]
								+ " ("
								+ new java.util.Date().toString().substring(11,
										19) + ")");
						System.out.flush();

						switch (cmd[0]) {
						case 1: // SNR_CMD_RDY
							// fname = Oscilloscope.app.get_command_handle()
							// .send_to_sensor_pin_config();
							fname = mycmd.send_to_sensor_pin_config_1(_pro);
							FileUtils.logFile.println(_pro.toString());
							FileUtils.logFile.flush();
							break;

						case 2: // PC_CMD_CFG
							// System.out.println(SPacket.toByteString(cmd));
							break;

						case 3: // SNR_CMD_LPW_BGN
							// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
							// will change in future
							// send_to_monitor_begin();
							if (fname.length() != 0) {
								Thread.sleep(1000 * _mtime / 2);
								System.out.println("running: IviDemo "
										+ fname
										+ ".txt ..."
										+ " ("
										+ new java.util.Date().toString()
												.substring(11, 19) + ")");

								String[] exec_cmd = { "IviDemo",
										String.valueOf(_mtime), fname + ".txt" };
								Process p = Runtime.getRuntime().exec(exec_cmd);
								int ret = p.waitFor();

								// hook up child process output to parent
								InputStream lsOut = p.getInputStream();
								InputStreamReader r = new InputStreamReader(
										lsOut);
								BufferedReader in = new BufferedReader(r);

								// read the child process' output
								String line;
								while ((line = in.readLine()) != null) {
									if (ret == 0) {
										_result = line;
										System.out.println(_result
												+ " ("
												+ new java.util.Date()
														.toString().substring(
																11, 19) + ")");
									} else {
										_result = null;
									}
								}

								p.destroy();
							}
							break;

						case 4: // SNR_CMD_LPW_END
							// send_to_monitor_end();
							bflag = true;
							break;

						default:
							// // save to files
							// int place = fileM.writePacket(packet);
							//
							// // draw the data
							// if (0 <= place && place <
							// GraphPanel.NUM_CHANNELS) {
							// // plot in panel
							// Oscilloscope.app.drawData(packet, place);
							// }

							break;
						}

					} else { // debug info
						byte[] cmd = new byte[128];
						int n = inputStream.read(cmd);

						System.out.println("[ dbug] (" + n + ")"
								+ SPacket.toByteString(cmd));
					}
				}

				if (bflag) {
					_finish = true;
				}
			} catch (IOException e) {
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

			break;
		}
	}

	public OutputStream getOutputStream() {
		return outputStream;
	}

	public InputStream getInputStream() {
		return inputStream;
	}

	public void setMeasureTime(int ns) {
		_mtime = ns;
	}

	private volatile boolean _finish;
	private String _result;
	private Profile _pro;

	private int _mtime; // in seconds

}
