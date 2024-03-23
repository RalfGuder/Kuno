using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.InputSystem;


namespace Turrican
{




    public class PlayerMovement : MonoBehaviour
    {
        [Header("Player Movement")]
        [SerializeReference] private GameObject player;
        [SerializeField] private float horizontalSpeed = 1f;
        [SerializeField] private float vertikalSpeed = 1f;


        private SpriteRenderer renderer;
        private Animator _animation;
        private Vector2 _velocity;
        private Vector2 _input;
        private bool _isFlip;
        private bool _isKneeling;
        private bool _isJump;
        private bool _isFire;


        private void Awake()
        {
            _isFlip = false;
            _isKneeling = false;
            _isJump = false;
            _isFire = false;
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

            // Zuerst Input abfragen
            Debug.Log("Move: x=" + _input.x.ToString() + ", y=" + _input.y.ToString());
            // Get the horizontal and vertical axis.
            // By default they are mapped to the arrow keys.
            // The value is in the range -1 to 1
            float yDelta = _input.y * vertikalSpeed;
            float xDelta = _input.x * horizontalSpeed;


            // Make it move 10 meters per second instead of 10 meters per frame...
            yDelta *= Time.deltaTime;
            xDelta *= Time.deltaTime;
            _velocity.x = xDelta;
            _velocity.y = yDelta;
            
            // Keine Bewegung wenn Kniend oder Schießend
            if(_isKneeling || _isFire)
            {
                _velocity.x = 0;
            }

            // Neue Position berechnen
            // Move translation along the object's z-axis
            transform.Translate(_velocity.x, _velocity.y, 0);


            // Animationen setzen
            if (xDelta > 0)
            {
                _isFlip = true;

            }
            else if (xDelta < 0)
            {
                _isFlip = false;
            }

           SetAnimation();





        }
        public void SetAnimation()
        {
            _animation.SetBool("isKneeling", _isKneeling);
            _animation.SetBool("isFire", _isFire);
            _animation.SetFloat("speed", Math.Abs(_velocity.x));
            renderer.flipX = _isFlip;
            Debug.Log("isWalk:" + _animation.GetBool("isWalk") + ", isKneeling:" + _animation.GetBool("isKneeling"));
            Debug.Log("speed:" + _animation.GetFloat("speed"));
        }

        public void OnMove(InputAction.CallbackContext context)
        {
            
            _input = context.ReadValue<Vector2>();
            _isKneeling = false;
            if (_input.y < 0)
            {
                _input.y = 0;
                _input.x = 0;
                _isKneeling = true;
            }
        }

        public void OnJump(InputAction.CallbackContext context)
        {
            Debug.Log("Jump");
            Console.WriteLine("Jump");
        }

        public void OnFire(InputAction.CallbackContext context)
        {
            if (context.started)
            {
                _isFire = true;
            }
            if (context.canceled)
            {
                _isFire = false;
            }
            
            Debug.Log("Fire");

        }
    }
}