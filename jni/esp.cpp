#include "esp.hpp"
#include "offsets.hpp"
#include <GLES3/gl3.h>
#include <algorithm>
#include <cmath>
static uintptr_t B=0;
using GetGameFn=void*(*)(void*);
using GetEnemiesFn=void*(*)(void*,void*);
using CountFn=int(*)(void*,void*);
using ItemFn=void*(*)(void*,int,void*);
using VecFn=Vector3(*)(void*,void*);
using CamFn=void*(*)();
using W2SFn=Vector3(*)(void*,Vector3);
static GLuint prog=0,vao=0,vbo=0; static GLint scr=-1;
static GLuint sh(GLenum t,const char*s){GLuint x=glCreateShader(t);glShaderSource(x,1,&s,nullptr);glCompileShader(x);GLint ok=0;glGetShaderiv(x,GL_COMPILE_STATUS,&ok);if(!ok){glDeleteShader(x);return 0;}return x;}
static bool init(){
 if(prog)return true;
 const char*vs="#version 300 es\nlayout(location=0) in vec2 p;uniform vec2 u;void main(){vec2 n=vec2(p.x/u.x*2.0-1.0,1.0-p.y/u.y*2.0);gl_Position=vec4(n,0,1);}";
 const char*fs="#version 300 es\nprecision mediump float;out vec4 c;void main(){c=vec4(1,0,0,1);}";
 GLuint v=sh(GL_VERTEX_SHADER,vs),f=sh(GL_FRAGMENT_SHADER,fs);if(!v||!f)return false;
 prog=glCreateProgram();glAttachShader(prog,v);glAttachShader(prog,f);glLinkProgram(prog);glDeleteShader(v);glDeleteShader(f);
 GLint ok=0;glGetProgramiv(prog,GL_LINK_STATUS,&ok);if(!ok){prog=0;return false;}
 scr=glGetUniformLocation(prog,"u");glGenVertexArrays(1,&vao);glGenBuffers(1,&vbo);return true;
}
static void box(float l,float t,float r,float b,int w,int h){
 if(!init())return; GLfloat a[]={l,t,r,t,r,b,l,b};
 GLint op=0,ov=0,ob=0;GLboolean blend=glIsEnabled(GL_BLEND),depth=glIsEnabled(GL_DEPTH_TEST);
 glGetIntegerv(GL_CURRENT_PROGRAM,&op);glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&ov);glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&ob);
 glDisable(GL_DEPTH_TEST);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
 glUseProgram(prog);glUniform2f(scr,w,h);glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,vbo);glBufferData(GL_ARRAY_BUFFER,sizeof(a),a,GL_STREAM_DRAW);
 glEnableVertexAttribArray(0);glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,8,nullptr);glLineWidth(2);glDrawArrays(GL_LINE_LOOP,0,4);
 glBindBuffer(GL_ARRAY_BUFFER,ob);glBindVertexArray(ov);glUseProgram(op);if(!blend)glDisable(GL_BLEND);if(depth)glEnable(GL_DEPTH_TEST);
}
void esp_set_unity_base(uintptr_t x){B=x;}
void esp_render(int w,int h){
 if(!B||w<=0||h<=0)return;
 auto game=((GetGameFn)(B+Offsets::GamePlay_GetGame))(nullptr);if(!game)return;
 auto list=((GetEnemiesFn)(B+Offsets::GetEnemyPawns))(game,nullptr);if(!list)return;
 int n=((CountFn)(B+Offsets::PawnList_Count))(list,nullptr);if(n<=0||n>128)return;
 auto item=(ItemFn)(B+Offsets::PawnList_Item);auto head=(VecFn)(B+Offsets::GetRealHeadPosition);auto body=(VecFn)(B+Offsets::GetSyncPosition);
 void*cam=((CamFn)(B+Offsets::CameraMain))();if(!cam)return;auto w2s=(W2SFn)(B+Offsets::WorldToScreen);
 for(int i=0;i<n;i++){void*p=item(list,i,nullptr);if(!p)continue;if(!*(uint8_t*)((uintptr_t)p+Offsets::AliveField))continue;
  Vector3 H=w2s(cam,head(p,nullptr)),F=w2s(cam,body(p,nullptr));if(!std::isfinite(H.x)||!std::isfinite(H.y)||!std::isfinite(H.z)||!std::isfinite(F.x)||!std::isfinite(F.y)||!std::isfinite(F.z)||H.z<=.01f||F.z<=.01f)continue;
  float hy=h-H.y,fy=h-F.y,bh=std::fabs(fy-hy);if(bh<4||bh>h*1.5f)continue;float bw=bh*.45f,cx=(H.x+F.x)*.5f;
  box(cx-bw*.5f,std::min(hy,fy),cx+bw*.5f,std::max(hy,fy),w,h);
 }
}
