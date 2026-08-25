require utils;

keeperbeeM : CMissile   

require utils, p_sprite_builder;

CSprite* beebee_aiming;                                                      
CSprite* beemisspr;                                                
CSprite* beelauncherspr;

void bee_spb::InitGraphic()                             
{   CFile* f;                                                            
    f = GetAttachment("bee_spb.png");                          
    beebee_aiming = LoadSprite(f, 1, 0);                                   
    f = GetAttachment("beemisspr.png");                           
    beemisspr = LoadSprite(f, 2, 0);                                
    f = GetAttachment("spb_homing_bee.png");
    beelauncherspr = LoadSprite(f, 2, 0);
}                                                                             
                                                                                 
override void CTurnGame::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData) { 
 super;                                                                          
  if (Type == M_FRAME) {                                                         
    if (gframe == 2) {                                                           
     local weapName; local sprite;


     CWormAnimParams* Params = new CWormAnimParams;
     weapName = "Keeper Bee";                                             
     sprite = beebee_aiming->Index;                                   
     Params->hand_radius = -14.5;                                                 
     Params->animate = false;
     Params->hand_rotation = -0.0;                                             
     Params->hand_radial_rotation = 0.0;
     Params->weap_radial_rotation = 0.0;     
     Params->weap_rotation = 0.5;
     Params->weap_radius = -12.5;
     Params->draw_hand = false;
     Params->hand_type = 1;     
     Params->hand_scale = 1.3;  
     Params->FSprite =  0;                                           

                                                
     local WA = new WormSprite(weapName, sprite, Params);              
    }                                                                           
  }                                                                             
}

//---------------------------------BEE LAUNCHER------------------------------//

override void CTurnGame::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData) {
 super;
  if (Type == M_FRAME) {
    if (gframe == 2) {
     local weapName; local sprite;
     CWormAnimParams* Params = new CWormAnimParams;
     weapName = "Homing Keeper Bee";
     sprite = beelauncherspr->Index;
     Params->draw_hand = false;
     Params->hand_radius = 0.0;
     Params->hand_rotation = 0.0;
     Params->hand_radial_rotation = 0.0;
     Params->weap_radial_rotation = 0.0;
     Params->weap_rotation = 1.0;
     Params->weap_radius = 3.5;
     Params->animate = true;
     Params->anim_speed = 1.80;
     local WA = new WormSprite(weapName, sprite, Params);
    }
  }
}
////////////////////////////////////////////
//////////////////MISSIlE//////////////////   wonky as fuck, i dont know how did i manage to make team assignment so unreliable
//////////////////////////////////////////
/*        //This is the stupidest way to avoid using FireFinal lol
override void CMissile::CMissile(CObject* parent,CWeaponLaunch* ldata,CShootDesc* sdata)
{
 if (ldata->windFactor == 6767)
 {
 ldata->windFactor = 0;
 
 beeteam = sdata->Team;
 
 new keeperbeeM(Root->GetObject( 25, 0 ), ldata, sdata);
 return;
 }
 
 super;
} */

keeperbeeM::keeperbeeM(CObject* parent,CWeaponLaunch* ldata,CShootDesc* sdata, int bbteam, CWorm* launcher)
{     
   beeMSprite = beeBaseSprite->Index; 
   ownerworm = launcher;
   imBeeM = true;
   beeFrame = 0;
   tbteam = bbteam; 
   ZPlane = 10.0;
   if (launcher!=NullObj) tbteam = launcher->WormTeam;
   
   if (abeeTeam!=0 && tbteam == 0)  tbteam = abeeTeam;  // man.
   
   super(Root->GetObject( 25, 0), ldata, sdata);
        
   if (PosX != sdata->X || PosY !=sdata->Y)
    { 
        PosX = sdata->X;    //Terrain on face.
        PosY = sdata->Y;
        Free(true);         //Bee Spawn code is here before freeing object.
    }
}  

void keeperbeeM::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
super;        
 if (Type == M_DRAWQUEUE)
   {
      DrawTheBee(beeFrame);
   }
 if (Type == M_FRAME)
   {                  
      beeFrame++; if (beeFrame>=3) beeFrame = 0; //double the speed than the other one
   }  
}

void keeperbeeM::ExplodeAt(fixed x,fixed y)
{
   //DoExplosion(PosX, PosY, 0, 0, 10, 0); 
   //GG->land->MakeHole( 20, PosX, PosY);
   super;
}
  
void keeperbeeM::DrawTheBee(int bframe)
{
   int faceDir;
   if (SpX >0)
      faceDir = 1;
   else
      faceDir = 0;
   
   if(faceDir == 0)
   {
     AddSpriteEx(ZPlane+0.03, PosX, PosY, 262144 + beeMSprite, bframe, 0, 0.93);
   }
   else
   {
     AddSpriteEx(ZPlane+0.03, PosX, PosY, beeMSprite, bframe, 0, 0.93);
   }  
}

void keeperbeeM::Free(bool FreeMem)
{    
 if (FreeMem)
 {  
      abeeTeam = tbteam;
      local w = CWorm(NullObj);
      if (ownerworm!=NullObj) {w = ownerworm; abeeTeam = ownerworm->WormTeam; tbteam = ownerworm->WormTeam; }
      createBee(PosX, PosY, tbteam, w, false);
 }
   super;
}    
   
override void CWorm::FireFinal(CWeapon* Weap,CShootDesc* Desc)
{
 if (Weap->CheckName( "Homing Keeper Bee" ))
  {        
   CWeaponLaunch* beeL = &Weap->launch;   
   abeeTeam = Desc->Team;
   beecms = new keeperbeeM(Root->GetObject( 25, 0 ), beeL, Desc, Desc->Team, this);
   
   return;
  }    
 else super;
}       


override void keeperbeeM::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData){
super;
  if (Type == M_FRAME){
         if (keeperbmAnim) {
              local sprite = beemisspr->Index;
              WeapSpriteParams* Params = new WeapSpriteParams;
              Params->animate = true;
              Params->anim_speed = 4.00;
              new WeapSprite(this, sprite, Params);
              //weap->launch.anim.spriteIndex = 0;
              keeperbmAnim = false;
         
     }
  }
}

override keeperbeeM::keeperbeeM(CObject* parent,CWeaponLaunch* ldata,CShootDesc* sdata, int bbteam, CWorm* launcher){
super;
  keeperbmAnim = true;
}         