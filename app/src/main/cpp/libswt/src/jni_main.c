//
// Created by zhangkai on 2018/9/19.
//

#include <jni.h>
#include <time.h>
#include <android/bitmap.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../swt.h"

#include <android/log.h>

static const char *kTAG = "__swt__";
#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))

#define GRAY(x) ((((x&0x00ff0000)>>16)+((x&0x0000ff00)>>8)+(x&0x000000ff))/3)
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

    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        LOGE("Bitmap format is not ARGB_8888 !");
        return 0;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return NULL;
    }
    unsigned char * gray_pixels = (unsigned char *)malloc(info.width * info.height);

    LOGE("%d", GRAY(*(((unsigned  int *)pixels) + 0*info.width + 0)));

    LOGE("%d %d %d %d", (*(((unsigned  int *)pixels))&0xff000000)>>24,
         (*(((unsigned  int *)pixels))&0x00ff0000)>>16,
            (*(((unsigned  int *)pixels))&0x0000ff00)>>8,
            (*(((unsigned  int *)pixels))&0x000000ff));
    LOGE("-----%d %d %d %d", ((unsigned char *)pixels)[0], ((unsigned char *)pixels)[1], ((unsigned char *)pixels)[2], ((unsigned char *)pixels)[3]);


    for (int i = 0; i < info.height; i++){
        for(int j = 0; j < info.width; j++){
            *(gray_pixels + i*info.width +j) = GRAY(*(((unsigned  int *)pixels) + i*info.width + j));
        }
    }
    swt_array *words = swt_detect(gray_pixels, info.width, info.height, NULL);
    swt_rect max;
    max.x = 0;
    max.y = 0;
    max.width = 0;
    max.height = 0;
    if (words) {
        LOGE("swt_len %d\n", swt_len(words));
        for (int i = 0; i < swt_len(words); i++) {
            swt_rect *rect = swt_get(words, i);
            if (max.width == 0) {
                max.x = rect->x;
                max.y = rect->y;
                max.height = rect->height;
                max.width = rect->width;
            } else {
                if (rect->width > max.width) {
                    max.x = rect->x;
                    max.y = rect->y;
                    max.height = rect->height;
                    max.width = rect->width;
                };
            }
            LOGE("%d %d %d %d\n", rect->x, rect->y, rect->width, rect->height);
        }
        swt_free(words);
    } else {
        LOGE("words null");
    }
    if (max.width > 0) {
        LOGE("creating new bitmap...");
        /*
        jclass bitmapCls = (*env)->GetObjectClass(env, bitmap);
        jmethodID createBitmapFunction = (*env)->GetStaticMethodID(env, bitmapCls, "createBitmap",
                                                                   "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
//        jstring configName = (*env)->NewStringUTF(env, "ARGB_8888");
        jstring configName = (*env)->NewStringUTF(env, "RGB_565");

        jclass bitmapConfigClass = (*env)->FindClass(env, "android/graphics/Bitmap$Config");
        jmethodID valueOfBitmapConfigFunction = (*env)->GetStaticMethodID(env, bitmapConfigClass,
                                                                          "valueOf",
                                                                          "(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;");
        jobject bitmapConfig = (*env)->CallStaticObjectMethod(env, bitmapConfigClass,
                                                              valueOfBitmapConfigFunction,
                                                              configName);
        jobject newBitmap = (*env)->CallStaticObjectMethod(env, bitmapCls, createBitmapFunction,
                                                           max->width, max->height, bitmapConfig);
                                                           */
        jclass bitmapCls = (*env)->FindClass(env, "android/graphics/Bitmap");
        jmethodID createBitmapFunction = (*env)->GetStaticMethodID(env, bitmapCls,
                                                                   "createBitmap",
                                                                   "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
        jstring configName = (*env)->NewStringUTF(env, "ARGB_8888");
        jclass bitmapConfigClass = (*env)->FindClass(env, "android/graphics/Bitmap$Config");
        jmethodID valueOfBitmapConfigFunction = (*env)->GetStaticMethodID(env,
                                                                          bitmapConfigClass,
                                                                          "valueOf",
                                                                          "(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;");
        jobject bitmapConfig = (*env)->CallStaticObjectMethod(env, bitmapConfigClass,
                                                              valueOfBitmapConfigFunction,
                                                              configName);
        LOGE("max  height %d width %d\n", max.height, max.width);
        jobject newBitmap = (*env)->CallStaticObjectMethod(env, bitmapCls,
                                                        createBitmapFunction, max.width,
                                                        max.height, bitmapConfig);

        AndroidBitmapInfo new_info;
        if ((ret = AndroidBitmap_getInfo(env, newBitmap, &new_info)) < 0) {
            LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
            return 0;
        }
        LOGE("new bmp height %d width %d", new_info.height, new_info.width);
        //
        // putting the pixels into the new bitmap:
        //
        unsigned char *newBitmapPixels;
        if ((ret = AndroidBitmap_lockPixels(env, newBitmap, (void **) &newBitmapPixels)) < 0) {
            LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
            return NULL;
        }

        for (int i = 0; i < max.height; i++) {
            memcpy(newBitmapPixels + i * max.width * 4,
                   pixels + (max.y + i) * info.width * 4 + max.x * 4,
                   max.width * 4);
        }

        AndroidBitmap_unlockPixels(env, newBitmap);
        AndroidBitmap_unlockPixels(env, bitmap);

        return newBitmap;
    }

    AndroidBitmap_unlockPixels(env, bitmap);

    return bitmap;

}
