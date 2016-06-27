package auto.func;

import java.util.Random;
import java.util.Scanner;


public class GreedyTest implements Runnable {
	private Profile _bestPro = null;
	private boolean _bConst[];
	private int _constMode[];

	public GreedyTest(int mtime) {
		DAQMeasure.getInstance().setMeasureTime(mtime);
		_bConst = new boolean[Profile.PIN_TOTAL_NUM];
		_constMode = new int[Profile.PIN_TOTAL_NUM];
	}

	@Override
	public void run() {
		try {

			DAQMeasure.getInstance().openPort();

			Scanner keyboard = new Scanner(System.in);
			System.out.print("Output file name: ");
			String s = keyboard.nextLine();
			FileUtils.setOutFile(s + ".txt");
			FileUtils.logFile.println("\r\n####" + new java.util.Date());
			FileUtils.logFile.flush();

			// for some known status pin
			Scanner scan = FileUtils.ScannerOpen("const.txt");
			if (scan != null) {
				while (scan.hasNextLine()) {
					String line = scan.nextLine().trim();
					if (!line.equals("")) {
						PinConfig pin = new PinConfig(line);
						setConstant(pin);
					}
				}
				scan.close();
			}

			// execute greedy algorithm
			execute();
			System.out.println("\r\nBest solution:\r\n" + _bestPro.toString());
			System.out.println("Min power: " + _bestPro.getPower());

			// print, save result
			System.out.println("\r\nGreedy Algorithm Done!");

			FileUtils.logFile.flush();
			FileUtils.outFile.flush();
			FileUtils.closeOutFile();

			keyboard.close();
			DAQMeasure.getInstance().closePort();

		} catch (Exception e) {
			System.out.println("ClassNotFoundException");
		}
	}
	
	public void execute() {
		// init profile
		_bestPro = createInitProfile();
		Double minPower = _bestPro.getPower();
		
		for (int iPin = 0; iPin < Profile.PIN_TOTAL_NUM; iPin++) {
			int minMode = -1;
			Profile pro = new Profile(_bestPro);
			
			FileUtils.outFile.println("#********************");
			FileUtils.outFile.println("#BEGIN:: ROUND " + (iPin+1));
			
			for (int cfg = 0; cfg < PinConfig.OPT.length; cfg++) {
				// form a profile, and get the key
				if (!_bConst[iPin]) {
					pro.getPin(iPin).setOpt(cfg);
				}
				String key = pro.profile2Key();
				
				try {
					// then, get the power
					Double power = Oscilloscope.tblPwr.getPower(key);
					if (power == null) {
						// no value in the hash table,
						// so start measurement using DAQ,
						// then add the measurement into table
						power = DAQMeasure.getInstance().measure(pro);
						pro.setPower(power);
						Oscilloscope.tblPwr.addMeassure(key, pro);
					} else {
						pro.setPower(power);
					}
					
					FileUtils.outFile.println(profileModes(pro) + pro.getPower());
					FileUtils.outFile.flush();
					
					if (power < minPower) {
						minPower = power;
						minMode = cfg;
					}
				} catch (Exception e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
			
			if (minMode != -1) {
				_bestPro.getPin(iPin).setOpt(minMode);
				_bestPro.setPower(minPower);
			}
			
			FileUtils.outFile.println("#LOWEST POWER: " + _bestPro.getPower() + " ( " + profileModes(_bestPro) + ")");
			FileUtils.outFile.println("#" + _bestPro.toString());
			FileUtils.outFile.println("#END:: ROUND " + (iPin+1));
			FileUtils.outFile.flush();
		}
	}
	
	public String profileModes(Profile pro) {
		String str = "";
		for (int j = 0; j < Profile.PIN_TOTAL_NUM; j++) {
			str += pro.getPin(j).getOpt() + " ";
		}
		return str;
	}
	
	/**
	 * Create initial profile randomly
	 * @return Initial profile
	 */
	public Profile createInitProfile() {
		Profile pro = null;
		
		if (ConfigDB.hasNext()) {
			pro = new Profile(ConfigDB.next());
			pro.setPower(Double.MAX_VALUE);
			
		} else {
			int[] mode = new int[Profile.PIN_TOTAL_NUM];
			for (int k = 0; k < Profile.PIN_TOTAL_NUM; k++) {
				if (_bConst[k]) {
					mode[k] = _constMode[k];
				} else {
					mode[k] = (new Random()).nextInt(PinConfig.OPT.length);
				}
			}
			
			pro = new Profile(mode);
			pro.setPower(Double.MAX_VALUE);
		}
		
		FileUtils.outFile.println("#********************");
		FileUtils.outFile.println("#INIT PROFILE ( " + profileModes(pro) + ")");
		FileUtils.outFile.println(pro.toString());
		FileUtils.outFile.flush();
		
		return pro;
	}
	
	// if we know how to configure this pin exactly
	public void setConstant(PinConfig pin) {
		// change specified pin configuration
		int idx = Profile.PIN_PER_GRP * pin.getGrp() + pin.getPin();
		_bConst[idx] = true;
		_constMode[idx] = pin.getOpt();
	}
}
