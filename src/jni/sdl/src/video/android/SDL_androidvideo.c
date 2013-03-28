/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2009 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/

#include <jni.h>
#include <android/log.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include <string.h> // for memset()

#include "SDL_config.h"
#include "SDL_version.h"

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "SDL_mutex.h"
#include "SDL_thread.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "../SDL_sysvideo.h"
#include "SDL_androidvideo.h"
#include "jniwrapperstuff.h"


// The device screen dimensions to draw on
int SDL_ANDROID_sWindowWidth  = 640;
int SDL_ANDROID_sWindowHeight = 480;

// Extremely wicked JNI environment to call Java functions from C code
static JavaVM* g_vm = NULL;
static jclass JavaRendererClass = NULL;
static jobject JavaRenderer = NULL;
static jmethodID JavaSwapBuffers = NULL;
static jmethodID JavaCheckPause = NULL;
static jmethodID JavaWaitForResume = NULL;

JNIEnv *SDL_ANDROID_GetJNIEnv()
{
	// don't hold on to JNIEnv or threads will crash it
	// get the JavaVM from the Android app and use that to get a safe JNIEnv
	
	// TODO figure out how to detect threads and only use this if it's needed
	// otherwise just cache the JNIEnv we use to get the JavaVM
    __android_log_print(ANDROID_LOG_DEBUG, "libSDL", "START: getting JNI env");
	// TODO what if the python thread calling this is a daemon?
	// there is also an AttachCurrentThreadAsDaemon function
    JNIEnv* env;
    int getEnvStat = (*g_vm)->GetEnv(g_vm, (void **)&env, JNI_VERSION_1_4);
    if (getEnvStat == JNI_EDETACHED) {
        if ((*g_vm)->AttachCurrentThread(g_vm, (void **) &env, NULL) != 0) {
	        // TODO handle this, means it couldn't attach
	        __android_log_print(ANDROID_LOG_ERROR, "libSDL", "failed to attach to current thread");
        } // else we now have a valid JNIEnv for the calling thread
    } else if (getEnvStat == JNI_OK) {
        //
        __android_log_print(ANDROID_LOG_DEBUG, "libSDL", "cool...");
    } else if (getEnvStat == JNI_EVERSION) {
        // TODO handle this but should it ever happen with SDK >= 9?
        __android_log_print(ANDROID_LOG_ERROR, "libSDL", "JNI version error");
    }
    
    __android_log_print(ANDROID_LOG_DEBUG, "libSDL", "about to return JNIEnv");
	// TODO per http://developer.android.com/training/articles/perf-jni.html "Threads"
	// FIXME or else nasty memory leak
	// see also http://pubs.opengroup.org/onlinepubs/009696799/functions/pthread_key_create.html
    //g_vm->DetachCurrentThread();
    return env;
}

int SDL_ANDROID_CallJavaSwapBuffers()
{
    int rv;

    SDL_ANDROID_drawTouchscreenKeyboard();
    SDL_ANDROID_processAndroidTrackballDampening();
        
    JNIEnv *JavaEnv = SDL_ANDROID_GetJNIEnv();
    rv = (*JavaEnv)->CallIntMethod( JavaEnv, JavaRenderer, JavaSwapBuffers );
    return rv;

}

int SDL_ANDROID_CheckPause()
{
    int rv;
    
    JNIEnv *JavaEnv = SDL_ANDROID_GetJNIEnv();
    rv = (*JavaEnv)->CallIntMethod( JavaEnv, JavaRenderer, JavaCheckPause );
    return rv;
}

void SDL_SYS_TimerQuit();
int SDL_SYS_TimerInit();

void SDL_ANDROID_WaitForResume()
{
    SDL_SYS_TimerQuit();
    JNIEnv *JavaEnv = SDL_ANDROID_GetJNIEnv();
    (*JavaEnv)->CallVoidMethod(JavaEnv, JavaRenderer, JavaWaitForResume);
    SDL_SYS_TimerInit();
}


JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(SDLSurfaceView_nativeResize) ( JNIEnv*  env, jobject  thiz, jint w, jint h )
{
    SDL_ANDROID_sWindowWidth  = w;
    SDL_ANDROID_sWindowHeight = h;
    __android_log_print(ANDROID_LOG_INFO, "libSDL", "Physical screen resolution is %dx%d", w, h);
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(SDLSurfaceView_nativeDone) ( JNIEnv*  env, jobject  thiz )
{
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "quitting...");
#if SDL_VERSION_ATLEAST(1,3,0)
	SDL_SendQuit();
#else
	SDL_PrivateQuit();
#endif
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "quit OK");
}

#define gref(x) (*env)->NewGlobalRef(env, (x))

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(SDLSurfaceView_nativeInitJavaCallbacks) ( JNIEnv*  env, jobject thiz )
{
	// safe to store vm as a shared global
	(*env)->GetJavaVM(env, &g_vm);
	
	JavaRenderer = gref(thiz);
	
	JavaRendererClass = (*env)->GetObjectClass(env, thiz);
	JavaSwapBuffers = (*env)->GetMethodID(env, JavaRendererClass, "swapBuffers", "()I");
        JavaCheckPause = (*env)->GetMethodID(env, JavaRendererClass, "checkPause", "()I");
	JavaWaitForResume = (*env)->GetMethodID(env, JavaRendererClass, "waitForResume", "()V");
	
	ANDROID_InitOSKeymap();	
}
