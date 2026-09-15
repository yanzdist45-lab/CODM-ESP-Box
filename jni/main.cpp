#include <sys/types.h>
#include <zygisk.hpp>
#include <android/log.h>
#include <EGL/egl.h>
#include <dlfcn.h>
#include <fstream>
#include <string>
#include "dobby.h"
#include "esp.hpp"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,"CODM-ESP",__VA_ARGS__)
static EGLBoolean(*orig)(EGLDisplay,EGLSurface)=nullptr;
static uintptr_t base(){std::ifstream f("/proc/self/maps");std::string s;while(std::getline(f,s)){if(s.find("libunity.so")==std::string::npos)continue;auto p=s.find('-');if(p!=std::string::npos)return strtoull(s.substr(0,p).c_str(),nullptr,16);}return 0;}
static EGLBoolean swap(EGLDisplay d,EGLSurface s){static uintptr_t b=0;if(!b){b=base();if(b){esp_set_unity_base(b);LOGI("unity=%p",(void*)b);}}EGLint w=0,h=0;eglQuerySurface(d,s,EGL_WIDTH,&w);eglQuerySurface(d,s,EGL_HEIGHT,&h);if(b)esp_render(w,h);return orig(d,s);}
class M:public zygisk::ModuleBase{zygisk::Api*a{};JNIEnv*e{};bool on{};public:void onLoad(zygisk::Api*x,JNIEnv*y)override{a=x;e=y;}void preAppSpecialize(zygisk::AppSpecializeArgs*x)override{if(!x||!x->nice_name)return;const char*n=e->GetStringUTFChars(x->nice_name,nullptr);on=n&&std::string(n)=="com.garena.game.codm";if(n)e->ReleaseStringUTFChars(x->nice_name,n);}void postAppSpecialize(const zygisk::AppSpecializeArgs*)override{if(!on)return;void*h=dlopen("libEGL.so",RTLD_NOW);void*p=h?dlsym(h,"eglSwapBuffers"):nullptr;if(p&&DobbyHook(
    p,
    reinterpret_cast<dobby_dummy_func_t>(swap),
    reinterpret_cast<dobby_dummy_func_t*>(&orig)
)==0)LOGI("hooked");else LOGI("hook failed");}};
REGISTER_ZYGISK_MODULE(M)
