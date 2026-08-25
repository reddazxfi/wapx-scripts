//Script by Lucho/reddaz
//replace all "CArrow" with CMissile and it'll work for weaps too
//just make sure to name the weap properly, and to tweak speed settings to your liking
//main variables: dist (distance from owner before searching target)
//homingStrength: how fast it turns, change to 0.35-0.45 for CMissile to get a smoother arc.
//homingCooldown: how often it tries to turn / find target. change the to 3-4 for CMissile.
require utils;     
bool shouldNextArrowHome;  // 1 __ change to "shouldNextYourWeapHome"
float activatonDist;
CWeapon* g_longbowH;  //2_ change to g_yourWeap

void homingScriptLB::FirstFrame()
{
g_longbowH = GetWeaponByName("Homing Longbow"); //2_
keySync->AddKey(49); // 1 button
keySync->AddKey(50); // 2 button
keySync->AddKey(51); // 3 button
keySync->AddKey(52); // 4 button
keySync->AddKey(53); // 5 button
activatonDist = 115;
}

override void CArrow::CArrow(CObject* Parent, int*damage, CShootDesc* desc) //Change to: override CMissile::CMissile(CObject* parent,CWeaponLaunch* ldata,CShootDesc* sdata).
{   
    shouldAHome = shouldNextArrowHome;  //same as 1_ shouldAHome = shouldNextYourWeapHome 
    if (shouldAHome)
    { 
    int homingCooldown;   
    isHomingActive = false;
    Target = CWorm(NullObj);   
    homingCooldown = 0;
    OwnerWormArr = GetCurrentWorm(); 
    if (OwnerWormArr != NullObj) PRegister(&OwnerWormArr);
    } 
    super;
}

override void CWorm::FireFinal(CWeapon* Weap, CShootDesc* Desc)
{
    if (Weap->CheckName("Homing Longbow"))//change this to your weap's accurate name (case sensitive)
    {
        shouldNextArrowHome = true;
        super(g_longbowH, Desc);   // same as 2_
        return;
    }
    shouldNextArrowHome = false;      //same as 1_
    super;
} 

override void CArrow::OnSink() //Sometimes it sinks and finds target underwater
{
super;
if (shouldAHome)
  {
  shouldAHome=false;
  if (Target!=NullObj)PUnregister(&Target);
  if (Target!=NullObj) Target=NullObj;
  }
}

override void CArrow::Message(CObject* sender, EMType Type, int MSize, CMessageData* MData)
{
    super;
    if (Type == M_FRAME){
    if (shouldAHome){    //No need to change this.
        float missileX = PosX;
        float missileY = PosY;
        float ownerX = OwnerWormArr->PosX;
        float ownerY = OwnerWormArr->PosY;
        float dist = sqrt((missileX - ownerX) * (missileX - ownerX) + (missileY - ownerY) * (missileY - ownerY));

        if (!isHomingActive)
        {
            if (dist > activatonDist)   //Don't home right away
            {
                isHomingActive = true;
                homingCooldown = 0;
                //ShowMessage("Homing activated");
            }
            return;
        }
        if (isHomingActive && homingCooldown == 0)
        {
            // Sentry-inspired: Check range and health
            float sensorRadiusSqr = 60000.0 * 60000.0;
            if (Target != NullObj)
            {
                float dx = Target->PosX - missileX;
                float dy = Target->PosY - missileY;
                float distSqr = dx * dx + dy * dy;
                if (distSqr > sensorRadiusSqr || Target->TargHealth <= 0)
                {
                    if (Target != NullObj) PUnregister(&Target);
                    Target = NullObj;
                }
            }

            if (Target == NullObj || Target == CWorm(NullObj))
            {                   
                PUnregister(&Target);
                Target = FindClosestEnemy(this);
                if (Target != NullObj)
                {
                   // GG->WriteToChat(5, "Target acquired", false);
                    PRegister(&Target);
                }
                else
                {
                   // GG->WriteToChat(5, "No target", false);
                    homingCooldown=2;
                    return; // Continue trajectory
                }
            }
            if(Target->PosX==0 && Target->PosY==0 || Target->ObjState==WS_DEAD || Target->ObjState==WS_SINKING|| Target->ObjState==WS_DEATH)
            {PUnregister(&Target);Target=NullObj;homingCooldown=3;return;} //invisible worm at 0,0 that idk how it got there and it's prolly my fault
                        
                // --- NEW DIRECT GUIDANCE LOGIC ---   thank you deep seek i cant do math

                // 1. Get the direction vector pointing straight at the target.
                float dirX = Target->PosX - missileX;
                float dirY = Target->PosY-5 - missileY;

                // 2. Normalize the vector to get a pure direction (length of 1).
                float distanceToTarget = sqrt(dirX * dirX + dirY * dirY);
                if (distanceToTarget > 0) // Avoid division by zero
                {
                    dirX = dirX / distanceToTarget;
                    dirY = dirY / distanceToTarget;
                }

                float missileSpeed = 14.0;   // How fast the missile tries to fly.
                float homingStrength = 0.48; // How sharply it can turn (0.0 to 1.0).

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
    }}
}  
                                                      
CWorm* CArrow::FindClosestEnemy(CArrow* missile)    //Sentry copy paste, thanks entuser i guess
{
    CWorm* bestTarget = NullObj;                 
    float bestScore = 0;
    float sensorRadiusSqr = 199900.0;
    int maxIterations = 100;
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
        // Comprehensive validity check
        if (worm == NullObj || 
            worm->ObjState == WS_DEAD ||   //Dead worms have health, thanks entuser.
            worm->ObjState == WS_SINKING || 
            worm->ObjState == WS_DEATH || 
            worm->TargHealth <= 0 || 
            worm == OwnerWormArr || 
            worm->WormTeam == OwnerWormArr->WormTeam)
        {
            continue;
        }
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

bool CArrow::IsTargetVisible(CArrow* missile, CGObject* target)
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

override void CWorm::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
super; 
  if (GetCurrentWorm()!=this) return;
  if (Type == M_FRAME)
  {
        if (CurWeapon != NullObj && CurWeapon->CheckName("Homing Longbow"))
        {
            if (keySync->KeyPressedNow(49)) // Player pressed "1"
            {
                activatonDist = 30.0;
                ShowTeamMessage(8, "Longbow: Home Right Away", WormTeam);
                PlayLocalSound(92, 1.0, 1.0, 1.0);
            }
            else if (keySync->KeyPressedNow(50)) // Player pressed "2"
            {
                activatonDist = 110.0;
                ShowTeamMessage(8, "Longbow: Swift Homing Activation", WormTeam);   
                PlayLocalSound(92, 1.0, 1.0, 1.0);
            }
            else if (keySync->KeyPressedNow(51)) // Player pressed "3"
            {
                activatonDist = 220.0;
                ShowTeamMessage(8, "Longbow: Slightly Delayed Homing Activation", WormTeam); 
                PlayLocalSound(92, 1.0, 1.0, 1.0);
            }
            else if (keySync->KeyPressedNow(52)) // Player pressed "4"
            {
                activatonDist = 580.0;
                ShowTeamMessage(8, "Longbow: Delayed Homing Activation", WormTeam); 
                PlayLocalSound(92, 1.0, 1.0, 1.0);
            }
        }
  }
  if (Type==M_SETWEAPON)
  {
     if(CurWeapon!=NullObj && CurWeapon->CheckName("Homing Longbow") == true)
     {
     activationDist = 110; //Reset variable
     ShowTeamMessage(8, "Swift Homing Activation", WormTeam);
     }    

  } 
   
}     