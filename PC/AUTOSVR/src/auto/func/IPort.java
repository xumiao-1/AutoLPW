package auto.func;

import java.io.InputStream;
import java.io.OutputStream;

public interface IPort {
	public enum Type {
		SERIAL_PORT, SOCKET_PORT
	}

	public boolean openPort();

	public boolean closePort();

	public OutputStream getOutputStream();

	public InputStream getInputStream();
}
