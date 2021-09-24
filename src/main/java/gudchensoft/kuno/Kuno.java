package gudchensoft.kuno;

import java.io.InputStream;

import org.newdawn.slick.Animation;
import org.newdawn.slick.AppGameContainer;
import org.newdawn.slick.BasicGame;
import org.newdawn.slick.Color;
import org.newdawn.slick.GameContainer;
import org.newdawn.slick.Graphics;
import org.newdawn.slick.Image;
import org.newdawn.slick.ImageBuffer;
import org.newdawn.slick.SlickException;
import org.newdawn.slick.SpriteSheet;
import org.newdawn.slick.opengl.ImageDataFactory;

/**
 * 
 * @author 10170328
 *
 */
public class Kuno  extends BasicGame{

	private Animation begin;

	/**
	 * Kuno-Spiel
	 */
	public Kuno() {
		super("Kuno der Ritter");
	}

	@Override
	public void init(GameContainer container) throws SlickException {
		int d = 100;
		InputStream is = Kuno.class.getResourceAsStream("/kuno-sprites");
		is = java.util.Base64.getDecoder().wrap(is);
		Image kuno = new Image(is, "Kuno Sprites", false);
		SpriteSheet ss = new SpriteSheet(kuno, 24, 21);
		begin = new Animation(ss, new int[] {0, 0, 1, 0, 2, 0, 3, 0}, new int[] {d, d, d, d});
		
	}

	@Override
	public void update(GameContainer container, int delta) throws SlickException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void render(GameContainer container, Graphics g) throws SlickException {
		begin.draw(160, 100);
		
	}

	/**
	 * JVM-Argumente für Native DLLs
	 * -Dorg.lwjgl.librarypath="Verzeichnis"
     * -Dnet.java.games.input.librarypath="Verzeichnis"
	 * @param args
	 */
	public static void main(String[] args) {
		try {
			AppGameContainer game = new AppGameContainer(new Kuno());
	          game.setMaximumLogicUpdateInterval(60);
	          game.setDisplayMode(640, 480, false);
	          game.setTargetFrameRate(60);
	          game.setAlwaysRender(true);
	          game.setVSync(true);
	          game.setShowFPS(false);
	          game.start();
		} catch (SlickException e) {
			e.printStackTrace();
		}
		System.exit(0);

	}

}
