package auto.func;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.PrintWriter;

public class MyLog {
	public MyLog(String fn) throws FileNotFoundException {
		fname = fn;
	}

	public void load() {
		System.out.println("Loading " + fname + " ...");

		try {
			File f = new File(fname);
			FileInputStream fis = new FileInputStream(f);
			PrintWriter pw = new PrintWriter(new FileWriter(new File("d_"
					+ fname.substring(fname.length() - 4) + ".txt"), false));

			while (fis.available() > 0) {
				SPacket_host packet = SPacket_host.read(fis);
				System.out.println(packet.toByteString());

				// // get LQI of its parent
				// double lqi_p = 0;
				// for (int i = 0; i < 8; i++) {
				// if (packet.getLQI(i).relation == LQI.NB_P) {
				// lqi_p = packet.getLQI(i).lqi * 100 / LQI.MAX_BEACONS;
				// break;
				// }
				// }

				String s = String.format("%04X %5d %s %s %.2f %5.2f %4d %.1f",
						packet.getPacketSrc(), packet.getSeqno(),
						Oscilloscope.dateFormat.format(packet.getTimestamp()),
						Oscilloscope.dateFormat.format(packet.getRecvTime()),
						packet.getPDR(), packet.getTmpr(),
						packet.getBufferedPkts(), packet.getVoltage_bat());
				pw.println(s);
				// pw.println("Read a packet: " + packet.toBytes());
				// pw.println(packet.toString());
				pw.flush();
			}

			pw.close();
			fis.close();
		} catch (Exception exp) {
			System.out.println("MyLog:load()");
		}

		System.out.println("Done!");
	}

	private String fname;
}
