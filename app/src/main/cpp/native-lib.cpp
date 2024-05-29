#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/native_window_jni.h>
#include "MNQueue.h"


#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,"初始化层",__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_INFO,"h264层",__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_INFO,"解码层",__VA_ARGS__)
#define LOGV(...) __android_log_print(ANDROID_LOG_INFO,"同步层",__VA_ARGS__)
#define LOGQ(...) __android_log_print(ANDROID_LOG_INFO,"队列层",__VA_ARGS__)
#define LOGA(...) __android_log_print(ANDROID_LOG_INFO,"音频层",__VA_ARGS__)

extern "C" {
#include "include/libavcodec/avcodec.h"
#include "include/libavformat/avformat.h"
#include "include/libavutil/avutil.h"
#include "include/libavutil/frame.h"
#include "include/libavutil/imgutils.h"
#include "include/libswscale/swscale.h"
#include "include/libavutil/time.h"
#include "include/libswresample/swresample.h"
#include "include/libavutil/channel_layout.h"
}


AVFormatContext * avFormatContext;
AVCodecContext *videoContext;
AVCodecContext *audioContext;
int videoIndex =-1;
int audioIndex =-1;
//视频队列和音频队列
MNQueue *videoQueue;
MNQueue *audioQueue;

//解码后的数据
uint8_t *out_buffer;

//RGB帧
AVFrame *rgbFrame;

//视频宽高
int videoWidth;
int videoHeight;

//swsContext
SwsContext *swsContext;
SwrContext *swrContext;
//nativeWindow
ANativeWindow *nativeWindow;
//nativeWindow_Buffer
ANativeWindow_Buffer nativeWindow_buffer;

//全局thiz
jobject mthiz;

//回调方法
jmethodID playTrack;
_JavaVM *javaVM=NULL;
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    jint result = -1;
    javaVM = vm;
    JNIEnv *env;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return result;
    }
    return JNI_VERSION_1_6;
}

//读取线程方法
bool isStart = false;
void *decodePacket(void *pVoid){
    LOGI("==========解码线程");
    while(isStart){
        if(audioQueue->size()>50||videoQueue->size()>50){
            continue;
        }
        //创建avpacket 空容器
        AVPacket *avPacket=av_packet_alloc();
        if(av_read_frame(avFormatContext,avPacket)<0){
            break;
        }

        if(avPacket->stream_index==videoIndex){
            //视频流方法
            videoQueue->push(avPacket);


        }else if(avPacket->stream_index==audioIndex){
            //音频流方法
            audioQueue->push(avPacket);
        }


    }
    return NULL;
}

//视频解码线程
void *decodeVideo(void *pVoid){
    LOGI("==========视频解码线程");
    while(isStart){
        AVPacket *videoPacket=av_packet_alloc();
        videoQueue->getAvPacket(videoPacket);
        LOGI("视频解码");
        //解码
        int ret=avcodec_send_packet(videoContext,videoPacket);
        //挂载数据流
        AVFrame *videoFrame=av_frame_alloc();
        ret=avcodec_receive_frame(videoContext,videoFrame);

        LOGI("解码成功");
        //rgb转换yuva
        sws_scale(swsContext,videoFrame->data,videoFrame->linesize,0,videoHeight,rgbFrame->data,rgbFrame->linesize);

        //绘制
        ANativeWindow_lock(nativeWindow,&nativeWindow_buffer,NULL);
        uint8_t *dst= (uint8_t *) nativeWindow_buffer.bits;
        int destStride=nativeWindow_buffer.stride*4;
        uint8_t *src=rgbFrame->data[0];
        int srcStride=rgbFrame->linesize[0];
        for(int i=0;i<videoHeight;i++){
            memcpy(dst+i*destStride,src+i*srcStride,srcStride);
        }
        ANativeWindow_unlockAndPost(nativeWindow);
        av_frame_free(&videoFrame);
        av_free(videoFrame);
        videoFrame=NULL;
        av_packet_free(&videoPacket);
        av_free(videoPacket);
        videoPacket=NULL;

    }
    return NULL;
}

//音频解码线程
void *decodeAudio(void *pVoid) {
    LOGI("==========音频解码线程");
    AVFrame *audioFrame=av_frame_alloc();
    swrContext=swr_alloc();
    //设置参数
    uint64_t out_ch_layout=AV_CH_LAYOUT_STEREO;
    enum AVSampleFormat out_format=AV_SAMPLE_FMT_S16;
    int out_sample_rate=audioContext->sample_rate;
    swr_alloc_set_opts(swrContext,out_ch_layout,out_format,out_sample_rate,
                       audioContext->channel_layout,audioContext->sample_fmt,audioContext->sample_rate,0,NULL);
    swr_init(swrContext);

    int out_channels_nb=av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);

    uint8_t *out_buffer=(uint8_t *)av_malloc(44100*2);
    while(isStart) {
        AVPacket *audioPacket = av_packet_alloc();
        audioQueue->getAvPacket(audioPacket);
        LOGI("音频 数据包 大小 %d", audioPacket->size);
        //解码
        int ret = avcodec_send_packet(audioContext, audioPacket);
        //挂载数据流

        ret = avcodec_receive_frame(audioContext, audioFrame);

//        if(ret>=0){
//            swr_convert(swrContext,&out_buffer,44100*2,(const uint8_t **)audioFrame->data,audioFrame->nb_samples);
//            int size= av_samples_get_buffer_size(NULL,out_channels_nb,audioFrame->nb_samples,out_format,1);
//        //回调java方法
//         JNIEnv *jniEnv;
//         LOGD("获取JNIEnv");
//         if(javaVM->AttachCurrentThread(&jniEnv,NULL)!=JNI_OK){
//             continue;
//         }
//         jbyteArray byteArrays =jniEnv->NewByteArray(size);
//         jniEnv->SetByteArrayRegion(byteArrays,0,size,(const jbyte *)out_buffer);
//         jniEnv->CallVoidMethod(mthiz,playTrack,byteArrays,size);
//         jniEnv->DeleteLocalRef(byteArrays);
//            javaVM->DetachCurrentThread();
//    }
           LOGI("解码后 大小:%d",audioFrame->pkt_size);
        }

        LOGI("解码成功");

        //重采样


    av_frame_free(&audioFrame);
    av_free(audioFrame);
    audioFrame=NULL;
    avcodec_close(audioContext);
    avformat_close_input(&avFormatContext);
    return NULL;

}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_ijkplayer_MNPlayer_play(JNIEnv *env, jobject thiz, jstring url_, jobject surface) {
    // TODO: implement play()
    const char *url = env->GetStringUTFChars(url_,0);//在 env->ReleaseStringUTFChars(url_,url);处释放
    LOGI("url:%s",url);
    //直播流
    avformat_network_init();
    avFormatContext=avformat_alloc_context();
    //打开视频
    if (avformat_open_input(&avFormatContext, url, NULL, NULL) < 0) {
        LOGI("无法打开文件: %s", url);
    }else{
        //验证视频是否可用
        if (avformat_find_stream_info(avFormatContext, NULL) < 0) {
            LOGI("无法获取流信息");
            avformat_close_input(&avFormatContext);
        }
    }
    LOGI("流数目:%d",avFormatContext->nb_streams);

    //视频流
    for(int i=0;i<avFormatContext->nb_streams;i++) {
        if (AVMEDIA_TYPE_VIDEO == avFormatContext->streams[i]->codecpar->codec_type) {//视频流
            videoIndex = i;
            LOGI("视频%d", i);
            AVCodecParameters *parameters = avFormatContext->streams[i]->codecpar;
            LOGI("宽度%d", parameters->width);
            LOGI("高度%d", parameters->height);
            //解码器
            AVCodec * dec = avcodec_find_decoder(parameters->codec_id);
            //解码器 上下文
            videoContext = avcodec_alloc_context3(dec);
            //设置上下文参数
            avcodec_parameters_to_context(videoContext, parameters);
            //开启解码
            avcodec_open2(videoContext, dec, 0);

        } else if (AVMEDIA_TYPE_AUDIO == avFormatContext->streams[i]->codecpar->codec_type) {//音频流
            audioIndex = i;
            LOGI("音频%d", i);
            AVCodecParameters *parameters = avFormatContext->streams[i]->codecpar;
            LOGI("宽度%d", parameters->width);
            LOGI("高度%d", parameters->height);
            //解码器
            AVCodec * dec = avcodec_find_decoder(parameters->codec_id);
            //解码器 上下文
            audioContext = avcodec_alloc_context3(dec);
            //设置上下文参数
            avcodec_parameters_to_context(audioContext, parameters);
            //开启解码
            avcodec_open2(audioContext, dec, 0);
        }
    }
        //获取宽高
        videoWidth=videoContext->width;
        videoHeight=videoContext->height;

        //回调jmethodID
        jmethodID onSizeChanged=env->GetMethodID(env->GetObjectClass(thiz),"onSizeChanged","(II)V");
        env->CallVoidMethod(thiz,onSizeChanged,videoWidth,videoHeight);

        //回调jmethodID
//        jmethodID createAudio=env->GetMethodID(env->GetObjectClass(thiz),"createTrack","(II)V");
//        env->CallVoidMethod(thiz,createAudio,videoContext->sample_rate,audioContext->channels);
//
//        mthiz=env->NewGlobalRef(thiz);
//        playTrack = env->GetMethodID(env->GetObjectClass(thiz),"playTrack","([BI)V");


        rgbFrame = av_frame_alloc();
        //设置rgbFrame的参数
        int numBytes =av_image_get_buffer_size(AV_PIX_FMT_RGBA,videoWidth,videoHeight,1);
        //初始化out_buffer
        out_buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

        //将rgbFrame的数据设置到out_buffer
        av_image_fill_arrays(rgbFrame->data,rgbFrame->linesize,out_buffer,AV_PIX_FMT_RGBA,videoWidth,videoHeight,1);

        //初始化swsContext
        swsContext=sws_getContext(videoWidth,videoHeight,videoContext->pix_fmt,videoWidth,videoHeight,AV_PIX_FMT_RGBA,SWS_BICUBIC,NULL,NULL,NULL);

        nativeWindow=ANativeWindow_fromSurface(env,surface);
        ANativeWindow_setBuffersGeometry(nativeWindow,videoWidth,videoHeight,WINDOW_FORMAT_RGBA_8888);
        //初始化队列
        audioQueue=new MNQueue;
        videoQueue=new MNQueue;
        //开辟线程
        isStart=true;
        pthread_t thread_decode;
        pthread_t thread_video;
        pthread_t thread_audio;

        //读取线程
        pthread_create(&thread_decode,NULL, decodePacket,NULL);
        //视频解码线程
        pthread_create(&thread_video,NULL, decodeVideo,NULL);
        //音频解码线程
        pthread_create(&thread_audio,NULL, decodeAudio,NULL);

    env->ReleaseStringUTFChars(url_,url);
}