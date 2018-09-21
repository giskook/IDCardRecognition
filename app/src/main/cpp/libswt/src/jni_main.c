//
// Created by zhangkai on 2018/9/19.
//

#include <jni.h>
#include <time.h>
#include <android/log.h>
#include <android/bitmap.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../swt.h"

static const char *kTAG = "__swt__";
#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))

JNIEXPORT jobject JNICALL Java_com_arseeds_idcard_CameraActivity_getTextRegion
        (JNIEnv *env, jobject thiz, jobject bitmap) {

    AndroidBitmapInfo info;
    void *pixels;
    int ret;

    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
        LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return 0;
    }
    LOGE("bmp height %d width %d", info.height, info.width);

    if (info.format != ANDROID_BITMAP_FORMAT_RGB_565) {
        LOGE("Bitmap format is not RGB_565 !");
        return 0;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return NULL;
    }
    swt_array *words = swt_detect(pixels, info.width, info.height, NULL);

    swt_rect *max = NULL;
    if (words) {
        for (int i = 0; i < swt_len(words); i++) {
            swt_rect *rect = swt_get(words, i);
            if (max == NULL){
                max = rect;
            }else{
                if (rect->width > max->width){
                    max = rect;
                };
            }
            LOGE("%d %d %d %d\n", rect->x, rect->y, rect->width, rect->height);
        }
        swt_free(words);
    }
    if (max != NULL) {
        LOGE("creating new bitmap...");
        jclass bitmapCls = (*env)->GetObjectClass(env,bitmap);
        jmethodID createBitmapFunction = (*env)->GetStaticMethodID(env, bitmapCls, "createBitmap",
                                                                    "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
        jstring configName = (*env)->NewStringUTF(env, "ARGB_8888");
        jclass bitmapConfigClass = (*env)->FindClass(env, "android/graphics/Bitmap$Config");
        jmethodID valueOfBitmapConfigFunction = (*env)->GetStaticMethodID(env,bitmapConfigClass, "valueOf",
                                                                       "(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;");
        jobject bitmapConfig = (*env)->CallStaticObjectMethod(env,bitmapConfigClass,
                                                           valueOfBitmapConfigFunction, configName);
        jobject newBitmap = (*env)->CallStaticObjectMethod(env,bitmapCls, createBitmapFunction,
                                                        max->height, max->height, bitmapConfig);
        //
        // putting the pixels into the new bitmap:
        //
        unsigned char* newBitmapPixels;
        if ((ret = AndroidBitmap_lockPixels(env, newBitmap, (void **) &newBitmapPixels)) < 0) {
            LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
            return NULL;
        }

        for (int i = 0; i < max->height; i++) {
            memcpy(newBitmapPixels, pixels + max->y*info.width*4+max->x*4, max->width*4);
        }

        AndroidBitmap_unlockPixels(env, newBitmap);
        AndroidBitmap_unlockPixels(env, bitmap);

        return newBitmap;
    }

    AndroidBitmap_unlockPixels(env, bitmap);

    return NULL;

}
