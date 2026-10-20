#pragma once
#include <array>
#include <cmath>
namespace beacon {
struct Vec { float x=0,y=0; };
inline float distance(Vec a,Vec b){return std::hypot(a.x-b.x,a.y-b.y);}
constexpr int Size=12, MissionCount=3;
constexpr Vec Boat{1.5f,1.5f}, Relay{10.5f,9.5f};
struct Patrol {Vec a,b;};
struct Level {
 const char *name;
 std::array<const char*,Size> tiles;
 std::array<Vec,3> cells;
 std::array<Patrol,3> patrols;
 int drones;
 float speed;
};
inline constexpr std::array<Level,MissionCount> Levels{{
 {"BREAKWATER",{{
 "............","............","....#.......","....#...#...",
 "....#.......","....#.......","............","......####..",
 "..#.........","............","............","............"}},
 {{{2.5f,10.5f},{10.5f,2.5f},{7.5f,5.5f}}},
 {{{{5.5f,2.5f},{10.5f,2.5f}},{{3.5f,9.5f},{9.5f,9.5f}},{{6.5f,1.5f},{6.5f,6.5f}}}},2,.95f},
 {"DRY DOCK",{{
 "............",".....#......",".....#......","..##.#......",
 "............","........##..","...###......",".........#..",
 "......#..#..","......#.....","............","............"}},
 {{{10.5f,1.5f},{2.5f,7.5f},{8.5f,10.5f}}},
 {{{{1.5f,4.5f},{10.5f,4.5f}},{{7.5f,6.5f},{7.5f,10.5f}},{{2.5f,5.5f},{2.5f,10.5f}}}},3,1.05f},
 {"STORM PIER",{{
 "............","....#.......","....#.......","....#..###..",
 ".......#....","..###..#....","............",".###........",
 ".....####...","............","...#........","............"}},
 {{{1.5f,9.5f},{10.5f,1.5f},{6.5f,5.5f}}},
 {{{{5.5f,2.5f},{10.5f,2.5f}},{{4.5f,9.5f},{10.5f,9.5f}},{{5.5f,1.5f},{5.5f,7.5f}}}},3,1.2f}
}};
inline bool crate(int x,int y,int mission=0){return x>=0&&y>=0&&x<Size&&y<Size&&Levels[mission].tiles[y][x]=='#';}
inline bool floor(int x,int y,int mission=0){return x>=0&&y>=0&&x<Size&&y<Size&&!crate(x,y,mission);}
inline bool free(Vec p,int mission=0){
 for(float x:{-.19f,.19f})for(float y:{-.19f,.19f})
  if(!floor(int(std::floor(p.x+x)),int(std::floor(p.y+y)),mission))return false;
 return true;
}
}
