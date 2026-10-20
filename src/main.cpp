#include "render.hpp"
#include "audio.hpp"
#include "demo.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>

struct Options {
 bool screenshot=false,fullscreen=false,demo=false;
 int frames=0,level=0;
 std::string output="preview.bmp",scene="play",report,audit;
};
void reportState(const std::string &path,const World &w,int frames,double seconds,
                 bool controller,const Audio &audio,const std::string &video,const std::string &backend){
 if(path.empty())return;
 std::string tmp=path+".tmp";
 std::ofstream f(tmp);
 f<<"{\"phase\":"<<int(w.phase)<<",\"mission\":"<<w.mission<<",\"x\":"<<w.player.x
  <<",\"y\":"<<w.player.y<<",\"health\":"<<w.health<<",\"cells\":"<<w.collected()
  <<",\"lit\":"<<(w.lit?"true":"false")<<",\"time\":"<<w.time<<",\"cooldown\":"<<w.cooldown
  <<",\"frames\":"<<frames<<",\"seconds\":"<<seconds<<",\"fps\":"<<(seconds>0?frames/seconds:0)
  <<",\"controller\":"<<(controller?"true":"false")<<",\"audio\":"<<(audio.open()?"true":"false")
  <<",\"notes\":"<<audio.notes<<",\"video\":\""<<video<<"\",\"renderer\":\""<<backend<<"\"}\n";
 f.close();if(f)std::rename(tmp.c_str(),path.c_str());
}
int main(int argc,char **argv){
 Options opt;
 for(int i=1;i<argc;i++){
  std::string arg=argv[i];auto value=[&](){if(i+1>=argc){std::cerr<<"Missing value: "<<arg<<'\n';std::exit(2);}return std::string(argv[++i]);};
  if(arg=="--screenshot"){opt.screenshot=true;opt.output=value();}
  else if(arg=="--scene")opt.scene=value();
  else if(arg=="--level")opt.level=std::clamp(std::atoi(value().c_str())-1,0,2);
  else if(arg=="--frames")opt.frames=std::max(0,std::atoi(value().c_str()));
  else if(arg=="--report")opt.report=value();
  else if(arg=="--audit")opt.audit=value();
  else if(arg=="--fullscreen")opt.fullscreen=true;
  else if(arg=="--demo")opt.demo=true;
  else if(arg=="--help"){
   std::cout<<"Last Beacon 0.2.1\n--fullscreen --level 1..3 --demo --frames N --report FILE --audit FILE\n--screenshot FILE.bmp --scene play|title|pause|won|lost\n";return 0;
  }else{std::cerr<<"Unknown option: "<<arg<<'\n';return 2;}
 }
 Uint32 subsystems=SDL_INIT_VIDEO|SDL_INIT_GAMECONTROLLER;
 if(!opt.screenshot)subsystems|=SDL_INIT_AUDIO;
 if(SDL_Init(subsystems)!=0){std::cerr<<SDL_GetError()<<'\n';return 1;}
 SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"0");
 std::string video=SDL_GetCurrentVideoDriver()?SDL_GetCurrentVideoDriver():"unknown";
 if(!opt.screenshot&&(video=="dummy"||video=="offscreen")){
  std::cerr<<"Invisible video driver rejected: "<<video<<". On muOS start using LastBeacon.sh.\n";SDL_Quit();return 1;
 }
 SDL_Window *window=nullptr;SDL_Surface *surface=nullptr;
 if(opt.screenshot){
  surface=SDL_CreateRGBSurfaceWithFormat(0,640,480,32,SDL_PIXELFORMAT_ARGB8888);
  if(surface)renderer=SDL_CreateSoftwareRenderer(surface);
 }else{
  Uint32 flags=opt.fullscreen?SDL_WINDOW_FULLSCREEN_DESKTOP:SDL_WINDOW_RESIZABLE;
  window=SDL_CreateWindow("Last Beacon",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,640,480,flags);
  if(window){renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);if(!renderer)renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_SOFTWARE);}
 }
 if(!renderer){std::cerr<<SDL_GetError()<<'\n';if(window)SDL_DestroyWindow(window);SDL_Quit();return 1;}
 SDL_RenderSetLogicalSize(renderer,320,240);SDL_RenderSetIntegerScale(renderer,SDL_TRUE);
 if(!initFonts()){std::cerr<<"Font atlas: "<<SDL_GetError()<<'\n';SDL_DestroyRenderer(renderer);if(surface)SDL_FreeSurface(surface);if(window)SDL_DestroyWindow(window);SDL_Quit();return 1;}
 Profile profile;std::string save;
 if(!opt.screenshot&&!opt.demo&&opt.audit.empty()&&opt.report.empty()){
  const char *custom=std::getenv("BEACON_DATA");
  if(custom)save=std::string(custom)+"/profile.txt";
  else{char *pref=SDL_GetPrefPath("Miolonixc","LastBeacon");if(pref){save=std::string(pref)+"profile.txt";SDL_free(pref);}}
  profile.load(save);
 }
 auto saveProfile=[&](){if(!profile.save(save))SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"Cannot save progress: %s",save.c_str());};
 int selected=opt.level;
 World w;w.start(selected);w.phase=Phase::Title;
 if(opt.screenshot){
  w.phase=Phase::Playing;w.time=21;w.player={6.5f,6.5f};profile.unlocked=2;
  if(opt.scene=="title")w.phase=Phase::Title;
  else if(opt.scene=="pause")w.phase=Phase::Paused;
  else if(opt.scene=="won"){w.phase=Phase::Won;w.lit=true;w.player=Boat;for(auto &c:w.cells)c.taken=true;profile.complete(w.mission,w.time);}
  else if(opt.scene=="lost"){w.phase=Phase::Lost;w.health=0;}
  draw(w,false,profile,selected);SDL_RenderPresent(renderer);
  int rc=SDL_SaveBMP(surface,opt.output.c_str());SDL_DestroyRenderer(renderer);SDL_FreeSurface(surface);SDL_Quit();return rc==0?0:1;
 }
 SDL_RendererInfo info{};SDL_GetRendererInfo(renderer,&info);std::string backend=info.name?info.name:"unknown";
 SDL_Log("VIDEO %s RENDERER %s",video.c_str(),backend.c_str());
 int result=0;
 {
  Audio audio;
  SDL_GameController *controller=nullptr;
  auto connect=[&](){if(!controller)for(int i=0;i<SDL_NumJoysticks();i++)if(SDL_IsGameController(i)){controller=SDL_GameControllerOpen(i);if(controller){SDL_Log("CONTROLLER %s",SDL_GameControllerName(controller));break;}}};
  connect();bool pad=controller!=nullptr,running=true;
  auto beep=[&](float hz,float duration=.1f){audio.tone(hz,profile.sound,duration);};
  if(opt.demo)w.start(selected);
  Uint64 begin=SDL_GetPerformanceCounter(),prev=begin;double freq=double(SDL_GetPerformanceFrequency());int frames=0;
  while(running){
   Input in;bool accept=false,pause=false,back=false,mute=false;int choose=0;SDL_Event e;
   while(SDL_PollEvent(&e)){
    if(e.type==SDL_QUIT)running=false;
    if(e.type==SDL_CONTROLLERDEVICEADDED)connect();
    if(e.type==SDL_CONTROLLERDEVICEREMOVED&&controller&&!SDL_GameControllerGetAttached(controller)){SDL_GameControllerClose(controller);controller=nullptr;connect();if(w.phase==Phase::Playing)w.phase=Phase::Paused;}
    if(e.type==SDL_WINDOWEVENT&&e.window.event==SDL_WINDOWEVENT_FOCUS_LOST&&w.phase==Phase::Playing)w.phase=Phase::Paused;
    if(e.type==SDL_KEYDOWN&&!e.key.repeat){
     pad=false;auto k=e.key.keysym.sym;accept|=k==SDLK_RETURN;pause|=k==SDLK_ESCAPE;back|=k==SDLK_q;mute|=k==SDLK_m;
     in.action|=k==SDLK_e;in.dash|=k==SDLK_SPACE;choose+=(k==SDLK_RIGHT)-(k==SDLK_LEFT);
    }
    if(e.type==SDL_CONTROLLERBUTTONDOWN){
     pad=true;auto b=e.cbutton.button;accept|=b==SDL_CONTROLLER_BUTTON_A||b==SDL_CONTROLLER_BUTTON_START;
     pause|=b==SDL_CONTROLLER_BUTTON_START;back|=b==SDL_CONTROLLER_BUTTON_BACK;
     if(b==SDL_CONTROLLER_BUTTON_B){if(w.phase!=Phase::Playing)pause=true;else in.dash=true;}
     mute|=b==SDL_CONTROLLER_BUTTON_X;in.action|=b==SDL_CONTROLLER_BUTTON_A;
     choose+=(b==SDL_CONTROLLER_BUTTON_DPAD_RIGHT)-(b==SDL_CONTROLLER_BUTTON_DPAD_LEFT);
    }
   }
   Phase before=w.phase;
   if(mute){profile.sound=!profile.sound;saveProfile();beep(440);}
   if(before==Phase::Title){
    if(choose){selected=(selected+choose+MissionCount)%MissionCount;w.start(selected);w.phase=Phase::Title;beep(480);}
    if(accept&&selected<=profile.unlocked){w.start(selected);in.action=false;beep(660);}
    if(back||(pause&&!accept))running=false;
   }else if(before==Phase::Playing){if(pause){w.phase=Phase::Paused;beep(330);}}
   else if(before==Phase::Paused){
    if(back){w.phase=Phase::Title;selected=w.mission;beep(330);}
    else if(pause||accept){w.phase=Phase::Playing;beep(660);}
   }else{
    if(back||pause){w.phase=Phase::Title;selected=w.mission;}
    else if(accept){int next=w.phase==Phase::Won?std::min(2,w.mission+1):w.mission;selected=next;w.start(next);beep(660);}
   }
   const Uint8 *keys=SDL_GetKeyboardState(nullptr);
   float sx=float(keys[SDL_SCANCODE_RIGHT]||keys[SDL_SCANCODE_D])-float(keys[SDL_SCANCODE_LEFT]||keys[SDL_SCANCODE_A]);
   float sy=float(keys[SDL_SCANCODE_DOWN]||keys[SDL_SCANCODE_S])-float(keys[SDL_SCANCODE_UP]||keys[SDL_SCANCODE_W]);
   if(controller){
    auto button=[&](SDL_GameControllerButton b){return SDL_GameControllerGetButton(controller,b)?1.f:0.f;};
    float ax=SDL_GameControllerGetAxis(controller,SDL_CONTROLLER_AXIS_LEFTX)/32767.f;
    float ay=SDL_GameControllerGetAxis(controller,SDL_CONTROLLER_AXIS_LEFTY)/32767.f;
    float cx=button(SDL_CONTROLLER_BUTTON_DPAD_RIGHT)-button(SDL_CONTROLLER_BUTTON_DPAD_LEFT);
    float cy=button(SDL_CONTROLLER_BUTTON_DPAD_DOWN)-button(SDL_CONTROLLER_BUTTON_DPAD_UP);
    if(std::abs(ax)>.22f)cx=ax;
    if(std::abs(ay)>.22f)cy=ay;
    if(cx!=0||cy!=0){sx=cx;sy=cy;pad=true;}
   }
   in.x=(sx+sy)*.707107f;in.y=(sy-sx)*.707107f;
   Uint64 now=SDL_GetPerformanceCounter();float elapsed=std::min(float((now-prev)/freq),.1f);prev=now;
   int steps=std::max(1,int(std::ceil(elapsed*120)));float dt=elapsed/steps;
   for(int s=0;s<steps;s++){
    if(opt.demo)in=demoInput(w);
    int cells=w.collected(),health=w.health;bool lit=w.lit;float cooldown=w.cooldown;Phase phase=w.phase;
    w.update(in,dt);in.action=false;in.dash=false;
    if(w.cooldown>cooldown)beep(220,.05f);
    if(w.collected()>cells)beep(880);
    if(w.health<health)beep(120,.18f);
    if(w.lit&&!lit)beep(1046,.35f);
    if(w.phase==Phase::Won&&phase!=Phase::Won){profile.complete(w.mission,w.time);saveProfile();beep(1320,.4f);}
    if(w.phase==Phase::Lost&&phase!=Phase::Lost)beep(98,.4f);
   }
   draw(w,pad,profile,selected);SDL_RenderPresent(renderer);frames++;
   double seconds=(SDL_GetPerformanceCounter()-begin)/freq;
   if(frames%6==0)reportState(opt.audit,w,frames,seconds,controller!=nullptr,audio,video,backend);
   if(opt.frames>0&&frames>=opt.frames)running=false;
   // Software fallback needs pacing; accelerated VSync already waits for the display.
   if(!(info.flags&SDL_RENDERER_PRESENTVSYNC)){double spent=(SDL_GetPerformanceCounter()-now)/freq;if(spent<1./60)SDL_Delay(Uint32((1./60-spent)*1000));}
  }
  double seconds=(SDL_GetPerformanceCounter()-begin)/freq;
  reportState(opt.audit,w,frames,seconds,controller!=nullptr,audio,video,backend);
  reportState(opt.report,w,frames,seconds,controller!=nullptr,audio,video,backend);
  if(controller)SDL_GameControllerClose(controller);
 }
 SDL_DestroyRenderer(renderer);if(window)SDL_DestroyWindow(window);SDL_Quit();return result;
}
