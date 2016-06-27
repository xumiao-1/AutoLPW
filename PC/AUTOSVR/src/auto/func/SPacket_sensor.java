package auto.func;

import java.io.IOException;
import java.io.InputStream;

public class SPacket_sensor extends SPacket {
	public final static int MAX_PKT_SIZE = 32;
	// byte[] pkt_bytes;
	int pkt_size;

	private SPacket_sensor(byte[] data, int len) {
		pkt_size = len;
		pkt_bytes = new byte[pkt_size];
		System.arraycopy(data, 0, pkt_bytes, 0, pkt_size);
	}

	public static SPacket_sensor read(InputStream is) throws IOException {
		byte[] buf = new byte[MAX_PKT_SIZE];

		// int len = is.read(buf);
		// if (len > 0) {
		// return new SPacket_sensor(buf, len);
		// }
		int i = 0;
		while (i < SPacket_sensor.MAX_PKT_SIZE) {
			int n = is.read();
			if (n != -1) {
				buf[i] = (byte) n;
				i++;
			}
		}

		// return null;
		return new SPacket_sensor(buf, MAX_PKT_SIZE);
	}

	public byte[] getPacketBytes() {
		return pkt_bytes;
	}

	public int getSize() {
		return pkt_size;
	}

	// public String toBytes() {
	// String s = "";
	//
	// for (int i = 0; i < pkt_size; i++) {
	// int v = pkt_bytes[i] & 0xFF;
	// if (v < 16) {
	// s += '0';
	// }
	// s += Integer.toHexString(v);
	// }
	//
	// return s.toUpperCase();
	// }

	public String toString() {
		return new String(pkt_bytes);
	}
}
