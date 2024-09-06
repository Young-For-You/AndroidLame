package com.xtc.common.util;

/**
 * New Class.
 * Date: 2024/9/4.
 * Version change description.
 *
 * @author taoling
 */
public class LameInterface {

    static {
        System.loadLibrary("lame");
    }

    public native String stringFromJNI();
    public native int pcmToMp3(String pcmPath, String mp3Path, int sampleRate, int channels, int bitRate, int quality);
    public native int mp3ToPcm(String mp3Path, String pcmPath);
}
