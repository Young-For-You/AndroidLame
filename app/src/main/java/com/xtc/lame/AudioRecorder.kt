package com.xtc.lame

import android.Manifest
import android.app.Activity
import android.content.pm.PackageManager
import android.media.AudioFormat
import android.media.AudioRecord
import android.media.MediaRecorder
import android.widget.Toast
import androidx.core.app.ActivityCompat
import java.io.File
import java.io.FileOutputStream
import java.io.IOException

/**
 * 音频录制类
 * Date: 2024/9/4.
 * Version change description.
 * @author taoling
 */
class AudioRecorder(private val context: Activity) {

    private var audioRecord: AudioRecord? = null
    private var bufferSize = 0
    var isRecording = false

    private val RECORD_SAMPLE_RATE = 44100
    private val RECORD_CHANNEL = AudioFormat.CHANNEL_IN_MONO
    private val RECORD_FORMAT = AudioFormat.ENCODING_PCM_16BIT
    private val RECORD_PCM_PATH = context.externalCacheDir?.path + File.separator + "record_pcm.pcm"

    fun initAndStartAudioRecord() {
        bufferSize = AudioRecord.getMinBufferSize(RECORD_SAMPLE_RATE, RECORD_CHANNEL, RECORD_FORMAT)
        if (ActivityCompat.checkSelfPermission(context, Manifest.permission.RECORD_AUDIO) != PackageManager.PERMISSION_GRANTED) {
            return
        }
        audioRecord = AudioRecord(
            MediaRecorder.AudioSource.MIC,
            RECORD_SAMPLE_RATE,
            RECORD_CHANNEL,
            RECORD_FORMAT,
            bufferSize
        )

        if (audioRecord?.state != AudioRecord.STATE_INITIALIZED) {
            Toast.makeText(context, "AudioRecord initialization failed", Toast.LENGTH_SHORT).show()
            return
        }

        startRecording()
    }

    private fun startRecording() {
        Thread {
            context.runOnUiThread {
                Toast.makeText(context, "start record", Toast.LENGTH_SHORT).show()
            }
            isRecording = true
            audioRecord?.startRecording()

            val audioData = ByteArray(bufferSize)
            var fos: FileOutputStream? = null

            try {
                fos = FileOutputStream(RECORD_PCM_PATH)
                while (isRecording) {
                    val read = audioRecord?.read(audioData, 0, bufferSize) ?: 0
                    if (read > 0) {
                        fos.write(audioData, 0, read)
                        fos.flush()
                    }
                }
                context.runOnUiThread {
                    Toast.makeText(context, "stop record", Toast.LENGTH_SHORT).show()
                }
                stopRecording()
            } catch (e: IOException) {
                e.printStackTrace()
            } finally {
                try {
                    fos?.close()
                } catch (e: IOException) {
                    e.printStackTrace()
                }
            }
        }.start()
    }

    fun stopRecording() {
        if (audioRecord?.recordingState == AudioRecord.RECORDSTATE_RECORDING) {
            audioRecord?.stop()
            audioRecord?.release()
            isRecording = false
        }
    }

}