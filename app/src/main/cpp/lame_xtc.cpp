#include <jni.h>
#include <string>
#include <android/log.h>
#include <lame.h>

std::string jstring2string(JNIEnv* env, jstring jStr) {
    if (!jStr) {
        return "";
    }

    const jclass stringClass = env->GetObjectClass(jStr);
    const jmethodID getBytes = env->GetMethodID(stringClass, "getBytes", "(Ljava/lang/String;)[B");
    const jbyteArray stringJbytes = (jbyteArray)env->CallObjectMethod(jStr, getBytes, env->NewStringUTF("UTF-8"));

    size_t length = (size_t)env->GetArrayLength(stringJbytes);
    jbyte* pBytes = env->GetByteArrayElements(stringJbytes, NULL);

    std::string ret = std::string((char*)pBytes, length);
    env->ReleaseByteArrayElements(stringJbytes, pBytes, JNI_ABORT);

    env->DeleteLocalRef(stringJbytes);
    env->DeleteLocalRef(stringClass);
    return ret;
}

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
    int read, i, samples;

    long cumulative_read = 0;

    const int PCM_SIZE = 8192;
    const int MP3_SIZE = 8192;

    // 输出左右声道
    short int pcm_l[PCM_SIZE], pcm_r[PCM_SIZE];
    unsigned char mp3_buffer[MP3_SIZE];

    //input输入MP3文件
    FILE *mp3 = fopen(jstring2string(env,mp3Path).c_str(), "rb");
    FILE *pcm = fopen(jstring2string(env,pcmPath).c_str(), "wb");
    fseek(mp3, 0, SEEK_SET);

    lame_t lame = lame_init();
    lame_set_decode_only(lame, 1);

    hip_t hip = hip_decode_init();

    mp3data_struct mp3data;
    memset(&mp3data, 0, sizeof(mp3data));

    int nChannels = -1;
    int mp3_len;

    while ((read = fread(mp3_buffer, sizeof(char), MP3_SIZE, mp3)) > 0) {
        mp3_len = read;
        cumulative_read += read * sizeof(char);
        do{
            samples = hip_decode1_headers(hip, mp3_buffer, mp3_len, pcm_l, pcm_r, &mp3data);
            // 头部解析成功
            if(mp3data.header_parsed == 1){
                nChannels = mp3data.stereo;
            }

            if(samples > 0){
                for(i = 0 ; i < samples; i++){
                    fwrite((char*)&pcm_l[i], sizeof(char), sizeof(pcm_l[i]), pcm);
                    if(nChannels == 2){
                        fwrite((char*)&pcm_r[i], sizeof(char), sizeof(pcm_r[i]), pcm);
                    }
                }
            }
            mp3_len = 0;
        }while(samples>0);
    }
    hip_decode_exit(hip);
    lame_close(lame);
    fclose(mp3);
    fclose(pcm);
    return 0;
}