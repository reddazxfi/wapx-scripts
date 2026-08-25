//Script by reddaz/lucho, sequencing shots without using gun params
//Set weap anim to a gun's (6 for minigun, uzi, shotgun, handgun also work)
//Cooldown not modifiable, so around 1 frame or 2, however there's a workararound for that;
//An array, and for (i = 0; i<19; i++) {shotCooldown[i] = [12*i], then shotFrames++ on M_FRAME and if (shotFrames==shotCooldown[shotFrames]) super;}  more or less, you can set any cooldown you want with that, but it wasn't that reliable, if ur better than me do use that.
require utils, p_sprite_builder, utils_red, utils_steps ;   

int MS_BulletAmount;
bool nerfBC;
CWeapon * g_megashark;
CSoundFile * item_11;

void megashark_firing::FirstFrame()
{
keySync->AddKey(49); // 1 button
keySync->AddKey(50); // 2 button
keySync->AddKey(51); // 3 button
keySync->AddKey(52); // 4 button
keySync->AddKey(53); // 5 button 
       
g_megashark = GetWeaponByName("MEGASHARK");  


MS_BulletAmount = 11;     //12
nerfBC=true;    
}

void megashark_firing::Init()
{
item_11 = new CSoundFile(GetAttachment("item_11.wav")) ;
}

void modifyMS(int msBLT,int msDMG,int msPUSH,int msBIAS,int msVAR, bool msnerf)
{                                                                         //for your scheme scripts.
MS_BulletAmount = msBLT;
g_megashark->launch.explosion.damage = msDMG;
g_megashark->launch.explosion.pushPower = msPUSH;
g_megashark->launch.explosion.bias = msBIAS;
g_megashark->launch.explosion.dmgVar = msVAR;
nerfBC=msnerf;
}
//---------------CWorm Class Overrides---------------//
override CWorm::CWorm(CObject * Parent, int aTeam, int aIndex, CWormParams * params)
{
    super;
    MShotsFired = 0;
    megaShooting = false;
}

override void CWorm::FireFinal(CWeapon* Weap, CShootDesc* Desc)
{
    if (Weap->CheckName("MEGASHARK") == true)
    {
       if (MShotsFired<=MS_BulletAmount)     //Quantity of shots
       {            
         pan = CalculateSoundPan(PosX,PosY);
         vol = CalculateSoundVolume(PosX,PosY);
         item_11->Play(vol, pan, false);
         
         MShotsFired++; //For some reason MShotsFired++ normally doesn't work.
         Desc->SpY+=RandomFloat(-0.7,0.7);  //Spread    
         Desc->Y    -=    3.0      ;      
         Desc->X    +=    1.3 * TurnSide;      
         Weap->launch.anim.spriteIndex = 0;  
         if (FireAngle < 0.164) {Desc->Y -=2.9;  Desc->X +=2*TurnSide; }   
         
         super(g_megashark,Desc);  
         megaShooting=true;
         return;   //Return HERE makes it loop.
       }
       megaShooting=false;  
       nShoots--; 
       MShotsFired = 0;
    }        
    else {megaShooting=false; //Shouldn't be neccessary, however i'm insecure.
    super; }
}
  
override void CWorm::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
super;
  if (Type == M_FRAME)
  {
        if (CurWeapon != NullObj && (CurWeapon->CheckName("MEGASHARK") == true))
        {
            if (!megaShooting){
            if (keySync->KeyPressedNow(49))
            {
                bulletsPerTarget = 1;
                ShowTeamMessage(8, "Megashark: Home 1 bullet per Target", WormTeam);
                PlayLocalSound(92, 1.0, 1.0, 1.0);
            }
            else if (keySync->KeyPressedNow(50))
            {
                bulletsPerTarget = 2;
                ShowTeamMessage(8, "Megashark: Home 2 bullets per Target", WormTeam);   
                PlayLocalSound(92, 1.0, 1.0, 1.0);
            }
            else if (keySync->KeyPressedNow(51))
            {
                bulletsPerTarget = 3;
                ShowTeamMessage(8, "Megashark: Home 3 bullets per Target", WormTeam); 
                PlayLocalSound(92, 1.0, 1.0, 1.0);
            }
            else if (keySync->KeyPressedNow(52))
            {
                bulletsPerTarget = 4;
                ShowTeamMessage(8, "Megashark: Home 4 bullets per Target", WormTeam); 
                PlayLocalSound(92, 1.0, 1.0, 1.0);
            }
            else if (keySync->KeyPressedNow(53))
            {
                if (!nerfBC){
                bulletsPerTarget = 10;
                ShowTeamMessage(8, "Megashark: Home 10 bullets per Target", WormTeam); 
                PlayLocalSound(92, 1.0, 1.5, 1.0); }
                else if (nerfBC)
                {
                bulletsPerTarget = 4;
                ShowTeamMessage(8, "Megashark: Home 4 bullets per Target", WormTeam); 
                PlayLocalSound(92, 0.7, 0.7, 0.7);
                }
            }}
        if (megaShooting)
        {
           // Persist animation.  
           nShoots = 1;
           nShooted = 0;
        }
     }
  }
  if (GetCurrentWorm()!=this) return;
  if (Type==M_SETWEAPON)
  {
     if(CurWeapon!=NullObj && (CurWeapon->CheckName("MEGASHARK") == true ))
     {
     MShotsFired=0;megaShooting=false; bulletsPerTarget=4; //Reset variables
     ShowTeamMessage(8, "MEGASHARK: Home 4 bullets per Target", WormTeam);
     }    

  } 
   
}

require utils, p_sprite_builder, sprite_aim_guns, sprite_sheet_red;                                              

override void CTurnGame::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData) { 
 super;                                                                             
  if (Type == M_FRAME) {                                                         
     if (gframe == 2) {                                                           
      local weapName; local sprite;

      CWormAnimParams* Params = new CWormAnimParams;
      weapName = "MEGASHARK";                                               
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
      
      Params->subSpriteNumber = 1;
      Params->subSpriteFiring = 1;  
      Params->numSpritesInSheet = 12;
                                    
      local WA = new WormSprite(weapName, sprite, Params);        
  }                              
  else if (Type==M_TURNBEGIN){  
  super;
  }                                                                                
 }
} 


CSprite * megabullet;

void mega_spb::InitGraphic()
{

CFile *f; f = GetAttachment("clorophyte.png"); megabullet = LoadSprite(f,1,0);

}

override void CMissile::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData){    
super;
  if (Type == M_FRAME){                                                          //
     if ( weap->CheckName("MEGASHARK") ) { 
      if (true){                                  //   11. replace with your weap name
         if (megashark_anim) {                                              //   12. replace "toxic_paintBall_anim" with variable from 15.
              local sprite = redWeapSheet->Index;                      //   13. replace "toxic_paintBall_bullet" with variable from 2.
              WeapSpriteParams* Params = new WeapSpriteParams;
              Params->animate = true;
              
              Params->useSpriteSystem = true;
              Params->subSpriteNumber = 4;
              Params->numSpritesInSheet = 12;
              
              new WeapSprite(this, sprite, Params);   
              megashark_anim = false;                                       //   14. replace "toxic_paintBall_anim" with variable from 15.
         }                                                                     
     }}                                                                                                                                                     
  }                                                                              //
}                                                                                //
override CMissile::CMissile(CObject* parent,CWeaponLaunch* ldata,CShootDesc* sdata){
super;                                                                           //
  megashark_anim = true;                                                    //   15. rename variable to YOUR_WEAP_anim
}                                                                                                                    