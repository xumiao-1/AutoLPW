package auto.func;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.Scanner;

// A configuration DB:
// store all possible configuration profiles
public class ConfigDB {

	private static int _cursor = 0;
	private static ArrayList<Profile> _profiles = new ArrayList<Profile>();

	// read pin config file
	public static int read_cfg_db(String fname) {
		int n = 0;

		try {
			Scanner scan = new Scanner(new File(fname));

			while (scan.hasNextLine()) {
				// read a line
				String line = scan.nextLine().trim();

				// parse the line, get a profile,
				// and then add to the DB
				_profiles.add(new Profile(line.toUpperCase()));

				n++;
			}

			scan.close();
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		return n;
	}

	// read measurement file
	public static LPWTest read_pwr_db(String fname) {
		LPWTest tbl = new LPWTest();
		Scanner scan = FileUtils.ScannerOpen(fname);

		while (scan.hasNextLine()) {
			// read a line
			String line = scan.nextLine().trim();
			
			if (line.startsWith("#")) {
				if (line.contains("INIT PROFILE")) {
					int iStart = line.indexOf("(");
					int iEnd = line.indexOf(")");
					String substr = line.substring(iStart+1, iEnd).trim();
					String[] ss = substr.split("\\s+");
					if (ss.length == Profile.PIN_TOTAL_NUM) {
						int[] mode = new int[Profile.PIN_TOTAL_NUM];
						for (int i = 0; i < Profile.PIN_TOTAL_NUM; i++) {
							mode[i] = Integer.parseInt(ss[i]);
						}
						_profiles.add(new Profile(mode));
					}
				}
			} else {
				String[] ss = line.split("\\s+");
				if (ss.length == Profile.PIN_TOTAL_NUM + 1) {
					int[] mode = new int[Profile.PIN_TOTAL_NUM];
					for (int i = 0; i < Profile.PIN_TOTAL_NUM; i++) {
						mode[i] = Integer.parseInt(ss[i]);
					}
					Profile pro = new Profile(mode);
					pro.setPower(Double.parseDouble(ss[Profile.PIN_TOTAL_NUM]));
					String key = pro.profile2Key();
					tbl.addMeassure(key, pro);
				}
			}
		}

		scan.close();
		return tbl;
	}

	public static boolean hasNext() {
		return _cursor < _profiles.size();
	}

	public static Profile next() {
		Profile p = _profiles.get(_cursor);
		_cursor++;
		return p;
	}

	public static int getCursor() {
		return _cursor;
	}
}
