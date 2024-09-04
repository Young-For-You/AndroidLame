package com.xtc.lame;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.xtc.common.util.LameInterface;
import com.xtc.lame.databinding.ActivityMainBinding;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    private ActivityMainBinding binding;
    private Button mBtRecordPcm;
    private Button mBtPcmToMp3;
    private Button mBtMp3ToPcm;
    private AudioRecorder mAudioRecorder;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        LameInterface lameInterface = new LameInterface();

        // Example of a call to a native method
        TextView tv = binding.sampleText;
        tv.setText(lameInterface.stringFromJNI());

        mBtRecordPcm = binding.btRecordPcm;
        mBtPcmToMp3 = binding.btPcmToMp3;
        mBtMp3ToPcm = binding.btMp3ToPcm;

        // 录制
        mBtRecordPcm.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                checkPermissionAndStartRecord();
            }
        });

        // mp3 -> pcm
        mBtMp3ToPcm.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mp3ToPcm();
            }
        });

        // pcm -> mp3
        mBtPcmToMp3.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                pcmToMp3();
            }
        });
    }

    private void pcmToMp3() {
        String pcmPath = getExternalCacheDir() + File.separator + "record_pcm.pcm";
        String mp3Path = getExternalCacheDir() + File.separator + "decode_mp3.mp3";
        LameInterface lameInterface = new LameInterface();
        new Thread(new Runnable() {
            @Override
            public void run() {
                File mp3File = new File(mp3Path);
                mp3File.deleteOnExit();
                int code = lameInterface.pcmToMp3(pcmPath, mp3Path, 44100 / 2, 2, 16, 5);
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if (code == 0) {
                            Toast.makeText(MainActivity.this, "convert success", Toast.LENGTH_SHORT).show();
                        }
                    }
                });
            }
        }).start();
    }

    private void mp3ToPcm() {
        String mp3Path = getExternalCacheDir() + File.separator + "一定要爱你.mp3";
        String pcmPath = getExternalCacheDir() + File.separator + "一定要爱你.pcm";
    }


    private void checkPermissionAndStartRecord() {
        if (
                ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED ||
                ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED ||
                ContextCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO) != PackageManager.PERMISSION_GRANTED
        ) {
            ActivityCompat.requestPermissions(
                    this,
                    new String[]{
                            Manifest.permission.READ_EXTERNAL_STORAGE,
                            Manifest.permission.WRITE_EXTERNAL_STORAGE,
                            Manifest.permission.RECORD_AUDIO},
                    1000);
        } else {
            record();
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
    }

    @SuppressLint("SetTextI18n")
    private void record() {
        if (mAudioRecorder != null && mAudioRecorder.isRecording()) {
            mBtRecordPcm.setText("RECORD PCM");
            mAudioRecorder.stopRecording();
        } else {
            mBtRecordPcm.setText("RECORDING...(click stop)");
            mAudioRecorder = new AudioRecorder(this);
            mAudioRecorder.initAndStartAudioRecord();
        }
    }
}