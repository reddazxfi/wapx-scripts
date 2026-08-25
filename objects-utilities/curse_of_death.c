// Worm dmg vulnerability debuff like in Worms Reloaded or WA2, and all objects home debuff.
require utils, utils_steps, pxparticles, utility_endure;

CSprite * mCrossHair;
bool someonesAssIsMarked;
int missileQnt;

void curse_of_death_script::InitGraphic()
{
  someonesAssIsMarked = false;
  ransubtract = false;     
  missileQnt = 0;
  
  CFile * f;
  f = GetAttachment("markcrosshair.png");
  mCrossHair = LoadSprite(f,6,0);
}

///////////////////////////////////////////Mark and Homing Code///////////////////////////////////////////

override void CMissile::CMissile(CObject* parent,CWeaponLaunch* ldata,CShootDesc* sdata)
{
 homingCD = 20;        //Cooldown
 homingIterations = 0; //For Holy Hand Grenade or others.
 homeActive = false;   
 CurseTarget = CWorm(NullObj);
 super;  
 missileQnt++;
} 

override void CMissile::Free(bool FreeMem)
{
 missileQnt--;
 super;
}

override void CMissile::ExplodeAt(fixed x,fixed y)
{
 deleteTracking = true;
 homeActive = false;  
 CurseTarget = CWorm(NullObj);
 super;
} 


//////////////////////HOMING BEHAVIOR//////////////////////

//Draw a line between missile and worm
bool CMissile::IsTargetVisibleAll(CMissile* missile, CGObject* target)
{
    if (missile == NullObj || target == NullObj)
    {
        return false;
    }
    int hitX; int hitY;
    if (TraceLine(missile, missile->PosX, missile->PosY, target->PosX, target->PosY, CMASK_TERRAIN, &hitX, &hitY) == NullObj)
    {
        return true;
    }
    return false;
}


//Sentry loop to find worm through a loop, limited to a certain range (optional)
CWorm* CMissile::FindClosestVisibleCursedWorm(CMissile* curMis)
{
    if (!someonesAssIsMarked) return NullObj;
    if (missileQnt > 50) return NullObj;  //fuckass scarab
    
    CWorm* closestWorm = CWorm(NullObj);
    
    CWorm* currWorm = GetCurrentWorm();
    float closestDistance = 149999.0; // Large number unless you want to change it
    
    for(int i = 0; i < Env->Objs.Count; i++)
    {
        local obj = CGObject(Env->Objs.Objs[i]);
        if (obj is NullObj) continue;       
        if (obj == NullObj) continue;
        if(obj is CWorm == false) continue;
        
        local worm = CWorm(obj);   
        if (worm == NullObj) continue;
        if(worm->isCursed && worm != currWorm) // Don't target yourself
        {
            // Check if visible
            if(IsTargetVisibleAll(curMis, worm))
            {
                // Calculate distance
                float dx = worm->PosX - PosX;
                float dy = worm->PosY - PosY;
                float distance = sqrt(dx*dx + dy*dy);
                
                // Find closest
                if(distance < closestDistance)
                {
                    closestDistance = distance;
                    worm->timesTargeted++;      //Target limit.
                    closestWorm = worm;
                }
            }
        }
    }
    return closestWorm;
}

override void CMissile::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
 local deleteTracking = false;     
 super;
 if (Type==M_FRAME)
   {
      if (CurseTarget == NullObj && !deleteTracking){homeActive = false; if (missileQnt <= 50) CurseTarget = FindClosestVisibleCursedWorm(this);}  //Check for targets with the function
     
      if (CurseTarget != NullObj) homeActive = true;   //If it finds target, activate homing
     
      if (homeActive && CurseTarget != NullObj)  //Homing
      {
        HomeToTarget();
      }         
   }
}

void CMissile::HomeToTarget()
{
                           // --- NEW DIRECT GUIDANCE LOGIC ---   thank you deep seek i cant do math
                if (CurseTarget == NullObj) return;
                float missX = PosX;
                float missY = PosY;  
                if (homingIterations>=280)
                   {
                   homingCD = 99000;       //Holy Grenade or others
                   SpX = 0;
                   deleteTracking = true;
                   homeActive = false;
                   }
                if (homingCD==0)
                {
                  // 1. Get the direction vector pointing straight at the target.
                  float dirX = CurseTarget->PosX - missX;
                  float dirY = CurseTarget->PosY - missY;

                  // 2. Normalize the vector to get a pure direction (length of 1).
                  float distanceToTarget = sqrt(dirX * dirX + dirY * dirY);
                  if (distanceToTarget > 0) // Avoid division by zero
                  {
                    dirX = dirX / distanceToTarget;
                    dirY = dirY / distanceToTarget;
                  }
                  float missileSpeed = 15.0;   // How fast the missile tries to fly.
                  float homingStrength = 0.40; // How sharply it can turn (0.0 to 1.0)
                                                                                         
                  if (this is heliShoot == true)
                  { 
                   missileSpeed = 3.0;   // How fast the missile tries to fly.
                   homingStrength = 0.2; // How sharply it can turn (0.0 to 1.0)
                  }
                  // 4. Calculate the ideal velocity (direction * speed).
                  float requiredSpX = dirX * missileSpeed;
                  float requiredSpY = dirY * missileSpeed;

                  // 5. Gently steer the current velocity towards the ideal velocity.
                  // This prevents the zig-zagging and creates a smooth turn.
                  SpX += (requiredSpX - SpX) * homingStrength;
                  SpY += (requiredSpY - SpY) * homingStrength;
                
                  homingCD = 3; // Cooldown before the next minor adjustment. 
                  homingIterations++;
                }
                else if (homingCD > 0)
                {
                     homingCD--;
                     homingIterations++;
                }
}
/////////////////////////////////////////// Worm Code ///////////////////////////////////////////

override void CWorm::CWorm(CObject* Parent,int aTeam,int aIndex,CWormParams* params)
{
 isCursed = false;  
 isMarked = false;
 timesHit = 0;
 dmgMultiplier = 0.0;
 targtHP =  GS->Info.Teams.Worms.Health;
 timesTargeted = 0;
 curseCoolDown = 0;
 markCoolDown = 0;
 invincible = false;
 turnBegan = false;
 
 indCrossHair = mCrossHair->Index;
 crosshairreverse = false;  
 crshrot = 0.0;
 crshrnim = 0.000;         
 crosshairspeed = 5.0;
 
 super;
}

override void CWorm::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{       
  super;
  if (Type == M_FRAME)  //Loop
  {  
  
  if (isCursed)
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
  }
  if (isMarked && markCoolDown == 0) isMarked = false;
  if (curseCoolDown == 0 && isCursed)
  {
  isCursed = false; timesTargeted = 0; curseCoolDown = 0;
  }
  if (timesTargeted >=2)   //Disable the curse
  {
    isCursed = false; timesTargeted = 0; curseCoolDown = 0;  someonesAssIsMarked = false; 
  }
  if (timesHit > 5 || markCoolDown == 0 ) if (isMarked){ isMarked = false; deleteTracking = true; someonesAssIsMarked = false; }  //Remove mark and reset variables
  if (!isMarked && deleteTracking) {markCoolDown = 0; timesHit=0; dmgMultilplier = 0.0; deleteTracking = false; someonesAssIsMarked = false; }
  if (!Endure && !invincible && WasHit && dmgMultiplier!=0.0 && targtHP!=GS->Info.GetWormHealth(WormTeam, WormNumber))
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
  }
  if (Type == M_FRAME)   //When turn begins, reduce cooldown or remove mark.
  { 
     if (Root->IsTimerActive() == true && !turnBegan && GetCurrentWorm()!=NullObj)
     if (isCursed||isMarked)
     {
     local cwm = GetCurrentWorm();
     if (cwm !=NullObj)
     {
        if (cwm->WormTeam == WormTeam)
        {
            if (curseCoolDown>0) curseCoolDown--;
            if (markCoolDown>0) markCoolDown--; 
            turnBegan = true;
        }                                         
     }
     }
  }
  if (Type == M_PRETURNSTART) turnBegan = false;   
  if (Type==M_GUNEXP || Type == M_EXPLOSION)
            WasHit = true; //Send the signal to modify HP
               
  if (Type==M_DRAWQUEUE) //Marked worms graphic effect
     {     
       //CURSE         
      if (isCursed)    
       {
         CrossHairCol = GetTeamColor(WormTeam) + 2;
         
         AddSpriteEx(14, PosX, PosY, indCrossHair, crshrnim, crshrot, 1.4);
       }
      if (isMarked) 
       //MARK
       {
        PxParticle *p;

             for(int i=0; i<10;i++)
             {
                p = new PxParticle(616, PosX+RandomInt(-5, 5), PosY-20);

                p->SetLifeTime(60);
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
                       w->targtHP = GS->Info.GetWormHealth(w->WormTeam, w->WormNumber);
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
                       w->isCursed = true;     //Set variables from here
                       w->curseCoolDown = 3;  //2 turns
                       w->timesTargeted = 0;
                       PlayLocalSound(31, 5, 1.0, 1.0);
                       PlayLocalSound(138, 5, 1.0, 1.0);
        }
        else return;
}
else if (Weap->CheckName("Wish Of Death"))
{
                  local Sx = SelectX/65536;
                  local Sy = SelectY/65536;
                  local obj = SCFindWorm(Sx-12, Sx+12, Sy-15, Sy+15);
		
		  if(obj != NullObj)                  
        	      {
				  	   CWorm * w = CWorm(obj);
                       w->isCursed = true;     //Set variables from here
                       w->curseCoolDown = 3;  //2 turns
                       w->timesTargeted = 0;
                       PlayLocalSound(31, 5, 1.0, 1.0);
                       PlayLocalSound(138, 5, 1.0, 1.0);
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
                  local Sx = SelectX/65536;
                  local Sy = SelectY/65536;
                  local obj = SCFindWorm(Sx-12, Sx+12, Sy-15, Sy+15);
		
		  if(obj != NullObj)                  
        	      {
                       CWorm * w = CWorm(obj);
                       w->isMarked = true;           //Set variables from here
                       w->markCoolDown = 4;  //(3 turns) Worms Reloaded makes it so that it lasts some turns
                       w->dmgMultiplier = 1.5;       //Can be revised/balanced
                       w->timesHit=0;                //Pontential mechanic to implement
                       w->targtHP = GS->Info.GetWormHealth(w->WormTeam, w->WormNumber);
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