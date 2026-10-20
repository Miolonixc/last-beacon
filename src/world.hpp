#pragma once
#include "levels.hpp"
#include <algorithm>
namespace beacon {
enum class Phase {Title,Playing,Paused,Won,Lost};
struct Cell {Vec pos;bool taken=false;};
struct Drone {Vec pos,a,b;bool reverse=false;};
struct Input {float x=0,y=0;bool action=false,dash=false;};
struct World {
 Phase phase=Phase::Title;
 int mission=0,health=3;
 Vec player=Boat,heading{1,0};
 std::array<Cell,3> cells{};
 std::array<Drone,3> drones{};
 bool lit=false;
 float time=0,invulnerable=0,cooldown=0,dashTime=0;
 World(){configure();}
 void configure(){
  const auto &level=Levels[mission];
  for(int i=0;i<3;i++){cells[i]={level.cells[i],false};const auto &p=level.patrols[i];drones[i]={p.a,p.a,p.b,false};}
 }
 int collected()const {int n=0;for(auto &c:cells)n+=c.taken;return n;}
 void start(int level=0){*this=World{};mission=std::clamp(level,0,MissionCount-1);configure();phase=Phase::Playing;}
 void move(Vec delta){
  Vec p{player.x+delta.x,player.y};if(free(p,mission))player=p;
  p={player.x,player.y+delta.y};if(free(p,mission))player=p;
 }
 void update(Input in,float dt){
  if(phase!=Phase::Playing)return;
  dt=std::clamp(dt,0.f,1.f/30);time+=dt;
  invulnerable=std::max(0.f,invulnerable-dt);cooldown=std::max(0.f,cooldown-dt);
  float len=std::hypot(in.x,in.y);if(len>1){in.x/=len;in.y/=len;}
  if(len>.05f)heading={in.x/std::hypot(in.x,in.y),in.y/std::hypot(in.x,in.y)};
  if(in.dash&&cooldown<=0){dashTime=.16f;cooldown=1.4f;}
  bool dashing=dashTime>0;dashTime=std::max(0.f,dashTime-dt);
  move(dashing?Vec{heading.x*8*dt,heading.y*8*dt}:Vec{in.x*2.5f*dt,in.y*2.5f*dt});
  for(auto &c:cells)if(distance(player,c.pos)<.55f)c.taken=true;
  if(in.action&&collected()==3&&distance(player,Relay)<1.2f)lit=true;
  for(int i=0;i<Levels[mission].drones;i++){
   auto &d=drones[i];Vec target=d.reverse?d.a:d.b;float n=distance(d.pos,target);
   if(n<.05f)d.reverse=!d.reverse;
   else{float step=std::min(n,dt*Levels[mission].speed*(lit?1.3f:1.f));d.pos.x+=(target.x-d.pos.x)/n*step;d.pos.y+=(target.y-d.pos.y)/n*step;}
   if(distance(player,d.pos)<.5f&&invulnerable<=0&&!dashing){health--;invulnerable=1.6f;if(health<=0)phase=Phase::Lost;}
  }
  if(lit&&distance(player,Boat)<.75f&&health>0)phase=Phase::Won;
 }
};
}
