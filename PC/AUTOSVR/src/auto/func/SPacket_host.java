package auto.func;

import java.io.IOException;
import java.io.InputStream;
import java.util.Date;

// from keil
/**
 * typedef struct { uint16_t nodeid; uint8_t cmd; uint8_t reserved; }
 * pkt_app_header_t;
 * 
 * typedef struct { pkt_app_header_t hdr; uint8_t data[MAX_APPDATA_LENGTH]; }
 * pkt_app_t;
 * 
 * 
 * typedef struct { uint32_t _tid; // sample time uint16_t _tmpr; // temperature
 * // uint8_t _ed; // energy uint16_t _reserved; // reserved uint32_t trecv; //
 * received time at host } sensor_rpt_data_t;
 */
// class LQI {
// final static int NB_U = 0;
// final static int NB_P = 1;
// final static int NB_C = 2;
//
// final static int MAX_BEACONS = 5;
//
// int nodeid;
// int relation;
// double lqi;
//
// public String toString() {
// String ch = "";
// if (relation == NB_U)
// ch = "U";
// else if (relation == NB_P)
// ch = "P";
// else if (relation == NB_C)
// ch = "C";
//
// if (this.nodeid == 0)
// return "";
//
// return String.format("0x%04X(%.0f%%%s)", this.nodeid, 100 * this.lqi
// / MAX_BEACONS, ch);
// }
// }

public class SPacket_host extends SPacket {
	// packet header
	final static int ID_LEN = 2;
	final static int CMD_LEN = 1;
	final static int RSSI_LEN = 1;
	final static int HEADER_LEN = ID_LEN + CMD_LEN + RSSI_LEN;

	// packet data
	final static int DATA_LEN = 28;// 12;//60;//

	final static int PKT_SIZE = HEADER_LEN + DATA_LEN;

	// all pakcet in bytes format
	// byte[] pkt_bytes;

	// packet src
	int pkt_src;

	// host cmd
	int host_cmd;

	// rssi
	int rssi;

	// sample time by sensor
	Date ts;

	// tempr
	double tmpr;

	// // PDR
	// double pdr;

	// log
	int log;

	// single hop
	// boolean is_one_hop;
	int hops;

	// is under water
	boolean is_under_e; // electrode
	boolean is_under_c; // capacitor

	// PDR
	double pdr;

	// recv time by host
	Date trecv;

	// voltage read by water level detector
	// int voltage_e; // electrode
	int voltage_c; // capacitor
	double voltage_bat; // battery voltage

	// intermediate nodes
	int[] nodes_hopped;

	// // link quality
	// LQI[] lqi;

	// buffered packets
	int buf_pkts;

	// seq. #
	int seqno;

	// // report src (i.e., which sensor the sensed data come from)
	// int rpt_src;

	private SPacket_host(byte[] data) {
		pkt_bytes = new byte[PKT_SIZE];
		System.arraycopy(data, 0, pkt_bytes, 0, PKT_SIZE);

		pkt_src = ((pkt_bytes[ID_LEN - 1] & 0x00FF) << 8)
				^ (pkt_bytes[0] & 0x00FF);

		host_cmd = (pkt_bytes[CMD_LEN + ID_LEN - 1] & 0x00FF);// ((pkt_bytes[CMD_LEN+ID_LEN-1]
																// & 0x00FF) <<
																// 8);

		rssi = (pkt_bytes[RSSI_LEN + CMD_LEN + ID_LEN - 1] & 0x00FF);

		ts = new Date(
				1000L * (((pkt_bytes[HEADER_LEN + 3] & 0x00FF) << 24)
						^ ((pkt_bytes[HEADER_LEN + 2] & 0x00FF) << 16)
						^ ((pkt_bytes[HEADER_LEN + 1] & 0x00FF) << 8) ^ (pkt_bytes[HEADER_LEN] & 0x00FF)));

		// Calendar tc = Calendar.getInstance();
		// tc.setTime(ts);
		// tc.add(Calendar.HOUR_OF_DAY, 12);
		// ts = tc.getTime();

		tmpr = ((((pkt_bytes[HEADER_LEN + 5] & 0x00FF) << 28) >> 20) ^ (pkt_bytes[HEADER_LEN + 4] & 0x00FF)) / 16.0;

		// pdr = ((pkt_bytes[HEADER_LEN + 5] >> 12) & 0x0F) / 0x0F;

		log = pkt_bytes[HEADER_LEN + 6] & 0x00FF;

		// is_one_hop = (pkt_bytes[HEADER_LEN+7] & 0x01) == 0;
		hops = (pkt_bytes[HEADER_LEN + 7] & 0x03) + 1;

		is_under_e = (pkt_bytes[HEADER_LEN + 7] & 0x04) == 0x04;

		is_under_c = (pkt_bytes[HEADER_LEN + 7] & 0x08) == 0x08;

		pdr = 1.0 * ((pkt_bytes[HEADER_LEN + 7] >> 4) & 0x0F) / 0x0F;

		trecv = new Date(
				1000L * (((pkt_bytes[HEADER_LEN + 11] & 0x00FF) << 24)
						^ ((pkt_bytes[HEADER_LEN + 10] & 0x00FF) << 16)
						^ ((pkt_bytes[HEADER_LEN + 9] & 0x00FF) << 8) ^ (pkt_bytes[HEADER_LEN + 8] & 0x00FF)));
		// tc.setTime(trecv);
		// tc.add(Calendar.HOUR_OF_DAY, 12);
		// trecv = tc.getTime();

		// voltage_e = ((pkt_bytes[HEADER_LEN + 13] & 0x00FF) << 8)
		// ^ (pkt_bytes[HEADER_LEN + 12] & 0x00FF);
		voltage_c = ((pkt_bytes[HEADER_LEN + 13] & 0x00FF) << 8)
				^ (pkt_bytes[HEADER_LEN + 12] & 0x00FF);
		voltage_bat = (1.225 * 4095)
				/ (((pkt_bytes[HEADER_LEN + 15] & 0x00FF) << 8) ^ (pkt_bytes[HEADER_LEN + 14] & 0x00FF));

		nodes_hopped = new int[3];
		for (int i = 0; i < 3; i++) {
			nodes_hopped[i] = ((pkt_bytes[HEADER_LEN + 17 + 2 * i] & 0x00FF) << 8)
					^ (pkt_bytes[HEADER_LEN + 16 + 2 * i] & 0x00FF);
		}

		// lqi = new LQI[8];
		// for (int i = 0; i < 8; i++) {
		// lqi[i] = new LQI();
		// lqi[i].nodeid = ((pkt_bytes[HEADER_LEN + 29 + 3 * i] & 0x00FF) << 8)
		// ^ (pkt_bytes[HEADER_LEN + 28 + 3 * i] & 0x00FF);
		// lqi[i].relation = pkt_bytes[HEADER_LEN + 30 + 3 * i] & 0x03;
		// lqi[i].lqi = (pkt_bytes[HEADER_LEN + 30 + 3 * i] & 0x00FC) >> 2;
		// }

		buf_pkts = ((pkt_bytes[HEADER_LEN + 23] & 0x00FF) << 8)
				^ ((pkt_bytes[HEADER_LEN + 22] & 0x00FF));

		seqno = ((pkt_bytes[HEADER_LEN + 27] & 0x00FF) << 24)
				^ ((pkt_bytes[HEADER_LEN + 26] & 0x00FF) << 16)
				^ ((pkt_bytes[HEADER_LEN + 25] & 0x00FF) << 8)
				^ ((pkt_bytes[HEADER_LEN + 24] & 0x00FF));

		// rpt_src = pkt_src;//((pkt_bytes[HEADER_LEN+1] & 0x00FF) << 8) ^
		// (pkt_bytes[HEADER_LEN] & 0x00FF);
	}

	// get each component in packet
	public byte[] getPacketBytes() {
		return pkt_bytes;
	}

	public int getPacketSrc() {
		return pkt_src;
	}

	public int getHostCmd() {
		return host_cmd;
	}

	public int getRSSI() {
		return rssi;
	}

	public Date getTimestamp() {
		return ts;
	}

	public double getTmpr() {
		return tmpr;
	}

	public int getLog() {
		return log;
	}

	// public boolean isSingleHop() {
	// return is_one_hop;
	// }
	public int getHops() {
		return hops;
	}

	public boolean isUnderWater_E() {
		return is_under_e;
	}

	public boolean isUnderWater_C() {
		return is_under_c;
	}

	public double getPDR() {
		return pdr;
	}

	public Date getRecvTime() {
		return trecv;
	}

	// public int getVoltage_E() {
	// return voltage_e;
	// }

	public int getVoltage_C() {
		return voltage_c;
	}

	public double getVoltage_bat() {
		return voltage_bat;
	}

	public int getHoppedNode(int i) {
		return nodes_hopped[i];
	}

	// public LQI getLQI(int i) {
	// return lqi[i];
	// }

	public int getBufferedPkts() {
		return buf_pkts;
	}

	public int getSeqno() {
		return seqno;
	}

	// public int getReportSrc() {
	// return rpt_src;
	// }

	public static SPacket_host read(InputStream is) throws IOException {
		byte[] buf = new byte[SPacket_host.PKT_SIZE];

		// for (int i=0; i<SPacket_host.PKT_SIZE; i++) {
		// buf[i] = (byte) is.read();
		// }

		int i = 0;
		while (i < SPacket_host.PKT_SIZE) {
			int n = is.read();
			if (n != -1) {
				buf[i] = (byte) n;
				i++;
			}
		}

		return new SPacket_host(buf);
	}

	// public String toBytes() {
	// String s = "";
	//
	// for (int i = 0; i < SPacket_host.PKT_SIZE; i++) {
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
		String s = String.format("\tNode ID: 0x%04X\n", this.getPacketSrc());
		s += String.format("\tSeq No.:%5d\n", this.getSeqno());
		s += String.format("\tTemperature: %.2fC\n", this.getTmpr());
		s += String.format("\tMeasure Time: %s\n",
				Oscilloscope.dateFormat.format(this.getTimestamp()));
		s += String.format("\tReceive Time: %s\n",
				Oscilloscope.dateFormat.format(this.getRecvTime()));
		s += String.format("\tRSSI: 0x%02X\n", this.getRSSI());
		s += String.format("\tPDR: %.2f\n", this.getPDR());
		s += String.format("\tElectrode: %5s\n", this.isUnderWater_E());
		s += String.format("\tBattery %.1fV:\n", this.getVoltage_bat());

		// === hop info
		s += String.format("\tHops: %d (", this.getHops());
		for (int i = 0; i < this.getHops() - 1; i++) {
			s += String.format("0x%04X, ", this.getHoppedNode(i));
		}
		s += "Host)\n";

		s += String.format("\tEvent: %s\n", LogParser.parse(this));

		// buffered packets
		s += String.format("\tBuffered Pkts:%3d\n", this.getBufferedPkts());

		// // === link quality
		// s += String.format("\tLink quality: ");
		// for (int i = 0; i < 8; i++) {
		// s += String.format("%s ", this.lqi[i]);
		// }
		s += "\n";

		return s;
	}
}
