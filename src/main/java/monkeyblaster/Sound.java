package monkeyblaster;

import com.jme3.asset.AssetManager;
import com.jme3.audio.AudioNode;
import java.util.Random;

public class Sound {
    private AudioNode music;
    private AudioNode[] shots;
    private AudioNode[] explosions;
    private AudioNode[] spawns;
    
    private AssetManager assetManager;
    
    public Sound(AssetManager assetManager) {
        this.assetManager = assetManager;
        shots = new AudioNode[4];
        explosions = new AudioNode[8];
        spawns = new AudioNode[8];
        
        loadSounds();
    }
    
    private void loadSounds() {
        music = new AudioNode(assetManager,"Sounds/Music.ogg");
        music.setPositional(false);
        music.setReverbEnabled(false);
        music.setLooping(true);
        
        for (int i=0; i<shots.length; i++) {
            shots[i] = new AudioNode(assetManager,"Sounds/shoot-0"+(i+1)+".wav");
            shots[i].setPositional(false);
            shots[i].setReverbEnabled(false);
            shots[i].setLooping(false);
        }
        
        for (int i=0; i<explosions.length; i++) {
            explosions[i]  = new AudioNode(assetManager,"Sounds/explosion-0"+(i+1)+".wav");
            explosions[i].setPositional(false);
            explosions[i].setReverbEnabled(false);
            explosions[i].setLooping(false);
        }
        
        for (int i=0; i<spawns.length; i++) {
            spawns[i]  = new AudioNode(assetManager,"Sounds/spawn-0"+(i+1)+".wav");
            spawns[i].setPositional(false);
            spawns[i].setReverbEnabled(false);
            spawns[i].setLooping(false);
        }
    }
    
    public void startMusic() {
        music.play();
    }
    
    public void shoot() {
        shots[new Random().nextInt(shots.length)].playInstance();
    }
    
    public void explosion() {
        explosions[new Random().nextInt(explosions.length)].playInstance();
    }
    
    public void spawn() {
        spawns[new Random().nextInt(spawns.length)].playInstance();
    }
}
