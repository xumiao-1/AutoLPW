package jmetal.problems;

import java.util.HashMap;
import java.util.Iterator;

import jmetal.core.Problem;
import jmetal.core.Solution;
import jmetal.core.SolutionSet;
import jmetal.encodings.solutionType.BinarySolutionType;
import jmetal.encodings.solutionType.IntSolutionType;
import jmetal.encodings.variable.Int;
import jmetal.util.Distance;
import jmetal.util.JMException;
import jmetal.util.Ranking;
import jmetal.util.wrapper.XInt;
import auto.func.DAQMeasure;
import auto.func.Oscilloscope;
import auto.func.PinConfig;
import auto.func.Profile;

public class Myprob extends Problem {

	public Myprob(String solutionType) throws ClassNotFoundException {
		this(solutionType, 1);
	}

	public Myprob(String solutionType, Integer numberOfVariables) {
		numberOfVariables_ = numberOfVariables;
		numberOfObjectives_ = 1;
		numberOfConstraints_ = 0;
		problemName_ = "Myprob";

		upperLimit_ = new double[numberOfVariables_];
		lowerLimit_ = new double[numberOfVariables_];

		// setup upper & lower limit
		for (int i = 0; i < numberOfVariables_; i++) {
			lowerLimit_[i] = 0;
			upperLimit_[i] = PinConfig.OPT.length - 1;
		} // for

		// // setup the length of every encodings.variable
		// length_ = new int[numberOfVariables_];
		// for (int var = 0; var < numberOfVariables_; var++) {
		// length_[var] = 5; // 5bits per variable
		// } // for

		if (solutionType.compareTo("Int") == 0)
			solutionType_ = new IntSolutionType(this);
		else if (solutionType.compareTo("Binary") == 0) {
			solutionType_ = new BinarySolutionType(this);
		} else {
			System.out.println("Error: solution type " + solutionType
					+ " invalid");
			System.exit(-1);
		}
	}

	@Override
	public void evaluate(Solution solution) throws JMException {
		XInt vars = new XInt(solution);

		int[] x = new int[numberOfVariables_];
		for (int i = 0; i < numberOfVariables_; i++)
			x[i] = vars.getValue(i);

		// form a profile, and get the key
		Profile pro = new Profile(x);
		String key = pro.profile2Key();
		// then, get the power
		Double power = Oscilloscope.tblPwr.getPower(key);
		while (power == null) {
			// no value in the hash table,
			// so start measurement using DAQ,
			// then add the measurement into table
			try {
				power = DAQMeasure.getInstance().measure(pro);
				pro.setPower(power);
				Oscilloscope.tblPwr.addMeassure(key, pro);
			} catch (Exception e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}

		solution.setObjective(0, power);
	}

	// if we know how to configure this pin exactly
	public void setConstant(PinConfig pin) {
		// change specified pin configuration
		int idx = Profile.PIN_PER_GRP * pin.getGrp() + pin.getPin();
		lowerLimit_[idx] = upperLimit_[idx] = pin.getOpt();
	}

	// if we have initial solution set
	public void createInitSet(HashMap<String, Profile> db) {
		// create an initial set from hash map
		_initSet = new SolutionSet(db.size());

		Iterator<String> iter = db.keySet().iterator();
		while (iter.hasNext()) {
			String key = iter.next();
			_initSet.add(createSol(db.get(key)));
		}

		// ranking ...
		Ranking ranking = new Ranking(_initSet);
		_initSet.clear();
		Distance distance = new Distance();
		for (int i = 0; i < ranking.getNumberOfSubfronts(); i++) {
			SolutionSet front = ranking.getSubfront(i);
			// Assign crowding distance to individuals
			distance.crowdingDistanceAssignment(front, getNumberOfObjectives());
			// Add the individuals of this front
			for (int k = 0; k < front.size(); k++) {
				_initSet.add(front.get(k));
			} // for
		}

		_initSet.printVariablesToFile("VARR");
		_initSet.printObjectivesToFile("FUNN");
	}

	// Solution = Profile + Power
	public Solution createSol(Profile pro) {
		Int[] vars = new Int[Profile.PIN_TOTAL_NUM];

		for (int k = 0; k < Profile.PIN_TOTAL_NUM; k++) {
			vars[k] = new Int(pro.getPin(k).getOpt(), 0,
					PinConfig.OPT.length - 1);
		}

		Solution sol = new Solution(this, vars);
		Double power = pro.getPower();
		if (power != null)
			sol.setObjective(0, power);
		else
			sol.setObjective(0, -1);
		return sol;
	}
}
