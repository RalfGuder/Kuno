package gudchensoft.kuno;

import org.newdawn.slick.AppGameContainer;
import org.newdawn.slick.BasicGame;
import org.newdawn.slick.GameContainer;
import org.newdawn.slick.Graphics;
import org.newdawn.slick.SlickException;

/**
 * 
 * @author 10170328
 *
 */
public class Kuno  extends BasicGame{

	/**
	 * Kuno-Spiel
	 */
	public Kuno() {
		super("Kuno der Ritter");
	}

	@Override
	public void init(GameContainer container) throws SlickException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void update(GameContainer container, int delta) throws SlickException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void render(GameContainer container, Graphics g) throws SlickException {
		// TODO Auto-generated method stub
		
	}

	/**
	 * JVM-Argumente für Native DLLs
	 * -Dorg.lwjgl.librarypath="Verzeichnis"
     * -Dnet.java.games.input.librarypath="Verzeichnis"
	 * @param args
	 */
	public static void main(String[] args) {
		try {
			AppGameContainer game = new AppGameContainer(new Kuno(), 320, 200, false);
			game.start();
		} catch (SlickException e) {
			e.printStackTrace();
		}
		System.exit(0);

	}

}
