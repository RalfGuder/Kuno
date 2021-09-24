package gudchensoft.kuno;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class Base64 {
	public static void main(String[] args) {
		Base64 base = new Base64();

		OutputStream os = null;
		InputStream is = null;


		
		try {


			is = Base64.class.getResourceAsStream("/kuno-sprites.png");

			String dateipfad = "C:/Anhang/kuno-sprites.b64";
			File file = new File(dateipfad);
			os = new FileOutputStream(file);
			base.write(is, os);
			os.close();
			is.close();


		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		} finally {

		}

	}

	private void write(InputStream is, OutputStream os) throws IOException {
		os = java.util.Base64.getEncoder().wrap(os);

		byte[] buffer = new byte[1024];
		int anzahl;
		while ((anzahl = is.read(buffer, 0, buffer.length)) != -1) {

			os.write(buffer, 0, anzahl);
		}

		
	}

}
