#pragma once
#include <algorithm>
#include <cmath>
#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
namespace beacon {
struct Profile {
 int unlocked=0;bool sound=true;
 std::array<float,3> best{{0,0,0}};
 bool load(const std::string &file){
  std::ifstream f(file);int version=0,u=0,s=1;std::array<float,3> times{};
  if(!(f>>version>>u>>s>>times[0]>>times[1]>>times[2])||version!=1)return false;
  for(float t:times)if(!std::isfinite(t)||t<0||t>86400)return false;
  unlocked=std::clamp(u,0,2);sound=s!=0;best=times;return true;
 }
 bool save(const std::string &file)const{
  if(file.empty())return true;
  std::error_code ec;auto parent=std::filesystem::path(file).parent_path();
  if(!parent.empty())std::filesystem::create_directories(parent,ec);
  if(ec)return false;
  std::string temp=file+".tmp";
  {std::ofstream f(temp);f<<1<<' '<<unlocked<<' '<<sound<<' '<<best[0]<<' '<<best[1]<<' '<<best[2]<<'\n';f.flush();if(!f)return false;}
  std::filesystem::rename(temp,file,ec);return !ec;
 }
 void complete(int level,float time){unlocked=std::max(unlocked,std::min(2,level+1));if(best[level]==0||time<best[level])best[level]=time;}
};
}
