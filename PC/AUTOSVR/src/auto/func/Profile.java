package auto.func;

//configuration profile (all pins)
public class Profile {

	public final static int PIN_PER_GRP = 16;
	public final static int PIN_TOTAL_NUM = 32;

	int pin_num;
	int _size; // size in bytes of the profile
	Double _power; // power of this profile

	PinConfig[] pin_cfgs;
	
	public Profile(Profile other) {
		_size = other._size;
		_power = other._power;
		pin_num = other.pin_num;
		pin_cfgs = new PinConfig[pin_num];
		
		for (int i = 0; i< pin_num; i++) {
			pin_cfgs[i] = new PinConfig(other.getPin(i));
		}
	}

	public Profile(String line) {

		pin_num = PIN_TOTAL_NUM;// subs.length;
		_size = 1 + pin_num * PinConfig.PIN_CFG_SIZE;
		pin_cfgs = new PinConfig[pin_num];

		// init
		for (int i = 0; i < pin_num; i++) {
			pin_cfgs[i] = new PinConfig(i / PIN_PER_GRP, i % PIN_PER_GRP, 0);
		}

		// change specified pin config
		String[] ss = line.split("\\|");
		for (int i = 0; i < ss.length; i++) {
			PinConfig pin = new PinConfig(ss[i]);
			pin_cfgs[PIN_PER_GRP * pin.getGrp() + pin.getPin()] = pin;
		}
	}

	public Profile(int[] mode) {
		pin_num = PIN_TOTAL_NUM;// mode.length;
		_size = 1 + pin_num * PinConfig.PIN_CFG_SIZE;
		pin_cfgs = new PinConfig[pin_num];

		for (int i = 0; i < pin_num; i++) {
			pin_cfgs[i] = new PinConfig(i / PIN_PER_GRP, i % PIN_PER_GRP,
					mode[i]);
		}
	}

	public int size() {
		return _size;
	}

	public PinConfig getPin(int i) {
		return pin_cfgs[i];
	}

	public Double getPower() {
		return _power;
	}

	public void setPower(Double power) {
		_power = power;
	}

	public byte[] toBytes() {
		byte[] ret = new byte[_size];

		ret[0] = (byte) pin_num;
		for (int i = 0, offset = 1; i < pin_num; i++) {
			for (int j = 0; j < PinConfig.PIN_CFG_SIZE; j++) {
				ret[offset++] = pin_cfgs[i]._val[j];
			}
		}

		return ret;
	}

	public String toByteString() {
		String s = "";

		byte[] ret = toBytes();

		for (int i = 0; i < ret.length; i++) {
			int v = ret[i] & 0xFF;
			if (v < 16) {
				s += '0';
			}
			s += Integer.toHexString(v);
		}

		return s.toUpperCase();
	}

	public String profile2Key() {
		String key = "";// BigInteger key = BigInteger.valueOf(0);

		for (int i = 0; i < pin_num; i++) {
			key += pin_cfgs[i].toKey();
		}

		return key;
	}

	public String toString() {
		String s = "";

		for (int i = 0; i < pin_num; i++) {
			s += pin_cfgs[i].toString() + "|";
		}

		return s.substring(0, s.length() - 1);
	}

	// parse the output format and
	// generate the code
	public String genCode() {
		String s = "";
		for (int i = 0; i < PIN_TOTAL_NUM; i++) {
			s += pin_cfgs[i].genCode();
		}

		s += "**********\r\n**********\r\n**********\r\n";
		return s;
	}
}
