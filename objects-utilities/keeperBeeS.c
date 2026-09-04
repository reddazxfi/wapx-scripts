require utils, utils_red, p_sprite_builder, utils_steps, utils_red, utils_r2;

bool stopBee;
int abeeTeam;  //Failsafe bee team check

#BEE 

CBee : CMine;  //Inherit Mine 

CSprite* beeBaseSprite;

CSoundFile* bzz1;
CSoundFile* bzz2;
CSoundFile* bzz3;
CSoundFile* bzz4;

void keeperBeeS::FirstFrame()
{
   stopBee = false;
}

void keeperBeeS::Init()
{
    bzz1 = new CSoundFile(GetAttachment("bzz1.wav"));
    bzz2 = new CSoundFile(GetAttachment("bzz2.wav"));
    bzz3 = new CSoundFile(GetAttachment("bzz3.wav"));
    bzz4 = new CSoundFile(GetAttachment("bzz4.wav"));
}

CColMask * beeCheckMask;
 
void keeperBeeS::InitGraphic()
{
    CFile *f;  
    f = GetAttachment("beespr.png");   beeBaseSprite = LoadSprite(f, 4, 0);    
	
    beeCheckMask = new CColMask(13,13,MakeCircleMask(13));
                                                                        
    f = GetAttachment("bee_spb.png");                          
    beebee_aiming = LoadSprite(f, 1, 0);                                   
    f = GetAttachment("beemisspr.png");                           
    beemisspr = LoadSprite(f, 2, 0);                                
    f = GetAttachment("spb_homing_bee.png");
    beelauncherspr = LoadSprite(f, 2, 0);
} 

CBee * createBee(fixed x, fixed y, int team, CWorm* owner, bool spawnedFromBarrel )
{  
     float safeX = x; float safeY = y;
     
     CShootDesc SDesc;
     zero(&SDesc);
     SDesc.Team = team;
     SDesc.SpX = 0;
     SDesc.SpY = -1;  
     SDesc.X = x;
     SDesc.Y = y;
     
     if (spawnedFromBarrel)
     {
         SDesc.SpX = RandomFloat(-2.0 , 2.0);    
         SDesc.SpY = RandomFloat(-2.0 ,-6.0);
     }    
     //////////////////////////////////////////////////////// 
     newbee = new CBee(Root->GetObject(25, 0), &SDesc, owner, team);
     if (newbee!=NullObj)
     {
        local spawnOK = CheckSpawnPoint(x, y, x, y, newbee->ColMask, 1, CMASK_TERRAIN, &safeX, &safeY, 50);  //ignore the game's constructor choices because it doesnt fucking have a clue sometimes
        if (spawnOK)                                                            //That 50 is the maximum radius spawn limit (dont go too far)
        {
           newbee->PosX = safeX;
           newbee->PosY = safeY;
        }
        else if (!spawnOK && owner!=NullObj) 
        {
           owner->PlayLocalSound(120, 3, 1, 1);
           owner->nAvalShoots = 1;
           owner->nTotalShoots = 0;
           newbee->FreeMe = true;
           return NullObj;
        } 
        else if (!spawnOK && owner==NullObj)
        {
           //let the map editor reconsider their life choices by seeing a bee inside a wall (they placed it there)
        }
        if (spawnedFromBarrel)
        {
           newbee->spin = true;
           newbee->spinN = -16;
        }
        return newbee;
     }
     else
     return NullObj;
}

///////////////OVERRIDES///////////////
void CBee::Render()
{
}

void CBee::LookAtMe(fixed x,fixed y,int priority)
{
priority = 100; //Never looks at bee

if (beeTarget!=NullObj) priority = 12;  //Gain priority    
else priority = 100; //Never looks at bee

super;      
}    

///////////////////////////////--NEW CLASS--///////////////////////////////
CBee::CBee(CObject *parent, CShootDesc *Sdesc, CWorm *launcher, int team)
{                                                                 
 FreeMe = false;         
 fflags = 4+8+16+32;  // 5 - ground, 6 - ?, 7 - ?, 22 - ?, 14 - ?       
          
 CMineParams MParams;
 zero(&MParams);
 MParams.Prefuse = 0;
 MParams.Fuse    = 9999;
 MParams.Radius  = 0;
 MParams.Damage  = 48;
 MParams.BlastPower = 35;  
 isMagnetic = false;
        
 SetLayerOverride(LAYER_OILDRUM);     
      
 super(Root->GetObject( 25, 0), &MParams, Sdesc, false, 0);  
 ColMask = new CColMask(10,10,MakeCircleMask(10));    
    
 ClType = EObjectClass(100);
    
 PosX = Sdesc->X; PosY = Sdesc->Y;   
 barreltype = 67 ;   //Init this in case other custom barrel scripts are running    
 trueflag =true;    falseflag = false;
 OwnerTeam = team;
 OwnerColor = -1;
 //savedRotation = 0.0;
    
 WhereTheFuckAmI = 0;
        
 SpX = Sdesc->SpX;
 SpY = Sdesc->SpY;  
 
 beeSpriteBase = beeBaseSprite->Index;               
 ZPlane = 10.0;      
 isBee = true;   
 stopMe = true; 
 cancelRoam = false;
 softSpin = false; 
 spawnFrames = 0;
   
 timesHit = 0;       
 fflags = 4+8+16+32;  // 5 - ground, 6 - ?, 7 - ?, 22 - ?, 14 - ?  
 Dmg = 35;     
 HitPoints = 35; 
                   
 spinframeLimit = 50;
 animFrame = 0.0;    
 timerInactiveFrames = 0;
 lookoutCooldown = 0;
 delayTally = 0;
 frameTally = 0;
 even = 0;    
 
     supercharged = false;
 
 beehomeActive = false;  
 didBeeStart = false;      
 beeRot = 0.0;          
 goalRot = 0.0;   
 targetTilt = 0.0;      
 rotationComplete = false;  
 cancelRotation = false; 
 faceRight = false;  //this might just be left though            
 spin = false;
 spinN = 0; 
    
 SpX = RandomFloat (-1.1, 1.1);
 SpY = RandomFloat (-5.5,-1.2);  
 gravity = GravityFactor;   
 
 turnStopped = false;
 
 StartPointX = PosX;  
 StartPointY = PosY-22.2;    //The bee will 'circle' this path forever.     
 ArrivePointX = PosX;
 ArrivePointY = PosY-22.2; 
 WhereTheFuckAmI = 0;   
 outRangeX = 0.0;
 outRangeY = 0.0;
 fixed bstpX = 0.0;
 fixed bstpY = 0.0;
 
 Dead = false;
 
 homingCD = 12;     
 homingCDRoam = 10;
 homingDuration = 0;  
 beeTarget = CWorm(NullObj);   
 OwnerWorm = launcher;   

 if (abeeTeam!=0 && OwnerTeam == 0) //what the fucking hell
 {
  OwnerTeam = abeeTeam;
  abeeTeam = 0;
 }    
 if (OwnerTeam != 0)
 OwnerColor = GetTeamColor(OwnerTeam);    
    
 MakeSound();
 
 deadframes = 0;
 
 flowering = false;
 lookingAtFlower = 0;
 randomInterval = 40;
 
 randDeathTimer = RandomInt(60,130);
 
 bonk = 0;
 
 softSpin = true; 
                                             
 linkedEffect = CEffectManager(NullObj);   
// linkedEffect->SetTrail(10,2,0);
}

/////////////////////////////////--CODE--/////////////////////////////////

void CBee::Taze()
{
   supercharged = true;
   if (linkedEffect != NullObj) linkedEffect->Free( true );
   linkedEffect = attachPulseEffect(this, 11.0, 90, 130, 190);
   linkedEffect->SetBeamThickness(2.0);        
   linkedEffect->SetBeamGlowThickness( 12.0);
   linkedEffect->SetNoise(3.1);
   linkedEffect->SetVanishSpeedClamped(0.65);   
   linkedEffect->glow = false;
   linkedEffect->SetTrail( 15, 2, 0);
   
   if (#ELECTRIC_PLUGIN)TazeMine();
   
   /*linkedEffect->customTrail = true;
   linkedEffect->InsertInTrail( 0, 0) ;  
   linkedEffect->InsertInTrail( 500, 20) ;  
   linkedEffect->InsertInTrail( 300, -100) ;*/
}

void CBee::UnTaze()
{
   linkedEffect->Free(true);
   supercharged = false;
   spin = true;
   spinN = -40;
   
   if (#ELECTRIC_PLUGIN)
   {
      if (linkedEffect1 != NullObj)linkedEffect1->Free(true);     
      if (linkedEffect2 != NullObj)linkedEffect2->Free(true); 
   }
}

void CBee::beeDraw()
{    
    float baseX = PosX;
    float baseY = PosY;
    int   bframe = animFrame;

    int   renderFlags = 0; 
    float renderAngle = beeRot;
    bool  hascolormod = false;
	
    if(isSinking())
    {
	hascolormod = true;
	SetColorMod(RGB(90, 90, 240), 6);
    }  
    else if (supercharged)
    {
     hascolormod = true;
     SetColorMod(RGB(200,230,255),14);
    }

    if (!faceRight)
    {
        renderFlags = 262144; 
        renderAngle = -beeRot; 
    }
    else
    {
        // Face Left (Original direction i think)
        renderFlags = 0;
        renderAngle = beeRot;
    }
    
    AddSpriteEx(ZPlane + 0.03, baseX, baseY, renderFlags + beeSpriteBase, bframe, renderAngle, 0.93);

    if(hascolormod) ClearColorMod();
    
    if (supercharged) drawGlowCircle(PosX, PosY, 13.2, 0.4, RGB(50,90,160));
}

bool CBee::IsThereLandThere(fixed trgtX, fixed trgtY, bool targeting)
{
    if (this == NullObj || FreeMe) return false;
    
    int hitX; int hitY;
    int flags = CMASK_TERRAIN;
    int b = 6; // buffer, square of 12px. Would be better if TraceLine allowed float of 6.5.
    if (!targeting) flags = -1;
    // Offset target upward
    fixed targetYOffset = trgtY - 3;  // Tune this value
    
    // check the CENTER path
    if (TraceLine(this, PosX, PosY, trgtX, targetYOffset, -1, &hitX, &hitY) != NullObj)return true;
    
    // check the 4 corners of the rectangle (start corners, target corners offset upward)
    if (TraceLine(this, PosX - b, PosY - b, trgtX - b, targetYOffset - b, -1, &hitX, &hitY) != NullObj)return true;
    if (TraceLine(this, PosX + b, PosY - b, trgtX + b, targetYOffset - b, CMASK_TERRAIN, &hitX, &hitY) != NullObj)return true;
    if (TraceLine(this, PosX-1 - b, PosY + (b-1), trgtX - b, targetYOffset + (b+1), CMASK_TERRAIN, &hitX, &hitY) != NullObj)return true;
    if (TraceLine(this, PosX+1 + b, PosY + (b-1), trgtX + b, targetYOffset + (b+1), -1, &hitX, &hitY) != NullObj) return true;
    if (!targeting && CheckMaskAt(this,ColMask,trgtX,trgtY+3,-1)!=NullObj) return true;
    
    return false;
}

void CBee::DetermineRotation()
{       
	if (FreeMe || Dead) return;
    if (cancelRotation) return;
    float dx = ArrivePointX - PosX;
    float dy = ArrivePointY - PosY;
    float dist = sqrt(dx*dx + dy*dy);
    
    
    if (flowering)
    { 
        if (#Flowers)
        {    
            dx = FlowerPosX[lookingAtFlower] - PosX;
            dy =(FlowerPosY[lookingAtFlower] - 28) - PosY;
        }
    }
    // 1. Determine Direction (Face Right/Left)
    // Only update direction if we are moving significantly to avoid "flickering"
    if (absfloat(dx) > 0.5) 
    {
        faceRight = (dx > 0);
    }

    // 2. Calculate targetTilt
    targetTilt = 0.0;
    
    // If we are far from the goal, tilt toward the path
    if (dist > 25.0) 
    {
        targetTilt = atan2(dy, absfloat(dx));
        
        // Clamp tilt so it doesn't do a backflip (max 45 degrees)
        if (targetTilt > 0.785) targetTilt = 0.785;
        if (targetTilt < -0.785) targetTilt = -0.785;
    }
    // If close (hovering), targetTilt remains 0.0 (Normalize/Level out)
    
    // 3. Dynamic Rotation Speed
    // If the angle gap is large, rotate faster. 
    // If we are very close to the target, snap faster to look at it.
    float angleGap = absfloat(targetTilt - beeRot);
    float baseStep = 0.125;
    
    if (angleGap > 0.5) baseStep = 0.25;  // Turn faster if gap is wide
    if (dist < 35.0) baseStep = 0.25;    // Quick snap when arriving

    // 4. Smoothly apply rotation
    if (angleGap <= baseStep) 
    {
        beeRot = targetTilt;
    } 
    else if (beeRot < targetTilt) 
    {
        beeRot += baseStep;
    } 
    else 
    {
        beeRot -= baseStep;
    }
}

void CBee::takeGun(int p3, int p4, int dmg, int type, CObject* tfhitme)
{       
       if (FreeMe || Dead || dmg <= 0  || HitPoints <= -15) return;
       float knock = 1.0;
       bool returnEarly;
       /* CObject* sent = CObject(tfhitme);
       if (tfhitme is CWeapon == true)
       {     
       GG->WriteToChat( 7, "cum", false) ;
       knock = 1.1;
       HitPoints = HitPoints - 20;  // Ninja star, javelin etc, except idk how to do this
       returnEarly = true;
       } */
       if (type == 5 || type == 9) //Arrow and melee prolly
       {
       knock = 1.2;
       HitPoints = HitPoints - 25;   
       returnEarly = true;
       }
       if (p3!=0) SpX = (p3 / 65536) * knock ;  //Dividing raw values because the oildrum cant get knockback
       if (p4!=0) SpY = (p4 / 65536) * knock ;   
       
       MakeSound();
       
       if (returnEarly) return;   
       if (type == 10 && HitPoints != 0)
       HitPoints = HitPoints / 2;     
       else
       HitPoints = HitPoints - dmg;   
       
       
        if (HitPoints<=-10)
        {
         Dead = true;
         HitPoints = 0;
        }
}  
 
void CBee::TakeExplosionDamage(fixed x, fixed y, int dmg) //Used
{                        
	if (FreeMe || Dead || dmg <= 0 || HitPoints <= -1) return;
	
        spin=true; //Bee is autistic and afraid of fireworks      
        spinN += 8;
        
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
	dmg = fdmg;
	if (dmg > 0) 
        {
         HitPoints -= dmg;
	 beeTarget = CWorm(NullObj);
	 spinN -= 8; 
         MakeSound();
        }
        if (HitPoints<=0)
        {
         Dead = true;
         HitPoints = 0;
        }
}

  /////////////////////////////////////////////////////////////////////////////// 
 /////////////////////////////////// MESSAGE ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void CBee::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{    
 if (Type == M_GUNEXP && timesHit == 0)
 {      
       if (spawnFrames < 60 && !FreeMe) return;
       softSpin=true;       
       spinN = 0;         
            
       timesHit = 1;   //Will tank ANY gun hit at first.        
       takeGun(MData->params[3], MData->params[4], MData->params[5], MData->params[0], sender);
       if (HitPoints <=0) HitPoints = 1; 
       //return;
 }
 if (Type == M_GUNEXP)
 {
       softSpin=true;   
       spinN = 0;
       takeGun(MData->params[3], MData->params[4], MData->params[5], MData->params[0], sender);    
       if (HitPoints <=0) spinframeLimit = 30;
       //return;
 }     
 if (Type == M_EXPLOSION && !FreeMe)
 {
       if (spawnFrames < 80) return;
       if (HitPoints > 0)spinN = RandomInt(11,25);
       
       TakeExplosionDamage(MData->fparams[1], MData->fparams[2], MData->params[4]);
 } 
 super(sender, Type, MSize, MData);
 
 if (Type==M_DRAWQUEUE) beeDraw(); 
    
 if (Type==M_FRAME)
 {  
    if (stopMe) //Outside of M_FRAME because why not;
    { cancelMovement(); } //Freeze
    //if (FreeMe){ Free(true); return; }
    if (Dead) deadframes++; 
    if (Root->IsTimerActive() == false) { timerInactiveFrames++; if (timerInactiveFrames > 145) turnStopped = true; }
    else if (Root->IsTimerActive() == true && turnStopped && beeTarget == NullObj ) { spin = true; spinN = 25; turnStopped = false; }
    
    if  (timerInactiveFrames > 200) { stopMe = true; }
     
    if (Root->IsTimerActive() == true)
    { timerInactiveFrames = 0; stopMe = false; } //GravityFactor = gravity;  }
    
    if (spawnFrames < 100) spawnFrames++;
    
    beeRot = NormalizeAngle(beeRot) ;  //Utils function
    
    if (HitPoints <= -18)   //Overkill, explode instantly instead of waiting for spin
    {   
       FuckingDie();	     
    }
    if (deadframes > randDeathTimer)  //Funny effect using a bug, bee has a heart attack, falls and then explodes.
       FuckingDie();
    
    if (OwnerWorm == NullObj)
     if (OwnerTeam>0 && OwnerTeam < 8)  //If team is valid but ownerworm isn't
       if (GetCurrentWorm()!=NullObj) {
         if (GetCurrentWorm()->WormTeam == OwnerTeam) OwnerWorm = GetCurrentWorm(); OwnerColor = GetTeamColor(OwnerTeam); }
    
    if (!Dead && !FreeMe && !isSinking()) animFrame += 0.001 * 4.0 * 50.0;      //Animation frame * Frame Amount * Frame Speed
    if (animFrame>1.0) animFrame = 0.0;    
 } 
 if (Type == M_FRAME) 
 {
    if (FreeMe == true) 
    {
       Dead = true; 
       Free(true); 
    }
     
    if (!isSinking() && !FreeMe) BeeThink() ; 
    else if ( isSinking() ) { stopMe = true; GravityFactor = gravity * 0.75;  cancelMovement(); SpY = 1.5; }
 }   
} 

void CBee::FuckingDie()
{       
//int do_custom_explosion(CGObject * sender, int flags, float x, float y, int dmg, int pushPower, float destroyRadius, bool destroy, bool particles, bool defSound, bool taze)
        if (supercharged)
        {
            local i = do_custom_explosion(this, -1, PosX, PosY, 55, 120, 40, true, false, true, supercharged, 0); 
            drawKachaw(PosX, PosY);
            local eff = createElectricExplosion(PosX,PosY,38);  
            eff->SetVanishSpeedClamped(0.85);
            eff->freeAfter = 20;
            eff->shouldEllipse = true;
        }
        else 
        {
            local i = do_custom_explosion(this, -1, PosX, PosY, 40, 80, 30, true, false, true, supercharged, 0);      
            drawKachaw(PosX, PosY);
            local eff = createEffectExplosion (PosX, PosY, 5.0, 40.0, 0.001, 255, 251, 0, false, 20, 0.8, 0.8);  
            eff->SetVanishSpeedClamped(0.80);
            eff->freeAfter = 18;
            eff->shouldEllipse = true;
        }
        
        
        //Free(true);   
        FreeMe = true;    
        Dead = true;
}

void CBee::HolyShitTheresAFlowerYippieee()
{
    flowering = true;
    if (#Flowers)
    {
        StartPointX = FlowerPosX[lookingAtFlower] ;    
        StartPointY = FlowerPosY[lookingAtFlower] - 28.0 ;
    }
}

bool CBee::IsThereADeadWormRightNOWOutThere()
{
    for (int i = 0; i < Env->Objs.Count; i++)
    {
        CGObject* obj = CGObject(Env->Objs.Objs[i]);
        // Only consider worm objects
        if (obj == NullObj)
            continue;
        if (obj->ClType!=OC_Worm)
            continue;  
            
        local worm = CWorm(obj);
        if (worm == NullObj) continue;    
        if (worm->ObjState == WS_IDLE && GS->Info.GetWormHealth(worm->WormTeam, worm->WormNumber) == 0) 
             return true;
    }
    
    return false;    
}

void CBee::resetStart()
{
        StartPointX = PosX;
        StartPointY = PosY;
 
        GravityFactor = gravity;
        WhereTheFuckAmI = 0;     
        didBeeStart = false;  
}

void CBee::cancelMovement()
{
        SpX = 0;
        SpY = 0;          
        if (isSinking() == false) 
        GravityFactor = 0;
}

void CBee::doSpin()
{
        cancelRotation = true;
        stopMe = true;
        beeRot = beeRot + 0.165; 
        spinN = spinN +  1;
        beeTarget = CWorm(NullObj);     
        GravityFactor = 0;
}

void CBee::doSoftSpin()
{
        if (HitPoints <=0) beeRot = beeRot + 0.155;    
        else beeRot = beeRot + 0.135;    
        cancelRotation = true;     
        cancelRoam = true;
        spinN = spinN +  1; 
}

void CBee::cancelSpin()
{
        cancelRotation = false;
        stopMe = false;
        softSpin = false;
        spin = false;
        spinN = 0;               
        cancelRoam = false;
        GravityFactor = gravity;
}

void CBee::HardReset()
{       
        cancelSpin();                  
        cancelRotation = false;
        stopMe=false;
        stopBee = false;
        homingCD = 5;
        didBeeStart = false;      
        cancelRoam = false;           
        seekCooldown  = 0;        
        cancelMovement();
      
        if (homingDuration > 160) 
        {
                beeTarget = CWorm(NullObj);
                homingCD = 5;
                homingDuration = 0;
                lookoutCooldown = 40;
        } 
        //delayTally = 0;
        //homingDuration = 0;
      
       // cancelSpin();
}

void CBee::RoamAround()
{
        if (#Flowers)
        {
                lookingAtFlower = CheckFlowerProximity(this, 140);
                if  (lookingAtFlower!=98 && FlowerData[lookingAtFlower] != 0 )
                {
                        HolyShitTheresAFlowerYippieee();
                }
                else
                {
                        lookingAtFlower = -1;
                        flowering = false;
                }
        }  
        beeRoam();
        if 	(flowering)	
        {              
                if (#Flowers)
                {
                        if (gframe % randomInterval == 0)                                              
                        {   
                                if (IsThereLandThere(FlowerPosX[lookingAtFlower], FlowerPosY[lookingAtFlower] - 30.0, false) == false)
                                {           
                                        didBeeStart = true;
                                        ArrivePointX = FlowerPosX[lookingAtFlower];   
                                        ArrivePointY = FlowerPosY[lookingAtFlower] - 30.0;
                                }
                        }
                }
        }   
        if (gframe % 140 == 0) randomInterval = RandomInt(60,120);
        DetermineRotation();
        HomeToPlace(ArrivePointX, ArrivePointY);    //Random roam code
        GravityFactor = gravity;         
}

void CBee::ChaseSomeDude()
{                 
        flowering = false;    
        beehomeActive = true;    
        HomeToTarget(beeTarget); 
        homingCD = 2; // Cooldown before the next minor adjustment.
        homingDuration++;  
}
         
void CBee::isTargetTeammate()
{
    if (OwnerWorm!=NullObj)
    {
        if (beeTarget!=NullObj) 
        { 
            if (beeTarget == NullObj) {return;}
            
            if (beeTarget is CWorm == false) return;
            
            else if (beeTarget != NullObj && beeTarget->WormTeam == OwnerWorm->WormTeam) //If it somehow targets its owner because it somehow has an invalid team N. 
            {
                OwnerTeam = OwnerWorm->WormTeam;
                beehomeActive = false; 
                beeTarget = FindClosestEnemy(this);
            } 
        }
    }
}

void CBee::Collide(CGObject* Obj,int type)
{
 bonk++;
 super;
}
 
void CBee::BeeThink()
{      
    if (!spin && !softSpin && HitPoints <1)
    {               
	Dead = true;
        FuckingDie();     
	return;
    }  
    if (FreeMe) return;
        
    if (WhereTheFuckAmI > 180 && !stopMe)
    {
         resetStart();           
    }
    
    if (gframe % 10 == 0 && bonk > 0) bonk--;
    if (bonk >= 10) 
    {
        spin = true;
        spinN = 35;             
        ArrivePointX = PosX;
        ArrivePointY = PosY;
        bonk = 0;
    }
    if ((PosX < 20 ) && (PosY < -300))  Free(true);
    
    float distToSpawn = sqrt((PosX - StartPointX) * (PosX - StartPointX) + (PosY - StartPointY) * (PosY - StartPointY));
    
    if (spin && spinN == 0)
    {   
        if(spinframeLimit!=30) spinframeLimit = RandomInt(37,53);   //frame limit is 30 for hp = 0
        cancelMovement();
    }
    if (spin == true)
    {
        doSpin(); 
    }   
    if (softSpin && spinN == 0)
    {   
        GravityFactor = 0;
    }
    if (softSpin)
    {
        doSoftSpin();      
    }    
        
    if (spinN >= spinframeLimit)
    {    
        cancelSpin();
    }
    
        frameTally++;
        delayTally++;
    
    if (stopMe)
    
    { cancelMovement(); } //Freeze   
    if ((frameTally % 25) == 0) //Periodic check
    {
        local itadwrnot = IsThereADeadWormRightNOWOutThere(); 
     
        if (!itadwrnot && Root->IsTimerActive() == true) 
        {
            stopBee = false;
            stopMe = false;    
            //if (didBeeStart && absfloat(SpX + SpY) < 2.4) didBeeStart = false;
        }
    }
        
    if (stopBee) stopMe=true;                       
    
    if (frameTally > 3000) frameTally = 0;  //Avoid integer overflow if match is 10^18 seconds long
    
    if (delayTally > 180) delayTally = 0;
        
    if (delayTally >= 155 && Root->IsTimerActive() == true)  //hard reset because i did something wrong and cant bother to look into it (i did the bee months ago)
    {                                                                                   
         if (!IsThereADeadWormRightNOWOutThere() && beeTarget!=NullObj){ spin = true; spinN = 29; }
    }    
    
    if (spin || softSpin)
    {
         return;
    }   
            
    if ( distToSpawn > 676.7 && beeTarget == NullObj) 
    {
        WhereTheFuckAmI++;
    }
      
    if ( spawnFrames > 100 && homingCD>10 && !cancelRoam)
    {
        // STALLED
	homingCD = 5;	spin = true;
        //if ( abs(abs(StartPointX + StartPointY) - abs(PosX + PosY)) > 500 )      didBeeStart = true;
        if ( lookoutCooldown <=0 ) beeTarget = FindClosestEnemy(this); //hopefully it'll replace to NullObj if it isn't visible
        homingCD--;    
        if ( distToSpawn > 650.0 && beeTarget == NullObj) 
        {
             didBeeStart = false;
        }  
        
        RoamAround();
    }
    
    if (beeTarget == NullObj && !cancelRoam && !stopMe) 
    {
    // NO TARGET        
        RoamAround();
           
        //if (frameTally % 75 == 0) { local i = RandomInt(1,2); if (i == 2) didBeeStart = false;}  //randomly reset the bee's target because it might get stuck
    }
    
    if (lookoutCooldown > 0) { lookoutCooldown --; }
    
    if (beeTarget == NullObj && !cancelRoam) 
    {
        beehomeActive = false; 
        beeTarget = FindClosestEnemy(this);
    }
    
    isTargetTeammate();
    
    //Decision
    if (beeTarget != NullObj && !cancelRoam) 
    {
        if (beeTarget == NullObj) return; //what the hell  
        if (beeTarget is CWorm == false) return;
        float distToTarg = sqrt((PosX - beeTarget->PosX) * (PosX - beeTarget->PosX) + (PosY - beeTarget->PosY) * (PosY - beeTarget->PosY));      
        
        DetermineRotation();
        
        if ( distToSpawn > 420.0 ) 
        {
             lookoutCooldown = 150;
           
             RoamAround();    
           
             WhereTheFuckAmI += 50;       
             spin=true;
             return;
        }
        if (distToTarg > 470.0) 
        {
             beeTarget = CWorm(NullObj);
             beehomeActive = false;
             homingDuration = 0;
             lookoutCooldown = 100;     
             spin = true;
             return;
        }
        
        if (distToTarg < 26.0 && homingDuration>12) //Ideally dist is (Barrel Height)/2+1 + (Worm Height/2) +1 , but since i know how tall bee is, this works fine.
        {     
	     FuckingDie();    
        }
        if (distToTarg >= 26.0 && distToTarg < 38.0 && homingDuration>36) //Eh, good enough  
        {    
	     FuckingDie();   
        }
        if (homingDuration>160)  //AntiStall
        {
             beehomeActive = false;   
             homingDuration = 0;
             lookoutCooldown = 200;
             homingCD = 10;   
             WhereTheFuckAmI += 50;
             spin = true;
             return;
        }  
        
        //Target
        homingCD--;
        if (homingCD <= 0)
        {
             ChaseSomeDude();   
        }
    }
    
   if (stopBee && Root->IsTimerActive() == true) stopMe = true;
}

void CBee::CalculatePlace(int multiplier, int * outx, int * outy)
{    
	if (Dead || stopMe) return;
   float roamPY;  
   float roamPX;
   
   local circleY = StartPointY;  //Actually a rectangle lol
   local circleX = StartPointX;
   
   findDirX = RandomInt(-40 , 40);  
   findDirY = RandomInt(-77 , 77);  //Increased Height  
   
   if (flowering)
   {
      findDirX = RandomInt(-33 , 33);  
      findDirY = RandomInt(-46 , 46);   
   }
   
   roamPY = circleY - findDirY * multiplier;    
   roamPX = circleX - findDirX * multiplier;
   
   outRangeX = roamPX;
   outRangeY = roamPY;
   
   if (outRangeY >= (Env->Water - 37))  outRangeY = Env->Water - 37;  //Still enters water btw
   
   *outx = roamPX;
   *outy = outRangeY;
}

bool CBee::isSinking()
{
  if (PosY > Env->Water) return true;
  
  else return false;
}

void CBee::beeRoam()
{        
    if (Dead || stopMe) return;
    if (this == NullObj) return;
    // Check distance to arrival point
    float distToGoal = sqrt((PosX - ArrivePointX) * (PosX - ArrivePointX) + (PosY - ArrivePointY) * (PosY - ArrivePointY));  
    // If we reached the point or the bee is a dipshit, find a new point
    if (distToGoal < 20.0 || !didBeeStart)
    {
        if (WhereTheFuckAmI < 20) cancelMovement();
        
        targetTilt = 0.0;
        
        if (seekCooldown <= 0)
        {
            int arrivex; int arrivey;
            CalculatePlace(2.5, &arrivex, &arrivey);
            if (IsThereLandThere(outRangeX, outRangeY, false) == false)
            {
                ArrivePointX = arrivex;
                ArrivePointY = arrivey;
                didBeeStart = true;    
                GravityFactor = gravity;
                seekCooldown = RandomInt(30,90); // wait a bit before picking a new spot       
                WhereTheFuckAmI = 0;
                return;
            }
            CalculatePlace(1.95, &arrivex, &arrivey); 
            if (IsThereLandThere(outRangeX, outRangeY, false) == false)
            {      
                 ArrivePointX = arrivex;
                 ArrivePointY = arrivey;
                 didBeeStart = true;   
                 GravityFactor = gravity;
                 seekCooldown = RandomInt(25,80); // wait a bit before picking a new spot          
                 WhereTheFuckAmI = 0;
                 return;
            }
            didBeeStart = false;         
            GravityFactor = gravity;  //the bee falls in confusion
            WhereTheFuckAmI ++;
        }    
    }
    if (seekCooldown > 0) seekCooldown--;
}      

void CBee::HomeToTarget(CWorm* targttt)
{             
	if (Dead || stopMe) return;    
	
        if (isSinking() == true) { didBeeStart = false; homingCD = 30; return;  }
        
                if (this == NullObj) return;
                if (targttt == NullObj) return;           
                  // --- NEW DIRECT GUIDANCE LOGIC ---   thank you deep seek i cant do math
                  float missX = PosX;
                  float missY = PosY;  
                  // 1. Get the direction vector pointing straight at the target.
                  float dirX = targttt->PosX - missX;
                  float dirY = targttt->PosY - missY; 
                  
                  ArrivePointX = targttt->PosX;
                  ArrivePointY = targttt->PosY;

                  // 2. Normalize the vector to get a pure direction (length of 1).
                  float distanceToTarget = sqrt(dirX * dirX + dirY * dirY);
                  if (distanceToTarget > 0) // Avoid division by zero
                  {
                    dirX = dirX / distanceToTarget;
                    dirY = dirY / distanceToTarget;
                  }
                  
                  float missileSpeed = 7.0;   // How fast the missile tries to fly.
                  float homingStrength = 0.305; // How sharply it can turn (0.0 to 1.0)
                  
                  if (supercharged) missileSpeed = missileSpeed * 1.75;

                  // 4. Calculate the ideal velocity (direction * speed).
                  float requiredSpX = dirX * missileSpeed;
                  float requiredSpY = dirY * missileSpeed;

                  // 5. Gently steer the current velocity towards the ideal velocity.
                  // This prevents the zig-zagging and creates a smooth turn.
                  SpX += (requiredSpX - SpX) * homingStrength;
                  SpY += (requiredSpY - SpY) * homingStrength;
                  
                  if (gframe % 12 == 0) MakeSound();
}

void CBee::MakeSound()
{             
	if (Dead) return;
                  vol = CalculateSoundVolume(PosX,PosY);
                  vol = vol * 0.7;
    			  pan = CalculateSoundPan(PosX,PosY);

			    local i = RandomInt(1, 4);
    			if      (i == 1) bzz1->Play(vol * 0.8, pan, false);
    			else if (i == 2) bzz2->Play(vol, pan, false);
    			else if (i == 3) bzz3->Play(vol, pan, false);
    			else if (i == 4) bzz4->Play(vol * 0.5, pan, false);
} 

void CBee::HomeToPlace(float PointX, float PointY)    //generalistic function
{              
	if (Dead || stopMe) return;
                if (this == NullObj || beeTarget != NullObj || isSinking()) return;
                float missX = PosX;                                                                    
                float missY = PosY;  
                if (homingCDRoam<=0)
                {
                  float dirX = PointX - missX;
                  float dirY = PointY - missY;

                  float distanceToTarget = sqrt(dirX * dirX + dirY * dirY);
                  if (distanceToTarget > 0) // Avoid division by zero
                  {
                    dirX = dirX / distanceToTarget;
                    dirY = dirY / distanceToTarget;
                  }
                  float missileSpeed = 1.455;   // How fast the missile tries to fly.
                  float homingStrength = 0.425; // How sharply it can turn (0.0 to 1.0)
                  float requiredSpX = dirX * missileSpeed;
                  float requiredSpY = dirY * missileSpeed;
                  SpX += (requiredSpX - SpX) * homingStrength;
                  SpY += (requiredSpY - SpY) * homingStrength;
                  if (supercharged){  missileSpeed+=10.0; homingStrength+=0.25; }
                  homingCDRoam = 1; // Cooldown before the next minor adjustment. 
                }
                else if (homingCDRoam > 0)
                {
                     homingCDRoam--;
                }
}

CWorm* CBee::FindClosestEnemy(CBee* missile)   //Sentry copy paste, thanks entuser i guess
{
    if (this == NullObj || Dead || stopMe) return;
    
    if (lookoutCooldown > 0)     return;
    
    if (isSinking() == true)  return;
    
    CWorm* bestTarget = NullObj;
    
    float bestScore = 0;
    float sensorRadiusSqr = 47900.0 ;
    int maxIterations = 10500;    
    if (Env == NullObj || Env->Objs == NullObj)
    {
        return bestTarget;
    }
    for (int i = 0; i < Env->Objs.Count && i < maxIterations; i++)
    {
        CGObject* obj = CGObject(Env->Objs.Objs[i]);
        // Only consider worm objects
        if (obj == NullObj)
            continue;
            
        if (obj->ClType != OC_Worm)
            continue;
            
        CWorm* worm = CWorm(obj);    
        if (worm == NullObj) continue;
        // Comprehensive validity check
        if (worm->ObjState!=WS_WALKING)
        if (worm->IsMoving() == false) continue; //WTF
        if
           (worm->TargHealth <= 0         ||  
            worm->ObjState == WS_DEAD     || 
            worm->ObjState == WS_SINKING  || 
            worm->ObjState == WS_DEATH    ||
            worm->ObjState == WS_IDLE     ||  
            worm->ObjState == WS_FIRED    ||     
            worm->ObjState == WS_FIRECONT ||   
            worm->ObjState == WS_SETPOWER ||
            worm->ObjState == WS_AIMING)
            {
            continue;
            } 
          if (OwnerTeam>=1 && OwnerTeam<8)
           {if (GetTeamColor(worm->WormTeam) == OwnerColor)   continue;}
              if (OwnerWorm != NullObj){ if (worm == OwnerWorm) continue;}  
               
        float dx = worm->PosX - missile->PosX;            
        float dy = worm->PosY - missile->PosY;
        float distSqr = dx * dx + dy * dy;
        if (distSqr < sensorRadiusSqr)
        {
            float score = 1000000 - distSqr;
            if (IsThereLandThere(worm->PosX, worm->PosY, true) == false)
            {
                if (score > bestScore)
                {
                    bestScore = score;
                    bestTarget = worm;
                }
            } 
        }
    }   
    return bestTarget;
} 
  
override void CWorm::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
 super;
 if (Type == M_FRAME)
     {      
     if (Root->IsTimerActive() == true && GS->Info.GetWormHealth(WormTeam, WormNumber) == 0)
     if (ObjState == WS_IDLE || !IsMoving())
        {
            stopBee = true;   //Stop bee if a worm NEEDS to die.
        }
     }
}

override void CWorm::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
 super;
 if (Type==M_PRETURNSTART && stopBee) stopBee = false;
}

override void CWorm::FireFinal(CWeapon* Weap, CShootDesc* Desc)
{
    if (Weap->CheckName("Keeper Bee"))
    {               
      Desc->Y = Desc->Y - 12;
      abeeTeam = Desc->Team;
      createBee(Desc->X,Desc->Y,Desc->Team, this, false);
      return;
    }
    super;
}

CSprite* beebee_aiming;                                                      
CSprite* beemisspr;                                                
CSprite* beelauncherspr;
                                                                            
                                                                                 
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

override void CMissile::CMissile(CObject* parent, CWeaponLaunch* ldata, CShootDesc* sdata)
{
    isBeeMissile = globalBeeMissile;   // off by default, opted in after spawn in FireFinal
    globalBeeMissile = false;
    beeMOwner    = GetCurrentWorm();
    if (beeMOwner != NullObj)
    {
    beeMTeam     = beeMOwner->WormTeam;
    }
    if (isBeeMissile && (sdata->X != PosX || sdata->Y != PosY))
	{PosX = sdata->X; PosY = sdata->Y; }
    beeMFrame    = 0.0;
    beeMRot      = 0.0;
    beeMFaceDir  = 1;
    FreeMe = false;

    super;
}

override void CMissile::Message(CObject* sender, EMType Type, int MSize, CMessageData* MData)
{
    super;

//    if (!isBeeMissile) return;

    if (Type == M_FRAME)
    if (isBeeMissile)
    {
        if (FreeMe){ Free(true);  }
        beeMFrame += 1.0;
        if (beeMFrame >= 3.0) beeMFrame = 0.0;   // 3-frame cycle, double speed vs the ground bee

        // Same rotation formula WeapSprite::CalculateAngle uses, just inlined here
        local spd = sqrt(SpX*SpX + SpY*SpY);
        if (spd > 0.05)
        {
            beeMRot = -atan2(SpX, SpY) - MATH_PI;
        }

        if (SpX > 0.0) beeMFaceDir = 1;
        else beeMFaceDir = 0;
    }

    if (Type == M_DRAWQUEUE)
    if (isBeeMissile)
    {
        local spr = beemisspr->Index;
        if (beeMFaceDir == 0) spr = spr + 262144;

        AddSpriteEx(10.03, PosX, PosY, spr, beeMFrame, beeMRot, 0.93);
    }
}

override void CMissile::Free(bool FreeMem)
{
    if (isBeeMissile && FreeMem)
    {
		local spawnTeam = beeMTeam;
        local wrm = beeMOwner;
        if (wrm != NullObj) { spawnTeam = wrm->WormTeam; abeeTeam = wrm->WormTeam; }
        else abeeTeam = spawnTeam;
        
        float safeX = PosX; float safeY = PosY;
        local spawnOK = CheckSpawnPoint(PosX, PosY, PosX, PosY, beeCheckMask, 3, CMASK_TERRAIN, &safeX, &safeY, 100);  
        createBee(safeX, safeY, spawnTeam, wrm, false);   
    super;
    }
    else
    super;
}

bool globalBeeMissile; 

override void CWorm::FireFinal(CWeapon* Weap, CShootDesc* Desc)
{
    if (Weap->CheckName("Homing Keeper Bee"))
    {
        globalBeeMissile = true;
        float spawnX = Desc->X; float spawnY = Desc->Y;
        local spawnOK = CheckSpawnPoint(PosX, PosY, PosX, PosY, beeCheckMask, 3, -1, &spawnX, &spawnY, 100);  
        Desc->X = spawnX;   Desc->Y = spawnY;
        super;
    }
    else
    super;
}