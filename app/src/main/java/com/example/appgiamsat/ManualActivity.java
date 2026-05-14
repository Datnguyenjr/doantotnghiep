package com.example.appgiamsat;

import android.os.Bundle;
import android.widget.Switch;
import androidx.appcompat.app.AppCompatActivity;

import com.google.firebase.database.*;

public class ManualActivity extends AppCompatActivity {

    Switch swFan, swLed, swServo;
    DatabaseReference controlRef;

    boolean isInit = true; // tránh trigger khi load dữ liệu

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_manual);

        swFan = findViewById(R.id.swFan);
        swLed = findViewById(R.id.swLed);
        swServo = findViewById(R.id.swServo);

        controlRef = FirebaseDatabase.getInstance().getReference("control");

        // ================== LOAD STATE TỪ FIREBASE ==================
        controlRef.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot snapshot) {

                if (!snapshot.exists()) return;

                Integer fan = snapshot.child("fan").getValue(Integer.class);
                Integer led = snapshot.child("led").getValue(Integer.class);
                Integer servo = snapshot.child("servo").getValue(Integer.class);

                if (fan != null)
                    swFan.setChecked(fan == 1);

                if (led != null)
                    swLed.setChecked(led == 1);

                if (servo != null)
                    swServo.setChecked(servo == 1);

                isInit = false; // load xong mới cho phép user điều khiển
            }

            @Override
            public void onCancelled(DatabaseError error) {}
        });

        // ================== FAN ==================
        swFan.setOnCheckedChangeListener((buttonView, val) -> {
            if (isInit) return;
            controlRef.child("fan").setValue(val ? 1 : 0);
        });

        // ================== LED ==================
        swLed.setOnCheckedChangeListener((buttonView, val) -> {
            if (isInit) return;
            controlRef.child("led").setValue(val ? 1 : 0);
        });

        // ================== SERVO ==================
        swServo.setOnCheckedChangeListener((buttonView, val) -> {
            if (isInit) return;
            controlRef.child("servo").setValue(val ? 1 : 0);
        });
    }
}