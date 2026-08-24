require utils, pxparticles, weapon_magnet, weapon_tazer, weapon_keeper_bee,weapon_nuke_rocket, weapon_air_911;

int TeleportBarrelFixer;

CSprite * beeHiveSPR;
CSprite * equalTowers;

void newdrums::Init()
{    
}

void oildrums::InitGraphic() {
    CFile* f;                                
    
    f = GetAttachment("beehive.png");                             
    beeHiveSPR = LoadSprite(f, 16, 0);   
    f = GetAttachment("twins.png");                             
    equalTowers = LoadSprite(f, 1, 0);                                    
}   

override COilDrum::COilDrum(CObject* Parent, fixed X, fixed Y, bool Snap)
{
    super;
    bbbframe = 0.0;
    //barreltype = (int( X )) % 8;
    barreltype = RandomInt(0,8);
    fdesc = new CFireDesc;
    fflags = 4+8+16+32;  // 5 - ground, 6 - ?, 7 - ?, 22 - ?, 14 - ?
    sparkscycle = 0.0;
    
    queueClustlets = false;
    
    if (barreltype == 8 && GS->IsCavern == true) 
    {
    barreltype = 9;   //tp
    firstSpawned = false; 
    }
    else if (barreltype == 8 && GS->IsCavern == false) firstSpawned = true;
    
    crashedAt = false;
    equalframe = 0;
    
    TeleportBarrelFixer = 0;  
    
    customSize = 0.48;
    animSeed = GS->Tick;
}

override void COilDrum::Render()
{
    local barrelsize = 1.0;
    local animcycle = GS->Tick*0.02;

    if (barreltype == 0)  // explosive
    {
       SetColorMod(ARGB(160, 180, 60, 60), 8);
       AddSpriteEx(14, PosX, PosY, 106, animcycle, 0, 0.8);
       SetColorMod(ARGB(255, 180, 60, 60), 8);
       barrelsize = 1.2;
    }    
    else if (barreltype == 1)  // heavy napalmed
    {
       SetColorMod(ARGB(160, 200, 90, 0), 8);
       AddSpriteEx(14, PosX+1.0, PosY+2.0, 126, animcycle, 0, 1.0);
       AddSpriteEx(15, PosX-1.0, PosY-2.0, 125, -animcycle, 0, 1.0);
       SetColorMod(ARGB(255, 200, 90, 0), 8);
       barrelsize = 1.0;    
    }  
    else if (barreltype == 2)  // nukes oildrum
    {
       SetColorMod(ARGB(170, 80, 20, 20), 8); 
       customSize =  0.7 + ((sin((GS->Tick/10)+animSeed)) * 0.2);
       AddSpriteEx(14, PosX, PosY, nuke_rocket_aiming->Index, 0, 0, customSize);     
       SetColorMod(ARGB(255, 255, 100, 255), 8);
       barrelsize = 1.2;
    } 
    else if (barreltype == 3)  // clusters oildrum
    {
       SetColorMod(ARGB(160, 120, 0, 0), 8);
       AddSpriteEx(15, PosX+2.0 + cos(animcycle*3)*2 , PosY+6.0 + sin(animcycle*3)*2, 54, animcycle , 0, 1.15);       
       AddSpriteEx(15, PosX-4.0 + cos(animcycle*3)*2, PosY-2.0 + sin(-animcycle*3)*2, 54, -animcycle, 0, 1.05);
       AddSpriteEx(15, PosX+4.0 + cos(animcycle*3)*2, PosY-7.0 + sin(animcycle*3)*2, 54, animcycle, 0, 0.95);       
       SetColorMod(ARGB(255, 175, 0, 0), 8);
       barrelsize = 1.0;
    }
    else if (barreltype == 4)  // icy oildrum
    {
       SetColorMod(ARGB(160, 0, 65, 255), 8);   //65
       AddSpriteEx(15.0, PosX-2, PosY-6, 610, -animcycle*3, animcycle, 0.7);
       AddSpriteEx(15.1, PosX-1, PosY-2, 610, -animcycle*3, -animcycle, 0.9); 
       AddSpriteEx(15.2, PosX+3, PosY+4, 610, -animcycle*3, animcycle, 1.1); 
       SetColorMod(ARGB(255, 0, 100, 255), 8);  
       barrelsize = 1.0;
    }
    else if (barreltype == 5)  // electric oildrum
    {
       SetColorMod(ARGB(255, 100, 100, 255), 8);
       AddSpriteEx(14, PosX+2, PosY, magnetSparks->Index, sparkscycle, animcycle*0.5, 1.1);
       SetColorMod(ARGB(255, 45, 45, 210), 8);  
       barrelsize = 1.0;
    } 
    else if (barreltype == 6)  // mine oildrum
    {
       SetColorMod(ARGB(160, 120, 60, 170), 8);
       AddSpriteEx(14, PosX+1.0, PosY+4.0, 47, animcycle, 0, 1.0);
       AddSpriteEx(15, PosX-3.0, PosY-4.0, 47, -animcycle, 0, 0.9);
       SetColorMod(ARGB(255, 120, 60, 170), 8);  
       barrelsize = 1.1;
    }
    else if (barreltype == 7)  // bee oildrum
    {
       AddSpriteEx(16, PosX, PosY-4.5, beeHiveSPR->Index, bbbframe, 0, 0.535);  

       barrelsize = 0.0;  //hide drum without return or something crashy
    } 
    else if (barreltype == 8)  // TRAGEDY oildrum
    {               
       customSize =  0.48 + ((sin((GS->Tick/10)+animSeed)) * 0.02);
       if (crashedAt) 
       SetColorMod(ARGB(255, 200, 60, 60), 8);
       AddSpriteEx(10, PosX, PosY-15, equalTowers->Index, 0, 0, customSize);  

       barrelsize = 0.0;
    }
    else if (barreltype == 9)  // TP OilDrum if (GS->IsCavern == true)  or 911 is under terrain
    {
       SetColorMod(ARGB(230, 100, 165, 130), 8);
       AddSpriteEx(15, PosX + 1, PosY, 421, animcycle, 0, 0.6);
       SetColorMod(ARGB(255, 60, 60, 175), 8);
       barrelsize = 1.0;
    }
    
    
    else if (barreltype == -1) return; 
    local sn = (4 * Dmg) / 50;
    if (sn > 4) sn = 4;
    AddSpriteEx(16, PosX, PosY, sn + 110, animcycle, 0, barrelsize);
    ClearColorMod();
}

override void COilDrum::Explosion()
{
       
        if (barreltype == 0)
        {
            local fs = Root->GetObject(TI_FireStore, 0);
            fdesc->X = PosX;
            fdesc->Y = PosY - 1;
            fdesc->Count = 18;
            fdesc->UnkB = 1;
            fdesc->Duration = 6300;
            fdesc->UpPower = 100;
            fdesc->Damage = 17;
            fdesc->OwnerTeam = OwnerTeam;
            fdesc->SpY = -3;
            fdesc->SpX = -3;
            local fire = new CFire(fs, fdesc, 0);
            fdesc->SpX = 3;
            fdesc->Y = PosY + 1;
            fire = new CFire(fs, fdesc, 0); 
            fdesc->SpX = 0;
            fdesc->SpY = -8;
            fdesc->Y = PosY - 2;
            fire = new CFire(fs, fdesc, 0); 
            DoExplosion(PosX, PosY, 100, 85, 0, OwnerTeam); 
        }
        else if (barreltype == 1)
        {
            local fs = Root->GetObject(TI_FireStore, 0);
            fdesc->X = PosX;
            fdesc->Y = PosY - 1;
            fdesc->Count = 25;
            fdesc->UnkB = 1;
            fdesc->Duration = 8300;
            fdesc->UpPower = 100;
            fdesc->Damage = 17;
            fdesc->OwnerTeam = OwnerTeam;
            fdesc->SpY = -8;
            fdesc->SpX = -8;
            local fire = new CFire(fs, fdesc, 0);
            fdesc->SpX = 8;
            fdesc->Y = PosY + 1;
            fire = new CFire(fs, fdesc, 0); 
            fdesc->SpX = 0;
            fdesc->SpY = -8;
            fdesc->Y = PosY - 2;
            fire = new CFire(fs, fdesc, 0); 
            fdesc->SpX = 4;        
            fire = new CFire(fs, fdesc, 0); 
            fdesc->SpX = -4;        
            fire = new CFire(fs, fdesc, 0); 
            fdesc->SpY = -2;          
            fire = new CFire(fs, fdesc, 0); 
            fdesc->SpX = 4;        
            fire = new CFire(fs, fdesc, 0); 
            DoExplosion(PosX, PosY, 100, 25, 0, OwnerTeam);     
        }
        else if (barreltype == 2)
        {
             DoExplosion(PosX, PosY, 80, 20, 0, OwnerTeam);
            
             local Nldata = getClustletsData();
             if (Nldata == NullObj) return;
	     ////////////////////////FUCK////////////////////////
             Nldata->explosion.flags=0; //ZERO Worms, NO Terrain, NONE Oildrums, NULL Crates, NOT Dnor Cards, HASNT Skimming.
             Nldata->explosion.bias = 50;
             Nldata->explosion.pushPower = 70;
             Nldata->explosion.damage = 0;
             Nldata->explosion.dmgVar = 0;
             Nldata->unk = 0;
             Nldata->anim.spriteIndex = 0; 
             Nldata->anim.type = WAT_TrackMovement;    
             Nldata->anim.trailIndex = 131;   
             Nldata->anim.trailPower = 0;
             Nldata->anim.trailSpeed = 100;
             Nldata->anim.unk = 50; 
             Nldata->speedMultipler = 100;
             Nldata->windFactor = 0;
             Nldata->gravityFactor = 100;
             Nldata->countdownFrom = 0;
             Nldata->timeBeforeExplosion = 1500;
             Nldata->sound.soundIndex = 66;           
             Nldata->sound.useExpSound = false;
             Nldata->sound.expSoundIndex = 0;
             Nldata->sound.expSoundDelay = 0; 
             Nldata->explodeOnSpace = false;
             Nldata->explodeInto = WExplode_Nothing;    
 ///////////////////////ASDFGHFAKLJHFGJLSJAK////////////////////////      
             CShootDesc sdata;
             zero(&sdata);
             sdata.X = PosX;  sdata.Y = PosY;
             sdata.SpY = RandomFloat(-0.5, -2.5);
             sdata.SpX = RandomFloat(-0.8,  0.8);
             sdata.Team = 0;
             sdata.Worm = 0;
             sdata.AddX = 0;
             sdata.AddY = 0;
              
             nuke_rocket = new CMissile(Root->GetObject(25, 0), Nldata, &sdata);     
             if (nuke_rocket!=NullObj)
             {
               nuke_rocket->nr_first = true;    
               nuke_rocket->NewBounce = true;
               nuke_rocket->ActivateBouncePhysics(Root->GetObject( 25, 0), Nldata, &sdata, 0, 5.2, 0.11, 0.03, 30, 113, false);
             }
             sdata.SpY = RandomFloat(-2.4, -5.2);
             sdata.SpX = RandomFloat(-2.5,  2.5);
             nuke_rocket = new CMissile(Root->GetObject(25, 0), Nldata, &sdata);
             if (nuke_rocket!=NullObj) 
             {
               nuke_rocket->nr_first = true;   
               nuke_rocket->NewBounce = true;
               nuke_rocket->ActivateBouncePhysics(Root->GetObject( 25, 0), Nldata, &sdata, 0, 5.2, 0.11, 0.03, 30, 113, false);
             }   
             delete Nldata;
             //PlayLocalSound(40, 5, 1.0,1.0);
            
        }
        else if (barreltype == 3)
        {
            DoExplosion(PosX, PosY, 100, 10, 0, OwnerTeam);
            
            queueClustlets = true; //they will just collide with self otherwise
        }
        else if (barreltype == 4)
        {
           DoExplosion(PosX, PosY, 100, 10, 0, OwnerTeam);
           
           PxParticle *p;
           
           for(int i=0; i<400;i++)
           {
               p = new PxParticle(611, PosX, PosY);
               
               p->SetLifeTime(70);
               p->SetMotionRandomness(0.4);
               p->SetAirResistance(0.02);
               float s = RandomFloat(0.75, 1.0);
               p->SetStartSize(s, s);
               p->SetEndSize(s-0.4, s-0.4);
               p->SetStartAlpha(255);
               p->SetEndAlpha(175); //0
               p->SetBlendMode(1);
               p->SetStartColor(0, 75, 255);
               p->SetEndColor(0, 125, 255);
               p->SetRandomVelocity(1.0,3.0);// (0.01, 4.0);
          }
          PlayLocalSound(24, 5, 1.0,1.0);
          
          Freeze();
        }
        
        else if (barreltype == 5)
        {
        TazerSegmentLength = 10.0;
	    TazerNumSegments = 65;
        TazerLifeTime = 30;
	    TazerLifeTimeAfterHit = 36;
	    TazerSearchRadius = TazerSegmentLength * TazerLifeTimeAfterHit;
	    TazerSearchRadiusSqr = TazerSearchRadius * TazerSearchRadius;
	    
	    local elect = new CTazerShot(GetObject(25, 0), COilDrum(this));
	    elect->AddToIgnoreList(GetCurrentWorm());
	    
            DoExplosion(PosX, PosY, 100, 10, 0, OwnerTeam);
        }
        else if (barreltype == 6)
        {
            CMineParams MParams;
            CShootDesc SDesc;
            int nmines = 7;
            for(local i=0; i<nmines; i+=1)  // default 10
            { 
              zero(&MParams);
              zero(&SDesc);
                          
              MParams.Radius = 48;
              MParams.Prefuse = 3000;
              MParams.Fuse = 3000;
              MParams.Flags = fflags;//6
              MParams.Bias = 0;
              MParams.Damage = 48;  // default 48
              MParams.BlastPower = 100;

              SDesc.Team = 0;
              SDesc.X = PosX;
              SDesc.Y = PosY;
              SDesc.SpX = RandomFloat(-7.0, 7.0);
              SDesc.SpY = RandomFloat(-7.0, -4.0);
        
              local Mine = new CMine(Root->GetObject(25, 0), &MParams, &SDesc, false, 0);                 
            }
            DoExplosion(PosX, PosY, 100, 10, 0, OwnerTeam);
            
        }        
        else if (barreltype == 7)    //BEE
        {
            DoExplosion(PosX, PosY, 0, 0, 0, OwnerTeam);   
            rndintntni = RandomInt(1,3);
            if (rndintntni == 1)        
            createBee(PosX, PosY-5, 0, CWorm(NullObj), true );       
            if (rndintntni == 2)             
            {createBee(PosX, PosY-5, 0, CWorm(NullObj), true ); createBee(PosX, PosY-5, 0, CWorm(NullObj), true );       }           
            if (rndintntni == 3)             
            {createBee(PosX, PosY-5, 0, CWorm(NullObj), true ); createBee(PosX, PosY-5, 0, CWorm(NullObj), true ); createBee(PosX, PosY-5, 0, CWorm(NullObj), true);       }
        }
        else if (barreltype == 8)
        {
        
        if (crashedAt) FreeMe = true;  
        }
        else if (crashedAt) FreeMe = true;
        
        else if (barreltype == 9)
        {
            DoExplosion(PosX, PosY, 100, 10, 0, OwnerTeam);

            local worm = GetCurrentWorm();
            if(worm != NullObj && TeleportBarrelFixer < 1)
            {
              worm->PosX = PosX;
              worm->PosY = PosY - 6;

              PlayLocalSound(68, 5, 1.0,1.0);
              PlayLocalSound(67, 5, 1.0,1.0);
            }
            if(worm != NullObj && TeleportBarrelFixer > 0)
            {
              worm->PosX = PosX;
              worm->PosY = PosY - 6;

              PlayLocalSound(68, 5, 1.0,1.0);
              PlayLocalSound(67, 5, 1.0,1.0);
            }
        }

        
        else if (barreltype == -1) return;
       
}
/*
override void CMissile::Collide(CGObject* Obj,int type)
{
 super;
 if (Obj!=NullObj && im_cnuke)
 {
 if (Obj->ClType == OC_Landscape) SpX*0.3;  
 if (Obj->ClType == OC_Landscape) SpX*0.3;
 }
 
} */

void COilDrum::SpawnTragedy()
{
if (!crashedAt)
        {                
        NineDesc.AddX = PosX; 
        NineDesc.AddY = PosY;                    
        
        // SET THE GLOBAL TARGET FOR THE PLANE
        planeTargetX = PosX;
        planeTargetY = PosY;
        planeTargetSet = true;
        
        NineDesc.Y = -288;
         
        rnddir = RandomInt(1,2);
        if (rnddir == 1)                            
            NineDesc.X = -500.0;      // spawnpos left side
        else          
            NineDesc.X = GS->LevelSX + 500;  // spawnpos right side
            //GG->WriteToChat( 3, ItoA(GS->LevelSY), false);
        if (rnddir == 1)
            NineDesc.SpX = 40.0;   // hover right
        else   
            NineDesc.SpX = -40.0;  // hover left
        
        NineDesc.Worm = 0; 
        NineDesc.Team = 0;              
        NineDesc.Delay = 3000; 
                          
        hitPL = false;
        
        if (GS->IsCavern) //unused for this script, keeping regardless
        {
         NineDesc.Y =  120;
        }
        float xDistanceSp = abs(NineDesc.X - planeTargetX);    
        if (xDistanceSp<700)   //If target is on the edge of the map, spawn further back and do a spin
        {
           NineDesc.Y -= 50; 
            if (NineDesc.SpX >= 0.0)
            {
               NineDesc.SpX = -90;
               NineDesc.X = NineDesc.X + 400;   
            }
            else
            {
               NineDesc.X = NineDesc.X - 400;
               NineDesc.SpX = 90;
            }    
           NineDesc.SpY = -6;
           ShowMessage("Executing Tactical Maneuver");
        }
        if (NineDesc.Team < 0) NineDesc.Team = 0;
        ldata2 = airplaneWEAP.launch;  //grab arbitrary weapon i made
        ldata2.gravityFactor = 100;
        
        if (GS->LevelSY > 4000)  //mmmfgh so huge
        {
         NineDesc.Y = planeTargetY - 700;
        }
        
        planeCrash* mis = new planeCrash(Root->GetObject(25, 0), &ldata2, &NineDesc);    //spawn missile
        } 
}

override void COilDrum::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{    
    if (Type == M_GUNEXP && barreltype == 8 && !crashedAt) {   
         barreltype = 8;
         SpawnTragedy();
         crashedAt = true;         
         GravityFactor = 0;         
         SpX = 0;
         SpY = 0;     
         zplane = 26;
         return;
        }
        
    if (Type == M_EXPLOSION && barreltype == 8 && !crashedAt && explosion_Damage(   MData->params[4],  DistBetweenPoints(MData->fparams[1], MData->fparams[2] , PosX, PosY), explosion_Radius(MData->params[4])   )  > 0 )
    {   
         barreltype = 8;
         SpawnTragedy();
         crashedAt = true;         
         GravityFactor = 0;         
         SpX = 0;
         SpY = 0;     
         zplane = 26;
         return;
    } 
    super;                               
    if(Type == M_TURNEND && barreltype == 9)
    {
       if(TeleportBarrelFixer < 1)
       {
          TeleportBarrelFixer = 1;
       }
    }
    if(Type == M_TURNBEGIN && TeleportBarrelFixer > 0)
    {
       TeleportBarrelFixer = 0;
    } 
    if (Type == M_FRAME && barreltype == 8 && gframe > 5)
    {
        if (crashedAt) equalframe++;
        if (equalframe > 145) Free(true);   
        if (firstSpawned)
        {
            int hitX; int hitY; 
            local foundsky = TraceLine(this, PosX, PosY - 25, PosX, -10, CMASK_TERRAIN, &hitX, &hitY) != NullObj;       //check for Sky
            if (foundsky) 
            {
                brl = RandomInt(0,16);  //found terrain instead of sky
                if (brl >= 8)brl = 9;    //replace for TP barrel.
                barreltype = brl;
            } 
            firstSpawned = false;
        }
    }    
    if (Type == M_FRAME && barreltype == 7) 
    {
         bbbframe += 0.001 * 16.0 * 1.7;
         if (bbbframe>=1.0) bbbframe = 0.0; 
    }
    if(Type == M_FRAME && barreltype == 5)  
    {
        if(sparkscycle >= 1.0) 
            sparkscycle = 0.0;
        else 
            sparkscycle += 0.01;
    }
    if (Type == M_FRAME && barreltype == 3)
    {
     if (queueClustlets)
       {
          makeClustlets(-90, RandomInt(18,24),true, 9.0, PosX, PosY, 6, 1, OwnerTeam);
          queueClustlets = false;
       }    
    }
}



///////////////////////Electric Drum//////////////////////////


CTazerShot::CTazerShot(CObject *parent, COilDrum *drum)
{
	super(parent, GS);
	ClType = OC_AirStrike;
	
	ClearIgnoreList();
	//AddToIgnoreList(drum);
	
        ang = MATH_HALF_PI;
	
	LifeTime = TazerLifeTime;
	
	Removed = false;
	Dead = false;
	
	DieFraction = 0.0;
	DieRate = 0.05;
	
	DirX = sin(ang);
	DirY = -cos(ang);
	PosX = drum->PosX + 4 * DirX;
	PosY = drum->PosY + 4 * DirY;
	
	LastX = drum->PosX;
	LastY = drum->PosY;
	
	NextProgress = 0;
	StopProgress = false;
	
	NumRemoveOnNextLink = 0;
	
	ClearPoints();
	AddPoint(PosX, PosY);
	
	SoundObj = tazerLoopSound->Play(1.0, 0.0, false);
}


require utils_steps;

void COilDrum::Freeze()
{
      CWorm * cur_worm = GetCurrentWorm();
        
      for(int k = 1; k < Env->Objs.Count; k++)
      {
          local obj = CGObject(Env->Objs.Objs[k]);
          if(obj == NullObj) continue;
          if(obj->ClType == OC_Worm)
	  	  {
	      CWorm *sel_worm = CWorm(obj);
	      
	      if(DistBetweenObjs(this, sel_worm) <= 150)  
	      {
                  local state = sel_worm->ObjState;
                  
                  if (state != WS_SINKING && state != WS_DEAD && state != WS_DEATH && state != WS_FROZEN && sel_worm->DispHealth > 0)    //   
                  {
                      if(sel_worm->swimsuit)
                      {
                              local ss_wep = GetWeaponByName("Swim Suit");
                          
                              if(ss_wep != NullObj)
                              {
                                  CShootDesc sdesc;
                                  zero(&sdesc);

                                  sel_worm->CurWeapon = ss_wep; 
                                  sel_worm->WeaponN = ss_wep->GetWeaponIndex();
                                  sel_worm->FireFinal(ss_wep, &sdesc);
                              }
                      }
                      
                      if(cur_worm != NullObj && cur_worm == sel_worm)
                      {
                          if(int(cur_worm->ObjState) == 109)   //kami
                          {
                              cur_worm->SetState(WS_JUMP);
                              GG->land->MakeHole(25, cur_worm->PosX, cur_worm->PosY);
                           
                              if(cur_worm->DispHealth > 1)
                              {
                                  CMessageData msgdamage;
                                  zero(&msgdamage);
                                  msgdamage.params[0] = 10;
                                  msgdamage.params[1] = cur_worm->PosX;
                                  msgdamage.params[2] = cur_worm->PosY;
                                  msgdamage.params[5] = 30;
                                  cur_worm->Message(NullObj, M_GUNEXP, 1032, &msgdamage);
                                  
                              }
                              
                              //cur_worm->dead = true;
                          }  
                      }   
                      
                      if(#KAMIKAZE_ROCKET)
                      {
                           if(sel_worm->kRocketCtrl)
                           {
                               sel_worm->kRocketCtrl = !sel_worm->kRocketCtrl;
                           }
                      }                   
                      
                      sel_worm->SetState(WS_FROZEN);
                      
                  }   
              }
           }
       }
}

override void CWorm::CWorm(CObject* Parent,int aTeam,int aIndex,CWormParams* params)
{
         swimsuit = false;
         super;
}

override void CWorm::FireFinal(CWeapon* Weap,CShootDesc* Desc)
{  
   super;
   if(Weap->CheckName("Kamikaze") || Weap->CheckName("Kamikaze Rocket"))
   {  
      dead = false;
   }
}