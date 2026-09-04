require utils, utils_pyroman, p_sprite_builder, weapon_magnet, pxeffects, map_object_saw_media;

CSprite* sawImg;   
CSprite* sawEle;     

CSoundFile* saw_cut_1;
CSoundFile* saw_cut_2;
CSoundFile* saw_cut_3;
CSoundFile* saw_cut_4;

CSoundFile* clank_2;    

CSoundFile* eshock;

#CSAW

void saw_script::FirstFrame()
{
	CFile *f;
	f = GetAttachment("saw_cut_1.wav");		saw_cut_1 = new CSoundFile(f);
	f = GetAttachment("saw_cut_2.wav");		saw_cut_2 = new CSoundFile(f);
	f = GetAttachment("saw_cut_3.wav");		saw_cut_3 = new CSoundFile(f);
	f = GetAttachment("saw_cut_4.wav");		saw_cut_4 = new CSoundFile(f);  
         
	f = GetAttachment("clank-sound.wav");		clank_2   = new CSoundFile(f);      
	f = GetAttachment("electric-shock.wav");	eshock    = new CSoundFile(f);
}

void saw_script::InitGraphic()
{
   sawImg = LoadSprite(GetAttachment("sawsheet.png"),11,0);   
}

CSaw : CMine;
CSawSpawn : CObject;
                                              
CSaw::CSaw(CObject* Parent, CShootDesc* Desc, float sScale)
{   
    CMineParams MParams;
	zero(&MParams);
	MParams.Prefuse = 0;
	MParams.Radius = 0;
	MParams.Fuse = 70;
	MParams.Flags = 0;
	MParams.Bias = 0;
	MParams.Damage = 0;
	MParams.BlastPower = 0;
	
    ZethPlane = float(Desc->Delay);
    
    
    if (ZethPlane < 10.0) ZethPlane = 10.1;
    else if (ZethPlane > 25.0) ZethPlane = 25.1;
    SetLayerOverride(LAYER_OILDRUM);
    
    OwnerTeam = Desc->Team;       
    OwnerColor = GetTeamColor(Desc->Team);
    SAWTEAM = -1;
    
    OnTurnHitLimit = 88; //Anti loop
    OnTurnHits = 0;
    	
    transition1 = 0;
    isMagnetic = false;
    isDestructible = false;
    
    sawHP = 20.0;
    stopRotation = false;
    stopped = false;
    
    SawOut = 0;
    SawOutBlood = subSprIndex(10, 1); SawOutBlood2 = subSprIndex(10, 2); 
    SawIn = subSprIndex(10, 3); SawInBlood = subSprIndex(10, 4); 
    
    EleIn = subSprIndex(10, 8);  EleOut = subSprIndex(10, 5); 
    EleOutDmg = subSprIndex(10, 6); EleOutDmg2 = subSprIndex(10, 7);
    
    Outline = subSprIndex(10,9); OutlineDmg = 1.0;
    
    eleFuel = 30; //30 Worm hits.
    
    rotationFactor = RandomFloat(-0.125, -0.08);  //Radians
    
    scaleToHitboxRatio = 0.6727777777777; //Unused, How the game scales down Images to actual hitboxes (not really but its quite close, not enough to be satisfying though, checkmask is better)
    sawScale = sScale; // 1.0 = 200x200 px
    
    hitboxSize = sawScale * 200 * scaleToHitboxRatio - 2; //I think this is unused too. Scale, Pixel Size, ratio and minus 2 for good game design
    sawRotation = 0;
    timesCollided = 0; // For blood

    SawIndex = sawImg->Index;
    
    spawnX = Desc->X;
    spawnY = Desc->Y;
    
    super(Parent, &MParams, Desc, false, 0);
    ClType = EObjectClass(99);
    
    SawCollisionUnit = sawScale * (200/2) ; //radians
    
    ColMask = new CColMask(SawCollisionUnit-3, SawCollisionUnit-3, MakeCircleMask(SawCollisionUnit-3));    
    SawCheckMask = new CColMask(SawCollisionUnit, SawCollisionUnit, MakeCircleMask(SawCollisionUnit)); //Bigger than ColMask to allow overlapping  
        
    firstSpawned = true;
    
    sawZapAngle = 0.08267 * 2.0;  // current rotation offset of the effect
    supercharged = false;
    eleSoundPlayed = false;   
    linkedEffect = CEffectManager(NullObj);            
    
    soundMultiplier = sawScale + 0.625;
    if (soundMultiplier > 1.2) soundMultiplier = 1.2;
    else if (soundMultiplier < 0.25) soundMultiplier = 0.25;
    
    useEffectManager = true;
    
    ElectricityType = RandomInt(1,3);
} 

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////MESSAGE/////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

void CSaw::Message(CObject* sender, EMType Type, int MSize, CMessageData* MData)
{
 if (Type == M_PRETURNSTART || Type == M_TURNBEGIN) {OnTurnHits = 0; stopRotation = false; if (supercharged) eleFuel--; if (!isDestructible) stopped = false; }
 /*if (Type == M_DRAWQUEUE && supercharged)
 {
    SetColorMod(ARGB(255, 90, 110, 170), 8);  
    Draw(); 
    if(gframe % 10 == 0)  
    super;
                    
    ClearColorMod();
 }*/
 else if (Type == M_DRAWQUEUE) 
 {
    Draw(); 
 }
 if (Type == M_GUNEXP) return;
 if(Type == M_EXPLOSION && isDestructible)
 {
      TakeExplosionDamage(MData->fparams[1], MData->fparams[2], MData->params[4]); 
 }
 else if (Type == M_EXPLOSION && !isDestructible) return;
 super;
 if (Type == M_FRAME)
 {
   if (supercharged && !eleSoundPlayed)
   {
      Zap();  // For spawn script and things i might have missed. Tazer and Lightning directly trigger saw->Zap() anyways
   }             
   if (!isDestructible){ sawHP = 999;  eleFuel = 20;}
   if (sawHP < 0) sawHP = 0;
   
   if (sawHP == 0 && isDestructible)
   {
       stopped = true;
   }   
   if (supercharged) 
   {
      stopped = false;
   }   
   if (supercharged && eleFuel == 0 && sawHP == 0 && isDestructible)
   {    
      UnZap();
   }
   
   PosX = spawnX;
   PosY = spawnY;
   
   SpX = 0;
   SpY = 0;
   GravityFactor = 0;   
   SpeedDivider = 9999;
   
   if (firstSpawned)
   {
      if (ColMask == NullObj || ColMask == CColMask(NullObj)) ColMask = new CColMask(SawCollisionUnit-5, SawCollisionUnit-5, MakeCircleMask(SawCollisionUnit));       
      if (SawCheckMask == NullObj || SawCheckMask == CColMask(NullObj)) SawCheckMask = new CColMask(SawCollisionUnit, SawCollisionUnit, MakeCircleMask(SawCollisionUnit));
      firstSpawned = false;
   }
   //if (!isDestructible) { stopped = false; eleFuel = 20; }        
   if (gframe % 10 == 0) stopRotation = false;    //only for worms
   
   if (stopped) stopRotation = true;  
   else stopRotation = false;
   
   if (stopped) return;
   
   //Main Code          
   
   processSawCollission();   

   if (OnTurnHits < OnTurnHitLimit && !stopRotation && !stopped) sawRotation = sawRotation - rotationFactor;   //stop saw realistically if limit.   
   sawRotation = NormalizeAngle(sawRotation);  
   
   if (timesCollided >=8 && transition == 0)
   {
        rotationFactor = rotationFactor - 0.05;  
        transition = 1;
   }
 }
}

void CSaw::Zap()
{
    supercharged = true;
 
    SawOutBlood = EleOutDmg; SawOutBlood2 = EleOutDmg2; 
    SawIn = EleIn; SawInBlood = EleIn; 
    
    SawOut = EleOut;
    
    eleFuel = 30; //30 Worm hits.
    
    eshock->Play(CalculateSoundVolume(PosX,PosY) * soundMultiplier, CalculateSoundPan(PosX,PosY), false);
    
    eleSoundPlayed = true;
    
    if (ElectricityType == 3 && linkedEffect == NullObj)
    {
      linkedEffect = new CEffectManager(this, PosX, PosY, 0, 25, SawCollisionUnit * 0.67, 0.67, 167, 210, 255) ;  //more to the center
      linkedEffect->ZPlane  = ZethPlane - 0.3;
      linkedEffect->gZPlane = ZethPlane - 0.02; 
    } 
    
    else if (ElectricityType == 2 && useEffectManager && linkedEffect == NullObj)
    {
      linkedEffect = new CEffectManager(this, PosX, PosY, 6, 12, SawCollisionUnit * 0.75, 3.5, 80, 140, 255) ;
      linkedEffect->ZPlane  = ZethPlane - 0.3;
      linkedEffect->gZPlane = ZethPlane - 0.02; 
    } 
    
    else if (ElectricityType == 1 && useEffectManager && linkedEffect == NullObj)
    {
      linkedEffect = new CEffectManager(this, PosX, PosY, 0, 22, SawCollisionUnit * 0.75, 5.1, 167, 210, 255) ;
      linkedEffect->ZPlane  = ZethPlane - 0.3;
      linkedEffect->gZPlane = ZethPlane - 0.02; 
    }
}

void CSaw::UnZap()
{    
    SawOut = 0.0;  //Reset sprites.
    SawOutBlood = subSprIndex(10, 1); SawOutBlood2 = subSprIndex(10, 2); 
    SawIn = subSprIndex(10, 3); SawInBlood = subSprIndex(10, 4); 
    
    //SawDamagedSpr  

    supercharged = false;
 
    eleFuel = 0; //0 Worm hits.
    
    if (linkedEffect != NullObj) linkedEffect->targetObj = NullObj;   //It will free itself.
}

////////////////////////////////////////////CODE////////////////////////////////////////////

void CSaw::TakeExplosionDamage(fixed x, fixed y, int dmg)
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
	
	if (fdmg>(0.05)) sawHP -= fdmg;
}


void CSaw::processSawCollission()
{
        if (OnTurnHits > OnTurnHitLimit) return;

        for(int i = 0; i < Env->Objs.Count; i += 1)
	{
		obj = CGObject(Env->Objs.Objs[i]);

		if(obj == this) continue;
		if(obj == NullObj) continue;
		if(obj is CPlatform) continue;
		if(obj->IsStatic == true) continue;
		if(obj->IsMaterial == false) continue;
                if(obj->ClType == OC_Landscape) continue;
                if(obj->ClType == OC_Cross) continue;      
                if(obj->ClType == OC_OldWorm) continue;

               	if(SawCheckMask->Check(PosX, PosY, obj->ColMask, obj->PosX, obj->PosY))
		{
			if (obj is CSaw) continue;
			if (obj is CGObject == false) continue;  //Had it happen

			local hitPX = obj->PosX;
                        local hitPY = obj->PosY;

			if (obj is CWorm == true)
			{
                                local worm = CWorm(obj);
                                if (SAWTEAM > -1 && GetTeamColor(worm->WormTeam) == SAWTEAM)
                                {
                                        stopRotation = true;
				}
                                else if((SAWTEAM > -1 && GetTeamColor(worm->WormTeam) != SAWTEAM || SAWTEAM < 0) && worm->hitFrame >= 7 && worm->timesSliced < 26)
				{
					 if(worm->ObjState!=WS_DEAD ||
					 worm->ObjState!=WS_DEATH   ||
					 worm->ObjState!=WS_SINKING ||
					 worm->ObjState!=WS_SUICIDEBOMBER )
					 {
                                                 LookAtMe(PosX, PosY, 2);
                                        	 CSawCollission(worm);
				        	 worm->hitFrame = 0;

                                                 int particleNumber = 119; //Blood

                                                 if (worm->ObjState==WS_FROZEN || worm->ObjState == WS_DRILLING)
                                                 {
                                         	 	 ClankSound(hitPX, hitPY, CalculateSoundVolume(hitPX,hitPY) * soundMultiplier);
				         	         particleNumber = 81; //Spark
				                 }
	                                 	 local p = new PxParticle(particleNumber, hitPX, hitPY);
                                         	 p->SetLifeTime(12);
                                         	 p->SetVelocity(0,0);
                                         	 p->SetStartAlpha(255);
	                                 	 p->SetEndAlpha(255);
                                         }
				}
                                continue;
			}

		        if (obj is CCrate == true)
			{
			        local crate = CCrate(obj);
			        crate->sawhitlimit+=1;
			        if (crate->sawhitlimit > 5) continue;

                                ClankSound(hitPX, hitPY, CalculateSoundVolume(hitPX,hitPY) * soundMultiplier);

			        if (Root->IsTimerActive() == true)
			        {
			                OnTurnHits++;
                                        obj->Collide(this, 1);
			                SendMessage(obj, 5);
			                if (obj!=NullObj)
                                        ApplySawKnockback(obj, PosX, PosY, 0.77);
                                }
                                else  //Dont explode outside turns.
                                {
                                        ApplySawKnockback(crate, PosX, PosY, 0.77);
                                }
				continue;
			}

			if (obj is CMine == true)
			{
			        local amine = CMine(obj);
			        if (amine == NullObj) return;
			        if (amine->sawhitlimit<20 && amine->hitFrame >= 5)
			        {
			                OnTurnHits++;
			                
				        if (#ELECTRIC_PLUGIN)
				        {
				             if (supercharged && amine->ClType == OC_Mine) amine->TazeMine();
                          	        }
                                        amine->sawhitlimit = amine->sawhitlimit + 1;  
                                        SendMessage(obj, 15);
                                        if (amine!=NullObj)
                                        {
                                                ApplySawKnockback(obj, PosX, PosY, 0.85);
                                                amine->hitFrame = 0;
                                        }
                                        ClankSound(hitPX, hitPY, CalculateSoundVolume(hitPX, hitPY) * soundMultiplier);
				}
                                        //continue;
				continue;
                        }

			if (obj is COilDrum == true)
			{
			        local drum = COilDrum(obj);
			        drum->sawhitlimit+=1;
                                OnTurnHits++;
			        SendMessage(obj, 20);
			        if (drum!=NullObj)
                                ApplySawKnockback(drum, PosX, PosY, 0.87);
				continue;
			}

			if (obj is CMissile == true)
			{
                                local mis = CMissile(obj);
			        if (mis->sawhitlimit>20 && mis->hitFrame <5) continue;

                                OnTurnHits++;

                                if (obj is CMeteoriteBullet)
			        {
                                        mis->sawhitlimit+=1;
                                        mis->hitFrame = 0;
			                ApplyConstantSpeed(obj, 12.0);
			                continue;
			        }

                                local p = new PxParticle(81, hitPX, hitPY);
                                p->SetLifeTime(12);
                                p->SetVelocity(0,0);
                                p->SetStartAlpha(255);
                                p->SetEndAlpha(255);

                                mis->Collide(this, 1);  // make it explode if its zook or just bounce if its nade 
                                if (mis != NullObj && mis->sawhitlimit<20 && mis->hitFrame >=5) //NullObj check because Collide might trigger Free
                                {
                                    ApplySawKnockback(obj, PosX, PosY, 0.88);
                                    ClankSound(hitPX, hitPY, CalculateSoundVolume(hitPX,hitPY) * soundMultiplier);
                                    mis->sawhitlimit+=1;
                                    mis->hitFrame = 0;
				}
                                continue;
			}
		}
         }
}

void CSaw::SendMessage(CGObject* obj, int dmg)
{
        if (obj == NullObj) return;
        
	local p = new PxParticle(81, obj->PosX, obj->PosY);   
	p->SetLifeTime(12);
	p->SetVelocity(0,0);
	p->SetStartAlpha(255);
	p->SetEndAlpha(255);
        CMessageData msg;
        msg.params[0]	= 0;
        msg.fparams[1]	= PosX;
 	msg.fparams[2]	= PosY;
	msg.fparams[3]	= 0;
	msg.fparams[4]	= 0;
	msg.params[5]	= dmg;
	msg.params[6]	= 0;
	
	if (obj is PxSentryGun == true)  
	{
		msg.params[5]	= dmg*4;
	}
	if (#WEAPON_BOWLINGBALL)
	{
		if (obj is CBowlingBall == true)
		{
                	CBowlingBall *ball = CBowlingBall(obj);

                	ball->Wake();
                	if (supercharged)
                	{
                        	if (ball->ZapTurnsRemaining == 0) ball->Zap(0);
                        	else ball->Zap(1);
                 	}
		}
	}
	if (#WEAPON_MAGNET)
	{
		if (obj is CMagnet == true)
		{
	        	local magnetobj = CMagnet(obj);
	        	magnetobj->Recharge();
		}    
	}
	if(#WEAPON_CUSTOMTURRET)
	{
		if(obj is CCustomTurret == true)
		{      
		        msg.params[5]	= dmg*6;  
			CCustomTurret *custur = CCustomTurret(obj);
			custur->ammoCurrent = 0;
		}
	}			
	obj->Message(this, M_GUNEXP, 1032, &msg);             
} 

void CSaw::SendWormMessage(CWorm* worm, int dmg)
{
        if (worm == NullObj) return;
        CMessageData msg;
        msg.params[0]	= 0;
        msg.fparams[1]	= PosX;
 	msg.fparams[2]	= PosY;
	msg.fparams[3]	= 0;
	msg.fparams[4]	= 0;
	msg.params[5]	= dmg;
	msg.params[6]	= 0;
				
	worm->Message(this, M_GUNEXP, 1032, &msg);       
}

void CSaw::DoSound() //Worms too
{
  vol = CalculateSoundVolume(PosX,PosY);
  pan = CalculateSoundPan(PosX,PosY);
                                   
  local j = RandomInt(1, 4);
  if      (j == 1) PlayLocalSound(165, 5, 1.0, 1.0);
  else if (j == 2) PlayLocalSound(166, 5, 1.0, 1.0);
  else if (j == 3) PlayLocalSound(167, 5, 1.0, 1.0);
  else if (j == 3) PlayLocalSound(168, 5, 1.0, 1.0);     
               
  vol = vol * soundMultiplier;
               
  local i = RandomInt(1, 4);
  if      (i == 1) saw_cut_1->Play(vol, pan, false);
  else if (i == 2) saw_cut_2->Play(vol, pan, false);
  else if (i == 3) saw_cut_3->Play(vol, pan, false);
  else if (i == 3) saw_cut_4->Play(vol, pan, false);
}

void ClankSound(fixed x, fixed y, float volume)
{         
  vol = volume;
  pan = CalculateSoundPan(x,y);
  
  //local clnksoundi = RandomInt(1, 2);
  //if      (clnksoundi == 1) clank_1->Play(vol * 1.3, pan, false);
  //else if (clnksoundi == 2) 
  clank_2->Play(vol * 1.4, pan, false);
}

void CSaw::CSawCollission(CWorm * w)
{    
    local damage = 15;
    if (w == NullObj) return;
    if (w is CWorm == false) return;

    if (w!=NullObj)
    {                                   
        w->timesSliced = w->timesSliced + 1; //should go up always, gets reset at sturn start anyways
        if (w->timesSliced <= 19)
        {
            damage = ApplySawKnockback(w, PosX, PosY, 0.77);
        
            DoSound();
              
            if(w->ObjState!=WS_DRILLING && w->ObjState!=WS_FROZEN) timesCollided = timesCollided + 1;   
            
            if (supercharged && eleFuel > 1) eleFuel--;
        }
        else if (w->timesSliced == 18) ShowMessage("Damn boy!");       
         
        if (!w->invincible)
        { 
            if (!supercharged && w->ObjState!=WS_FROZEN && w->ObjState!=WS_DRILLING)
            {
                DamageWorm(w, damage);   //This changes state of worm too to flying
            }
            else if (supercharged)  
            {
                DamageWorm(w, damage);   //This changes state of worm too to flying 
            } 
        }     
        w->hitFrame = 0;
     }
}

void CSaw::DamageWorm(CWorm* w, int calc)
{
       if (w == NullObj) return;
       local team = w->WormTeam;
       local worm = w->WormNumber;
       local hp_now = GS->Info.GetWormHealth(team, worm);

       if (w->dmgDIVIDER != 0.0) calc = calc / w->dmgDIVIDER;   //Armor Vest adjustment.
       
       if (w->timesSliced <= 27) //Higher cap than knockback in case worm is still alive
       {
       int rnd = RandomInt(1,3);

       if (rnd == 1 && (hp_now!=0 || w->ObjState != WS_DEATH || w->ObjState != WS_DEAD )) w->SetState( WS_FLYING ); else if (rnd == 2) w->SetState( WS_FLYING2 ); else w->SetState( WS_POWERFLY );
       }
       if (w->invincible) return;

       if (hp_now <= 15 && w->Endure)
       {
       GS->Info.SetWormHealth(team, worm, 15);
       return;
       }
       if (hp_now - calc <= 15 && w->Endure)
       {
         GS->Info.SetWormHealth(team, worm, 15);
         return;
       }

       /*if (hp_now - calc <= 0)
       {
         GS->Info.SetWormHealth(team, worm, 0);
         return;
       } */

       if (hp_now > 1) SendWormMessage(w, calc);
}

int CSaw::ApplySawKnockback(CGObject* worm, fixed sawX, fixed sawY, float sawSizeFactor)
{
    stopRotation = false;
    if (worm == NullObj) return 0;
    
    sawRotation += rotationFactor * 2.0; //xD
    
    if (supercharged) sawSizeFactor = sawSizeFactor + (sawSizeFactor * 0.77);
    // Directional Math (Away from Saw Center)
    float dx = worm->PosX - sawX;
    float dy = worm->PosY - sawY;
    float dist = sqrt(dx*dx + dy*dy);
    if (dist < 0.1) dist = 0.1;

    float nx = dx / dist; 
    float ny = dy / dist; 

    // Incoming Speed Dynamics
    float vx = worm->SpX;
    float vy = worm->SpY;
    float speedIn = sqrt(vx*vx + vy*vy);

    // Return Force (Conservation of Energy)
    // Takes 75% of incoming speed and adds a gentle base push
    float basePush = 1.5 + (sawSizeFactor * 1.5); 
    if (supercharged) basePush = basePush * 1.55;
    float bounceStrength = basePush + (speedIn * 0.75); 
    // Dot Product Reduction
    float dot = vx * nx + vy * ny;
    if (dot > 0.0) 
    {
        bounceStrength = bounceStrength * 0.5; // Properly lowers force if already moving away
    }

    // Velocity Assembly
    // Keep 80% of tangential sliding velocity for smooth arcs
    float tangentX = vx - (dot * nx);
    float tangentY = vy - (dot * ny);

    float finalSpX = (nx * bounceStrength) + (tangentX * 0.8);
    float finalSpY = (ny * bounceStrength) + (tangentY * 0.8);

    float total = sqrt(finalSpX*finalSpX + finalSpY*finalSpY);

    lowerLimit = 4.0;  
    
    if (supercharged) lowerLimit = 5.5;

    // Enforce a Gradual Minimum
    // If the final speed is too weak, scale it up to 4.0       
    if (total < lowerLimit && total > 0.01) 
    {
        finalSpX = (finalSpX / total) * lowerLimit;
        finalSpY = (finalSpY / total) * lowerLimit;
        total = lowerLimit;
    }
    else if (total <= 0.01) // If completely stationary
    {
        finalSpX = nx * lowerLimit;
        finalSpY = ny * lowerLimit;
        total = lowerLimit;
    }

    // Dynamic Speed Capping
    maxAllowed = 13.9;
    
    if (worm == NullObj) return 0;
    if (worm is CWorm == false) maxAllowed = 17.5;   
    
    if (supercharged) maxAllowed += 3.0;
    
    if (total > maxAllowed)
    {
        finalSpX = (finalSpX / total) * maxAllowed;
        finalSpY = (finalSpY / total) * maxAllowed;
        total = maxAllowed;
    }
    if (obj is CCustomTurret == true)
    {
        finalSpX = finalSpX * 2;
        finalSpY = finalSpY * 2;  
    }  
    //GG->WriteToChat(7, ftoa(finalSpY), false);

    // Damage Calculation
    dmgLimit = 33; if (supercharged) dmgLimit = 42;
    dmg = int((speedIn * 2.25) + (sawSizeFactor * 4.0));
    if (dmg > dmgLimit) dmg = dmgLimit; 
    if (dmg < 15) dmg = 15;  
    
    if (supercharged) dmg = dmg * 1.5;
    
    if (worm is CWorm == true && supercharged)
    {
       CWorm * wormtz = CWorm(worm);
       TazeAmount = dmg * 1.15;
       wormtz->TazeArtificial(TazeAmount, finalSpX, finalSpY);
    }
    else
    {
       worm->SpX = finalSpX;
       worm->SpY = finalSpY;
    }
    return dmg;
}

void CSaw::Render()
{

}

void CSaw::Draw()
{
    local hasColorMod = false;
    if(timesCollided <= 14)    {
        AddSpriteEx(ZethPlane, PosX, PosY, SawIndex, SawIn, 0, sawScale);  
    }
    else if(timesCollided >= 14) {   
        AddSpriteEx(ZethPlane, PosX, PosY, SawIndex, SawInBlood, 0, sawScale);
    }
    
    if(timesCollided < 7) {
        AddSpriteEx(ZethPlane + 0.2, PosX, PosY, SawIndex, SawOut, sawRotation, sawScale);
    }
    
    if(timesCollided >= 7 && timesCollided < 14) {
        AddSpriteEx(ZethPlane + 0.2, PosX, PosY, SawIndex, SawOutBlood, sawRotation, sawScale);
    }

    else if(timesCollided >= 14) {
        AddSpriteEx(ZethPlane + 0.2, PosX, PosY, SawIndex, SawOutBlood2, sawRotation, sawScale); 
    } 
    
    local blendT = 14;
    if (SAWTEAM == 5) blendT = 8;
    
    if (timesCollided < 7 && SAWTEAM>=0) 
    {  hasColorMod = true;   
        SetColorMod(GetTeamColorRGB(SAWTEAM + 1), blendT); 
        AddSpriteEx(ZethPlane + 0.3, PosX, PosY, SawIndex, Outline, sawRotation, sawScale);  
    }  
    else if (timesCollided >= 8 && SAWTEAM>=0) 
    {  hasColorMod = true;
        SetColorMod(GetTeamColorRGB(SAWTEAM + 1), blendT); 
        AddSpriteEx(ZethPlane + 0.3, PosX, PosY, SawIndex, OutlineDmg, sawRotation, sawScale);  
    } 
    
    if (hasColorMod) ClearColorMod();
}
    
////////////////////////////////////////////////////////////////////////////

override void CMissile::ExplodeAt(fixed x,fixed y)
{
    if(weap->CheckName("Saw Grenade") || weap->CheckName("Place Saw"))
    {
        if(true)
        {
                local w = GetCurrentWorm();
        
                CShootDesc SDesc;
                zero(&SDesc);

                SDesc.X = x;
                SDesc.Y = y;
                SDesc.Team = SAWTEAM;
                SDesc.Delay = 11;
                
                GG->land->ApplyMask(4,SDesc.X,SDesc.Y);
                GG->land->ApplyMask(4,SDesc.X,SDesc.Y-10);
                GG->land->ApplyMask(4,SDesc.X,SDesc.Y-15);

                vol = CalculateSoundVolume(PosX,PosY);
                pan = CalculateSoundPan(PosX,PosY);
		saw_cut_1->Play(vol, pan, false);
		saw_cut_2->Play(vol, pan, false);
  
                local saw = new CSaw(Root->GetObject( 25, 0), &SDesc, 0.5);
                saw->SAWTEAM = GetTeamColor(SAWTEAM);
        }
    }
    super;
}
////////////////////////////////////////////////////// 
//////////////////////           /////////////////////
///////////////////// OVERRIDES //////////////////////
////////////////////           ///////////////////////
//////////////////////////////////////////////////////
/*override void CMissile::Collide(CGObject* Obj,int type)
{     
 super;
 
 if (Obj!=NullObj) if (Obj->ClType == OC_Landscape && (weap->CheckName("Saw Grenade") || weap->CheckName("Place Saw"))) ClankSound(PosX, PosY, CalculateSoundVolume(PosX,PosY)); 
} */

override void CMine::CMine(CObject* Parent,CMineParams* Params,CShootDesc* SDesc,bool Snap,int Unk)
{  
 super; 
  
 hitFrame = 5; 
 sawhitlimit = 0;
} 

override void CCrate::CCrate(CObject* Parent,CCrateDesc* desc,EDropType type)
{  
 super; 
  
 sawhitlimit = 0;
}



override void COilDrum::COilDrum(CObject* Parent,fixed X,fixed Y,bool Snap)
{
 super;
 
 sawhitlimit = 0;
}    

override void CMine::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
  super;
  if (Type == M_FRAME && hitFrame<5) hitFrame++;
  if (Type == M_PRETURNSTART) sawhitlimit = 0; 
} 

override void COilDrum::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
  super;
  if (Type == M_PRETURNSTART) sawhitlimit = 0; 
}  

override void CCrate::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
  super;
  if (Type == M_PRETURNSTART) sawhitlimit = 0; 
} 

//////////////// WORM ////////////////
override void CWorm::CWorm(CObject* Parent,int aTeam,int aIndex,CWormParams* params)
{
   super; 
   sawhitlimit = 0;
   dmgDIVIDER = 0.0; //Armor Vest
   hitFrame = 6;    //Hit Cooldown
   Endure = false;  
   invincible = false;
   timesSliced = 0;    //Hit limit
}

override void CWorm::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
 super;  
 
 if (Type == M_PRETURNSTART)
     timesSliced = 0;
 if (Type == M_FRAME)
     { 
     if (hitFrame<7) hitFrame++;
     if (Root->IsTimerActive() == true && GS->Info.GetWormHealth(WormTeam, WormNumber) == 0 && GetCurrentWorm() == this && !died && !Endure && !invincible)
     if (ObjState == WS_IDLE || ObjState == WS_WALKING ) // self fastdeath
     {
         SetState(WS_DEATH);   
         IsStatic  = false;    
         fastdying = true;  
         firstdied = true;
          dieframe = gframe;
              died = true;
     }
     }
} 

////////////////////////////////////////////////////////////////////////////
///////////////////////////// MAP OBJECT ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////

$editor
 $object Saw
  $origin -15 -15
  $image "saw_ed.png"
  $func SpawnSaw
  $radius 19
  $param SawSize radius 100
 // &param SawZPlane int 10    idk
 $object_end
$end

CSawSpawn::CSawSpawn(CEditorObject* obj)
{
   super(Root, GS);

   bx = obj->PosX;
   by = obj->PosY;
   SawRadius = float(obj->GetInt("SawSize") / 100);
  // Sawzplane = float(obj->GetInt("SawZPlane"));

   DidSpawn = false;
}

void CSawSpawn::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
     if (DidSpawn == false)
     {
      if (Type == M_FRAME && gframe > 2)
      {
       DoSpawn();
      }
     }
}  

void CSawSpawn::DoSpawn()
{
     CShootDesc SDesc;
     zero(&SDesc);

     SDesc.Team = 0;
     SDesc.X = bx;
     SDesc.Y = by;
     SDesc.SpX = 0;
     SDesc.SpY = 0;

     //Size = SawRadius/100; 
     saw = new CSaw(GetObject(25, 0), &SDesc, SawRadius);
     if (saw!=NullObj) {DidSpawn = true; saw->PosX = bx; saw->PosY = by;} // saw->ZethPlane = Sawzplane + 0.01; }
}

void SpawnSaw(CEditorObject* obj)
{   
  sawobj = new CSawSpawn(obj);
};