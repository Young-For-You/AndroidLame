#include <jni.h>
#include <string>
#include <android/log.h>
#include <lame.h>

extern "C"
JNIEXPORT jstring JNICALL
Java_com_xtc_common_util_LameInterface_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT int JNICALL
Java_com_xtc_common_util_LameInterface_pcmToMp3(JNIEnv *env, jobject, jstring pcmPath, jstring mp3Path, jint sampleRate, jint channelCount, jint audioFormatBit, jint quality) {
    const char *pcm_path = env->GetStringUTFChars(pcmPath, NULL);
    const char *mp3_path = env->GetStringUTFChars(mp3Path, NULL);

    FILE *pcmFile = fopen(pcm_path, "rb");
    FILE *mp3File = fopen(mp3_path, "wb");

    if (pcmFile == NULL || mp3File == NULL) {
        if (pcmFile) fclose(pcmFile);
        if (mp3File) fclose(mp3File);
        env->ReleaseStringUTFChars(pcmPath, pcm_path);
        env->ReleaseStringUTFChars(mp3Path, mp3_path);
        return -1;
    }

    lame_t lame = lame_init();
    lame_set_in_samplerate(lame, sampleRate);
    lame_set_num_channels(lame, channelCount);
    lame_set_brate(lame, audioFormatBit);
    lame_set_quality(lame, quality);
    lame_init_params(lame);

    short pcm_buffer[8192];
    unsigned char mp3_buffer[8192];

    int read, write;

    do {
        read = fread(pcm_buffer, sizeof(short) * channelCount, sizeof(pcm_buffer) / (sizeof(short) * channelCount), pcmFile);
        if (read == 0) {
            write = lame_encode_flush(lame, mp3_buffer, sizeof(mp3_buffer));
        } else {
            if (channelCount == 2) {
                write = lame_encode_buffer_interleaved(lame, pcm_buffer, read, mp3_buffer, sizeof(mp3_buffer));
            } else {
                write = lame_encode_buffer(lame, pcm_buffer, pcm_buffer, read, mp3_buffer, sizeof(mp3_buffer));
            }
        }
        fwrite(mp3_buffer, write, 1, mp3File);
    } while (read != 0);

    lame_mp3_tags_fid(lame, mp3File);

    lame_close(lame);
    fclose(pcmFile);
    fclose(mp3File);

    env->ReleaseStringUTFChars(pcmPath, pcm_path);
    env->ReleaseStringUTFChars(mp3Path, mp3_path);
    return 0;
}

extern "C"
JNIEXPORT int JNICALL
Java_com_xtc_common_util_LameInterface_mp3ToPcm(JNIEnv *env, jobject, jstring mp3Path, jstring pcmPath) {
    return 0;
}