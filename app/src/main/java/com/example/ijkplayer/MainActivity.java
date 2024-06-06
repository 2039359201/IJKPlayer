package com.example.ijkplayer;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;
import com.example.ijkplayer.databinding.ActivityMainBinding;
import android.Manifest;
import android.widget.Toast;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    SurfaceView surfaceView;
    Surface surface;
    public boolean checkPermission(){
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M && checkSelfPermission(
                Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{
                    Manifest.permission.READ_EXTERNAL_STORAGE,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE,
            },1);
        }
        return false;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Thread thread = new Thread(){
            @Override
            public void run(){
                super.run();
            }
        };
        checkPermission();
        surfaceView = (SurfaceView) findViewById(R.id.surface);
        surfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(@NonNull SurfaceHolder holder) {
                surface=holder.getSurface();
            }

            @Override
            public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {

            }

            @Override
            public void surfaceDestroyed(@NonNull SurfaceHolder holder) {

            }
        });

        playButton = findViewById(R.id.pause);

        //注册点击事件
        // 设置 SurfaceView 点击监听器
        surfaceView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (playButton.getVisibility() == View.VISIBLE) {
                    playButton.setVisibility(View.GONE);
                } else {
                    playButton.setVisibility(View.VISIBLE);
                }
                if (mnPlayer!=null) {
                    mnPlayer.pause();//暂停继续
                }else{
                    // 提示用户播放器未初始化
                    Toast.makeText(MainActivity.this, "Player not initialized", Toast.LENGTH_SHORT).show();
                }

            }
        });

    }

    public void play(View view){
        String folderurl = new File(Environment.getExternalStorageDirectory(),"input.mp4").getAbsolutePath();
        //网络地址
        //String folderurl = "https://devstreaming-cdn.apple.com/videos/streaming/examples/img_bipbop_adv_example_fmp4/master.m3u8";
        mnPlayer = new MNPlayer(surfaceView);
        playButton.setVisibility(View.GONE);
        // 调用 MNPlayer 的 play 方法
        mnPlayer.play(folderurl,surface);

    }


    //横竖屏切换
    @Override
    public void onConfigurationChanged(@NonNull android.content.res.Configuration newConfig) {
        super.onConfigurationChanged(newConfig);

        ViewGroup.LayoutParams params = surfaceView.getLayoutParams();
        if (newConfig.orientation == android.content.res.Configuration.ORIENTATION_LANDSCAPE) {
            //横屏
            Toast.makeText(MainActivity.this, "横屏", Toast.LENGTH_SHORT).show();
            params.height = surfaceView.getContext().getResources().getDisplayMetrics().heightPixels;
            params.width = surfaceView.getContext().getResources().getDisplayMetrics().widthPixels;
            if(mnPlayer!=null){
                params.width = (int) (params.height*mnPlayer.videoRatio);
            }
            //隐藏状态栏
            getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_FULLSCREEN);
        } else {
            //竖屏
            Toast.makeText(MainActivity.this, "竖屏", Toast.LENGTH_SHORT).show();
            params.height = surfaceView.getContext().getResources().getDisplayMetrics().heightPixels;
            params.width = surfaceView.getContext().getResources().getDisplayMetrics().widthPixels;
            if(mnPlayer!=null){
                params.height = (int) (params.width/mnPlayer.videoRatio);
            }
            //显示状态栏
            getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_VISIBLE);
        }
        mnPlayer.surfaceView.setLayoutParams(params);
    }




    private Button playButton;
    private MNPlayer mnPlayer;

}