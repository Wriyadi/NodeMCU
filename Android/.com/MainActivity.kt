package com.example.firebase_iot

import android.content.Intent
import android.os.Bundle
import android.text.Editable
import android.util.Log
import android.widget.Button
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.example.firebase_iot.databinding.ActivityMainBinding
import com.google.firebase.auth.FirebaseAuth
import com.google.firebase.database.DatabaseReference
import com.google.firebase.database.FirebaseDatabase

class MainActivity : AppCompatActivity() {
    private lateinit var binding: ActivityMainBinding
    private lateinit var database: DatabaseReference
    private lateinit var auth: FirebaseAuth

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Initialize Firebase Database reference
        database = FirebaseDatabase.getInstance().getReference("Output")
        // Initialize Firebase Auth
        auth = FirebaseAuth.getInstance()

        val email = intent.getStringExtra("email")
        email?.let {
            binding.uid.text = it.toEditable()
        } ?: run {
            Log.e("MainActivity", "Email extra is null")
            Toast.makeText(this, "Email extra is null", Toast.LENGTH_SHORT).show()
        }
        // Set onClickListeners for buttons
        binding.button1.setOnClickListener { setData(binding.button1, "16") }
        binding.button2.setOnClickListener { setData(binding.button2, "5") }
        binding.button3.setOnClickListener { setData(binding.button3, "4") }
        binding.button4.setOnClickListener { setData(binding.button4, "0") }
        binding.button5.setOnClickListener { setData(binding.button5, "2") }
        binding.button6.setOnClickListener { setData(binding.button6, "14") }
        binding.getButton.setOnClickListener { getTempHum() } // Set click listener for getButton
        binding.logoutButton.setOnClickListener { logoutUser() } // Set click listener for logoutButton
    }

    private fun setData(button: Button, childId: String) {
        // Toggle the value in the database
        database.child(childId).get().addOnSuccessListener { dataSnapshot ->
            val currentValue = dataSnapshot.getValue(Int::class.java)
            val newValue: Int = if (currentValue == 0) 1 else 0
            database.child(childId).setValue(newValue)
                .addOnSuccessListener {
                    Toast.makeText(this@MainActivity, "Success", Toast.LENGTH_SHORT).show()
                    updateButtonUI(button, newValue) // Call updateButtonUI method
                }
                .addOnFailureListener {
                    Toast.makeText(this@MainActivity, "Failed", Toast.LENGTH_SHORT).show()
                }
        }.addOnFailureListener {
            Toast.makeText(this@MainActivity, "Failed", Toast.LENGTH_SHORT).show()
        }
    }

    private fun updateButtonUI(button: Button, value: Int?) {
        // Update button text based on the retrieved value
        button.text = if (value == 1) "ON" else "OFF"
    }

    private fun getTempHum() {
        val temperatureChildId = "temp"
        val humidityChildId = "hum"

        database.child(temperatureChildId).get().addOnCompleteListener { temperatureTask ->
            if (temperatureTask.isSuccessful) {
                val temperatureSnapshot = temperatureTask.result
                if (temperatureSnapshot != null && temperatureSnapshot.exists()) {
                    val temperature = temperatureSnapshot.value.toString().toFloat()
                    val temperatureString = "$temperature °C"
                    this.binding.temperature.setText(temperatureString)
                    Toast.makeText(this@MainActivity, "Temperature data retrieved successfully", Toast.LENGTH_SHORT).show()
                } else {
                    Toast.makeText(this@MainActivity, "Temperature data not available", Toast.LENGTH_SHORT).show()
                }
            } else {
                Toast.makeText(this@MainActivity, "Failed to retrieve temperature data", Toast.LENGTH_SHORT).show()
            }
        }

        database.child(humidityChildId).get().addOnCompleteListener { humidityTask ->
            if (humidityTask.isSuccessful) {
                val humiditySnapshot = humidityTask.result
                if (humiditySnapshot != null && humiditySnapshot.exists()) {
                    val humidity = humiditySnapshot.value.toString().toFloat()
                    val humidityString = "$humidity %"
                    this.binding.humidity.setText(humidityString)
                    Toast.makeText(this@MainActivity, "Humidity data retrieved successfully", Toast.LENGTH_SHORT).show()
                } else {
                    Toast.makeText(this@MainActivity, "Humidity data not available", Toast.LENGTH_SHORT).show()
                }
            } else {
                Toast.makeText(this@MainActivity, "Failed to retrieve humidity data", Toast.LENGTH_SHORT).show()
            }
        }
    }


    private fun logoutUser() {
        // Sign out the user
        auth.signOut()
        // Navigate back to LoginActivity
        startActivity(Intent(this@MainActivity, LoginActivity::class.java))
        finish() // Finish MainActivity
    }
}

private fun String.toEditable(): Editable = Editable.Factory.getInstance().newEditable(this)
