using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.InputSystem;


namespace Turrican
{




    public class PlayerMovement : MonoBehaviour
    {
        [SerializeReference] private GameObject player;
        [SerializeField] private float horizontalSpeed = 1f;
        [SerializeField] private float vertikalSpeed = 1f;


        private SpriteRenderer renderer;
        private Animator _animation;
        private Vector2 _velocity;
        private bool _isFlip;
        private bool _isIdle;
        private bool _isWalk;
        private bool _isKneeling;


        private void Awake()
        {
            _isFlip = false;
            _isIdle = true;
            _isWalk = false;
            _isKneeling = false;
        }

        // Start is called before the first frame update
        void Start()
        {
            this.renderer = this.GetComponent<SpriteRenderer>();
            this._animation = this.GetComponent<Animator>();

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
            if (xDelta > 0)
            {
                _isFlip = true;
                _isWalk = true;
                _isIdle = false;

            }
            else if (xDelta < 0)
            {
                _isFlip = false;
                _isWalk = true;
                _isIdle = false;
            }
            else
            {
                _isWalk = false;
                _isIdle = true;
            }
            setAnimation();

            renderer.flipX = _isFlip;



            // Make it move 10 meters per second instead of 10 meters per frame...
            yDelta *= Time.deltaTime;
            xDelta *= Time.deltaTime;

            // Move translation along the object's z-axis
            transform.Translate(xDelta, yDelta, 0);

        }
        public void setAnimation()
        {
            _animation.SetBool("isWalk", _isWalk);
            _animation.SetBool("isIdle", _isIdle);
            _animation.SetBool("isKneeling", _isKneeling);
        }
        public void OnMove(InputAction.CallbackContext context)
        {
            _velocity = context.ReadValue<Vector2>();
        }

        public void OnJump(InputAction.CallbackContext context)
        {
            Debug.Log("Jump");
            Console.WriteLine("Jump");
        }
    }
}