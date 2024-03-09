using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerMovement : MonoBehaviour
{
    public GameObject player;
    public float horizontalSpeed = 1f;
    public float vertikalSpeed = 1f;


    private SpriteRenderer renderer;

    


    // Start is called before the first frame update
    void Start()
    {
        this.renderer = this.GetComponent<SpriteRenderer>();
    }

    // Update is called once per frame
    void Update()
    {
        // Get the horizontal and vertical axis.
        // By default they are mapped to the arrow keys.
        // The value is in the range -1 to 1
        float yDelta = Input.GetAxis("Vertical") * vertikalSpeed;
        float xDelta = Input.GetAxis("Horizontal") * horizontalSpeed;

        // Flip
        renderer.flipX = xDelta > 0;


        // Make it move 10 meters per second instead of 10 meters per frame...
        yDelta *= Time.deltaTime;
        xDelta *= Time.deltaTime;

        // Move translation along the object's z-axis
        transform.Translate(xDelta, yDelta, 0);

    }
}
