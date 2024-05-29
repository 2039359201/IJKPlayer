package com.example.ijkplayer;

import android.media.AudioAttributes;
import android.media.AudioFormat;
import android.media.AudioTrack;
import android.view.Surface;
import android.view.SurfaceView;
import android.widget.RelativeLayout;

public class MNPlayer {
    static {
        System.loadLibrary("ijkplayer");
    }
    SurfaceView surfaceView;

    //Audio Track
    private AudioTrack audioTrack;
    public MNPlayer(SurfaceView surfaceView){
        this.surfaceView=surfaceView;
    }

    public native void play(String url,Surface surface);

    //通道数 采样位数 频率
//    public void createTrack(int sampleRateInHz,int nb_channals){
//        int channelConfig;
//        if(nb_channals==1){
//            channelConfig = android.media.AudioFormat.CHANNEL_OUT_MONO;
//        }else if(nb_channals==2){
//            channelConfig = android.media.AudioFormat.CHANNEL_OUT_STEREO;
//        }else{
//            channelConfig = android.media.AudioFormat.CHANNEL_OUT_MONO;
//        }
//        int minBufferSize = android.media.AudioTrack.getMinBufferSize(sampleRateInHz,channelConfig,android.media.AudioFormat.ENCODING_PCM_16BIT);
//        // 使用 AudioTrack.Builder 创建 AudioTrack 实例
//        AudioTrack audioTrack = new AudioTrack.Builder()
//                .setAudioAttributes(new AudioAttributes.Builder()
//                        .setUsage(AudioAttributes.USAGE_MEDIA)
//                        .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
//                        .build())
//                .setAudioFormat(new AudioFormat.Builder()
//                        .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
//                        .setSampleRate(sampleRateInHz)
//                        .setChannelMask(channelConfig)
//                        .build())
//                .setBufferSizeInBytes(minBufferSize)
//                .build();
//
//        audioTrack.play();
//
//    }
//    public void playTrack(byte[] audioData,int length){
//        audioTrack.write(audioData,0,length);
//    }

    public void onSizeChanged(int width,int height){
        float ratio = (float)width/height;
        int screenwidth = surfaceView.getContext().getResources().getDisplayMetrics().widthPixels;
        int videoWidth =0;
        int videoHeight =0;
        videoWidth = screenwidth;
        videoHeight = (int) (screenwidth/ratio);
        RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(videoWidth,videoHeight);
        //调整播放器大小
        surfaceView.setLayoutParams(params);
    }
}
