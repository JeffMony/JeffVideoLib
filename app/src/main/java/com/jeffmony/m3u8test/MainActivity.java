package com.jeffmony.m3u8test;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import android.Manifest;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import com.github.florent37.runtimepermission.RuntimePermission;
import com.jeffmony.m3u8library.Constants;
import com.jeffmony.m3u8library.VideoProcessManager;
import com.jeffmony.m3u8library.listener.IVideoTransformListener;

import java.io.File;
import java.text.DecimalFormat;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    private EditText mSrcTxt;
    private EditText mDestTxt;
    private Button mConvertBtn;
    private TextView mTransformProgressTxt;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        RuntimePermission.askPermission(this)
                .request(Manifest.permission.READ_EXTERNAL_STORAGE,
                        Manifest.permission.WRITE_EXTERNAL_STORAGE)
                .onAccepted((result -> {
                    /**
                     * 权限已经申请成功
                     */
                })).ask();
        initViews();
    }

    private void initViews() {
        mSrcTxt = findViewById(R.id.src_path_txt);
        mDestTxt = findViewById(R.id.dest_path_txt);
        mConvertBtn = findViewById(R.id.convert_btn);
        mTransformProgressTxt = findViewById(R.id.video_transform_progress_txt);

        mConvertBtn.setOnClickListener(this);
    }

    private void doConvertVideo(String inputPath, String outputPath) {
        if (TextUtils.isEmpty(inputPath) || TextUtils.isEmpty(outputPath)) {
            Log.i(Constants.TAG, "InputPath or OutputPath is null");
            return;
        }
        File inputFile = new File(inputPath);
        if (!inputFile.exists()) {
            return;
        }
        File outputFile = new File(outputPath);
        if (!outputFile.exists()) {
            try {
                outputFile.createNewFile();
            } catch (Exception e) {
                Log.w(Constants.TAG, "Create file failed, exception = " + e);
                return;
            }
        }
        Log.i(Constants.TAG, "inputPath="+inputPath+", outputPath="+outputPath);
        VideoProcessManager.getInstance().transformM3U8ToMp4(inputPath, outputPath, new IVideoTransformListener() {

            @Override
            public void onTransformProgress(float progress) {
                Log.i(Constants.TAG, "onTransformProgress progress="+progress);
                DecimalFormat format = new DecimalFormat(".00");
                mTransformProgressTxt.setText("已经转换的进度: " + format.format(progress) + "%");
            }

            @Override
            public void onTransformFinished() {
                Log.i(Constants.TAG, "onTransformFinished");
                mTransformProgressTxt.setText("转换完成");
            }

            @Override
            public void onTransformFailed(int err) {
                Log.e(Constants.TAG, "onTransformFailed err="+err);
            }
        });
    }

    @Override
    public void onClick(View v) {
        if (v == mConvertBtn) {
            doConvertVideo(mSrcTxt.getText().toString(), mDestTxt.getText().toString());
        }
    }
}