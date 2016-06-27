package auto.func;

import gnu.io.CommPortIdentifier;
import gnu.io.SerialPort;
import gnu.io.SerialPortEvent;
import gnu.io.SerialPortEventListener;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Enumeration;

enum SCmd {
	S_UNKNOWN,

	S_CMD_SYNC, // sync time with pc
	S_CMD_UPLD, // upload data to pc
	S_CMD_DUMP, // dump memory to pc
	S_CMD_INTV, // change app interval
	S_CMD_UPDT, // image info
	S_CMD_SHUT, // shutdown node
	S_CMD_IMGS, S_CMD_IMGE, S_CMD_IMGD // image detail
}

public class MySerialPort implements SerialPortEventListener, IPort {
	// private static final int S_CMD_UPDT = 0x05;
	// private static final int S_CMD_SHUT = 0x06;

	private CommPortIdentifier portId;
	private SerialPort serialPort;

	private InputStream inputStream; // input stream from serial port
	private OutputStream outputStream; // output stream to serial port

	private PrintWriter logOut;
	// private PrintWriter cmdOut;
	// private PrintWriter debugOut;

	private FileM fileM;
	private String fname = null;

	public MySerialPort(String port_info) throws Exception {
		fileM = new FileM();
		// cmdOut = new PrintWriter(System.out);

		portId = CommPortIdentifier.getPortIdentifier(port_info);
	}

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
			// open existing files in append mode
			fileM.openFiles();

			serialPort = (SerialPort) portId.open("MySerial", 2000);
			inputStream = serialPort.getInputStream();
			outputStream = serialPort.getOutputStream();

			logOut = new PrintWriter(new FileWriter(new File("log.txt"), true));
			// debugOut = new PrintWriter(new FileWriter(new File("debug.txt"),
			// true));

			logOut.println("####" + new java.util.Date());

			serialPort.addEventListener(this);

			serialPort.notifyOnDataAvailable(true);

			serialPort.setSerialPortParams(115200, SerialPort.DATABITS_8,
					SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);

			// // Add ownership listener to allow ownership event handling.
			// portId.addPortOwnershipListener(this);
		} catch (Exception exp) {
			return false;
		}

		return true;
	}

	public boolean closePort() {
		try {
			// close files
			fileM.closeFiles();

			inputStream.close();
			outputStream.close();

			logOut.close();
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
						if (n != 2)
							continue;

						// logOut.println("Read a packet: " + cmd[0]);
						System.out.println("[ data] " + cmd[0]);

						switch (cmd[0]) {
						case 1: // SNR_CMD_RDY
//							fname = Oscilloscope.app.get_command_handle()
//									.send_to_sensor_pin_config();
							break;

						case 2: // PC_CMD_CFG
							// System.out.println(SPacket.toByteString(cmd));
							break;

						case 3: // SNR_CMD_LPW_BGN
							// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
							// will change in future
							// send_to_monitor_begin();
							if (fname.length() != 0) {
								Thread.sleep(1500);
								System.out.println("running: IviDemo " + fname
										+ ".txt ...");

								String[] exec_cmd = { "IviDemo", "40",
										fname + ".txt" };
								Process p = Runtime.getRuntime().exec(exec_cmd);
								// p.waitFor();

								// hook up child process output to parent
								InputStream lsOut = p.getInputStream();
								InputStreamReader r = new InputStreamReader(
										lsOut);
								BufferedReader in = new BufferedReader(r);

								// read the child process' output
								String line;
								while ((line = in.readLine()) != null) {
									logOut.println(line);
									System.out.println(line);
								}
								_result = line;
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

						logOut.flush();
						System.out.flush();
					} else { // debug info
						byte[] cmd = new byte[128];
						int n = inputStream.read(cmd);

						System.out.println("[ dbug] (" + n + ")"
								+ SPacket.toByteString(cmd));
					}
				}

				if (bflag)
					_finish = true;
			} catch (IOException e) {
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

			break;
		}
	}

	@Override
	public OutputStream getOutputStream() {
		return outputStream;
	}

	@Override
	public InputStream getInputStream() {
		return inputStream;
	}

	private boolean _finish = false;
	private String _result = "";

	boolean isFinish() {
		return _finish;
	}

	String getResult() {
		return _result;
	}
}
