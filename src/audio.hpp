#pragma once
#include <SDL.h>
#include <vector>
#include <cmath>
namespace beacon {
class Audio {
 SDL_AudioDeviceID device=0;
public:
 int notes=0;
 Audio(){SDL_AudioSpec wanted{},actual{};wanted.freq=44100;wanted.format=AUDIO_S16SYS;wanted.channels=2;wanted.samples=1024;device=SDL_OpenAudioDevice(nullptr,0,&wanted,&actual,0);if(!device)SDL_LogWarn(SDL_LOG_CATEGORY_AUDIO,"Audio unavailable: %s",SDL_GetError());}
 ~Audio(){if(device)SDL_CloseAudioDevice(device);}
 bool open()const{return device!=0;}
 void tone(float frequency,bool enabled,float duration=.10f){
  if(!device||!enabled)return;
  if(SDL_GetQueuedAudioSize(device)>44100*4)SDL_ClearQueuedAudio(device);
  int count=int(44100*duration);std::vector<Sint16> pcm(count*2);
  for(int i=0;i<count;i++){float t=float(i)/44100;float envelope=std::min(1.f,t*150)*std::min(1.f,(duration-t)*35);Sint16 sample=Sint16(std::sin(t*frequency*6.2831853f)*envelope*6500);pcm[i*2]=sample;pcm[i*2+1]=sample;}
  if(SDL_QueueAudio(device,pcm.data(),Uint32(pcm.size()*sizeof(Sint16)))==0){notes++;SDL_PauseAudioDevice(device,0);}
 }
};
}
