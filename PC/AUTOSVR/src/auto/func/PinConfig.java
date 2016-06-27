package auto.func;

//configuration per pin (2 bytes)
//1st byte:
//bit 0~3: grp #
//bit 4~7: pin #
//2nd byte:
//bit 0~2: reserved
//bit 3~7: mode, such as PP-PU
public class PinConfig {
	
	public PinConfig(PinConfig other) {
		_val = setConfig(other._grp, other._pin, other._opt);
	}

	public PinConfig(int grp, int pin, int mode) {
		_val = setConfig(grp, pin, mode);
	}

	public PinConfig(String cfg) {
		int i;
		String[] ss = cfg.split(":", 4);

		for (i = 0; i < OPT.length; i++) {
			if (ss[3].equals(OPT[i])) {
				_val = setConfig(ss[0].charAt(1) - 'A',
						Integer.parseInt(ss[1]), i);
				break;
			}
		}
		
		if (i >= OPT.length) {
			System.out.println("Error for PinConfig: unrecognized pin cfg");
			System.exit(-1);
		}
	}
	
	public void setOpt(int mode) {
		_opt = mode;
		
		// also update val[]
		_val[1] = 0;
		_val[1] |= _opt << NUMBITS_RES;
	}

	private byte[] setConfig(int grp, int pin, int mode) {
		_grp = grp;
		_pin = pin;
		_opt = mode;

		// PinConfig (2 bytes)
		byte[] val = new byte[PIN_CFG_SIZE];

		// group (lower 4bit)
		val[0] = (byte) (_grp);
		// pin (higher 4bit)
		val[0] |= (byte) (_pin << NUMBITS_GRP);

		// reserved bits (3bit)
		// pin configuration...
		val[1] |= _opt << NUMBITS_RES;

		return val;
	}

	public int getGrp() {
		return _grp;
	}

	public int getPin() {
		return _pin;
	}

	public int getOpt() {
		return _opt;
	}

	public char toKey() {
		return (char) ('A' + _opt);

		// return BigInteger
		// .valueOf(_opt << (NUMBITS_OPT * ((_grp << NUMBITS_OPT) + _pin)));
	}

	public String toString() {
		String s = "P";

		s += (char) (_grp + 'A') + ":";
		s += _pin + ":400K:";
		s += OPT[_opt];

		return s;
	}

	public String genCode() {
		String s = "";
		String GPIO_Pin = "GPIO_Pin_" + _pin;
		String GPIO_Mode = "";
		String GPIO_Speed = "GPIO_Speed_400KHz";
		String GPIO_OType = "";
		String GPIO_PuPd = "";
		String GPIO_HILO = "";

		s += "\r\nGPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;";
		switch (_opt) {
		case 0:
			GPIO_Mode = "GPIO_Mode_OUT";
			GPIO_OType = "GPIO_OType_PP";
			GPIO_PuPd = "GPIO_PuPd_NOPULL";
			GPIO_HILO = "LO";
			break;
		case 1:
			GPIO_Mode = "GPIO_Mode_OUT";
			GPIO_OType = "GPIO_OType_PP";
			GPIO_PuPd = "GPIO_PuPd_NOPULL";
			GPIO_HILO = "HI";
			break;
		case 2:
			GPIO_Mode = "GPIO_Mode_OUT";
			GPIO_OType = "GPIO_OType_PP";
			GPIO_PuPd = "GPIO_PuPd_UP";
			GPIO_HILO = "LO";
			break;
		case 3:
			GPIO_Mode = "GPIO_Mode_OUT";
			GPIO_OType = "GPIO_OType_PP";
			GPIO_PuPd = "GPIO_PuPd_UP";
			GPIO_HILO = "HI";
			break;
		case 4:
			GPIO_Mode = "GPIO_Mode_OUT";
			GPIO_OType = "GPIO_OType_PP";
			GPIO_PuPd = "GPIO_PuPd_DOWN";
			GPIO_HILO = "LO";
			break;
		case 5:
			GPIO_Mode = "GPIO_Mode_OUT";
			GPIO_OType = "GPIO_OType_PP";
			GPIO_PuPd = "GPIO_PuPd_DOWN";
			GPIO_HILO = "HI";
			break;

		case 6:
			GPIO_Mode = "GPIO_Mode_OUT";
			GPIO_OType = "GPIO_OType_OD";
			GPIO_PuPd = "GPIO_PuPd_NOPULL";
			GPIO_HILO = "LO";
			break;
		case 7:
			GPIO_Mode = "GPIO_Mode_OUT";
			GPIO_OType = "GPIO_OType_OD";
			GPIO_PuPd = "GPIO_PuPd_NOPULL";
			GPIO_HILO = "HI";
			break;
		case 8:
			GPIO_Mode = "GPIO_Mode_OUT";
			GPIO_OType = "GPIO_OType_OD";
			GPIO_PuPd = "GPIO_PuPd_UP";
			GPIO_HILO = "LO";
			break;
		case 9:
			GPIO_Mode = "GPIO_Mode_OUT";
			GPIO_OType = "GPIO_OType_OD";
			GPIO_PuPd = "GPIO_PuPd_UP";
			GPIO_HILO = "HI";
			break;
		case 10:
			GPIO_Mode = "GPIO_Mode_OUT";
			GPIO_OType = "GPIO_OType_OD";
			GPIO_PuPd = "GPIO_PuPd_DOWN";
			GPIO_HILO = "LO";
			break;
		case 11:
			GPIO_Mode = "GPIO_Mode_OUT";
			GPIO_OType = "GPIO_OType_OD";
			GPIO_PuPd = "GPIO_PuPd_DOWN";
			GPIO_HILO = "HI";
			break;

		case 12:
			GPIO_Mode = "GPIO_Mode_IN";
			GPIO_OType = "GPIO_OType_PP";
			GPIO_PuPd = "GPIO_PuPd_NOPULL";
			break;

		case 13:
			GPIO_Mode = "GPIO_Mode_IN";
			GPIO_OType = "GPIO_OType_PP";
			GPIO_PuPd = "GPIO_PuPd_UP";
			break;

		case 14:
			GPIO_Mode = "GPIO_Mode_IN";
			GPIO_OType = "GPIO_OType_PP";
			GPIO_PuPd = "GPIO_PuPd_DOWN";
			break;

		case 15:
			GPIO_Mode = "GPIO_Mode_AN";
			GPIO_OType = "GPIO_OType_PP";
			GPIO_PuPd = "GPIO_PuPd_UP";
			break;

		default:
			break;
		}

		s = String
				.format("GPIO_InitStructure.GPIO_Pin = %s;\r\nGPIO_InitStructure.GPIO_Mode = %s;\r\nGPIO_InitStructure.GPIO_Speed = %s;\r\nGPIO_InitStructure.GPIO_OType = %s;\r\nGPIO_InitStructure.GPIO_PuPd = %s;\r\n",
						GPIO_Pin, GPIO_Mode, GPIO_Speed, GPIO_OType, GPIO_PuPd);

		s += "GPIO_Init(GPIO" + (char) ('A' + _grp)
				+ ", &GPIO_InitStructure);\r\n";

		if (GPIO_HILO.equals("HI"))
			s += String.format("GPIO_SetBits(GPIO%c, %s);\r\n", ('A' + _grp),
					GPIO_Pin);
		else if (GPIO_HILO.equals("LO"))
			s += String.format("GPIO_ResetBits(GPIO%c, %s);\r\n", ('A' + _grp),
					GPIO_Pin);

		s += "\r\n";

		return s;
	}

	// 2 bytes for each pin configuration
	public final static int PIN_CFG_SIZE = 2;
	// first byte
	public final static int NUMBITS_GRP = 4;
	public final static int NUMBITS_PIN = 8 - NUMBITS_GRP;
	// secode byte
	public final static int NUMBITS_RES = 3;
	public final static int NUMBITS_OPT = 8 - NUMBITS_RES;

	public final static String OPT[] = { "GP:PP:0", "GP:PP:1", "GP:PP:PU:0",
			"GP:PP:PU:1", "GP:PP:PD:0", "GP:PP:PD:1", "GP:OD:0", "GP:OD:1",
			"GP:OD:PU:0", "GP:OD:PU:1", "GP:OD:PD:0", "GP:OD:PD:1", "IN:FLOAT",
			"IN:PU", "IN:PD", "AN" };
//	public final static String OPT[] = { "GP:PP:0", "GP:PP:1", "GP:OD:0",
//			"GP:OD:1", "IN:FLOAT", "IN:PU", "IN:PD", "AN" };

	byte[] _val;
	int _grp;
	int _pin;
	int _opt;
}
