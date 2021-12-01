package gudchensoft.kuno;

import java.io.InputStream;

import org.newdawn.slick.Animation;
import org.newdawn.slick.BasicGame;
import org.newdawn.slick.GameContainer;
import org.newdawn.slick.Graphics;
import org.newdawn.slick.Image;
import org.newdawn.slick.SlickException;
import org.newdawn.slick.SpriteSheet;

public class Turrican extends BasicGame{
	private static final int SCREEN_HIGH = 200;
	private static final int SCREEN_WIDTH = 320;
	private Image background;
	private Animation begin;

	public Turrican() {
		super("Turrican II");
	
	}

	@Override
	public void render(GameContainer container, Graphics g) throws SlickException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void init(GameContainer container) throws SlickException {
		int d = 100;
		background = new Image("Turrican2-background.png");
		SpriteSheet ss = new SpriteSheet(new Image("player-sprites-c64.png"), 24, 21);
		begin = new Animation(ss, new int[] {0, 0, 1, 0, 2, 0, 3, 0}, new int[] {d, d, d, d});
		
	}

	@Override
	public void update(GameContainer container, int delta) throws SlickException {
		// TODO Auto-generated method stub
		
	}

}
