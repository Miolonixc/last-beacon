#include "../src/world.hpp"
#include "../src/profile.hpp"
#include "../src/demo.hpp"
#include <cassert>
#include <queue>
#include <vector>
#include <cstdio>
using namespace beacon;
void tick(World &w,Input in,int n=1){for(int i=0;i<n;i++)w.update(in,1.f/120);}
void go(World &w,Vec target){
 int from=int(w.player.y)*Size+int(w.player.x),to=int(target.y)*Size+int(target.x);
 std::array<int,Size*Size> parent;parent.fill(-1);parent[from]=from;std::queue<int> q;q.push(from);
 while(!q.empty()){int p=q.front();q.pop();for(auto delta:std::array<Vec,4>{{{1,0},{-1,0},{0,1},{0,-1}}}){int x=p%Size+int(delta.x),y=p/Size+int(delta.y);if(!floor(x,y,w.mission))continue;int next=y*Size+x;if(parent[next]<0){parent[next]=p;q.push(next);}}}
 assert(parent[to]>=0);std::vector<int> route;for(int p=to;p!=from;p=parent[p])route.push_back(p);
 for(auto it=route.rbegin();it!=route.rend();++it){Vec goal{*it%Size+.5f,*it/Size+.5f};int limit=0;while(distance(w.player,goal)>.035f&&limit++<200){Vec d{goal.x-w.player.x,goal.y-w.player.y};float len=std::hypot(d.x,d.y);tick(w,{d.x/len,d.y/len});if(w.phase==Phase::Won)return;}assert(limit<200);assert(free(w.player,w.mission));}
}
int main(int argc,char **argv){
 assert(argc==2);
 World w;tick(w,{1,0},120);assert(w.player.x==Boat.x);w.start();
 w.player={3.7f,3.5f};tick(w,{1,0},240);assert(w.player.x<3.82f);assert(free(w.player,w.mission));
 w.start();w.player={.25f,.25f};tick(w,{-1,-1},120);assert(free(w.player,w.mission));
 w.start();tick(w,{0,0,true});assert(!w.lit);w.phase=Phase::Paused;float t=w.time;tick(w,{1,0},120);assert(w.time==t);
 w.start();tick(w,{1,0,false,true});assert(w.cooldown>1);float cd=w.cooldown;tick(w,{1,0,false,true});assert(w.cooldown<cd);
 w.start();w.player=w.drones[0].pos;tick(w,{});assert(w.health==2);tick(w,{});assert(w.health==2);
 w.start();w.health=1;w.player=w.drones[0].pos;tick(w,{});assert(w.phase==Phase::Lost);
 // Run real movement along reachable floor tiles through all mission stages.
 for(int level=0;level<MissionCount;level++){
  w.start(level);w.invulnerable=10000;
  for(auto &c:w.cells){assert(free(c.pos,level));go(w,c.pos);}assert(w.collected()==3);
  go(w,Relay);tick(w,{0,0,true});assert(w.lit);go(w,Boat);assert(w.phase==Phase::Won);
  // Every patrol segment stays on walkable floor.
  for(int i=0;i<Levels[level].drones;i++)for(int step=0;step<=100;step++){
   auto p=Levels[level].patrols[i];float t=step/100.f;
   assert(free({p.a.x+(p.b.x-p.a.x)*t,p.a.y+(p.b.y-p.a.y)*t},level));
  }
  // Full unattended renderer demo follows the actual rules (damage disabled only in test).
  w.start(level);w.invulnerable=10000;int steps=0;
  while(w.phase==Phase::Playing&&steps++<30000)tick(w,demoInput(w));
  assert(w.phase==Phase::Won);
 }
 Profile profile;profile.complete(0,42);assert(profile.unlocked==1);profile.complete(0,50);assert(profile.best[0]==42);
 profile.complete(1,65);assert(profile.unlocked==2);profile.sound=false;assert(profile.save(argv[1]));
 Profile loaded;assert(loaded.load(argv[1]));assert(loaded.unlocked==2&&!loaded.sound&&loaded.best[0]==42);
 {std::ofstream corrupt(argv[1]);corrupt<<"99 200 1 2 3 4";}assert(!loaded.load(argv[1]));
 std::filesystem::remove(argv[1]);
 puts("PASS: boundaries, collision, pause, dash cooldown, damage grace, defeat, all 3 missions, patrol geometry, demo paths, progress persistence");
}
