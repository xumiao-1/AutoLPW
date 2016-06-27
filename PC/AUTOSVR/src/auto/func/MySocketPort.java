package auto.func;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Scanner;

public class MySocketPort implements Runnable, IPort {
	// private static SocketPort instance; // singleton instance

	private static Thread thread;

	private Socket cltSkt;
	private String server_ip;
	private int server_port;

	// send/receive byte array from the server
	private InputStream inputStream; // input stream from serial port
	private OutputStream outputStream; // output stream to serial port

	private PrintWriter logOut;
	private PrintWriter cmdOut;
	private PrintWriter debugOut;

	private FileM fileM;

	public MySocketPort(String ip, int port) throws UnknownHostException,
			IOException {
		fileM = new FileM();
		cmdOut = new PrintWriter(System.out);

		// server info
		server_ip = ip;
		server_port = port;
	}

	public OutputStream getOutputStream() {
		return outputStream;
	}

	public boolean openPort() {
		try {
			// open existing files in append mode
			fileM.openFiles();

			cltSkt = new Socket(server_ip, server_port);
			inputStream = (cltSkt.getInputStream());
			outputStream = (cltSkt.getOutputStream());

			logOut = new PrintWriter(new FileWriter(new File("log.txt"), true));
			debugOut = new PrintWriter(new FileWriter(new File("data.dat"),
					true));

			// the thread listen to the incoming stream
			thread = new Thread(this);
			thread.start();
		} catch (Exception e) {
			return false;
		}
		return true;
	}

	public boolean closePort() {

		if (thread != null) {
			try {
				thread.stop();

				// close files
				fileM.closeFiles();

				// close input/output stream
				inputStream.close();
				outputStream.close();
				cltSkt.close();

				// close other output
				logOut.close();
				debugOut.close();
			} catch (IOException e) {
				return false;
			}
		}

		return true;
	}

	// keep waiting for the incoming command from the server
	public void run() {
		while (true) {
			try {
				if (inputStream.available() > 0) {
					int cmd = inputStream.read();

					if (cmd == 0) { // data info
						SPacket_host packet = SPacket_host.read(inputStream);

						logOut.println("Read a packet: "
								+ packet.toByteString());
						cmdOut.println("[ data] " + packet.toByteString());

						switch (packet.getHostCmd()) {
						case 0x00A5: // wakeup response from host
							break;

						default:
							// save to files
							int place = fileM.writePacket(packet);

							// draw the data
							if (0 <= place && place < GraphPanel.NUM_CHANNELS) {
								// plot in panel
								Oscilloscope.app.drawData(packet, place);
							}

							break;
						}

						logOut.println(packet.toString());
						logOut.println();
						logOut.println();

						logOut.flush();
						cmdOut.flush();
					} else if (cmd == 1 || cmd == 2) { // debug info
						String s = new Scanner(inputStream).nextLine();

						if (cmd == 1) {
							debugOut.println("[debug] " + s);
							cmdOut.println("[debug] " + s);
						} else {
							debugOut.println("[ info] " + s);
							cmdOut.println("[ info] " + s);
						}

						debugOut.flush();
						cmdOut.flush();
					}
				}
			} catch (IOException e) {
			}
		}
	}

	@Override
	public InputStream getInputStream() {
		return inputStream;
	}

}
