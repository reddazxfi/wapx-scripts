require utils, utils_steps, weapon_air_bunker_buster, utils_red, sprite_sheet_red;

CSprite * jewt;
CSoundFile * jew;             

void israelStrike::InitGraphic()
{
  CFile *f;
       
  f = GetAttachment("jewt.png"); //jet but jew
  jewt   = LoadSprite(f, 1, 0);
  f = GetAttachment("havajew.wav");
  jew = new CSoundFile(f);
}        

override void CWorm::FireFinal(CWeapon* Weap, CShootDesc* Desc)
{
 if (Weap->CheckName("Israel Strike") == true)
 {
   local Directn = 0;
   if (Desc->SpX > 0) Directn = 1;
   else Directn = -1;                                      
   PlayLocalSound(52, 5, 1.0, 1.0);
   Weap->launch.anim.spriteIndex = 1500;
   local strike = new Jewish(Root->GetObject(25, 0), Desc->AddX, Desc->AddY, Directn, &Weap->launch); 
 }
  else super;
}

Jewish : CMine;

Jewish::Jewish(CObject* Parent, fixed X, fixed Y, int dir, CWeaponLaunch* lnchdt)
{
    CMineParams MParams;
    zero(&MParams);
    MParams.Prefuse = 0;
    MParams.Fuse    = 9999;
    MParams.Radius  = 0;
    MParams.Damage  = 0;
    MParams.BlastPower = 0;
    
    ClType = EObjectClass(100);
    jewdata = lnchdt;
    strikeX = X; strikeY = Y;
    hittingX = 0.0; hittingY = 0.0 ;   
    local startX = 0.0;
    endingX = 0.0;   
    direction = RandomInt (1,2); 
    if (direction == 2) direction = -1;
    dir = direction;
    
    if (direction == -1) { startX = GS->LevelSX + 300; endingX = -300.0; }
    else           { startX = -300.0; endingX = GS->LevelSX + 300.0; }
    
    CShootDesc SDesc;
    zero(&SDesc);
    SDesc.X = startX;
    SDesc.Y = -300;
    SDesc.SpX = dir * 12;
    SDesc.SpY = 0; 
    
    misIndex = gframe;
      
    super(Parent, &MParams, &SDesc, false, 0);
    
    jewdata = lnchdt;
    striking = false;  
    strikeHit = false;
     
    wrmsp = RandomInt(1,2);       
    if (wrmsp == 1) wrmsp = 242;
    else wrmsp = 189;  
          
    wrmsp2 = RandomInt(1,2);       
    if (wrmsp2 == 1) wrmsp2 = 242;
    else wrmsp2 = 181;      
 
    hosp = RandomInt(1,2);
    if (hosp == 1)hosp = 20;
    else hosp = -20;
      
    jew->Play( CalculateSoundVolume(strikeX,strikeY), CalculateSoundPan(strikeX,strikeY), false);
}
     
void Jewish::DrawVictims()
{    
   
 if (!strikeHit)
 {
  Frame = GS->Tick * 0.016;
  int HitX; int HitY;
  sickWorm = TraceLine(this, int(strikeX) , -100, int(strikeX), Env->Water, -1, &HitX, &HitY);
  hittingX = HitX; hittingY = HitY;
  if (sickWorm!=NullObj)
  {
      AddSpriteEx( 7.2 ,HitX , HitY-5, wrmsp, Frame+ 0.002, 0, 0.8);
      
      hospital = TraceLine(this, int(strikeX+hosp) , -100, int(strikeX+hosp), Env->Water, -1, &HitX, &HitY);
      
      if (hospital!=NullObj)
      {
          AddSpriteEx( 15.1 ,HitX, HitY -10, 97, Frame + 0.008, 0, 1.0);
      } 
      poisonedWorm =  TraceLine(this, int(strikeX-hosp) , -100, int(strikeX-hosp), Env->Water, -1, &HitX, &HitY);    
          
      if (poisonedWorm!=NullObj)
      {   
          AddSpriteEx( 7.3 , HitX , HitY -5, 262144 +wrmsp2, Frame, 0, 0.76);
      }
  }
 }
}   

void Jewish::Draw()
{     
   if (direction == 1) AddSpriteEx( 5.1 ,PosX , PosY, jewt->Index, GS->Tick * 0.022, 0, 1.4);
   else AddSpriteEx( 5.1 ,PosX , PosY, 262144 + jewt->Index, GS->Tick * 0.022, 0, 1.4);
}

void Jewish::Render()
{

} 

void Jewish::TakeExplosionDamage(fixed x, fixed y, int dmg)
{
	if(dmg == 0)
        {
          return;
	}    
	float fdmg = dmg;
	float dx = hittingX - x;
	float dy = hittingY - y;
	
	float distsqr = dx*dx + dy*dy;
	float fdmgsqr = fdmg*fdmg*4.0;
	
	fdmg = (1.0 - distsqr/fdmgsqr) * fdmg;
	
	if (fdmg>(0.05)) strikeHit = true;
}

void Jewish::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
 super;
 if (Type == M_DRAWQUEUE) 
 {
      DrawVictims();
                  
      if ((direction == 1 && PosX <= endingX) || (direction == -1 && PosX >= endingX)) Draw();
 }   		
 if(Type == M_EXPLOSION)
 {
      TakeExplosionDamage(MData->fparams[1], MData->fparams[2], MData->params[4]);
 }
  if (Type == M_FRAME)
 {       
      if (gframe > misIndex + 800) Free(true);
      if (strikeHit &&  (direction == 1 && PosX >= endingX) || (direction == -1 && PosX <= endingX)) Free(true);
      GravityFactor = 0;
      SpY = 0;
      if (!striking && ((direction == 1 && PosX >= strikeX) || (direction == -1 && PosX <= strikeX))  )  
      {
      CShootDesc SDesc;
      zero(&SDesc);
      SDesc.X = PosX;
      SDesc.Y = PosY+20;
      SDesc.SpX = 0;
      SDesc.SpY = 3.1;
      SDesc.Worm = 0;
      SDesc.Team = 0;
      SpX=SpX * 1.5;
      
      jewdata->explosion.damage = 80;                          
      jewdata->explosion.bias = 20;
      jewdata->explosion.pushPower = 115;    
      jewdata->explosion.flags = -1;
      jewdata->timeBeforeExplosion = 15000;
      jewdata->gravityFactor = 250;
      jewdata->speedMultipler = 92;
      jewdata->spriteSize = 4;
      jewdata->explodeOnSpace = false;
      jewdata->anim.trailIndex = 116;
      jewdata->anim.type = WAT_TrackMovement;
      jewdata->anim.trailPower = 20;
      jewdata->anim.trailSpeed = 60;
      jewdata->windFactor = 0;
      mis = new CMissile(Root->GetObject(25,0), jewdata, &SDesc);
      if (mis!=NullObj)
      {
        mis->WindFactor = 0;
        mis->SpX = 0;
        mis->misIndex = 99967;
        bunkerbusterFallSound->Play(CalculateSoundVolume(PosX,PosY), CalculateSoundPan(PosX,PosY), false);
      }
      striking = true;
      }
 }
}  

override void CMissile::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData){    
super;
  if (Type == M_FRAME){                                                    
     if (launchdata.anim.spriteIndex == 1500) {                            
         if (hydrogenbomb) {                                            
              local sprite = redWeapSheet->Index;                    
              WeapSpriteParams* Params = new WeapSpriteParams;
              Params->animate = true;
              
              Params->useSpriteSystem = true;
              Params->subSpriteNumber = 5;
              Params->numSpritesInSheet = 12;
              
              new WeapSprite(this, sprite, Params);   
              hydrogenbomb = false;                                    
         }                                                                     
     }                                                                                                                                                     
  }                                                                              
}                                                                               
override CMissile::CMissile(CObject* parent,CWeaponLaunch* ldata,CShootDesc* sdata){
super;                                                                           
  hydrogenbomb = true;                                              
}   