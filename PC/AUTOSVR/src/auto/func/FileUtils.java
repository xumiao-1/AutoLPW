package auto.func;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.util.Scanner;

/*********************************************************************
 * FileUtils class of static constants and basic file checking routines. This is
 * the header file of constants and methods that might be used in any and all
 * contexts. <br>
 * <br>
 * DUMMYVALUE is just to have a -1 dummy value defined as a constant<br>
 * DUMMYSTRING is just to have a dummy string value defined as a constant<br>
 * 
 * @author Duncan Buell
 **/
public class FileUtils {
	private static final String classLabel = "FileUtils: ";

	// public static Scanner inFile;
	public static PrintWriter outFile = null;
	public static PrintWriter logFile = null;

	/*********************************************************************
	 * Accessor methods
	 **/

	/*********************************************************************
	 * Mutator methods
	 **/
	/*********************************************************************
	 * Method to set the logFile given the name of the file.
	 * 
	 * @param logFileName
	 *            The 'String' name of the log file.
	 **/
	public static void setLogFile(String logFileName) {
		FileUtils.logFile = FileUtils.PrintWriterOpen(logFileName);
	}

	/*********************************************************************
	 * Method to set the logFile given the 'PrintWriter' file itself.
	 * 
	 * @param logFile
	 *            The 'PrintWriter' log file.
	 **/
	public static void setLogFile(PrintWriter logFile) {
		FileUtils.logFile = logFile;
	}

	/*********************************************************************
	 * Method to set the output file given the name of the file.
	 * 
	 * @param outFile
	 *            The 'PrintWriter' output file.
	 **/
	public static void setOutFile(PrintWriter outFile) {
		FileUtils.outFile = outFile;
	}

	/*********************************************************************
	 * Method to set the output file given the name of the file.
	 * 
	 * @param outFileName
	 *            The 'String' name of the output file.
	 **/
	public static void setOutFile(String outFileName) {
		FileUtils.outFile = FileUtils.PrintWriterOpen(outFileName);
	}

	/*********************************************************************
	 * General methods
	 **/
	/*********************************************************************
	 * Method to check the number of arguments to the main.
	 * 
	 * @param countArgs
	 *            the number of required arguments
	 * @param args
	 *            the argument list at the time the class is invoked
	 * @param usage
	 *            descriptions of the required arguments
	 **/
	public static void checkArgs(int countArgs, String[] args, String usage) {
		String message;
		if (countArgs > args.length) {
			if (1 == countArgs) {
				message = "ERROR: required " + countArgs + " (" + usage
						+ ") arg not present";
			} else {
				message = "ERROR: required " + countArgs + " (" + usage
						+ ") args not present";
			}
			FileUtils.logFile.printf("%s%n", message);
			FileUtils.logFile.flush();
			System.exit(1);
		}
	} // public static void checkArgs(int countArgs,String [] args,String usage)

	/*********************************************************************
	 * Method to close a PrintWriter file.
	 * 
	 * @param theFile
	 *            The 'PrintWriter' file to close.
	 **/
	public static void closeFile(PrintWriter theFile) {
		// FileUtils.logFile.printf("%s enter (PrintWriter) CloseFile%n",classLabel);
		theFile.flush();
		theFile.close();
		// FileUtils.logFile.printf("%s leave (PrintWriter) CloseFile%n",classLabel);
	} // public static void CloseFile(PrintWriter theFile)

	/*********************************************************************
	 * Method to close the PrintWriter class log file.
	 **/
	public static void closeLogFile() {
		// FileUtils.logFile.printf("%s enter (PrintWriter) CloseLogFile%n",classLabel);
		FileUtils.logFile.flush();
		FileUtils.logFile.close();
		// FileUtils.logFile.printf("%s leave (PrintWriter) CloseLogFile%n",classLabel);
	} // public static void CloseLogFile()

	/*********************************************************************
	 * Method to close the PrintWriter class output file.
	 **/
	public static void closeOutFile() {
		// FileUtils.logFile.printf("%s enter (PrintWriter) CloseOutFile%n",classLabel);
		FileUtils.outFile.flush();
		FileUtils.outFile.close();
		// FileUtils.logFile.printf("%s leave (PrintWriter) CloseOutFile%n",classLabel);
	} // public static void CloseOutFile()

	/*********************************************************************
	 * Method to close a Scanner file.
	 * 
	 * @param theFile
	 *            The 'Scanner' file to close.
	 **/
	public static void closeFile(Scanner theFile) {
		// FileUtils.logFile.printf("%s enter (Scanner) CloseFile%n",classLabel);
		theFile.close();
		// FileUtils.logFile.printf("%s leave (Scanner) CloseFile%n",classLabel);
	} // public static void CloseFile(Scanner theFile)

	/*********************************************************************
	 * PrintWriterOpen method to open a file as a PrintWriter.
	 * 
	 * The main purpose of this method is to do the error checking in a
	 * subordinate method so as not to clutter up the code flow in methods that
	 * have to open files.
	 * 
	 * @param outFileName
	 *            the 'String' name of the file to open
	 * @return The opened 'PrintWriter' known to be not null
	 **/
	public static PrintWriter PrintWriterOpen(String outFileName) {
		PrintWriter localPrintWriter = null;

		// FileUtils.logFile.printf("%s enter PrintWriterOpen%n",classLabel);

		if (outFileName.equals("System.out")) {
			localPrintWriter = new PrintWriter(System.out);
		} else {
			try {
				localPrintWriter = new PrintWriter(new File(outFileName));
			} catch (FileNotFoundException ex) {
				FileUtils.logFile.println(classLabel + "ERROR opening outFile "
						+ outFileName);
				FileUtils.logFile.println(ex.getMessage());
				FileUtils.logFile
						.println("in" + System.getProperty("user.dir"));
				FileUtils.logFile.flush();
				System.exit(1);
			}
		}

		// FileUtils.logFile.printf("%s leave PrintWriterOpen%n",classLabel);
		return localPrintWriter;
	} // public static PrintWriter PrintWriterOpen(String outFileName)

	/*********************************************************************
	 * ScannerOpen method to open a file as a Scanner.
	 * 
	 * The main purpose of this method is to do the error checking in a
	 * subordinate method so as not to clutter up the code flow in methods that
	 * have to open files.
	 * 
	 * @param inFileName
	 *            the 'String' name of the file to open
	 * @return The opened 'Scanner' known to be not null
	 **/
	public static Scanner ScannerOpen(String inFileName) {
		Scanner localScanner = null;

		// FileUtils.logFile.printf("%s enter ScannerOpen%n",classLabel);
		if (inFileName.equals("System.in")) {
			localScanner = new Scanner(System.in);
		} else {
			try {
				localScanner = new Scanner(new File(inFileName));
			} catch (FileNotFoundException ex) {
//				FileUtils.logFile.println("classLabel + ERROR opening inFile "
//						+ inFileName);
//				FileUtils.logFile.println(ex.getMessage());
//				FileUtils.logFile
//						.println("in" + System.getProperty("user.dir"));
//				FileUtils.logFile.flush();
//				System.exit(1);
			}
		}

		// FileUtils.logFile.printf("%s leave ScannerOpen%n",classLabel);
		return localScanner;
	} // public static Scanner ScannerOpen(String inFileName)

} // public class FileUtils
