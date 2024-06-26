package com.example.firebase_iot

import android.content.Intent
import android.os.Bundle
import android.text.InputType
import android.util.Log
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.example.firebase_iot.databinding.ActivityLoginBinding
import com.google.firebase.auth.FirebaseAuth

class LoginActivity : AppCompatActivity() {

    private lateinit var auth: FirebaseAuth
    private lateinit var binding: ActivityLoginBinding // Declare binding variable

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_login)
        binding = ActivityLoginBinding.inflate(layoutInflater) // Initialize binding
        setContentView(binding.root)

        // Initialize Firebase Auth
        auth = FirebaseAuth.getInstance()

        // Set up click listeners
        binding.loginButton.setOnClickListener {
            loginUser(
                binding.emailEditText.text.toString(),
                binding.passwordEditText.text.toString()
            )
        }
        binding.checkBoxShowPassword.setOnCheckedChangeListener { checkBoxShowPassword, isChecked ->
            val passwordEditText = binding.passwordEditText
            if (isChecked) {
                // Show Password
                passwordEditText.inputType = InputType.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD
            } else {
                // Hide Password
                passwordEditText.inputType = InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_VARIATION_PASSWORD
            }
        }
    }

    private fun loginUser(email: String, password: String) {
        auth.signInWithEmailAndPassword(email, password)
            .addOnCompleteListener(this) { task ->
                if (task.isSuccessful) {
                    // Sign in success, update UI with the signed-in user's information
                    val user = auth.currentUser
                    user?.let { currentUser ->
                        val intent = Intent(this@LoginActivity, MainActivity::class.java)
                        intent.putExtra("email", currentUser.email) // Send email to MainActivity
                        startActivity(intent)
                    } ?: run {
                        Log.e("LoginActivity", "signInWithEmail: No user found after successful login")
                        Toast.makeText(baseContext, "No user found after successful login", Toast.LENGTH_SHORT).show()
                    }
                } else {
                    // If sign in fails, display a message to the user.
                    Log.e("LoginActivity", "signInWithEmail:failure", task.exception)
                    Toast.makeText(baseContext, "Authentication failed.", Toast.LENGTH_SHORT).show()
                }
            }
    }
}
