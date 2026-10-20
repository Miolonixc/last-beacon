#pragma once
#include "world.hpp"
#include <queue>
namespace beacon {
// Diagnostic autopilot uses the same movement and collision code as the player.
inline Input demoInput(const World &w){
 Vec goal=w.lit?Boat:Relay;
 for(auto &c:w.cells)if(!c.taken){goal=c.pos;break;}
 if(distance(w.player,goal)<.7f&&w.collected()==3&&!w.lit)return {0,0,true,false};
 int from=int(w.player.y)*Size+int(w.player.x),to=int(goal.y)*Size+int(goal.x);
 std::array<int,Size*Size> parent;parent.fill(-1);parent[from]=from;
 std::queue<int> pending;pending.push(from);
 while(!pending.empty()){
  int p=pending.front();pending.pop();
  for(Vec d:std::array<Vec,4>{{{1,0},{-1,0},{0,1},{0,-1}}}){
   int x=p%Size+int(d.x),y=p/Size+int(d.y);if(!floor(x,y,w.mission))continue;
   int next=y*Size+x;if(parent[next]<0){parent[next]=p;pending.push(next);}
  }
 }
 if(parent[to]<0)return {};
 if(to!=from){while(parent[to]!=from)to=parent[to];goal={to%Size+.5f,to/Size+.5f};}
 Vec delta{goal.x-w.player.x,goal.y-w.player.y};float len=std::hypot(delta.x,delta.y);
 return len>.02f?Input{delta.x/len,delta.y/len,false,false}:Input{};
}
}
