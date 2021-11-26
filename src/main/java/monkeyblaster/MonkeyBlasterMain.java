package monkeyblaster;

import com.jme3.app.SimpleApplication;
import com.jme3.cursors.plugins.JmeCursor;
import com.jme3.input.KeyInput;
import com.jme3.input.MouseInput;
import com.jme3.input.controls.ActionListener;
import com.jme3.input.controls.AnalogListener;
import com.jme3.input.controls.KeyTrigger;
import com.jme3.input.controls.MouseButtonTrigger;
import com.jme3.material.Material;
import com.jme3.material.RenderState.BlendMode;
import com.jme3.math.FastMath;
import com.jme3.math.Vector2f;
import com.jme3.math.Vector3f;
import com.jme3.post.FilterPostProcessor;
import com.jme3.post.filters.BloomFilter;
import com.jme3.renderer.RenderManager;
import com.jme3.scene.Node;
import com.jme3.scene.Spatial;
import com.jme3.system.AppSettings;
import com.jme3.texture.Texture2D;
import com.jme3.ui.Picture;
import java.awt.Rectangle;
import java.util.Random;

/**
 * Urheberrecht (c) 2013, Daniel Gallenberger
 * 
 * MonkeyBlaster is a Port of Shape Blaster by Michael Hoffmann.
 * Shape Blaster is a Clone of Geometry Wars.
 * 
 */


public class MonkeyBlasterMain extends SimpleApplication implements ActionListener, AnalogListener {
    private Hud hud;
    private Sound sound;
    private ParticleManager particleManager;
    private Grid grid;
    
    private long bulletCooldown;
    private long enemySpawnCooldown;
    private float enemySpawnChance = 80;
    private long spawnCooldownBlackHole;
    
    private Spatial player;
    private Node bulletNode;
    private Node enemyNode;
    private Node blackHoleNode;
    private Node particleNode;
    
    private boolean gameOver = false;
    
    
    public static void main(String[] args) {
        MonkeyBlasterMain app = new MonkeyBlasterMain();
        app.start();
    }

    @Override
    public void simpleInitApp() {
//        setup camera for 2D games
        cam.setParallelProjection(true);
        cam.setLocation(new Vector3f(0,0,0.5f));
        getFlyByCamera().setEnabled(false);
        
//        turn off stats (you can leave it on, if you want)
        setDisplayStatView(true);
        setDisplayFps(false);
        
//        initializing the bloom filter
        FilterPostProcessor fpp=new FilterPostProcessor(assetManager);
        BloomFilter bloom=new BloomFilter();
        bloom.setBloomIntensity(2f);
        bloom.setExposurePower(2);
        bloom.setExposureCutOff(0f);
        bloom.setBlurScale(1.5f);
        fpp.addFilter(bloom);
        guiViewPort.addProcessor(fpp);
        guiViewPort.setClearColor(true);
        
//        setup input handling
        inputManager.setMouseCursor((JmeCursor) assetManager.loadAsset("Textures/Pointer.ico"));
        
        inputManager.addMapping("mousePick", new MouseButtonTrigger(MouseInput.BUTTON_LEFT));
        inputManager.addListener(this, "mousePick");
        
        inputManager.addMapping("left", new KeyTrigger(KeyInput.KEY_LEFT));
        inputManager.addMapping("right", new KeyTrigger(KeyInput.KEY_RIGHT));
        inputManager.addMapping("up", new KeyTrigger(KeyInput.KEY_UP));
        inputManager.addMapping("down", new KeyTrigger(KeyInput.KEY_DOWN));
        inputManager.addMapping("return", new KeyTrigger(KeyInput.KEY_RETURN));
        inputManager.addListener(this, "left");
        inputManager.addListener(this, "right");
        inputManager.addListener(this, "up");
        inputManager.addListener(this, "down");
        inputManager.addListener(this, "return");
        
//        setup the hud
        hud = new Hud(assetManager, guiNode, settings.getWidth(), settings.getHeight());
        hud.reset();
        
//        sounds
        sound = new Sound(assetManager);
        sound.startMusic();
        sound.spawn();
        
//        particles
        particleManager = new ParticleManager(guiNode, getSpatial("Laser"), getSpatial("Glow"), settings.getWidth(), settings.getHeight());
        
//        grid
        Rectangle size = new Rectangle(0, 0, settings.getWidth(), settings.getHeight());
        Vector2f spacing = new Vector2f(25,25);
        grid = new Grid(size, spacing, guiNode, assetManager);
        
//        setup the player
        player = getSpatial("Player");
        player.setUserData("alive",true);
        player.move(settings.getWidth()/2, settings.getHeight()/2, 0);
        player.addControl(new PlayerControl(settings.getWidth(), settings.getHeight(), particleManager));
        guiNode.attachChild(player);
        
//        setup the bulletNode
        bulletNode = new Node("bullets");
        guiNode.attachChild(bulletNode);
//        setup the enemyNode
        enemyNode = new Node("enemies");
        guiNode.attachChild(enemyNode);
//        setup the blackholeNode
        blackHoleNode = new Node("black_holes");
        guiNode.attachChild(blackHoleNode);
//        setup the particleNode
        particleNode = new Node("particles");
        guiNode.attachChild(particleNode);
    }

    @Override
    public void simpleUpdate(float tpf) {
        if ((Boolean) player.getUserData("alive")) {
            spawnEnemies();
            spawnBlackHoles();
            handleCollisions();
            handleGravity(tpf);
        } else if (System.currentTimeMillis() - (Long) player.getUserData("dieTime") > 4000f && !gameOver) {
            // spawn player
            player.setLocalTranslation(500,500,0);
            guiNode.attachChild(player);
            player.setUserData("alive",true);
            sound.spawn();
            grid.applyDirectedForce(new Vector3f(0,0,5000), player.getLocalTranslation(), 100);
        }
        grid.update(tpf);
        hud.update();
    }
    
    private void spawnEnemies() {
        if (System.currentTimeMillis() - enemySpawnCooldown >= 17) {
            enemySpawnCooldown = System.currentTimeMillis();
            
            if (enemyNode.getQuantity() < 50) {
                if (new Random().nextInt((int) enemySpawnChance) == 0) {
                    createSeeker();
                }
                if (new Random().nextInt((int) enemySpawnChance) == 0) {
                    createWanderer();
                }
            }
            
            //increase Spawn Time
            if (enemySpawnChance >= 1.1f) {
                enemySpawnChance -= 0.005f;
            }
        }
    }
    
        private void createSeeker() {
        Spatial seeker = getSpatial("Seeker");
        seeker.setLocalTranslation(getSpawnPosition());
        seeker.addControl(new SeekerControl(player));
        seeker.setUserData("active",false);
        enemyNode.attachChild(seeker);
    }
    
    private void createWanderer() {
        Spatial wanderer = getSpatial("Wanderer");
        wanderer.setLocalTranslation(getSpawnPosition());
        wanderer.addControl(new WandererControl(settings.getWidth(), settings.getHeight()));
        wanderer.setUserData("active",false);
        enemyNode.attachChild(wanderer);
    }
    
    private void spawnBlackHoles() {
        if (blackHoleNode.getQuantity() < 2) {
             if (System.currentTimeMillis() - spawnCooldownBlackHole > 10f) {
                spawnCooldownBlackHole = System.currentTimeMillis();
                if (new Random().nextInt(1000) == 0) {
                    createBlackHole();
                }
            }
        }
    }
    
    private void createBlackHole() {
        Spatial blackHole = getSpatial("Black Hole");
        blackHole.setLocalTranslation(getSpawnPosition());
        blackHole.addControl(new BlackHoleControl(particleManager, grid));
        blackHole.setUserData("active",false);
        blackHoleNode.attachChild(blackHole);
    }
    
    private Vector3f getSpawnPosition() {
        Vector3f pos;
        do {
            pos = new Vector3f(new Random().nextInt(settings.getWidth()), new Random().nextInt(settings.getHeight()),0);
        } while (pos.distanceSquared(player.getLocalTranslation()) < 8000);
        return pos;
    }
    
    private void handleCollisions() {
        // should the player die?
        for (int i=0; i<enemyNode.getQuantity(); i++) {
            if ((Boolean) enemyNode.getChild(i).getUserData("active")) {
                if (checkCollision(player,enemyNode.getChild(i))) {
                    killPlayer();   
                }
            }
        }
        
        //should an enemy die?
        int i=0;
        while (i < enemyNode.getQuantity()) {
            int j=0;
            while (j < bulletNode.getQuantity()) {
                if (checkCollision(enemyNode.getChild(i),bulletNode.getChild(j))) {
                    // add points depending on the type of enemy
                    if (enemyNode.getChild(i).getName().equals("Seeker")) {
                        hud.addPoints(2);
                    } else if (enemyNode.getChild(i).getName().equals("Wanderer")) {
                        hud.addPoints(1);
                    }
                    particleManager.enemyExplosion(enemyNode.getChild(i).getLocalTranslation());
                    enemyNode.detachChildAt(i);
                    bulletNode.detachChildAt(j);
                    sound.explosion();
                    break;
                }
                j++;
            }
            i++;
        }
        
        //is something colliding with a black hole?
        for (i=0; i<blackHoleNode.getQuantity(); i++) {
            Spatial blackHole = blackHoleNode.getChild(i);
            if ((Boolean) blackHole.getUserData("active")) {
                //player
                if (checkCollision(player,blackHole)) {
                    killPlayer();
                }
                
                //enemies
                int j=0;
                while (j < enemyNode.getQuantity()) {
                    if (checkCollision(enemyNode.getChild(j),blackHole)) {
                        particleManager.enemyExplosion(enemyNode.getChild(j).getLocalTranslation());
                        enemyNode.detachChildAt(j);
                    }
                    j++;
                }
                
                //bullets
                j=0;
                while (j < bulletNode.getQuantity()) {
                    if (checkCollision(bulletNode.getChild(j),blackHole)) {
                        bulletNode.detachChildAt(j);
                        blackHole.getControl(BlackHoleControl.class).wasShot();
                        particleManager.blackHoleExplosion(blackHole.getLocalTranslation());
                        if (blackHole.getControl(BlackHoleControl.class).isDead()) {
                            blackHoleNode.detachChild(blackHole);
                            sound.explosion();
                        }
                    }
                    j++;
                }
            }
        }
    }
    
    private boolean checkCollision(Spatial a, Spatial b) {
        float distance = a.getLocalTranslation().distance(b.getLocalTranslation());
        float maxDistance =  (Float)a.getUserData("radius") + (Float)b.getUserData("radius");
        return distance <=maxDistance;
    }
    
    private void killPlayer() {
        player.removeFromParent();
        player.getControl(PlayerControl.class).reset();
        player.setUserData("alive", false);
        player.setUserData("dieTime", System.currentTimeMillis());
        enemyNode.detachAllChildren();
        blackHoleNode.detachAllChildren();
        particleManager.playerExplosion(player.getLocalTranslation());
        if (!hud.removeLife()) {
            hud.endGame();
            gameOver = true;
        }
    }
    
    private void handleGravity(float tpf) {
        for (int i=0; i<blackHoleNode.getQuantity(); i++) {
            if (!(Boolean)blackHoleNode.getChild(i).getUserData("active")) {continue;}
            int radius = 250;
            
            // check Player
            if (isNearby(player,blackHoleNode.getChild(i),radius)) {
                applyGravity(blackHoleNode.getChild(i), player, tpf);
            }
            // check Bullets
            for (int j=0; j<bulletNode.getQuantity(); j++) {
                if (isNearby(bulletNode.getChild(j),blackHoleNode.getChild(i),radius)) {
                    applyGravity(blackHoleNode.getChild(i), bulletNode.getChild(j), tpf);
                }
            }
            // check Enemies
            for (int j=0; j<enemyNode.getQuantity(); j++) {
                if (!(Boolean)enemyNode.getChild(j).getUserData("active")) {continue;}
                if (isNearby(enemyNode.getChild(j),blackHoleNode.getChild(i),radius)) {
                    applyGravity(blackHoleNode.getChild(i), enemyNode.getChild(j), tpf);
                }
            }
            // check Particles
            for (int j=0; j<particleNode.getQuantity(); j++) {
                if ((boolean) particleNode.getChild(j).getUserData("affectedByGravity")) {
                    applyGravity(blackHoleNode.getChild(i), particleNode.getChild(j), tpf);
                }
            }
        }
    }
    
    private boolean isNearby(Spatial a, Spatial b, float distance) {
        Vector3f pos1 = a.getLocalTranslation();
        Vector3f pos2 = b.getLocalTranslation();
        return pos1.distanceSquared(pos2) <= distance * distance;
    }
    
    private void applyGravity(Spatial blackHole, Spatial target, float tpf) {
        Vector3f difference = blackHole.getLocalTranslation().subtract(target.getLocalTranslation());
        
        Vector3f gravity = difference.normalize().multLocal(tpf);
        float distance = difference.length();
        
        if (target.getName().equals("Player")) {
            gravity.multLocal(250f/distance);
            target.getControl(PlayerControl.class).applyGravity(gravity.mult(80f));
        } else if (target.getName().equals("Bullet")) {
            gravity.multLocal(250f/distance);
            target.getControl(BulletControl.class).applyGravity(gravity.mult(-0.8f));
        } else if (target.getName().equals("Seeker")) {
            target.getControl(SeekerControl.class).applyGravity(gravity.mult(150000));
        } else if (target.getName().equals("Wanderer")) {
            target.getControl(WandererControl.class).applyGravity(gravity.mult(150000));
        } else if (target.getName().equals("Laser") || target.getName().equals("Glow")) {
            target.getControl(ParticleControl.class).applyGravity(gravity.mult(15000), distance);
        }
    }
    
    @Override
    public void simpleRender(RenderManager rm) {}

    public void onAction(String name, boolean isPressed, float tpf) {
        if ((Boolean) player.getUserData("alive")) {
            if (name.equals("up")) {
               player.getControl(PlayerControl.class).up = isPressed;
            } else if (name.equals("down")) {
                player.getControl(PlayerControl.class).down = isPressed;
            } else if (name.equals("left")) {
                player.getControl(PlayerControl.class).left = isPressed;
            } else if (name.equals("right")) {
                player.getControl(PlayerControl.class).right = isPressed;
            }
        }
    }

    public void onAnalog(String name, float value, float tpf) {
        if ((Boolean) player.getUserData("alive")) {
            if (name.equals("mousePick")) {
                //shoot Bullet
                if (System.currentTimeMillis() - bulletCooldown > 83f) {
                    bulletCooldown = System.currentTimeMillis();
                    
                    Vector3f aim = getAimDirection();
                    Vector3f offset = new Vector3f(aim.y/3,-aim.x/3,0);
                    
//                    init bullet 1
                    Spatial bullet = getSpatial("Bullet");
                    Vector3f finalOffset = aim.add(offset).mult(30);
                    Vector3f trans = player.getLocalTranslation().add(finalOffset);
                    bullet.setLocalTranslation(trans);
                    bullet.addControl(new BulletControl(aim, settings.getWidth(), settings.getHeight(), particleManager, grid));
                    bulletNode.attachChild(bullet);
                    
//                    init bullet 2
                    Spatial bullet2 = getSpatial("Bullet");
                    finalOffset = aim.add(offset.negate()).mult(30);
                    trans = player.getLocalTranslation().add(finalOffset);
                    bullet2.setLocalTranslation(trans);
                    bullet2.addControl(new BulletControl(aim, settings.getWidth(), settings.getHeight(), particleManager, grid));
                    bulletNode.attachChild(bullet2);
                    
                    sound.shoot();
                }
            }
        }
    }
    
    private Vector3f getAimDirection() {
        Vector2f mouse = inputManager.getCursorPosition();
        Vector3f playerPos = player.getLocalTranslation();
        Vector3f dif = new Vector3f(mouse.x-playerPos.x,mouse.y-playerPos.y,0);
        return dif.normalizeLocal();
    }
    
    public static float getAngleFromVector(Vector3f vec) {
        Vector2f vec2 = new Vector2f(vec.x,vec.y);
        return vec2.getAngle();
    }
    
    public static Vector3f getVectorFromAngle(float angle) {
        return new Vector3f(FastMath.cos(angle),FastMath.sin(angle),0);
    }
    
    private Spatial getSpatial(String name) {
        Node node = new Node(name);
//        load picture
        Picture pic = new Picture(name);
        Texture2D tex = (Texture2D) assetManager.loadTexture("Textures/"+name+".png");
        pic.setTexture(assetManager,tex,true);
        
//        adjust picture
        float width = tex.getImage().getWidth();
        float height = tex.getImage().getHeight();
        pic.setWidth(width);
        pic.setHeight(height);
        pic.move(-width/2f,-height/2f,0);
        
//        add a material to the picture
        Material picMat = new Material(assetManager, "Common/MatDefs/Gui/Gui.j3md");
        picMat.getAdditionalRenderState().setBlendMode(BlendMode.AlphaAdditive);
        node.setMaterial(new Material());
        
//        set the radius of the spatial
//        (using width only as a simple approximation)
        node.setUserData("radius", width/2);
        
//        attach the picture to the node and return it
        node.attachChild(pic);
        return node;
    }
    
    public AppSettings getSettings() {return settings;}
}
