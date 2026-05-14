package com.example.appgiamsat;

import android.content.Intent;
import android.os.Bundle;
import android.widget.Button;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import com.google.firebase.database.*;

public class MainActivity extends AppCompatActivity {

    TextView txtTemp, txtHum, txtLux, txtMode;
    Button btnAuto, btnManual;

    DatabaseReference sensorRef, modeRef;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // ===== ÁNH XẠ VIEW =====
        txtTemp = findViewById(R.id.txtTemp);
        txtHum = findViewById(R.id.txtHum);
        txtLux = findViewById(R.id.txtLux);
        txtMode = findViewById(R.id.txtMode);

        btnAuto = findViewById(R.id.btnAuto);
        btnManual = findViewById(R.id.btnManual);

        // ===== FIREBASE =====
        sensorRef = FirebaseDatabase.getInstance().getReference("sensor");
        modeRef = FirebaseDatabase.getInstance().getReference("control/mode");

        // ================= SENSOR =================
        sensorRef.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot snapshot) {

                Object tObj = snapshot.child("temp").getValue();
                Object hObj = snapshot.child("hum").getValue();
                Object lObj = snapshot.child("lux").getValue();

                String t = (tObj != null) ? tObj.toString() : "--";
                String h = (hObj != null) ? hObj.toString() : "--";
                String l = (lObj != null) ? lObj.toString() : "--";

                txtTemp.setText("\uD83C\uDF21 TEMPERATURE: " + t + " °C");
                txtHum.setText("\uD83D\uDCA7 HUMIDITY: " + h + " %");
                txtLux.setText("\uD83D\uDCA1 LIGHT: " + l);
            }

            @Override
            public void onCancelled(DatabaseError error) {}
        });

        // ================= MODE REALTIME =================
        modeRef.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot snapshot) {

                String mode = snapshot.getValue(String.class);

                if ("auto".equals(mode)) {

                    txtMode.setText("MODE: AUTO");
                    //txtMode.setBackgroundColor(0xFF4CAF50); // xanh lá

                    btnAuto.setAlpha(0.5f);
                    btnManual.setAlpha(1f);

                } else {

                    txtMode.setText("MODE: MANUAL");
                    //txtMode.setBackgroundColor(0xFF2196F3); // xanh dương

                    btnAuto.setAlpha(1f);
                    btnManual.setAlpha(0.5f);
                }
            }

            @Override
            public void onCancelled(DatabaseError error) {}
        });

        // ================= BUTTON =================
        btnAuto.setOnClickListener(v -> {
            modeRef.setValue("auto");
            startActivity(new Intent(this, AutoActivity.class));
        });

        btnManual.setOnClickListener(v -> {
            modeRef.setValue("manual");
            startActivity(new Intent(this, ManualActivity.class));
        });
    }
}