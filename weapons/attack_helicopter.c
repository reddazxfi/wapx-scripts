require utils, weapon_air_911, utils_red, sprite_aim_guns, sprite_sheet_red, p_sprite_builder:

heliShoot:CMissile
CSprite * helicoptrempty;
CSprite * wrmAim;

CSoundFile *HKShoot1;   
CSoundFile *HKShoot2;    
CSoundFile *HKShoot3;
CSoundFile *heli2Wav;

 void attack_helicopter::FirstFrame()
 {
    HKShoot1 = new CSoundFile(GetAttachment("hks1.wav"));    
    HKShoot2 = new CSoundFile(GetAttachment("hks2.wav"));
    HKShoot3 = new CSoundFile(GetAttachment("hks3.wav"));
    heli2Wav = new CSoundFile(GetAttachment("helicopterloop2.wav"));
 }
 
 void attack_helicopter::InitGraphic()
 {
  CFile *f;
  f = GetAttachment("heliempty.png");
  helicoptrempty = LoadSprite(f,2,0); 
  f = GetAttachment("def_shot_aim_p.png");
  wrmAim = LoadSprite(f,32,1);
 }
 
heliShoot::heliShoot(CObject* parent,CWeaponLaunch* ldata,CShootDesc* sdata)
{         
    bulletCount = 17;
    CurseTarget = CWorm(NullObj);
    fireBulletAnim = false;   
    FreeMe = false;
    regDir = 0.0;
    ShotsFired = 0;
    Hfalling = false;
    isShooting = false;
    adjustCooldown = 0;   
    OwnerTeam = sdata->Team;
    targetXP = sdata->AddX; targetYP = sdata->AddY;  
    shootAngle = 0.0;       gunAngle = -MATH_PI;        
    spriteXOffset = 0.0;    spriteYOffset = 0.0;
    wormFrame = 0.0;        heliRotation = 0.0;
    super; 
    ColMask = new CColMask(15,15,MakeCircleMask(15));
    heli2Wav->Play(1.2, 0.0, false);   
    FortySeven = 47; 
    misIndex-=2;
   //UpdateRotation(true);
};
            
void CMissile::PlayShootingSound(bool isDry)
{
  if (isDry)
  {
    dryShoot->Play( CalculateSoundVolume(PosX,PosY), CalculateSoundPan(PosX,PosY), false);
    return;
  }
  local rndsnd = RandomInt(1,3);
  if (rndsnd == 1)
    HKShoot1->Play(CalculateSoundVolume(PosX,PosY) * 0.8, CalculateSoundPan(PosX,PosY), false);
  else if (rndsnd == 2)    
    HKShoot2->Play(CalculateSoundVolume(PosX,PosY) * 0.8, CalculateSoundPan(PosX,PosY), false);
  else      
    HKShoot3->Play(CalculateSoundVolume(PosX,PosY) * 0.8, CalculateSoundPan(PosX,PosY), false);  
} 

void heliShoot::UpdateRotation(bool spawned)
{
    // Instantaneous angle from the helicopter to the target
    float instantAngle = atan2(targetYP - PosY, abs(targetXP - PosX));

    // Clamp
    if (instantAngle > MATH_HALF_PI) instantAngle = MATH_HALF_PI; // Straight down
    if (instantAngle < -MATH_PI)  instantAngle = - MATH_PI;  // Horizontal

    // PROPORTIONAL INTERPOLATION
    float angleDifference = instantAngle - gunAngle;
    if (spawned) 
    gunAngle = instantAngle;
    else 
    gunAngle = gunAngle + (angleDifference * 0.75);
    
    // Worm
    local normalizedAngle = (gunAngle + MATH_PI) / (MATH_PI * 1.5); // 0.0 to 1.0
    if (normalizedAngle < 0.0) normalizedAngle = 0.0;
    if (normalizedAngle > 1.0) normalizedAngle = 1.0;

    wormFrame = int((1.0 - normalizedAngle) * 31.0); // 0=down, 31=up
}

void heliShoot::UpdateHelicopterRotation()
{
     if (regDir >= 0)
     {
        if ((SpX < -14.0 || SpX > 14.0) && ShotsFired < bulletCount) 
        {
           heliRotation = 0.3;
        }
        else if ((SpX > -14.0 && SpX < 14.0) && ShotsFired < bulletCount)
        {
          if (heliRotation>-0.115) heliRotation-=0.01;        
        }
        else if (ShotsFired >= bulletCount)
        {
          if (heliRotation<0.3) heliRotation+=0.01;   
        }
     }
     else
     {
        if ((SpX < -14.0 || SpX > 14.0) && ShotsFired < bulletCount) 
        {
           heliRotation = -0.3;
        }
        else if ((SpX > -14.0 && SpX < 14.0) && ShotsFired < bulletCount)
        {
          if (heliRotation<0.115) heliRotation+=0.01;        
        }
        else if (ShotsFired >= bulletCount)
        {
          if (heliRotation>-0.3) heliRotation-=0.01;   
        }
     }
}

void heliShoot::Draw()                                      
{              
    local flyingRight = (regDir >= 0.0);
    local turnflag = 0; 
    local targetIsRight = (targetXP >= PosX);
    
    local wormTurnflag = 0;
    if (flyingRight) wormTurnflag = 262144;
    else wormTurnflag = 0;
    
    if (!flyingRight)
     turnflag = 262144 ;
    else
     turnflag = 0;     
              
    if (gframe % 2 == 0) // Helicopter sprite needs work tbh
    {
      if (CurseTarget==NullObj) AddSpriteEx(10.0 + 0.01, PosX, PosY, turnflag + helicoptrempty->Index, 0, heliRotation, 1.0); 
    }
    else       
    {
      if (CurseTarget==NullObj) AddSpriteEx(10.0 + 0.01, PosX, PosY, turnflag + helicoptrempty->Index, 1, heliRotation, 1.0);
    }
    
    float finalRenderAngle = gunAngle;
    float spriteXOffset = 5.0; 
    float spriteYOffset = 12.0;
    
    if (Hfalling)
    {
       spriteXOffset = 5.0;  
       spriteYOffset = -15.0;
    }
        
    if (!flyingRight)
    {
        finalRenderAngle = MATH_PI - gunAngle; 
        spriteXOffset = -5.0;  
    }
    // Worm       
    AddSpriteEx(5.1, PosX + spriteXOffset - 2, PosY + spriteYOffset / 2, wrmAim->Index + wormTurnflag, subSprIndex(31, wormFrame), heliRotation * 0.9, 0.85);  
    // Draw the Gun  
    if (!fireBulletAnim) {
    AddSpriteEx(4.9, PosX + spriteXOffset, PosY + spriteYOffset, turnflag + gunAimSheet->Index, subSprIndex(11,4), MATH_HALF_PI + finalRenderAngle, 0.8); }
    else {
    AddSpriteEx(4.9, PosX + spriteXOffset, PosY + spriteYOffset, turnflag + gunAimSheet->Index, subSprIndex(11,5), MATH_HALF_PI + finalRenderAngle, 0.8); }           
}  

void heliShoot::TakeExplosionDamage(fixed x, fixed y, int dmg)
{
	if(dmg == 0)
        {
          return;
	}    
	float fdmg = dmg;
	float dx = PosX - x;
	float dy = PosY - y;
	
	float distsqr = dx*dx + dy*dy;
	float fdmgsqr = fdmg*fdmg*4.0;
	
	fdmg = (1.0 - distsqr/fdmgsqr) * fdmg;
	
	if (fdmg>(25)) Collide(this,1); //Collide with self, explosion flag is -1  :)
}                                                                                  

void heliShoot::Message(CObject* sender, EMType Type, int MSize, CMessageData* MData)
{            
    if(Type == M_EXPLOSION)
    {
        TakeExplosionDamage(MData->fparams[1], MData->fparams[2], MData->params[4]);
    }
    if (Type == M_DRAWQUEUE)
    {
        Draw();
    }
    if (Type == M_FRAME)
    {     
        if(CurseTarget == NullObj)
        {
          GravityFactor = 0;
          SpY = 0;
        }
        else
        { 
          GravityFactor = 1.0;    
        }
        if (SpX >= 0.0 && PosX >= GS->LevelSX + 550.0) 
            {Free(true); return;  }
        else if (SpX < 0.0 && PosX < -550.0)    
            {Free(true); return;  }
            
        if (FreeMe) Free(true);
        
        if (CurseTarget!=NullObj && !Hfalling)
        {
          local hParams = new WeapSpriteParams;
          new WeapSprite(this, helicoptrempty->Index, hParams);
          Hfalling = true;
        }
        //if (helicopter && (SpX != 0 || SpY != 0)){ if (gframe % 47) {} else HeliWav->Play(1.2, 0.0, false); }
        //if ((gframe - misIndex) % 47) {} else heli2Wav->Play(1.2, 0.0, false);
        if ((gframe - misIndex) > FortySeven)        
        {
           heli2Wav->Play(1.2, 0.0, false); // Fuck off
           FortySeven+= 47;
        } 
        if (ShotsFired >= bulletCount && !isShooting)
        if(PosY >= Env->Water && PosX<(GS->LevelSX+300) && PosX>(0-300)) //exploding too far crashes, dunno why
        {
            ExplodeAt(PosX, PosY);
            FreeMe = true; 
        }          
      //gunAngle = CalculateSmoothRotation(float(PosX), -288.000, targetXP, targetYP, gunAngle, 0.3, true, 1.57);
        UpdateHelicopterRotation();
        
        if (!isShooting && ShotsFired < bulletCount)
        {    
            UpdateRotation(false);
            
            float xDistanceP = abs(PosX - targetXP); 
        
            if (xDistanceP < 450)
            {       
                UpdateRotation(true);
                adjustCooldown = 40;
                isShooting = true;
                offsetPLANE = 0;
            }     
        }
        
        if (ShotsFired >= bulletCount && isShooting)
        {                              
            isShooting = false;
            SpX = 1.0 * (regDir * 0.15);
        }
        if (isShooting) SpX = SpX * 0.984;
        if (isShooting && adjustCooldown == 0 )
        {                                 
            fireBulletAnim = true;
            UpdateRotation(false);
            shootBullet();
            ShotsFired = ShotsFired + 1;
            adjustCooldown = 4;
        }
        else if (isShooting && adjustCooldown > 0)
        {      
            UpdateRotation(false);
            adjustCooldown = adjustCooldown - 1;    
            fireBulletAnim = false;
        } 
        
        if (ShotsFired >= bulletCount && !isShooting)
        {
            if (abs(SpX) < 15.0) SpX = SpX * 1.06;   
            fireBulletAnim = false;
        }     
    } 
    super(sender, Type, MSize, MData);  
}

void heliShoot::shootBullet()
{
    //Sprite Params
    PlayShootingSound(false);
    local blParams = new WeapSpriteParams;
    blParams->useSpriteSystem = true;
    blParams->numSpritesInSheet = 12;
    blParams->subSpriteNumber  = 1;
    //Launch Data
    local launchdt = getClustletsData();
    launchdt->explosion.damage = 13;
    launchdt->explosion.pushPower = 150;     
    launchdt->explosion.bias = 0;
    launchdt->anim.spriteIndex = 0;
    launchdt->anim.trailIndex = 0;
    launchdt->anim.type = WAT_TrackMovement;
    launchdt->timeBeforeExplosion = 3000;
    launchdt->countdownFrom = 0;
    //Shoot Desc
    CShootDesc SDesc;
    zero(&SDesc);
    SDesc.Team = OwnerTeam;
    SDesc.X = spriteXOffset + PosX + 10.0;
    SDesc.Y = spriteYOffset + PosY + 20.0; 
    SDesc.SpX = 0; //let M_FRAME handle direction
    SDesc.SpY = 0;
    //Projectile
    local mis = new CMissile(Root->GetObject(25, 0), launchdt, &SDesc);
    if (mis!=NullObj)
    {
       int hitX; int hitY;
       local targetxP = targetXP;
       local targetyP = targetYP;
       bool targVisible = false;
       
       local wormVisible = false; 
       local curwrm = GetCurrentWorm();
       
       local shootworm = FindWormInBox(targetXP - 130, targetXP + 130, targetYP - 70, targetYP + 80, true); //has curWorm check on that true    
       
       if (shootworm!=NullObj)
       {
         wormVisible = (TraceLine(this, PosX, PosY, shootworm->PosX, shootworm->PosY, CMASK_TERRAIN, &hitX, &hitY) == NullObj);  
       }
       if (!wormVisible)  //fallback (no traceline in find function, may have discarded visible worm to target a covered one)
       {
         local shootobj = FindShootableInBox(targetXP - 130, targetXP + 130, targetYP - 50, targetYP + 70); //no curWorm check, compare manually
       
         if (shootobj!=NullObj && shootobj!=curwrm)
         targVisible = (TraceLine(this, PosX, PosY, shootobj->PosX, shootobj->PosY, CMASK_TERRAIN, &hitX, &hitY) == NullObj);
       
         if (!targVisible)
         {
         if (regDir < 0)   //check underneath
         shootobj = FindShootableInBox(PosX - 30, targetXP + 80, targetYP - 90, targetYP + 120);  
         else
         shootobj = FindShootableInBox(targetXP - 80, PosX + 30, targetYP - 90, targetYP + 120);  
         }
         if (shootobj!=NullObj && shootobj!=curwrm)
         targVisible = (TraceLine(this, PosX, PosY, shootobj->PosX, shootobj->PosY, CMASK_TERRAIN, &hitX, &hitY) == NullObj);  
               
         if (targVisible) //tbh i'd fix it by adding a traceline in the function but shooting at bullshit points sometimes makes it kinda fun
         {
           targetxP = shootobj->PosX;
           targetyP = shootobj->PosY;       
         }
       }
       else if (wormVisible)
       {                                                                      
         targetxP = shootworm->PosX;
         targetyP = shootworm->PosY;  
       }
       mis->ClType = EObjectClass(1415);   
       
       mis->JumpAngle = targetxP + (RandomInt(-1,1));
       mis->JumpForce = targetyP; 
       new WeapSprite(mis, redWeapSheet->Index, blParams);
    }
    
    fireBulletAnim = true;
    
    PxParticle *p = new PxParticle(115, PosX, PosY + 10.0);
    p->SetAnimSpeed(0.1);
    p->SetLifeTime(100);
    float eject_ang = gunAngle + (1.570797) + RandomFloat(-0.3, 0.3);
    float eject_vel = RandomFloat(2.0, 3.0);
    
    p->SetVelocity(eject_vel * cos(eject_ang), eject_vel * sin(eject_ang));
    p->SetAirResistance(0.0);
    p->GravityFactor(1.0);
    p->SetAlpha(255);
    p->SetSize(1.0, 1.0); 
}
// i couldnt figure out cosine math with angle lol  
override void CMissile::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
 if (Type == M_FRAME && ClType == EObjectClass(1415) && (gframe - misIndex) < 3) //misIndex is gframe
 {
   GravityFactor = 0; //Stable direction.
   homeToPlace(JumpAngle,JumpForce, this, 35.0, 1.0); //very fast, instant turn. 
 }
  super;
}   
  
void heliShoot::ExplodeAt(fixed x,fixed y)
{        
            local fs = Root->GetObject(TI_FireStore, 0);
            planefdesc->X = PosX;
            planefdesc->Y = PosY - 1;
            planefdesc->Count = 7;
            planefdesc->UnkB = 1;
            planefdesc->Duration = 1500;
            planefdesc->UpPower = 55;
            planefdesc->Damage = 9;
            planefdesc->SpY = -3;
            planefdesc->SpX = -3;
            local fire = new CFire(fs, planefdesc, 0);
            planefdesc->SpX = 3;
            planefdesc->Y = PosY + 20;
            fire = new CFire(fs, planefdesc, 0); 
            planefdesc->SpX = 0;
            planefdesc->SpY = -8;
            planefdesc->Y = PosY - 3;
            fire = new CFire(fs, planefdesc, 0); 
            if (PosX<GS->LevelSX && PosX>0){
            local smoker = new CSmoker(Root, x, y, false);
            if (smoker!=NullObj)
            {
              smoker->turn = 0;  
              smoker->life = 1;    
            }
            }
            super(x,y);
} 

override void CWorm::FireFinal(CWeapon* Weap,CShootDesc* Desc)
{
    if (Weap->CheckName("COLADOR AEREO") == true)
    {     
        NineDesc.AddX = Desc->AddX; 
        NineDesc.AddY = Desc->AddY;   
        // SET THE GLOBAL TARGET FOR THE PLANE
        //planeTargetX = Desc->AddX;
        //planeTargetY = Desc->AddY;
        //planeTargetSet = true;
        
        Desc->Y = -288.0;
        
        if (Desc->SpX <= 0.0)                            
            Desc->X = -500.0;      // spawnpos left side
        else          
            Desc->X = GS->LevelSX + 500.0;  // spawnpos right side
            //GG->WriteToChat( 3, ItoA(GS->LevelSY), false);
        
        if (Desc->SpX <= 0.0)
            Desc->SpX = 15.0;   // hover right
        else   
            Desc->SpX = -15.0;  // hover left
        
        NineDesc.X = Desc->X;   
        NineDesc.Y = Desc->Y; 
        NineDesc.SpX = Desc->SpX; 
        NineDesc.SpY = Desc->SpY; 
        NineDesc.AddX = Desc->AddX;
        NineDesc.AddY = Desc->AddY;
        NineDesc.Worm = Desc->Worm; 
        NineDesc.Team = Desc->Team;              
        NineDesc.Delay = 3000;                   
        hitPL = false;
        
        if (NineDesc.Team < 0) NineDesc.Team = 0;
         
         //AirStrike params are bugged, gotta set them up
         Weap->launch.explodeInto = WExplode_Nothing;
         Weap->launch.anim.spriteIndex = 0;
         Weap->launch.anim.trailIndex = 0;
         Weap->launch.anim.type = WAT_TrackMovement;
         Weap->launch.timeBeforeExplosion = 20000;
         Weap->launch.countdownFrom = 0;
         Weap->launch.speedMultipler = 100;
         Weap->launch.movementRandomness = 0;  
         Weap->launch.sound.soundIndex = 0;
         Weap->launch.action = WAction_None;
         Weap->launch.gravityFactor = 0;
         Weap->launch.windFactor = 0;
         Weap->launch.launchSpeed = 100;   
         Weap->launch.explosion.flags = -1;    
         Weap->launch.explosion.bias = 10;
         Weap->launch.explosion.pushPower = 120;
         Weap->launch.explosion.damage = 80;
         
        //if (GS->LevelSY > 4000)  //mmmfgh so huge
        //{
         //NineDesc.Y = planeTargetY - 1000;
        //}
        
        local misss = new heliShoot(Root->GetObject(25, 0), &Weap->launch, &NineDesc);    //spawn missile
        if (misss!=NullObj)
        {  
         misss->regDir = NineDesc.SpX;
        }    
    }
    else 
    {
        super;
    }
} 