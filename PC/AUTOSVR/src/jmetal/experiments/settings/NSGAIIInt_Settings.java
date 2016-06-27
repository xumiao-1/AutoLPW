package jmetal.experiments.settings;

import java.util.HashMap;

import jmetal.core.Algorithm;
import jmetal.core.Operator;
import jmetal.experiments.Settings;
import jmetal.metaheuristics.nsgaII.NSGAII;
import jmetal.operators.crossover.CrossoverFactory;
import jmetal.operators.mutation.MutationFactory;
import jmetal.operators.selection.SelectionFactory;
import jmetal.problems.ProblemFactory;
import jmetal.util.JMException;

public class NSGAIIInt_Settings extends Settings {

	int populationSize_;
	int maxEvaluations_;

	double mutationProbability_;
	double crossoverProbability_;

	/**
	 * Constructor
	 */
	public NSGAIIInt_Settings(String problem) {
		super(problem);

		Object[] problemParams = { "Int" };
		try {
			problem_ = (new ProblemFactory()).getProblem(problemName_,
					problemParams);
		} catch (JMException e) {
			e.printStackTrace();
		}

		// Default experiments.settings
		populationSize_ = 100;
		maxEvaluations_ = 2000;

		mutationProbability_ = 0.1;// 1.0 / problem_.getNumberOfBits();//
		crossoverProbability_ = 0.9;
	} // NSGAIIBinary_Settings

	@Override
	public Algorithm configure() throws JMException {
		// TODO Auto-generated method stub
		Algorithm algorithm;
		Operator selection;
		Operator crossover;
		Operator mutation;

		HashMap parameters; // Operator parameters

		// Creating the problem
		algorithm = new NSGAII(problem_);

		// Algorithm parameters
		algorithm.setInputParameter("populationSize", populationSize_);
		algorithm.setInputParameter("maxEvaluations", maxEvaluations_);

		// Mutation and Crossover Binary codification
		parameters = new HashMap();
		parameters.put("probability", crossoverProbability_);
		crossover = CrossoverFactory.getCrossoverOperator(
				"SinglePointCrossover", parameters);

		parameters = new HashMap();
		parameters.put("probability", mutationProbability_);
		mutation = MutationFactory.getMutationOperator("BitFlipMutation",
				parameters);

		// Selection Operator
		parameters = null;
		selection = SelectionFactory.getSelectionOperator("BinaryTournament2",
				parameters);

		// Add the operators to the algorithm
		algorithm.addOperator("crossover", crossover);
		algorithm.addOperator("mutation", mutation);
		algorithm.addOperator("selection", selection);

		return algorithm;
	}

	public void setPopsize(int size) {
		populationSize_ = size;
	}

}
