package auto.func;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map.Entry;

public class FileM {
	LinkedHashMap<Integer, FileOutputStream> handles; // id -> file handler
	LinkedHashMap<Integer, Integer> indexes; // id -> index
//	PrintWriter logOut; // print to log file

	public FileM() throws FileNotFoundException {
		handles = new LinkedHashMap<Integer, FileOutputStream>();
		indexes = new LinkedHashMap<Integer, Integer>();
//		logOut = new PrintWriter("log.txt");
	}

	/**
	 * 
	 * @param pkt
	 * @return index of the file handle
	 * @throws IOException
	 */
	public int writePacket(SPacket_host pkt) throws IOException {
		int nodeid = pkt.getPacketSrc();

		FileOutputStream h = handles.get(nodeid);

		if (h == null)
			h = createFile(nodeid);

		writeFile(h, pkt);

		return indexes.get(nodeid);
	}

	public void openFiles() throws FileNotFoundException {
		Iterator<Integer> iter = handles.keySet().iterator();
		while (iter.hasNext()) {
			openFileAppend(iter.next(), true); // append to existing file
		}
	}

	public void closeFiles() throws IOException {
		Iterator<Entry<Integer, FileOutputStream>> iter = handles.entrySet()
				.iterator();
		while (iter.hasNext()) {
			Entry<Integer, FileOutputStream> e = iter.next();
			FileOutputStream h = e.getValue();
			h.close();
		}
//		logOut.close();
	}

	// ================
	// private methods
	// ================
	/**
	 * 
	 * @param nodeid
	 * @param append
	 *            Create a new file if false. Otherwise append to existing file.
	 * @return
	 * @throws FileNotFoundException
	 */
	private FileOutputStream openFileAppend(int nodeid, boolean append)
			throws FileNotFoundException {
		FileOutputStream h = new FileOutputStream(
				String.format("%04X", nodeid), append);
		handles.put(nodeid, h);
		return h;
	}

	private FileOutputStream createFile(int nodeid)
			throws FileNotFoundException {
		FileOutputStream h = openFileAppend(nodeid, true/*false*/); // create new file
		indexes.put(nodeid, handles.size() - 1);
		return h;
	}

	private void writeFile(FileOutputStream out, SPacket_host pkt)
			throws IOException {
		// write packet data
		out.write(pkt.getPacketBytes(), 0, SPacket_host.PKT_SIZE);
		out.flush();

		// write log data
//		writeLog(pkt);
	}

//	private void writeLog(SPacket_host pkt) throws IOException {
//		String log = LogParser.parse(pkt);
//		// if (!log.equals(""))
////		logOut.print(log);
////		logOut.flush();
//	}
}

class LogParser {

	public static final int L_ADD_N = 0x01;
	public static final int L_DEL_N = 0x02;
	public static final int L_REP_P = 0x04;
	public static final int L_SYNC = 0x08;
	public static final int L_NO_NB = 0x10;
	// public static final int L_IN_WT = 6;
	// public static final int L_OUT_WT =7;

	private static final String[] info = { "Unknown", "Add neighbor",
			"Delete neighbor", "Replace parent", "Sync with parent",
			"Empty neighbor",
	// "Go under water",
	// "Go above water"
	};

	public static String parse(SPacket_host pkt) {
		String s = "";

		int log = pkt.getLog();
		if (log != 0) {
			int flag = 0x01;
			for (int i = 1; i <= info.length; i++) {
				if ((log & flag) == flag) {
					s += info[i] + ", ";
				}
				flag = flag << 1;
			}
			// s = String.format("%04X @ %s : %s\n", pkt.getPacketSrc(),
			// pkt.getTimestamp(), info[log]);
		}

		return s;
	}
}
