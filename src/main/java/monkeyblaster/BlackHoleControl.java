package monkeyblaster;

import com.jme3.math.ColorRGBA;
import com.jme3.math.FastMath;
import com.jme3.math.Vector3f;
import com.jme3.renderer.RenderManager;
import com.jme3.renderer.ViewPort;
import com.jme3.scene.Node;
import com.jme3.scene.control.AbstractControl;
import com.jme3.ui.Picture;
import java.util.Random;


public class BlackHoleControl extends AbstractControl {
    private ParticleManager particleManager;
    private Grid grid;
    private Random rand;
    
    private long spawnTime, lastSprayTime;
    private int hitpoints;
    private float sprayAngle;
    
    public BlackHoleControl(ParticleManager particleManager, Grid grid) {
        this.particleManager = particleManager;
        this.grid = grid;
        spawnTime = System.currentTimeMillis();
        hitpoints = 10;
        sprayAngle = 0;
        
        rand = new Random();
    }
    
    @Override
    protected void controlUpdate(float tpf) {
        if ((Boolean) spatial.getUserData("active")) {
            long sprayDif = System.currentTimeMillis() - lastSprayTime;
            if ((System.currentTimeMillis() / 250) % 2 == 0 && sprayDif > 20) {
                lastSprayTime = System.currentTimeMillis();
                
                Vector3f sprayVel = MonkeyBlasterMain.getVectorFromAngle(sprayAngle).mult(rand.nextFloat()*3+6);
                Vector3f randVec = MonkeyBlasterMain.getVectorFromAngle(rand.nextFloat()*FastMath.PI*2);
                randVec.multLocal(4+rand.nextFloat()*4);
                Vector3f position = spatial.getLocalTranslation().add(sprayVel.mult(4f)).addLocal(randVec);
                
                particleManager.sprayParticle(position,sprayVel.mult(30f));
            }
            sprayAngle -= FastMath.PI*tpf/10f;
        } else {
            // handle the "active"-status
            long dif = System.currentTimeMillis() - spawnTime;
            if (dif >= 1000f) {
                spatial.setUserData("active",true);
            }
            
            ColorRGBA color = new ColorRGBA(1,1,1,dif/1000f);
            Node spatialNode = (Node) spatial;
            Picture pic = (Picture) spatialNode.getChild("Black Hole");
            pic.getMaterial().setColor("Color",color);
        }
        
        grid.applyImplosiveForce(FastMath.sin(sprayAngle/2) * 10 +20, spatial.getLocalTranslation(), 250);
    }

    @Override
    protected void controlRender(RenderManager rm, ViewPort vp) {}
    
    public void wasShot() {
        hitpoints--;
    }
    
    public boolean isDead() {
        return hitpoints <= 0;
    }
}
