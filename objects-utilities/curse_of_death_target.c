require utils;        
 
override void COilDrum::COilDrum(CObject* Parent,fixed X,fixed Y,bool Snap)
{
 homingCD = 20;
 homingIterations = 0; //Holy Hand Grenade or others.
 homeActive = false;   
 CurseTarget = CWorm(NullObj);
 super;  
}

override void COilDrum::Explosion()
{
 deleteTracking = true;
 homeActive = false;
 CurseTarget = CWorm(NullObj);
 super;
} 


override void COilDrum::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{    
 super;
 if (Type==M_FRAME)
   {
      if (CurseTarget == NullObj && !deleteTracking){homeActive = false; CurseTarget = FindClosestVisibleCursedWorm(this);}
     
      if (CurseTarget != NullObj) homeActive = true; 
     
      if (homeActive && CurseTarget != NullObj)  //Homing
      {
                        // --- NEW DIRECT GUIDANCE LOGIC ---   thank you deep seek i cant do math
                if (CurseTarget == NullObj) return;
                float missX = PosX;
                float missY = PosY;  
                if (homingIterations>=200)
                   {
                   homingCD = 9900;       //Holy Grenade or others
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

                  float missileSpeed = 12.0;   // How fast the missile tries to fly.
                  float homingStrength = 0.33; // How sharply it can turn (0.0 to 1.0).

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
   }
   if (Type==M_PRETURNSTART)
   {
      HomingCD = 50;
   }  
}

bool COilDrum::IsThereLandThereAll(fixed trgtX, fixed trgtY)
{     
    if (ClType!=OC_OilDrum) return NullObj;
    if (this == NullObj) return true;
    
    int hitX; int hitY;
    int b = 4;
    
    // Offset target upward
    fixed targetYOffset = trgtY - 3;  // Tune this value (try 4-8)
    
    // 1. Check the CENTER path
    if (TraceLine(this, PosX, PosY, trgtX, targetYOffset, CMASK_TERRAIN, &hitX, &hitY) != NullObj) return true;
    
    // 2. Check the 4 corners
    if (TraceLine(this, PosX - b, PosY - b, trgtX - b, targetYOffset - b, CMASK_TERRAIN, &hitX, &hitY) != NullObj) return true;
    if (TraceLine(this, PosX + b, PosY - b, trgtX + b, targetYOffset - b, CMASK_TERRAIN, &hitX, &hitY) != NullObj) return true;
    if (TraceLine(this, PosX - b, PosY + b, trgtX - b, targetYOffset + b, CMASK_TERRAIN, &hitX, &hitY) != NullObj) return true;
    if (TraceLine(this, PosX + b, PosY + b, trgtX + b, targetYOffset + b, CMASK_TERRAIN, &hitX, &hitY) != NullObj) return true;
    
    return false;
}

CWorm* COilDrum::FindClosestVisibleCursedWorm(COilDrum* curMis)
{
    if (ClType!=OC_OilDrum) return NullObj;
    CWorm* closestWorm = CWorm(NullObj);
    
    CWorm* currWorm = GetCurrentWorm();
    float closestDistance = 999999.0; // Large number
    
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
            if(IsThereLandThereAll(worm->PosX, worm->PosY) == false)
            {
                // Calculate distance
                float dx = worm->PosX - PosX;
                float dy = worm->PosY - PosY;
                float distance = sqrt(dx*dx + dy*dy);
                
                // Find closest
                if(distance < closestDistance)
                {
                    closestDistance = distance;
                    //worm->timesTargeted++;         //Oildrums are free
                    closestWorm = worm;
                }
            }
        }
    }
    return closestWorm;
}

require utils;        

override void CArrow::CArrow(CObject* Parent,int* damage,CShootDesc* desc)
{
 
 homingCD = 20;
 homingIterations = 0; //Holy Hand Grenade or others.
 homeActive = false;
 CurseTarget = CWorm(NullObj);
 super;
}

override void CArrow::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{    
 if (Type==M_FRAME)
   {
      if (CurseTarget == NullObj){homeActive = false; CurseTarget = FindClosestVisibleCursedWorm(this);}
     
      if (CurseTarget != NullObj) homeActive = true;
     
      if (homeActive)  //Homing
      {
                        // --- NEW DIRECT GUIDANCE LOGIC ---   thank you deep seek i cant do math
                if (CurseTarget == NullObj)return;
                float missX = PosX;
                float missY = PosY;  
                if (homingIterations>=110)
                   {
                   homingCD = 9000;       //Holy Grenade or others
                   SpX = 0;
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

                  float missileSpeed = 12.0;   // How fast the missile tries to fly.
                  float homingStrength = 0.33; // How sharply it can turn (0.0 to 1.0).

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
   }  
 super;
}


bool CArrow::IsTargetVisibleAll(CArrow* missile, CWorm* target)
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

CWorm* CArrow::FindClosestVisibleCursedWorm(CMissile* curMis)
{
    CWorm* closestWorm = CWorm(NullObj);
    
    CWorm* currWorm = GetCurrentWorm();
    float closestDistance = 999999.0; // Large number
    
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
                    worm->timesTargeted++;
                    closestWorm = worm;
                }
            }
        }
    }
    return closestWorm;
}

require utils;        

override void CMine::CMine(CObject* Parent,CMineParams* Params,CShootDesc* SDesc,bool Snap,int Unk)
{
 homingCD = 20;
 homingIterations = 0; //Holy Hand Grenade or others.
 homeActive = false;
 CurseTarget = CWorm(NullObj);
 super;
}

override void CMine::Explosion()
{
 deleteTracking = true;
 homeActive = false;
 super;
} 

override void CMine::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{    
 super;
 if (Type==M_FRAME)
   {
      if (CurseTarget == NullObj && !deleteTracking){homeActive = false; CurseTarget = FindClosestVisibleCursedWorm(this);}
     
      if (CurseTarget != NullObj) homeActive = true; 
     
      if (homeActive && CurseTarget != NullObj)  //Homing
      {   
       if (#BEE) if (this is CBee) { homeActive = false; return;}
       
       if (this is CMine == false) {homeActive = false; return; }
                        // --- NEW DIRECT GUIDANCE LOGIC ---   thank you deep seek i cant do math
                if (CurseTarget == NullObj)return;                
                float missX = PosX;
                float missY = PosY;  
                if (homingIterations>=200)
                   {
                   homingCD = 9000;       //Holy Grenade or others
                   SpX = 0;
                   deleteTracking  = true;
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

                  float missileSpeed = 12.0;   // How fast the missile tries to fly.
                  float homingStrength = 0.33; // How sharply it can turn (0.0 to 1.0).

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
   }  
}

bool CMine::IsTargetVisibleAll(CMine* missile, CWorm* target)
{   
    if (this is CMine == false || this is CBowlingBall || this is PxSentryGun || this is Jewish || this is CMagnet || this is PxFan || this is CTurretBase || this is CBee) return false;
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

CWorm* CMine::FindClosestVisibleCursedWorm(CMine* curMis)
{
    if (#BEE) if (this is CBee) return NullObj;   
    if (this is CBowlingBall || this is PxSentryGun || this is Jewish || this is PxAirstrike ) return NullObj;
    if (this is CMine == false || ClType!=OC_Mine) return NullObj;
    if (this == NullObj) return NullObj;
    CWorm* closestWorm = CWorm(NullObj);
    
    CWorm* currWorm = GetCurrentWorm();
    float closestDistance = 999999.0; // Large number
    
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
                    worm->timesTargeted++;
                    closestWorm = worm;
                }
            }
        }
    } 
    return closestWorm;
}