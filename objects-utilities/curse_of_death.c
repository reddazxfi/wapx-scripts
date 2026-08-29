require utils, utils_steps, pxparticles, utility_endure, utils_red;

CSprite * mCrossHair;
bool someonesAssIsMarked;
int missileQnt;
int checkCoolDown;
bool ransubtract;

void curse_of_death_script::InitGraphic()
{
  someonesAssIsMarked = false;
  ransubtract = false;     
  missileQnt = 0;
  checkCoolDown = 2;
  
  CFile * f;
  f = GetAttachment("markcrosshair.png");
  mCrossHair = LoadSprite(f,6,0);
}

///////////////////////////////////////////Mark and Homing Code///////////////////////////////////////////
override void CMissile::CMissile(CObject* parent,CWeaponLaunch* ldata,CShootDesc* sdata)
{
 super;        
 homingIterations = 0; //Holy Hand Grenade or others.
 missileQnt++;
} 

override void CMissile::Free(bool FreeMem)
{                          
 super;        
 missileQnt--;
}
/////////////////////////////////////////////
override void CMine::CMine(CObject* Parent,CMineParams* Params,CShootDesc* SDesc,bool Snap,int Unk)
{          
 homingIterations = 0; 
 homeActive = false;
 super;
}

override void COilDrum::COilDrum(CObject* Parent,fixed X,fixed Y,bool Snap)
{          
 homingIterations = 0; 
 super;   
}

//////////////////////HOMING BEHAVIOR//////////////////////

CWorm* CursedWormList[48];
int    CursedWormCount;

void RefreshCursedWormList()
{
    CursedWormCount = 0;
    if (!someonesAssIsMarked) return;

    for (int i = 0; i < Env->Objs.Count; i++)
    {
        local obj = CGObject(Env->Objs.Objs[i]);
        if (obj == NullObj) continue;
        if (obj is CWorm == false) continue;

        local worm = CWorm(obj);
        if (worm == NullObj || !worm->isCursed) continue;

        if (CursedWormCount < 22)
        {
            CursedWormList[CursedWormCount] = worm;
            CursedWormCount++;
        }
    }
}

bool ObjWantsCurseHoming(CGObject* obj)
{             
    if (obj->PosY > Env->Water || obj->IsMaterial == false) return false;             
            //Avoid classes that say they're OC but class itself isn't Class
    if      (obj->ClType == OC_Mine    && obj is CMine)    { CMine    * mine   = CMine(obj);    if (mine   -> homingIterations < 50) return true; }
    else if (obj->ClType == OC_Missile && obj is CMissile) { CMissile * mis    = CMissile(obj); if (mis    -> homingIterations < 50) return true; }
    else if (obj->ClType == OC_OilDrum && obj is COilDrum) { COilDrum * aodrum = COilDrum(obj); if (aodrum -> homingIterations < 50) return true; }
    return  (obj is CArrow);
}

void AssignCurseHomingTargets()
{                 
    if (missileQnt > 50)   return;
    if (Env->Objs.Count > 120) checkCoolDown = 6;
    else checkCoolDown = 2;
    if (CursedWormCount == 0) return;

    CWorm* currWorm = GetCurrentWorm();

    for (local i = 0; i < Env->Objs.Count; i++)
    {
        local obj = CGObject(Env->Objs.Objs[i]);
        if (obj == NullObj) continue;
        if (!ObjWantsCurseHoming(obj)) continue; 

        CWorm* closestWorm = CWorm(NullObj);
        float closestDistance = 149999.0;
        float ox = obj->PosX;
        float oy = obj->PosY;

        for (local j = 0; j < CursedWormCount; j++)
        {
            local worm = CursedWormList[j];
            if (worm == NullObj || worm == currWorm) continue;

            int hitX; int hitY;
            float dx = worm->PosX - ox;
            float dy = worm->PosY - oy;
            float distance = sqrt(dx*dx + dy*dy);

            if (distance < closestDistance)
            {
                if (TraceLine(obj, ox, oy, worm->PosX, worm->PosY, CMASK_TERRAIN, &hitX, &hitY) == NullObj)
                {
                    closestDistance = distance;
                    closestWorm     = worm;
                }
                else continue;
            }
            else continue
        }

        if (closestWorm != NullObj)
        {
           local check = CheckTargetArray(obj,closestWorm,i);
           
           if (!check) continue;

           chaseTargetCurse(obj, closestWorm);
        }                                   
    }
}

void chaseTargetCurse(CGObject * obj, CWorm * worm)
{
            if (obj == NullObj || worm == NullObj) return;
            float speed = 12.5;
            float turn  = 0.25;           
            if (obj is CMine)    { CMine    * amine = CMine(obj);    speed = 13.0; turn = 0.12; /* if (amine -> hadntlockedtarget == true){ */ amine -> homingIterations++; }//}  
            if (obj is CMissile) { CMissile * mis   = CMissile(obj); speed = 25.0; turn = 0.09; /* if (mis   -> hadntlockedtarget == true){ */ mis   -> homingIterations++; }//}
            if (obj is COilDrum) { COilDrum * drum  = COilDrum(obj); speed = 9.0;  turn = 0.10; /* if (drum  -> hadntlockedtarget == true){ */ drum  -> homingIterations++; }//}  
            
            if (obj is heliShoot){ speed = 4.0;  turn = 0.20; }
            if (obj is CMeteoriteBullet) { speed = 15.0; turn = 0.8; }

            homeToPlace(worm->PosX, worm->PosY - 5, obj, speed, turn);
}

bool CheckTargetArray(CGObject *obj, CWorm * worm, int i)
{      
        if (obj == NullObj || worm == NullObj) return;
        if (worm != NullObj)
        {
            bool alreadyTargeting = false;
            int emptySlot = -1;
            int currentTargeterID = i; 

            // Check if this missile is already locking on
            for (local k = 0; k < 5; k++)
            {
                if (worm->beingTargetedBy[k] == currentTargeterID)
                {
                    alreadyTargeting = true;
                    return true;
                }
                if (worm->beingTargetedBy[k] == -1 && emptySlot == -1)
                {
                    emptySlot = k;
                }
            }

            // If it's a brand new missile, register it
            if (!alreadyTargeting)
            {
                if (emptySlot != -1 && worm->timesTargeted<3) 
                {
                    worm->beingTargetedBy[emptySlot] = currentTargeterID;
                    worm->timesTargeted++; // Only increments for NEW lock-ons
                    return true;
                }
            }
        }
        return false;
}

override void CTurnGame::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
     super;
     if (!someonesAssIsMarked || gframe < 50) return;
     if (Type == M_FRAME)
     {
          if ((gframe % checkCoolDown) == 0 && Root->IsTimerActive() == true)    //With Homing Cooldown
          {
              AssignCurseHomingTargets();
          }
          else if ((gframe % (checkCoolDown + 3)) == 0)
          {
              RefreshCursedWormList();
          }
     }
} 

/////////////////////////////////////////// Worm Code ///////////////////////////////////////////

override void CWorm::CWorm(CObject* Parent,int aTeam,int aIndex,CWormParams* params)
{
 isCursed = false;  
 isMarked = false;
 timesHit = 0;
 dmgMultiplier = 0.0;
 //targtHP =  GS->Info.Teams.Worms.Health;
 timesTargeted = 0;
 curseCoolDown = 0;
 markCoolDown = 0;
 invincible = false;
 turnBegan = false;
 
 timesTargeted = 0;

 beingTargetedBy = new int[5];
 for (int i = 0; i < 5; i++)
 {
   beingTargetedBy[i] = -1; // -1 means empty slot
 }
 
 indCrossHair = mCrossHair->Index;
 crosshairreverse = false;  
 crshrot = 0.0;
 crshrnim = 0.000;         
 crosshairspeed = 5.0;
 
 super;
}

void CWorm::MultiplyExplosionDamage(CObject *sender, int dPosX, int dPosY, int dmg)
{
        if (dmg == 0) return;
        float dist = sqrt((PosX-dPosX)*(PosX-dPosX) + (PosY-dPosY)*(PosY-dPosY));
        float radius  = explosion_Radius(dmg);
        int   baseDmg = explosion_Damage(dmg, dist, radius);
        dmgDone = int(float(baseDmg) * (dmgMultiplier - 1.0));  //Add damage onto EXISTING one.
        
        Penetrate(this, dPosX, dPosY, dmgDone);    
}

void Penetrate(CWorm * worm, int dPosX, int dPosY, int dmgDone)
{
        CMessageData msg;
        zero(&msg);
        msg.params[0]   = 6767;
        msg.fparams[1]	= dPosX;
 	msg.fparams[2]	= dPosY;
	msg.fparams[3]	= 0;
	msg.fparams[4]	= 0;
	msg.params[5]	= dmgDone;
	msg.params[6]	= 0;
        worm->Message(Root, M_GUNEXP, 1032, &msg);
} 

override void CWorm::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
 {
    if (Type == M_GUNEXP && isMarked && dmgMultiplier != 0.0 && !invincible && MData->params[0]!= 6767)
    {
        MData->params[5] = int(float(MData->params[5]) * dmgMultiplier);   //REPLACE damage taken (gunexp supports it)      
    }
    else if (Type == M_GUNEXP && MData->params[0] == 6767) MData->params[0] = 0;
    
    if (Type == M_EXPLOSION && isMarked && dmgMultiplier != 0.0 && !invincible)
    {   
         //Test 1; just increase base dmg.      
        //MData->params[4] = int(MData->params[4] *  dmgMultiplier);  
        
        //Test 2; apply damage on top of dmg already done.
        MultiplyExplosionDamage(sender, MData->fparams[1], MData->fparams[2], MData->params[4]); 
        
        //Both work, but option 1 will increase/decrease push power.  
    }
    super;
    if (Type == M_FRAME)  //Loop
    {  
        if (isCursed && timesTargeted < 3)                                 
        {  
            someonesAssIsMarked = true;   
            int cspd =  crosshairspeed/3;
            if(!crosshairreverse) 
            {                        
                crshrnim += 0.001 * 6.0 * cspd ;   //Animation frame * Frame Amount * Frame Speed (int)   
                if (gframe % 3 == 0) crosshairspeed += 1.0;             
            }
            else if (crosshairreverse)
            {
                crshrnim += 0.001 * 6.0 * cspd ;
                crshrot += 0.02 + crosshairspeed / 120.0;   
                if (gframe % 2 == 0) crosshairspeed -= 1.0;  
            }
            if (crosshairspeed > 15.01)
            {
                crosshairreverse = true;
                crosshairspeed = 15.0;
            }
            if (crosshairspeed <5.01)   
            {
                crosshairreverse = false; 
                crosshairspeed = 6.0;
            } 
            if (crshrnim > 1)
                crshrnim = 0;
   
            crshrot = NormalizeAngle(crshrot);      
            if (isMarked && markCoolDown == 0) isMarked = false;
            if (curseCoolDown == 0 && isCursed)
            {
                isCursed = false; timesTargeted = 0; curseCoolDown = 0;
            }
        }       
	if (isMarked)
	{
	    PxParticle *p;
	    for(int i=0; i<3;i++)
	    {
	        p = new PxParticle(616, PosX+RandomInt(-5, 5), PosY-20);

                p->SetLifeTime(50);
                p->SetMotionRandomness(1.0);
                p->SetAirResistance(0.50);
                p->SetStartSize(0.5, 0.5);
                p->SetEndSize(0.0, 0.0);
                p->SetStartAlpha(1);
                p->SetEndAlpha(100);
                p->SetBlendMode(0);
                p->SetStartColor(80, 80, 80);
		p->SetEndColor(0, 0, 0);
                p->SetRandomVelocity(0.01,1.0);// (0.01, 4.0);
                p->GravityFactor(0.1);
		p->SetWindFactor(0.0);
		p->SetAnimSpeed(0.5);
            }
        }	
        if (timesHit > 5 || markCoolDown == 0 ) if (isMarked){ isMarked = false; deleteTracking = true; someonesAssIsMarked = false; }  //Remove mark and reset variables
        if (!isMarked && deleteTracking) {markCoolDown = 0; timesHit=0; dmgMultilplier = 0.0; deleteTracking = false; someonesAssIsMarked = false; }
		
        CrossHairCol = GetTeamColor(WormTeam) + 2;
        if (Root->IsTimerActive() == true && !turnBegan && GetCurrentWorm()!=NullObj)
        {
        if (isCursed||isMarked)
        {
            local cwm = GetCurrentWorm();
            if (cwm !=NullObj)
            {
                if (cwm->WormTeam == WormTeam)
                {      
                    if (timesTargeted >=2)   //Disable the curse
                    {      
                       ResetCurse();
                    }
                    if (curseCoolDown>0) curseCoolDown--;
                    if (markCoolDown>0) markCoolDown--; 
                    turnBegan = true;
                    missileQnt = 0;
                }     
            }
        }
        }
    } 
/*  if (!Endure && !invincible && WasHit && dmgMultiplier!=0.0 && targtHP!=GS->Info.GetWormHealth(WormTeam, WormNumber))
     {
          //GG->WriteToChat( 5, "fuck", false);
          local dmgAmount = targtHP - GS->Info.GetWormHealth(WormTeam, WormNumber); //Calculate the damage done with the stored HP
          local FinalHealth2 = targtHP - (dmgAmount*dmgMultiplier);                //Calculate the damage it should do
          if (targtHP < (targtHP - dmgAmount) || targtHP < (targtHP - (dmgAmount*dmgMultiplier))) FinalHealth2 = targtHP; //dont change 
          targtHP =  FinalHealth2;                                               //Store new health value
          WasHit = false; 
          if (FinalHealth2 <=0)
          {           
              GS->Info.SetWormHealth(WormTeam, WormNumber, int(0));
              local cwww = GetCurrentWorm()  ;
              if (ObjState!=WS_DEATH && cwww==this && !died && Root->IsTimerActive() == true)
              { SetState( WS_DEATH ); isStatic = false; died = true; fastdied = true; }  //Apply self fast death
          } 
          else { if (dmgAmount != FinalHealth2) GS->Info.SetWormHealth(WormTeam, WormNumber, int(FinalHealth2));  targtHP = GS->Info.GetWormHealth(WormTeam, WormNumber);  } 
     }  
  }*/     
    if (Type == M_PRETURNSTART) turnBegan = false;   
               
    if (Type==M_DRAWQUEUE) //Marked worms graphic effect
    {     
        if (isCursed && timesTargeted < 3)  //CURSE   
        {
            AddSpriteEx(14, PosX, PosY, indCrossHair, crshrnim, crshrot, 1.4);
        }
    }
}

CWorm * CWorm::SCFindWorm(int l, int r, int t, int b)   // left, right, top, bottom  Copied from MineFinder
{
      for(int i = 0; i < Env->Objs.Count; i++)
      {
          local obj = CGObject(Env->Objs.Objs[i]);
          if(obj == NullObj) continue;
          if(obj == this) continue;
          if(obj->IsInBox( l, t, r, b) == false) continue;   //discard everything out of bounds
          
          if(obj is CWorm == false) continue; 
          
          returnWorm = CWorm(obj); 
          return returnWorm;
          
      }
      return NullObj;
}

CGObject * CWorm::DeathMarkFindTarget(int l, int r, int t, int b)
{
      for(int i = 0; i < Env->Objs.Count; i++)
      {
          local obj = CGObject(Env->Objs.Objs[i]);
          if(obj == NullObj) continue;
          if(obj == this) continue;
          if(obj->ClType != OC_Worm) continue;
          w = CWorm(obj);
          if(w->isMarked) continue;
          if(w->isCursed) continue;
          if(obj->IsInBox( l, t, r, b))
              return obj;
      }
      return CGObject(NullObj);
}

override void CWorm::Free(bool FreeMem)
{
 if (FreeMem) someonesAssIsMarked = false;
 super;
}

void CWorm::ResetCurse()
{
                        for (local i = 0; i < 5; i++)
                        {
                            beingTargetedBy[i] = -1; 
                        }
                        isCursed = false; timesTargeted = 0; curseCoolDown = 0;  someonesAssIsMarked = false; 
}

void CWorm::ApplyCurse()
{                       
                       ResetCurse();
                       isCursed = true;     //Set variables from here
                       curseCoolDown = 3;  //2 turns
                       timesTargeted = 0;
                       PlayLocalSound(31, 5, 1.0, 1.0);
                       PlayLocalSound(138, 5, 1.0, 1.0);
}

override void CWorm::FireFinal(CWeapon* Weap,CShootDesc* Desc)
{
if (Weap->CheckName("Death Mark"))
{
        if(TurnSide > 0)
            local obj = DeathMarkFindTarget(PosX, PosX + 28, PosY - 15, PosY + 15);
        else if(TurnSide < 0)
            local obj = DeathMarkFindTarget(PosX - 28, PosX, PosY - 15, PosY + 15);
        else return;
        if(obj != NullObj)
        {
             CWorm * w = CWorm(obj);
                       w->isMarked = true;           //Set variables from here
                       w->markCoolDown = 4;  //(3 turns) Worms Reloaded makes it so that it lasts some turns
                       w->dmgMultiplier = 1.5;       //Can be revised/balanced
                       w->timesHit=0;                //Pontential mechanic to implement
                       //w->targtHP = GS->Info.GetWormHealth(w->WormTeam, w->WormNumber);
                       PlayLocalSound(31, 5, 1.0, 1.0);
                       PlayLocalSound(138, 5, 1.0, 1.0);
        }
        else return;
}
else if (Weap->CheckName("Homing Curse"))
{
        if(TurnSide > 0)
            local obj = DeathMarkFindTarget(PosX, PosX + 28, PosY - 15, PosY + 15);
        else if(TurnSide < 0)
            local obj = DeathMarkFindTarget(PosX - 28, PosX, PosY - 15, PosY + 15);
        else return;
        if(obj != NullObj)
        {
                       CWorm * w = CWorm(obj);
                       w->ApplyCurse();
        }
        else return;
}
else if (Weap->CheckName("Wish Of Death"))
{
                  local Sx = Desc->AddX;
                  local Sy = Desc->AddY;
                  local obj = SCFindWorm(Sx-12, Sx+12, Sy-15, Sy+15);
		
		  if(obj != NullObj)                  
        	      {
	               CWorm * w = CWorm(obj);
                       w->ApplyCurse();
        	      }
                  if (obj==NullObj)
		      {
         	       local wepMR = GS->GetWeaponByName("Wish Of Death");
                       GS->Info.AddWeapon(WormTeam, wepMR->GetWeaponIndex(), 1);   // adding one weapon back
        	       ShowMessage("Nothing found!");
                       PlayLocalSound(120, 3, 1.0, 1.0);
                       return;
                      }  
                }
else if (Weap->CheckName("Marked For Death"))
{
                  local Sx = Desc->AddX;
                  local Sy = Desc->AddY;
                  local obj = SCFindWorm(Sx-12, Sx+12, Sy-15, Sy+15);
		
		  if(obj != NullObj)                  
        	      {
                       CWorm * w = CWorm(obj);
                       w->isMarked = true;           //Set variables from here
                       w->markCoolDown = 4;  //(3 turns) Worms Reloaded makes it so that it lasts some turns
                       w->dmgMultiplier = 1.5;       //Can be revised/balanced
                       w->timesHit=0;                //Pontential mechanic to implement
                      // w->targtHP = GS->Info.GetWormHealth(w->WormTeam, w->WormNumber);
                       PlayLocalSound(31, 5, 1.0, 1.0);
                       PlayLocalSound(138, 5, 1.0, 1.0);
        	      }
                  if (obj==NullObj)
		      {
         	       local wepMR = GS->GetWeaponByName("Marked For Death");
                       GS->Info.AddWeapon(WormTeam, wepMR->GetWeaponIndex(), 1);   // adding one weapon back
        	       ShowMessage("Nothing found!");
                       PlayLocalSound(120, 3, 1.0, 1.0);
                       return;
                      }  
                }
else super;
}         