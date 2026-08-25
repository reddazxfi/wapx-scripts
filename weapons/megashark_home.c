//Script by Lucho/reddaz
//just make sure to name the weap properly, and to tweak speed settings to your liking
//main variables: dist (distance from owner before searching target)
//homingStrength: how fast it turns, change to 0.35-0.45 for CMissile to get a reduced arc.
//homingCooldown: how often it tries to turn / find target.  (less = more often) change the to 2 for CMissile to get a smoother arc, but reduce homingStrength value
require utils;     

#MEGASHARK      //just in case

int bulletsPerTarget;      //Max current bullets on target. Change on CWorm params.

override void CWorm::CWorm(CObject* Parent,int aTeam,int aIndex,CWormParams* params)
{
 super;
 bulletsOnMe = 0;
 cooldown = 0;
}

override void CMissile::CMissile(CObject* parent, CWeaponLaunch* ldata, CShootDesc* sdata)
{
    super(parent, ldata, sdata); 
    if(weap->CheckName("MEGASHARK") == true)
    {                                                               //custom params for instancing the weapon      
    bool isHomingActive = false;
    CWorm* Target = CWorm(NullObj);  //Declare this as global if you want all of them to share their target
    int homingCooldown = 0; 
    bulletsMSPT = bulletsPerTarget;
    OwnerWorm = GetCurrentWorm(); 
    OwnerTeamM = sdata->Team;
    OwnerColorM = GetTeamColor(sdata->Team);
    if (OwnerWorm != NullObj) PRegister(&OwnerWorm); 
    MSBullet = false;
    misRangeDivider = 1.0;
    
    bounceCounter = 0;
    }
}

override void CMissile::Free(bool FreeMem)
{     
 if (Target!=NullObj) if (Target->bulletsOnMe>0) Target->bulletsOnMe--;
 super;
}

override void CMissile::Message(CObject* sender, EMType Type, int MSize, CMessageData* MData)
{
   super(sender, Type, MSize, MData);
   if(weap->CheckName("MEGASHARK") == true )  {
    if (Type == M_FRAME)
    {
        float missileX = PosX;
        float missileY = PosY;
        float ownerX = OwnerWorm->PosX;
        float ownerY = OwnerWorm->PosY;
        float dist = sqrt((missileX - ownerX) * (missileX - ownerX) + (missileY - ownerY) * (missileY - ownerY));

        if (!isHomingActive)
        {
            if (dist > 90.0)//Activation distance from firing worm
            {
                isHomingActive = true;
                homingCooldown = 0;
            }
            return;
        }
        if (isHomingActive && homingCooldown == 0)
        {
            // Sentry-inspired: Check range and health
            float sensorRadiusSqr = 40000.0 * 40000.0;   //15000 * 15000 is sentrygun range
            if (Target != NullObj)
            {
                //changeIndexTo = 1;
                float dx = Target->PosX - missileX;
                float dy = Target->PosY - missileY;
                float distSqr = dx * dx + dy * dy;
                if (distSqr > sensorRadiusSqr || Target->TargHealth <= 0)
                {
                    if (Target->bulletsOnMe>0) Target->bulletsOnMe--;
                    PUnregister(&Target);
                    Target = NullObj;
                }
            }

            if (Target == NullObj || Target->ObjState==WS_DEAD || Target->ObjState==WS_SINKING|| Target->ObjState==WS_DEATH)
            {                   
                PUnregister(&Target);
                Target = FindClosestEnemy(this);
                if (Target != NullObj)
                {
                    Target->bulletsOnMe++;
                    PRegister(&Target);
                    //string targBul = itoa(Target->bulletsOnMe);
                    //GG->WriteToChat(4, targBul, false);
                }
                else
                {
                    homingCooldown=7;
                    return; // Continue trajectory
                }
            }
            /*
            if (Target!=NullObj && Target->bulletsOnMe>bulletsPerTarget)  //bulletsPerTarget on shoot script
            {
                PUnregister(&Target);
                homingCooldown=8;
                Target=NullObj;
                return;
            } */
            if(Target->PosX==0&& Target->PosY==0){PUnregister(&Target);Target=NullObj;homingCooldown=2;return;} //invisible worm at 0,0 that idk how it got there and it's prolly my fault
                        
                // --- NEW DIRECT GUIDANCE LOGIC ---   thank you deep seek i cant do math

                // 1. Get the direction vector pointing straight at the target.
                float dirX = Target->PosX - missileX;
                float dirY = Target->PosY - missileY;

                // 2. Normalize the vector to get a pure direction (length of 1).
                float distanceToTarget = sqrt(dirX * dirX + dirY * dirY);
                if (distanceToTarget > 0) // Avoid division by zero
                {
                    dirX = dirX / distanceToTarget;
                    dirY = dirY / distanceToTarget;
                }

                float missileSpeed = 15.5;   // How fast the missile tries to fly.
                float homingStrength = 0.53; // How sharply it can turn (0.0 to 1.0).

                // 4. Calculate the ideal velocity (direction * speed).
                float requiredSpX = dirX * missileSpeed;
                float requiredSpY = dirY * missileSpeed;

                // 5. Gently steer the current velocity towards the ideal velocity.
                // This prevents the zig-zagging and creates a smooth turn.
                SpX += (requiredSpX - SpX) * homingStrength;
                SpY += (requiredSpY - SpY) * homingStrength;
                
                homingCooldown = 2; // Cooldown before the next minor adjustment.
        }
        else if (homingCooldown > 0)
        {
            homingCooldown--;
        }
    }
   }
}
CWorm* CMissile::FindClosestEnemy(CMissile* missile)   //Sentry copy paste, thanks entuser i guess
{
    CWorm* bestTarget = NullObj;
    float bestScore = 0;
    float sensorRadiusSqr = 199900.0 ;
    if (misRangeDivider!=0) sensorRadiusSqr/misRangeDivider;
    int maxIterations = 99999; 
    local teamColor = -1;
    if (OwnerWorm!=NullObj)
    teamColor = GetTeamColor(OwnerWorm->WormTeam);   
    if (Env == NullObj || Env->Objs == NullObj)
    {
        return bestTarget;
    }
    for (int i = 0; i < Env->Objs.Count && i < maxIterations; i++)
    {
        CGObject* obj = CGObject(Env->Objs.Objs[i]);
        // Only consider worm objects
        if (obj == NullObj || obj->ClType != OC_Worm)
            continue;
        CWorm* worm = CWorm(obj);
        enColor = GetTeamColor(worm->WormTeam);
        // Comprehensive validity check
        if (worm == NullObj              || 
            worm->ObjState == WS_DEAD    || 
            worm->ObjState == WS_SINKING || 
            worm->ObjState == WS_DEATH   || 
            worm->TargHealth <= 0        || 
            worm    == OwnerWorm         ||
            enColor == teamColor         ||
            enColor == OwnerColorM       ||
            worm->WormTeam == OwnerTeamM)     
        {
            continue;
        }
        if (worm->bulletsOnMe>=bulletsMSPT) if(!MSBullet) continue; 
        float dx = worm->PosX - missile->PosX;            
        float dy = worm->PosY - missile->PosY;
        float distSqr = dx * dx + dy * dy;
        if (distSqr < sensorRadiusSqr)
        {
            float score = 1000000 - distSqr;
            if (IsTargetVisible(missile, worm))
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

bool CMissile::IsTargetVisible(CMissile* missile, CGObject* target)
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