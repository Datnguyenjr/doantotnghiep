package com.example.appgiamsat;

import android.os.Bundle;
import android.widget.*;
import androidx.appcompat.app.AppCompatActivity;

import com.google.firebase.database.*;

import java.util.HashMap;

public class AutoActivity extends AppCompatActivity {

    EditText edtTemp, edtLux, edtLight, edtFeed;
    Button btnSave;

    DatabaseReference autoRef;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_auto);

        edtTemp = findViewById(R.id.edtTemp);
        edtLux = findViewById(R.id.edtLux);
        edtFeed = findViewById(R.id.edtFeed);
        btnSave = findViewById(R.id.btnSave);

        autoRef = FirebaseDatabase.getInstance().getReference("auto");

        // ================== LOAD DATA CŨ ==================
        autoRef.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot snapshot) {

                if (snapshot.exists()) {

                    String temp = snapshot.child("temp_threshold").getValue() + "";
                    String lux = snapshot.child("lux_threshold").getValue() + "";
                    String feed = snapshot.child("feed_hour").getValue() + "";

                    edtTemp.setHint("🌡 Temp (0-100 °C | old: " + temp + ")");
                    edtLux.setHint("💡 Lux (0-4095 | old: " + lux + ")");
                    edtFeed.setHint("🍽 Feed hour (0-24h | old: " + feed + ")");
                }
            }

            @Override
            public void onCancelled(DatabaseError error) {}
        });

        // ================== SAVE ==================
        btnSave.setOnClickListener(v -> {

            try {
                String tempStr = edtTemp.getText().toString().trim();
                String luxStr = edtLux.getText().toString().trim();
                String feedStr = edtFeed.getText().toString().trim();

                HashMap<String, Object> map = new HashMap<>();

                // ===== TEMP (0 - 100 °C) =====
                if (!tempStr.isEmpty()) {
                    int temp = Integer.parseInt(tempStr);

                    if (temp < 0 || temp > 100) {
                        Toast.makeText(this,
                                "Temp phải từ 0 - 100°C",
                                Toast.LENGTH_SHORT).show();
                        return;
                    }
                    map.put("temp_threshold", temp);
                }

                // ===== LUX (0 - 4095) =====
                if (!luxStr.isEmpty()) {
                    int lux = Integer.parseInt(luxStr);

                    if (lux < 0 || lux > 4095) {
                        Toast.makeText(this,
                                "Lux phải từ 0 - 4095",
                                Toast.LENGTH_SHORT).show();
                        return;
                    }
                    map.put("lux_threshold", lux);
                }

                // ===== FEED HOUR (0 - 24h) =====
                if (!feedStr.isEmpty()) {
                    int feed = Integer.parseInt(feedStr);

                    if (feed < 0 || feed > 24) {
                        Toast.makeText(this,
                                "Giờ cho ăn phải từ 0 - 24h",
                                Toast.LENGTH_SHORT).show();
                        return;
                    }
                    map.put("feed_hour", feed);
                }

                autoRef.updateChildren(map);

                Toast.makeText(this,
                        "Saved successfully!",
                        Toast.LENGTH_SHORT).show();

            } catch (Exception e) {
                Toast.makeText(this,
                        "Input không hợp lệ!",
                        Toast.LENGTH_SHORT).show();
                e.printStackTrace();
            }
        });
    }
}