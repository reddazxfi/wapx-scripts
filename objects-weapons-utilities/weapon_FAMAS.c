require utils, utils_steps, p_sprite_builder;   

CSoundFile* famasburst;       
CSoundFile* famas1;

void fam_script::InitGraphic()
{
famasburst = new CSoundFile(GetAttachment("famas-burst.wav"));   
famas1 = new CSoundFile(GetAttachment("famas-2.wav"));
}                                              

CWeapon* famas;
int famshoots;

void fam_script::FirstFrame()
{
famas = GetWeaponByName("FAMAS");
famshoots = 3;     //12 (4 bursts)
  /*
   local cfgFamas = new CWeaponFiringParams();
   cfgFamas->isInstant       = true;
   cfgFamas->bulletsPerBurst = 3;
 //cfgFamas->launchSoundFile = famasburst;
   cfgFamas->shotSound       = 0;
   cfgFamas->launchSpd       = 16.0;
   cfgFamas->aimSheet        =  gunAimSheet->Index;
   cfgFamas->useSubSpr = true;
   cfgFamas->subSprFire  = 3  ; 
   cfgFamas->numSprsInSheet = 12;
   cfgFamas->subSprNum = 2  ;
   cfgFamas->shotDelay = 2;
   AddToChuteList(GetWeaponByName("FAMAS"), cfgFamas);
*/}

//---------------CWorm Class Overrides---------------//
override CWorm::CWorm(CObject * Parent, int aTeam, int aIndex, CWormParams * params)
{
    super;
    famascooldown = 0;
    shotsRemaining = 4;
    GunShotsFired = 0;
    SpreadsFired = 0;
    GunShooting = false;
}

override void CWorm::FireFinal(CWeapon* Weap, CShootDesc* Desc)
{      
   
    if (Weap->CheckName("FAMAS") && !drawGun)
    {     
       Desc->SpX = (TurnSide * (0.5 - sqrt((0.5 - FireAngle)*(0.5 - FireAngle)))) * 40;
       Desc->SpY = 40*(0.5 - FireAngle);
       Desc->Y  -=    3.0      ; 
                       
       if (FireAngle < 0.167) {Desc->Y -=3.15;  Desc->X +=2*TurnSide; }
       if (GunShotsFired<2)
       {                    
         if (GunShotsFired == 0 && SpreadsFired != famshoots)
         { 
             vol = CalculateSoundVolume(PosX,PosY);
             pan = CalculateSoundPan(PosX,PosY);
             famasburst->Play(vol * 0.81, pan, false);
         }  
         else if (SpreadsFired == famshoots)  
         { 
             vol = CalculateSoundVolume(PosX,PosY);
             pan = CalculateSoundPan(PosX,PosY);
             famas1->Play(vol * 0.81, pan, false);
         }  
         if (SpreadsFired == famshoots) GunShotsFired +=1;  //Last burst is just 2 bullets :)
         GunShotsFired++;
         super(famas, Desc);  
         GunShooting=true;
         return;   //Return HERE makes it loop.
       } 
        if (SpreadsFired == famshoots)  
        { 
             vol = CalculateSoundVolume(PosX,PosY);
             pan = CalculateSoundPan(PosX,PosY);
             famas1->Play(vol * 0.81, pan, false);
        }          
        super(famas, Desc); //Separate shots here bc of the sound   
        GunShooting = false;   
        if (SpreadsFired < famshoots) //4
        {        
        GunShotsFired = 0;
        nAvalShoots = 1;
        nTotalShoots = 0; 
        SpreadsFired++;     
        shotsRemaining--;
        ShowMessage2(itoa(shotsRemaining), " Shots remaining") ;
        SetState(WS_AIMING);    
        return;
        }        
        dryShoot->Play(vol,pan,false); //Empty trigger
        
        nTotalShoots = 1; 
        GunShooting = false;
        GunShotsFired = 0;
        SpreadsFired = 0;     
        shotsRemaining = 4;
        SetState(WS_IDLE);   //Needed to get out of the loop, easier way to do it lol
    }  
   /* else if (drawGun && activeCfg!=NullObj)
    {       
       if (activeCfg->numBursts == chBurstsDone) activeCfg->bulletsPerBurst = 2;
       if (activeCfg->numBursts <  chBurstsDone  && chBulletsFired == 0) famasburst->Play( CalculateSoundVolume(PosX,PosY) * 0.76, CalculateSoundPan(PosX,PosY), false);
       if (activeCfg->numBursts == chBurstsDone && chFiring) famas1->Play( CalculateSoundVolume(PosX,PosY) * 0.76, CalculateSoundPan(PosX,PosY), false);
       super;  
    }*/      
    else {GunShooting=false; //Shouldn't be neccessary, however i'm insecure.
    super; }
}

override void CMissile::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
 super;

 if (Type == M_FRAME)
 if ((weap->CheckName("FAMAS") == true) && NewBounce) 
 {
   NewBounce = false;
 
   ColMask = new CColMask(1.7,1.7,MakeCircleMask(1.7));
 }
}
  
override void CWorm::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
super;
  if (Type == M_SETWEAPON)  GunShotsFired = 0;
  if (Type == M_FRAME)
  {
        if (CurWeapon != NullObj && (CurWeapon->CheckName("FAMAS") == true ))
        {
        if (GunShooting)
        {
           // Persist animation.  
           nShoots = 1;
           nShooted = 0;
        }
     }
  }
  if (GetCurrentWorm() == this)
  {
  if (Type==M_SETWEAPON)
  {
     if(CurWeapon!=NullObj && CurWeapon->CheckName("FAMAS") == true)
     {
     GunShotsFired=0; GunShooting=false;  //Reset variables
     }    
  }
  } 
   
}   


//////Sprite Builder///////


require utils, p_sprite_builder, sprite_aim_guns, sprite_sheet_red;

override void CTurnGame::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData) { 
 super;                                                                             
  if (Type == M_FRAME) {                                                         
     if (gframe == 2) {                                                           
      local weapName; local sprite;

      CWormAnimParams* Params = new CWormAnimParams;
      weapName = "FAMAS";                                               
      sprite = gunAimSheet->Index;   
      Params->FSprite = gunAimSheet->Index;    
                                        
      Params->hand_radius = -6.0;                                                         
      Params->animate = false;
      Params->hand_radial_rotation = 1.0;
      Params->fix_flip_rotation = true;
      Params->hand_rotation = -0.5;                                                                                                                                                             
      Params->weap_radial_rotation = 1.0;
      Params->weap_rotation = 1.0;
      Params->weap_radius = 0.0;
      Params->draw_hand = false;          
      Params->zplane = 6;
      
              Params->useSpriteSystem = true;
              Params->subSpriteNumber   = 2 ;
              Params->subSpriteFiring   = 3 ;
              Params->numSpritesInSheet = 12;
                                    
      local WA = new WormSprite(weapName, sprite, Params);          
  }                              
  else if (Type==M_TURNBEGIN){  
  super;
  }                                                                                
 }
} 


//Well organized workflow oriented SPB
override void CMissile::Message(CObject* sender,EMType Type,
int MSize,CMessageData* MData){super
;if(Type==M_FRAME
){if(weap->CheckName("FAMAS"
)){if(gunfblt_anim){local sprite
=
redWeapSheet->Index;WeapSpriteParams* Params= new WeapSpriteParams;Params->animate=true;Params->
anim_speed=1.50; 
Params->useSpriteSystem = 
true; Params->
ZPlane = 22.0; Params->subSpriteNumber 
= 2; Params->numSpritesInSheet = 12; new WeapSprite(this,sprite,Params);weap->launch.anim.spriteIndex=0;gunfblt_anim=false;}
}}} override CMissile::CMissile
(CObject* parent,CWeaponLaunch* ldata,
CShootDesc* sdata){super;gunfblt_anim = true;}     