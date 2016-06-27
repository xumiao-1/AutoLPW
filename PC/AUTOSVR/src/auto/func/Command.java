package auto.func;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.Date;

public class Command {

	private OutputStream outputStream;

	// public Command(OutputStream os) {
	// outputStream = os;
	// }

	// ==================
	// public methods
	// ==================
	public void setOutputStream(OutputStream os) {
		outputStream = os;
	}

	// stime: sensor's sleep time
	public boolean sendSync(int stime) {
		// // wakeup cmd
		// pc_send_cmd_wakeup();

		// wait for a while
		try {
			// Thread.sleep(500);
			pc_send_cmd_sync(stime);
		} catch (Exception exp) {
			exp.printStackTrace();
			return false;
		}

		return true;
	}

	public boolean sendUpload() {
		// wakeup cmd
		pc_send_cmd_wakeup();

		// wait for a while
		try {
			Thread.sleep(500);
			pc_send_cmd_upload(); // upload cmd
		} catch (Exception exp) {
			exp.printStackTrace();
			return false;
		}

		return true;
	}

	public boolean sendDump() {
		// wakeup cmd
		pc_send_cmd_wakeup();

		// wait for a while
		try {
			Thread.sleep(500);
			pc_send_cmd_dump();
		} catch (Exception exp) {
			exp.printStackTrace();
			return false;
		}

		return true;
	}

	public boolean sendPeriod(int interval, int duration) {
		// wakeup cmd
		pc_send_cmd_wakeup();

		// wait for a while
		try {
			Thread.sleep(500);
			pc_send_cmd_period(interval, duration);
		} catch (Exception exp) {
			exp.printStackTrace();
			return false;
		}

		return true;
	}

	/**
	 * Update sensor's image
	 * 
	 * @param name
	 * @return
	 */
	// public boolean sendUpdate(File f, int checksum) {
	// // wakeup cmd
	// pc_send_cmd_wakeup();
	//
	// // wait for a while
	// try {
	// Thread.sleep(500);
	//
	// // send start cmd and file length
	// // File f = new File(name);
	// // int checksum = calc_checksum(f);
	// pc_send_cmd_reprog_start((int) (f.length()), checksum);
	//
	// Thread.sleep(5000);
	//
	// // now send file
	// int pgno = 0;
	// int seqno = 0;
	// byte[] buf = new byte[64];
	// FileInputStream fis = new FileInputStream(f);
	// // int nTotal = 0;
	// while (fis.available() > 0) {
	// int nread = fis.read(buf);
	// pc_send_cmd_reprog_data(pgno, seqno, buf, nread);
	//
	// seqno++;
	// if (seqno >= (256 / 64)) {
	// pgno++;
	// seqno = 0;
	// }
	// Thread.sleep(500);
	// }
	// fis.close();
	//
	// // send end cmd and file length
	// pc_send_cmd_reprog_end((int) (f.length()), checksum);
	//
	// } catch (Exception exp) {
	// exp.printStackTrace();
	// return false;
	// }
	// return true;
	// }

	// turn off the node
	public boolean sendShut(String id) {
		// wakeup cmd
		pc_send_cmd_wakeup();

		// wait for a while
		try {
			Thread.sleep(500);
			pc_send_cmd_shut(id);
		} catch (Exception exp) {
			exp.printStackTrace();
			return false;
		}

		return true;
	}

	// restart the node for updating image
	public boolean sendUpdateRestart(String id) {
		// wakeup cmd
		pc_send_cmd_wakeup();

		// wait for a while
		try {
			Thread.sleep(500);
			pc_send_cmd_update_restart(id);
		} catch (Exception exp) {
			exp.printStackTrace();
			return false;
		}

		return true;
	}

	// ==================
	// private methods
	// ==================
	private void pc_send_cmd_wakeup() {
		byte[] val = new byte[SPacket_pc.PKT_SIZE];
		for (int i = 0; i < SPacket_pc.PKT_SIZE; i++)
			val[i] = (byte) (0xA5 & 0x00FF);

		try {
			outputStream.write(val);
		} catch (Exception exp) {
		}
	}

	private void pc_send_cmd_sync(int stime) {
		byte[] val = new byte[SPacket_pc.PKT_SIZE];
		val[0] = (byte) 0x00; // cmd: time sync
		val[1] = (byte) (stime & 0x00FF);
		val[2] = (byte) ((stime >> 8) & 0x00FF);
		val[3] = (byte) 0x0;
		val[4] = (byte) 0x0;
		try {
			outputStream.write(val);
		} catch (Exception exp) {
		}
	}

	private void pc_send_cmd_upload() {
		byte[] val = new byte[SPacket_pc.PKT_SIZE];
		val[0] = 0x02; // data upload

		try {
			outputStream.write(val);
		} catch (Exception exp) {
		}
		;
	}

	private void pc_send_cmd_dump() {
		byte[] val = new byte[SPacket_pc.PKT_SIZE];
		val[0] = 0x03; // data dump

		try {
			outputStream.write(val);
		} catch (Exception exp) {
		}
		;
	}

	private void pc_send_cmd_period(int interval, int duration) {
		byte[] val = new byte[SPacket_pc.PKT_SIZE];

		val[0] = 0x04; // change period
		// interval
		val[1] = (byte) (interval & 0x00FF);
		val[2] = (byte) ((interval >> 8) & 0x00FF);
		// duration
		val[3] = (byte) (duration & 0x00FF);
		val[4] = (byte) ((duration >> 8) & 0x00FF);

		try {
			outputStream.write(val);
		} catch (Exception exp) {
		}
		;
	}

	private void pc_send_cmd_reprog_start(
	/* Oscilloscope.BinType type, */int size, int checksum) {
		byte[] val = new byte[SPacket_pc.PKT_SIZE];

		// if (type == Oscilloscope.BinType.HOST)
		// val[0] = S_CMD_UPDH;
		// else if (type == Oscilloscope.BinType.SENSOR)
		// val[0] = S_CMD_UPDS;
		val[0] = 0x07;
		val[1] = (byte) (size & 0x00FF);
		val[2] = (byte) ((size >> 8) & 0x00FF);
		val[3] = (byte) (checksum & 0x00FF);// (byte)((size >> 16) & 0x00FF);
		val[4] = (byte) ((checksum >> 8) & 0x00FF);// (byte)((size >> 24) &
													// 0x00FF);
		try {
			outputStream.write(val);
		} catch (Exception exp) {
		}
		;
	}

	private void pc_send_cmd_reprog_end(int size, int checksum) {
		byte[] val = new byte[SPacket_pc.PKT_SIZE];

		val[0] = 0x08;
		val[1] = (byte) (size & 0x00FF);
		val[2] = (byte) ((size >> 8) & 0x00FF);
		val[3] = (byte) (checksum & 0x00FF);// (byte)((size >> 16) & 0x00FF);
		val[4] = (byte) ((checksum >> 8) & 0x00FF);// (byte)((size >> 24) &
													// 0x00FF);
		try {
			outputStream.write(val);
		} catch (Exception exp) {
		}
		;
	}

	// private void pc_send_cmd_reprog_data(int pgno, int seqno, byte[] buf,
	// int len) {
	// byte[] val = new byte[SPacket_img.PKT_SIZE];
	//
	// try {
	// val[0] = 0x09;
	// val[1] = (byte) pgno;
	// val[2] = (byte) seqno;
	// System.arraycopy(buf, 0, val, 3, len);
	// outputStream.write(val);
	// } catch (IOException e) {
	// }
	// }

	private void pc_send_cmd_shut(String id) {
		byte[] val = new byte[SPacket_pc.PKT_SIZE];
		val[0] = (byte) 0x06; // shutdown cmd
		val[1] = (byte) ((Character.digit(id.charAt(2), 16) << 4) + Character
				.digit(id.charAt(3), 16)); // to HEX
		val[2] = (byte) ((Character.digit(id.charAt(0), 16) << 4) + Character
				.digit(id.charAt(1), 16)); // to HEX

		// String s = "";
		// for (int i = 0; i < SPacket_pc.PKT_SIZE; i++) {
		// int v = val[i] & 0xFF;
		// if (v < 16) {
		// s += '0';
		// }
		// s += Integer.toHexString(v);
		// }
		// System.out.println(s.toUpperCase());

		try {
			outputStream.write(val);
		} catch (Exception exp) {
		}
	}

	private void pc_send_cmd_update_restart(String id) {
		byte[] val = new byte[SPacket_pc.PKT_SIZE];
		val[0] = (byte) 0x0A; // S_CMD_RESTART_REPROG
		val[1] = (byte) ((Character.digit(id.charAt(2), 16) << 4) + Character
				.digit(id.charAt(3), 16)); // to HEX
		val[2] = (byte) ((Character.digit(id.charAt(0), 16) << 4) + Character
				.digit(id.charAt(1), 16)); // to HEX

		try {
			outputStream.write(val);
		} catch (Exception exp) {
		}
	}

	// //////////////////////////////////////////
	// for low power test
	public String send_to_sensor_pin_config() {
		String fname = "X";

		if (ConfigDB.hasNext()) {
			fname += Integer.toString(ConfigDB.getCursor() + 1);
			Profile pro = ConfigDB.next();
			try {
				byte[] val = new byte[1 + pro.size()];
				val[0] = 0x02; // PC_CMD_CFG
				System.arraycopy(pro.toBytes(), 0, val, 1, pro.size());
				// System.out.println(Profile.toBytes(val));
				outputStream.write(val);
			} catch (Exception exp) {
				System.out.println("Err: send_to_sensor_pin_config!");
			}
		}

		return fname;

		// if (!Oscilloscope._pin_config.isEmpty()) {
		// fname = Oscilloscope._pin_config.get(0).toUpperCase();
		// String[] s = fname.split(":", 4);
		//
		// // 4 bytes
		// byte[] val = new byte[4];
		// // group
		// val[0] = (byte) (s[0].charAt(1) - 'A');
		// // pin
		// val[1] = (byte) (Integer.parseInt(s[1]));
		// // speed
		// if (s[2].equals("400K")) {
		// val[2] = 0;
		// } else if (s[2].equals("2M")) {
		// val[2] = 1;
		// } else if (s[2].equals("10M")) {
		// val[2] = 2;
		// } else {
		// val[2] = 3;
		// }
		// // config
		// if (s[3].equals("IN:X:NOPULL")) {
		// val[3] = 0;
		// } else if (s[3].equals("IN:X:UP")) {
		// val[3] = 1;
		// } else if (s[3].equals("IN:X:DOWN")) {
		// val[3] = 2;
		// } else if (s[3].equals("AN:X:X")) {
		// val[3] = 3;
		// } else if (s[3].equals("OUT:OD:UP")) {
		// val[3] = 4;
		// } else if (s[3].equals("OUT:OD:DOWN")) {
		// val[3] = 5;
		// } else if (s[3].equals("OUT:PP:UP")) {
		// val[3] = 6;
		// } else {
		// val[3] = 7;
		// }
		//
		// try {
		// outputStream.write(val);
		// } catch (Exception exp) {
		// System.out.println("eeeeeeeeeeeee!");
		// }
		//
		// // System.out.println(Oscilloscope._pin_config.get(0));
		// Oscilloscope._pin_config.remove(0);
		// }
	}

	public String send_to_sensor_pin_config_1(Profile pro) {
		String fname = "X";

		byte[] val = new byte[1 + pro.size()];
		val[0] = 0x02; // PC_CMD_CFG
		System.arraycopy(pro.toBytes(), 0, val, 1, pro.size());
		try {
			outputStream.write(val);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		return fname;
	}
}
