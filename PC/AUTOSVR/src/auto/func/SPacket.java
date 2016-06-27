package auto.func;

public abstract class SPacket {

	public static String toByteString(byte[] val) {
		String s = "";

		for (int i = 0; i < val.length; i++) {
			int v = val[i] & 0xFF;
			if (v < 16) {
				s += '0';
			}
			s += Integer.toHexString(v);
		}

		return s.toUpperCase();
	}

	public String toByteString() {
		String s = "";

		for (int i = 0; i < pkt_bytes.length; i++) {
			int v = pkt_bytes[i] & 0xFF;
			if (v < 16) {
				s += '0';
			}
			s += Integer.toHexString(v);
		}

		return s.toUpperCase();
	}

	byte[] pkt_bytes;

}
