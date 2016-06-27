package jmetal.experiments.studies;

import java.util.Scanner;

import jmetal.core.Algorithm;
import jmetal.core.SolutionSet;
import jmetal.experiments.Settings;
import jmetal.experiments.settings.NSGAIIInt_Settings;
import jmetal.problems.Myprob;
import jmetal.util.JMException;
import auto.func.DAQMeasure;
import auto.func.FileUtils;
import auto.func.Oscilloscope;
import auto.func.PinConfig;
import auto.func.Profile;

public class mytest implements Runnable {

	public mytest(int mtime) {
		DAQMeasure.getInstance().setMeasureTime(mtime);
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

			// HashMap parameter = new HashMap();// operator
			// parameter.put("mutationProbability_", MUTATION);

			// setup the problem
			Myprob problem = new Myprob("Int", Profile.PIN_TOTAL_NUM);
			// for some known status pin
			Scanner scan = FileUtils.ScannerOpen("const.txt");
			if (scan != null) {
				while (scan.hasNextLine()) {
					String line = scan.nextLine().trim();
					if (!line.equals("")) {
						PinConfig pin = new PinConfig(line);
						problem.setConstant(pin);
					}
				}
				scan.close();
			}

			// init solution set
			problem.createInitSet(Oscilloscope.tblPwr.getMap());

			// algorithm settings
			NSGAIIInt_Settings mysetting = new NSGAIIInt_Settings("Myprob");
			System.out.print("Pop size: ");
			s = keyboard.nextLine();
			mysetting.setPopsize(Integer.parseInt(s));
			mysetting.setProblem(problem);
			Algorithm nsgaII = mysetting.configure(/* parameter */);

			// GA execution
			SolutionSet resultPop = nsgaII.execute();

			// print, save result
			resultPop.printVariablesToFile("VAR");
			resultPop.printObjectivesToFile("FUN");
			System.out.println("GA Done!");

			FileUtils.logFile.flush();
			FileUtils.outFile.flush();
			FileUtils.closeOutFile();

			DAQMeasure.getInstance().closePort();

		} catch (ClassNotFoundException e) {
			System.out.println("ClassNotFoundException");
		} catch (JMException e) {
			System.out.println("JMException");
		}
	}
}
