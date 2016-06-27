package auto.func;

public class SPacket_pc {
	static final int CMD_LEN = 1;// 4;
	static final int DATA_LEN = 4;
	static final int PKT_SIZE = CMD_LEN + DATA_LEN;

	byte[] _cmd;
	byte[] _data;

	public SPacket_pc(byte[] val) {
		_cmd = new byte[CMD_LEN];
		_data = new byte[DATA_LEN];

		System.arraycopy(val, 0, _cmd, 0, CMD_LEN);
		System.arraycopy(val, CMD_LEN, _data, 0, DATA_LEN);
	}
}
