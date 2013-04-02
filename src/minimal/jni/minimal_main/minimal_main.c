#ifdef ANDROID

#include <unistd.h>
#include <stdlib.h>
#include <jni.h>
#include <android/log.h>
#include "SDL_thread.h"
#include "minimal_main.h"

#define LOG(x) __android_log_write(ANDROID_LOG_INFO, "main", (x))

/* JNI-C wrapper stuff */

#ifdef __cplusplus
#define C_LINKAGE "C"
#else
#define C_LINKAGE
#endif


#ifndef MINIMAL_JAVA_PACKAGE_PATH
#error You have to define MINIMAL_JAVA_PACKAGE_PATH to your package path with dots replaced with underscores, for example "com_example_SanAngeles"
#endif
#define JAVA_EXPORT_NAME2(name,package) Java_##package##_##name
#define JAVA_EXPORT_NAME1(name,package) JAVA_EXPORT_NAME2(name,package)
#define JAVA_EXPORT_NAME(name) JAVA_EXPORT_NAME1(name,MINIMAL_JAVA_PACKAGE_PATH)


static int isSdcardUsed = 0;


// JNI env stuff from sdl/src/video/android/SDL_androidvideo.c
// Extremely wicked JNI environment to call Java functions from C code
static JavaVM* g_vm = NULL;
static jobject JavaActivity = NULL;
static jclass JavaActivityClass = NULL;
static jmethodID JavaCheckPause = NULL;
static jmethodID JavaWaitForResume = NULL;

// for pyjnius
JNIEnv *SDL_ANDROID_GetJNIEnv()
{
	// don't hold on to JNIEnv or threads will crash it
	// get the JavaVM from the Android app and use that to get a safe JNIEnv
	
	// TODO figure out how to detect threads and only use this if it's needed
	// otherwise just cache the JNIEnv we use to get the JavaVM
        //__android_log_print(ANDROID_LOG_DEBUG, "libSDL", "START: getting JNI env");
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
        //__android_log_print(ANDROID_LOG_DEBUG, "libSDL", "cool...");
    } else if (getEnvStat == JNI_EVERSION) {
        // TODO handle this but should it ever happen with SDK >= 9?
        __android_log_print(ANDROID_LOG_ERROR, "libSDL", "JNI version error");
    }
    
    //__android_log_print(ANDROID_LOG_DEBUG, "libSDL", "about to return JNIEnv");
	// TODO per http://developer.android.com/training/articles/perf-jni.html "Threads"
	// FIXME or else nasty memory leak
	// see also http://pubs.opengroup.org/onlinepubs/009696799/functions/pthread_key_create.html
    //g_vm->DetachCurrentThread();
    return env;
}

int SDL_ANDROID_CheckPause()
{
    int rv;
    
    JNIEnv* JavaEnv = SDL_ANDROID_GetJNIEnv();
    rv = (*JavaEnv)->CallIntMethod( JavaEnv, JavaActivityClass, JavaCheckPause );
    return rv;
}

void SDL_ANDROID_WaitForResume()
{
    JNIEnv* JavaEnv = SDL_ANDROID_GetJNIEnv();
    (*JavaEnv)->CallVoidMethod(JavaEnv, JavaActivityClass, JavaWaitForResume);
}


// for PythonActivity
#define gref(x) (*env)->NewGlobalRef(env, (x))
extern C_LINKAGE void
JAVA_EXPORT_NAME(PythonActivity_nativeInit) ( JNIEnv*  env, jobject thiz )
{
	int argc = 1;
	char * argv[] = { "minimal" };

	// from sdl/src/video/android/SDL_androidvideo.c
	(*env)->GetJavaVM(env, &g_vm);
	
	JavaActivity = gref(thiz);
	
	JavaActivityClass = gref((*env)->GetObjectClass(env, thiz));
	
//	JavaCheckPause = gref((*env)->GetMethodID(env, JavaActivityClass, "checkPause", "()I"));
//	JavaWaitForResume = gref((*env)->GetMethodID(env, JavaActivityClass, "waitForResume", "()V"));

	main( argc, argv );
};

extern C_LINKAGE void
JAVA_EXPORT_NAME(PythonActivity_nativeIsSdcardUsed) ( JNIEnv*  env, jobject thiz, jint flag )
{
	isSdcardUsed = flag;
}

extern C_LINKAGE void
JAVA_EXPORT_NAME(PythonActivity_nativeSetEnv) ( JNIEnv*  env, jobject thiz, jstring j_name, jstring j_value )
{
    jboolean iscopy;
    const char *name = (*env)->GetStringUTFChars(env, j_name, &iscopy);
    const char *value = (*env)->GetStringUTFChars(env, j_value, &iscopy);
    setenv(name, value, 1);
    (*env)->ReleaseStringUTFChars(env, j_name, name);
    (*env)->ReleaseStringUTFChars(env, j_value, value);
}

#undef JAVA_EXPORT_NAME
#undef JAVA_EXPORT_NAME1
#undef JAVA_EXPORT_NAME2
#undef C_LINKAGE

#endif
