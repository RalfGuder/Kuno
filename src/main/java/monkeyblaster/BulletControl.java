package monkeyblaster;

import com.jme3.math.Vector3f;
import com.jme3.renderer.RenderManager;
import com.jme3.renderer.ViewPort;
import com.jme3.scene.control.AbstractControl;

public class BulletControl extends AbstractControl {
    private ParticleManager particleManager;
    private Grid grid;
    private int screenWidth, screenHeight;
    
    private float speed = 1100f;
    public Vector3f direction;
    private float rotation;
    
    public BulletControl(Vector3f direction, int screenWidth, int screenHeight, ParticleManager particleManager, Grid grid) {
        this.particleManager = particleManager;
        this.grid = grid;
        this.direction = direction;
        this.screenWidth = screenWidth;
        this.screenHeight = screenHeight;
    }

    @Override
    protected void controlUpdate(float tpf) {
//        movement
        spatial.move(direction.mult(speed*tpf));
        
//        rotation
        float actualRotation = MonkeyBlasterMain.getAngleFromVector(direction);
        if (actualRotation != rotation) {
            spatial.rotate(0,0,actualRotation - rotation);
            rotation = actualRotation;
        }
        
//        check boundaries
        Vector3f loc = spatial.getLocalTranslation();
        if (loc.x<0 || loc.y <0 || loc.x > screenWidth || loc.y > screenHeight) {
            particleManager.bulletExplosion(loc);
            spatial.removeFromParent();
        }
        
        grid.applyExplosiveForce(direction.length()*(18f), spatial.getLocalTranslation(), 80);
    }

    @Override
    protected void controlRender(RenderManager rm, ViewPort vp) {}
    
    public void applyGravity(Vector3f gravity) {
        direction.addLocal(gravity);
    }
}
