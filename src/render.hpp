#pragma once
#include <SDL.h>
#include "world.hpp"
#include "profile.hpp"
#include "font_data.hpp"
#include <string>
#include <vector>
using namespace beacon;
struct Color{Uint8 r,g,b,a=255;};
constexpr Color Ink{12,25,38},Paper{240,229,193},Gold{255,192,87},Mint{103,222,197},Red{245,111,106};
SDL_Renderer *renderer=nullptr;
void color(Color c){SDL_SetRenderDrawColor(renderer,c.r,c.g,c.b,c.a);}
void rect(int x,int y,int w,int h,Color c){color(c);SDL_Rect r{x,y,w,h};SDL_RenderFillRect(renderer,&r);}
void line(int x,int y,int xx,int yy,Color c){color(c);SDL_RenderDrawLine(renderer,x,y,xx,yy);}
SDL_Texture *fontTexture=nullptr;
bool initFonts(){
 std::vector<Uint32> pixels;pixels.reserve(typography::Width*typography::Height);
 for(size_t i=0;i<sizeof(typography::PixelsRle);i+=2){
  Uint32 value=typography::PixelsRle[i+1]?0xffffffff:0x00ffffff;
  for(int n=0;n<typography::PixelsRle[i];n++)pixels.push_back(value);
 }
 if(pixels.size()!=size_t(typography::Width*typography::Height))return false;
 SDL_Surface *surface=SDL_CreateRGBSurfaceWithFormatFrom(pixels.data(),typography::Width,typography::Height,32,typography::Width*4,SDL_PIXELFORMAT_ARGB8888);
 if(!surface)return false;
 fontTexture=SDL_CreateTextureFromSurface(renderer,surface);SDL_FreeSurface(surface);
 if(!fontTexture)return false;
 SDL_SetTextureBlendMode(fontTexture,SDL_BLENDMODE_BLEND);SDL_SetTextureScaleMode(fontTexture,SDL_ScaleModeNearest);return true;
}
int textWidth(const std::string &s,int style){
 int width=0;for(unsigned char ch:s){if(ch<32||ch>126)ch='?';width+=typography::Glyphs[style*95+ch-32].advance;}return width;
}
void fontLine(const std::string &s,int x,int y,Color c,int style){
 SDL_SetTextureColorMod(fontTexture,c.r,c.g,c.b);SDL_SetTextureAlphaMod(fontTexture,c.a);
 for(unsigned char ch:s){
  if(ch<32||ch>126)ch='?';
  const auto &g=typography::Glyphs[style*95+ch-32];
  SDL_Rect src{g.x,g.y,g.w,g.h},dst{x+g.left,y+g.top,g.w,g.h};
  if(g.w&&g.h)SDL_RenderCopy(renderer,fontTexture,&src,&dst);
  x+=g.advance;
 }
}
void text(std::string s,int x,int y,Color c=Paper,int scale=1){
 int style=scale>=2?1:0;
 if(style)fontLine(s,x+1,y+1,{91,59,40},style);
 fontLine(s,x,y,c,style);
}
void centered(std::string s,int y,Color c=Paper,int scale=1){text(s,(320-textWidth(s,scale>=2?1:0))/2,y,c,scale);}
SDL_Point project(Vec v,int z=0){return {160+int((v.x-v.y)*12),48+int((v.x+v.y)*6)-z};}
void diamond(int x,int y,int w,int h,Color c){for(int yy=-h;yy<=h;yy++){int dx=w*(h-std::abs(yy))/h;line(x-dx,y+yy,x+dx,y+yy,c);}}
void block(Vec v,int h,Color top,Color side){auto p=project(v);for(int n=0;n<h;n++)diamond(p.x,p.y-n,11,5,side);diamond(p.x,p.y-h,11,5,top);line(p.x,p.y-h+5,p.x,p.y+5,{28,44,49});}
void panel(int x,int y,int w,int h){rect(x+2,y+3,w,h,{3,13,24});rect(x,y,w,h,Ink);rect(x,y,w,1,{61,90,101});}
void draw(const World &w,bool pad,const Profile &profile,int selected){
 rect(0,0,320,240,{16,38,55});
 for(int y=40;y<240;y+=12)for(int x=0;x<320;x+=34){int drift=int(w.time*7)%34;line(x+drift,y,x+drift+10,y,{26,54,69});}
 // Backdrop: distant harbor lights and layered silhouettes.
 for(int x=0;x<320;x+=22){int h=8+(x*7)%19;rect(x,38-h,17,h,{22,47,61});rect(x+4,34-h,2,2,{190,148,80});}
 for(int sum=0;sum<23;sum++)for(int x=0;x<Size;x++){int y=sum-x;if(y<0||y>=Size)continue;
  auto p=project({x+.5f,y+.5f});diamond(p.x,p.y+5,12,6,{27,45,49});diamond(p.x,p.y,12,6,(x+y)%2?Color{65,83,81}:Color{71,90,85});
  line(p.x-11,p.y,p.x,p.y+5,{50,69,69});}
 // Mooring posts frame the walkable platform without covering its routes.
 for(int n=0;n<12;n+=3)for(Vec edge:std::array<Vec,2>{{{n+.5f,11.85f},{11.85f,n+.5f}}}){
  auto post=project(edge);rect(post.x-1,post.y-5,3,7,{42,58,60});rect(post.x-2,post.y-6,5,2,Gold);
 }
 auto boat=project(Boat);
 if(w.lit){diamond(boat.x,boat.y,12,6,Gold);text("HOME",boat.x-12,boat.y-23,Gold);}diamond(boat.x,boat.y,10,5,{33,60,65});
 diamond(boat.x,boat.y-3,9,4,Mint);rect(boat.x-4,boat.y-5,9,4,{35,71,76});
 rect(boat.x-3,boat.y-8,5,4,Paper);line(boat.x+7,boat.y-3,boat.x+12,boat.y+1,Gold);
 struct Item {float depth;int type,index;Vec pos;};std::vector<Item> objects;
 for(int y=0;y<Size;y++)for(int x=0;x<Size;x++)if(crate(x,y,w.mission))objects.push_back({float(x+y+1),0,0,{x+.5f,y+.5f}});
 objects.push_back({Relay.x+Relay.y,1,0,Relay});
 for(int i=0;i<3;i++)if(!w.cells[i].taken)objects.push_back({w.cells[i].pos.x+w.cells[i].pos.y,2,i,w.cells[i].pos});
 for(int i=0;i<Levels[w.mission].drones;i++)objects.push_back({w.drones[i].pos.x+w.drones[i].pos.y,3,i,w.drones[i].pos});
 objects.push_back({w.player.x+w.player.y,4,0,w.player});
 std::stable_sort(objects.begin(),objects.end(),[](auto a,auto b){return a.depth<b.depth;});
 for(auto obj:objects){auto p=project(obj.pos);switch(obj.type){
 case 0:block(obj.pos,12,{126,112,85},{82,76,65});line(p.x-7,p.y-13,p.x+7,p.y-7,{175,149,102});break;
 case 1:block(obj.pos,5,{69,100,107},{41,65,73});rect(p.x-4,p.y-30,8,24,{106,135,137});rect(p.x+1,p.y-30,3,24,{68,101,108});
  rect(p.x-4,p.y-23,8,3,{179,115,71});rect(p.x-4,p.y-13,8,3,{179,115,71});
  diamond(p.x,p.y-36,8,3,{43,66,76});rect(p.x-6,p.y-34,12,7,w.lit?Gold:Color{66,85,92});
  if(!w.lit&&w.collected()==3){diamond(p.x,p.y-43,4,3,Gold);}
  if(w.lit){float angle=w.time*.8f;int dx=int(std::cos(angle)*42),dy=int(std::sin(angle)*15);line(p.x,p.y-32,p.x+dx,p.y-32+dy,Gold);line(p.x,p.y-32,p.x-dx,p.y-32-dy,Gold);}break;
 case 2:{int bob=int(std::sin(w.time*4+obj.index)*2);diamond(p.x,p.y,5,2,{28,55,58});rect(p.x-3,p.y-9+bob,7,7,Gold);rect(p.x-1,p.y-8+bob,2,4,Paper);break;}
 case 3:diamond(p.x,p.y,6,3,{35,51,53});rect(p.x-5,p.y-12,10,6,{124,64,69});rect(p.x-3,p.y-11,6,2,Red);line(p.x-8,p.y-15,p.x+8,p.y-15,Paper);line(p.x,p.y-16,p.x,p.y-9,Paper);break;
 case 4:if(w.invulnerable>0&&int(w.time*12)%2)break;
  diamond(p.x,p.y,5,2,{21,40,48});rect(p.x-3,p.y-6,2,6,Ink);rect(p.x+1,p.y-6,2,6,Ink);rect(p.x-4,p.y-13,8,8,Mint);rect(p.x-3,p.y-19,6,6,Gold);rect(p.x-2,p.y-17,5,2,Ink);rect(p.x-6,p.y-12,3,7,{215,141,65});break;
 }}
 panel(8,8,304,22);text("BEACON "+std::to_string(w.mission+1),15,15,Gold);text("CELL "+std::to_string(w.collected())+"/3",90,15);text("HP",168,15);
 for(int i=0;i<3;i++)rect(192+i*9,15,6,10,i<w.health?Mint:Color{49,63,74});
 text(w.cooldown<=0?"DASH OK":"CHARGE",242,15,w.cooldown<=0?Mint:Paper);
 panel(8,205,304,27);
 std::string objective=w.lit?"RETURN TO THE BOAT":w.collected()==3?"LIGHT THE BEACON":"FIND 3 POWER CELLS";
 if(!w.lit&&w.collected()==3&&distance(w.player,Relay)<1.2f)objective=pad?"A: LIGHT THE BEACON":"E: LIGHT THE BEACON";
 text(objective,15,211,Gold);text(pad?"A:USE  B:DASH  START:PAUSE":"E:USE  SPACE:DASH  ESC:PAUSE",15,222);
 if(w.phase==Phase::Title){
  panel(24,45,272,151);centered("LAST BEACON",56,Gold,2);
  centered("SALVAGE. LIGHT. COME HOME.",79);
  centered(std::string("< ")+std::to_string(selected+1)+" / "+Levels[selected].name+" >",99,Mint);
  centered(selected<=profile.unlocked?"EXPEDITION READY":"FINISH PREVIOUS EXPEDITION",113,selected<=profile.unlocked?Paper:Red);
  centered(pad?"LEFT / RIGHT: CHOOSE":"LEFT / RIGHT: CHOOSE",129);
  rect(55,144,210,17,{45,70,77});centered(pad?"A / START: DEPART":"ENTER: DEPART",149);
  centered(pad?"X: SOUND  SELECT: QUIT":"M: SOUND  Q: QUIT",171,Mint);
  centered(profile.sound?"SOUND ON":"SOUND OFF",184,Paper);
 }
 if(w.phase==Phase::Paused){panel(54,83,212,78);centered("PAUSED",95,Gold,2);centered(pad?"A / B / START: RESUME":"ESC: RESUME",123);centered(pad?"SELECT: MENU":"Q: MENU",141,Mint);}
 if(w.phase==Phase::Won||w.phase==Phase::Lost){
  panel(30,76,260,96);bool won=w.phase==Phase::Won;
  centered(won?"HARBOR LIT":"SIGNAL LOST",86,won?Gold:Red,2);
  centered(won?"YOU BROUGHT THE LIGHT BACK.":"THE DRONES FOUND YOU.",112);
  if(won){centered("TIME "+std::to_string(int(w.time))+"S / BEST "+std::to_string(int(profile.best[w.mission]))+"S",128,Mint);}
  centered(won&&w.mission<2?(pad?"A: NEXT EXPEDITION":"ENTER: NEXT EXPEDITION"):(pad?"A: TRY AGAIN":"ENTER: TRY AGAIN"),145,Paper);
  centered(pad?"B: MENU":"ESC: MENU",159,Mint);
 }
}
