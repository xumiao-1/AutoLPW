package auto.func;

import java.util.LinkedHashMap;

public class LPWTest {

	private LinkedHashMap<String, Profile> tblPower;

	public LPWTest() {
		tblPower = new LinkedHashMap<String, Profile>();
	}

	// add a <key, power> to the hash table
	public void addMeassure(String key, Profile pro) {
		if (!tblPower.containsKey(key)
				|| tblPower.get(key).getPower() > pro.getPower()) {
			tblPower.put(key, pro);
		}
	}

	// get power
	public Double getPower(String key) {
		Profile pro = tblPower.get(key);
		if (pro != null)
			return pro.getPower();

		return null;
	}

	// get the hash table
	public LinkedHashMap<String, Profile> getMap() {
		return tblPower;
	}

	// reset the table
	public void reset() {
		tblPower.clear();
	}
}
