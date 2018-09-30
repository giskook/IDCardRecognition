package com.arseeds.idcard;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.ColorMatrix;
import android.graphics.ColorMatrixColorFilter;
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.hardware.Camera;
import android.media.AudioManager;
import android.media.ToneGenerator;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.Toast;

import com.googlecode.tesseract.android.TessBaseAPI;

import java.io.ByteArrayOutputStream;
import java.io.IOException;

public class CameraActivity extends Activity implements SurfaceHolder.Callback , Camera.PreviewCallback {

    private SurfaceView sfv;
    private CameraManager cameraManager;
    private static final String TAG = "CameraActivity";
    private boolean hasSurface;

    //训练数据路径，必须包含tesseract文件夹
    static final String TESSBASE_PATH = Environment.getExternalStorageDirectory() + "/";
    //识别语言英文
    static final String DEFAULT_LANGUAGE = "eng";
    private ImageView iv_result;

    static {
        System.loadLibrary("swt");
    }
    private native Bitmap getTextRegion(Bitmap bmp);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            Window window = getWindow();
            window.addFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
        }

        setContentView(R.layout.activity_camera);
        try {
            initView();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void initView() throws IOException {
        sfv = (SurfaceView) findViewById(R.id.sfv);
        SurfaceHolder surfaceHolder = sfv.getHolder();
        iv_result = (ImageView) findViewById(R.id.iv_result);
        iv_result.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View view) {
                Log.wtf(TAG,"click");
            }
            });
        if (hasSurface) {
            // activity在paused时但不会stopped,因此surface仍旧存在；
            // surfaceCreated()不会调用，因此在这里初始化camera
            initCamera(surfaceHolder);
        } else {
            // 重置callback，等待surfaceCreated()来初始化camera
            surfaceHolder.addCallback(this);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        cameraManager.stopPreview();
        cameraManager.closeDriver();
            SurfaceHolder surfaceHolder = sfv.getHolder();
            surfaceHolder.removeCallback(this);

    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        try {
            initCamera(holder);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void initCamera(SurfaceHolder holder) throws IOException {
        cameraManager = new CameraManager();
        if (holder == null) {
            throw new IllegalStateException("No SurfaceHolder provided");
        }
        if (cameraManager.isOpen()) {
            return;
        }
        try {
            // 打开Camera硬件设备
            cameraManager.openDriver(holder, this);
            // 创建一个handler来打开预览，并抛出一个运行时异常
            cameraManager.startPreview(this);
        } catch (Exception ioe) {
            Log.d("zk", ioe.toString());

        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        hasSurface = false;
    }

    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
//        Log.d("zka", "bitmap is nlll");
//        return;
        camera.addCallbackBuffer(data);

        ByteArrayOutputStream baos;
        byte[] rawImage;
        Bitmap bitmap;
        Camera.Size previewSize = camera.getParameters().getPreviewSize();//获取尺寸,格式转换的时候要用到
        BitmapFactory.Options newOpts = new BitmapFactory.Options();
        newOpts.inJustDecodeBounds = true;
        YuvImage yuvimage = new YuvImage(
                data,
                ImageFormat.NV21,
                previewSize.width,
                previewSize.height,
                null);
        baos = new ByteArrayOutputStream();
        yuvimage.compressToJpeg(new Rect(0, 0, previewSize.width, previewSize.height), 100, baos);// 80--JPG图片的质量[0-100],100最高
        rawImage = baos.toByteArray();
        //将rawImage转换成bitmap
        BitmapFactory.Options options = new BitmapFactory.Options();
//        options.inPreferredConfig = Bitmap.Config.ARGB_8888;
        options.inPreferredConfig = Bitmap.Config.RGB_565;

        bitmap = BitmapFactory.decodeByteArray(rawImage, 0, rawImage.length, options);
        if (bitmap == null) {
            Log.d("zka", "bitmap is nlll");
            return;
        } else {

            int height = bitmap.getHeight();
            int width = bitmap.getWidth();
            //final Bitmap bitmap1 = Bitmap.createBitmap(bitmap, width/2 - dip2px(150),height / 2 - dip2px(92), dip2px(300), dip2px(185));
            final Bitmap bitmap1 = Bitmap.createBitmap(bitmap, dip2px(100),height / 2 - dip2px(90),dip2px(90), dip2px(180));
            int x, y, w, h;
            x = 0;
            y = 0;
            w = (int) (bitmap1.getWidth()  + 0.5f);
            h = (int) (bitmap1.getHeight() + 0.5f);
            Bitmap bit_hm = Bitmap.createBitmap(bitmap1, x, y, w, h);
            Bitmap rotatedBitmap = rotateBitmap(bit_hm, 90);
//            iv_result.setImageBitmap(rotatedBitmap);
            if(rotatedBitmap != null){
                Bitmap bm=this.getTextRegion(toGrayscale(rotatedBitmap));
                if (bm != null) {
                    iv_result.setImageBitmap(bm);
                }
                String localre = localre(rotatedBitmap);
                Log.e(TAG, "onPreviewFrame1: "+localre );
                //localre = localre.replaceAll("\\s+","");
                //if (localre.length() == 11 && localre.substring(0,1).equals("1")) {
                if (localre.length() == 18 || localre.length() == 10) {
                    ToneGenerator toneG = new ToneGenerator(AudioManager.STREAM_ALARM, 200);
                    toneG.startTone(ToneGenerator.TONE_CDMA_ALERT_CALL_GUARD, 100); // 200 is duration in ms
                    Log.e(TAG, "onPreviewFrame: "+localre );
                    Toast.makeText(getApplicationContext(),localre,Toast.LENGTH_SHORT).show();
                }
            }
        }
    }

    public Bitmap toGrayscale(Bitmap bmpOriginal)
    {
        int width, height;
        height = bmpOriginal.getHeight();
        width = bmpOriginal.getWidth();

        Bitmap bmpGrayscale = Bitmap.createBitmap(width, height, Bitmap.Config.RGB_565);
        //Bitmap bmpGrayscale = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        Canvas c = new Canvas(bmpGrayscale);
        Paint paint = new Paint();
        ColorMatrix cm = new ColorMatrix();
        cm.setSaturation(0);
        ColorMatrixColorFilter f = new ColorMatrixColorFilter(cm);
        paint.setColorFilter(f);
        c.drawBitmap(bmpOriginal, 0, 0, paint);
        return bmpGrayscale;
    }

    public static Bitmap rotateBitmap(Bitmap source, float angle)
    {
        Matrix matrix = new Matrix();
        matrix.postRotate(angle);
        return Bitmap.createBitmap(source, 0, 0, source.getWidth(), source.getHeight(), matrix, true);
    }

    private String localre(Bitmap bm) {
        String content = "";
        bm = bm.copy(Bitmap.Config.ARGB_8888, true);
//        iv_result.setImageBitmap(bm);
        TessBaseAPI baseApi = new TessBaseAPI();
        baseApi.init(TESSBASE_PATH, DEFAULT_LANGUAGE);
        //设置识别模式
        baseApi.setPageSegMode(TessBaseAPI.PageSegMode.PSM_SINGLE_LINE);
        //设置要识别的图片
        baseApi.setImage(bm);
        baseApi.setVariable("tessedit_char_whitelist", "0123456789xX-");
        Log.e(TAG, "localre: "+ baseApi.getUTF8Text());
        content = baseApi.getUTF8Text();
        baseApi.clear();
        baseApi.end();
        return content;
    }

    public int dip2px(int dp) {
        float density = this.getResources().getDisplayMetrics().density;
        return (int) (dp * density + 0.5);
    }
}
