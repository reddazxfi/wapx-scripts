require utils, utils_steps, utils_red;

#EXPLODER

CExplosionManager : CObject;        

CExplosionManager * Exploder;

override void CTurnGame::SecondFrame()
{
 super;
 if (Exploder == NullObj)
 {
    Exploder = new CExplosionManager();
 }
 //Test
 /*
 CWeaponLaunch newdata;
 zero(&newdata);
 newdata. explosion.damage = 40;
 newdata.explosion.pushPower = 100;
 newdata.explosion.bias = 40;
 newdata.explosion.flags = CMASK_ALL_WORMS + CMASK_OILDRUM + CMASK_MINE + CMASK_CRATE;
 newdata.timeBeforeExplosion = 100; //frames

 Exploder->NewExplosion( 400, 400, 45, &newdata, NullObj, false);  
 newdata.explosion.bias = 50;
 Exploder->NewExplosion( 400, 500, 45, &newdata, NullObj, false);     
 newdata.explosion.bias = 60;
 Exploder->NewExplosion( 400, 600, 45, &newdata, NullObj, false); */
}

CExplosionManager::CExplosionManager()
{
 ParamExplosionX = new float[99];
 ParamExplosionY = new float[99];   
 ParamDamage     = new int[99];
 ParamPushPower  = new int[99];
 ParamDestroyRad = new int[99];    
 ParamSound      = new int[99];  
 ParamCooldown   = new int[99];
 ParamFlags      = new int[99];
 ParamAlive      = new bool[99];  
 ParamTaze       = new bool[99];
 DoEffects = new CEffectManager*[99];
 n_b_e = 0;
 SomethingActive = false;
 
 for (int i = 0; i < 99; i++)
 {
    ParamAlive[i] = false;
    DoEffects[i]  = CEffectManager(NullObj);
 }

 super(Root,GS);
}                       

int CExplosionManager::NewExplosion(fixed x, fixed y, int sound, CWeaponLaunch *EData, CEffectManager *bufferedEffect, bool taze)
{      
 if (y > Env->Water + 185) return 98; // Not out of bounds but will do nothing
 
 if (Exploder == NullObj) return;
 local i = FindFreeIndex();
 if (i < 0) return i;
 
 
 //Clamp
 if (y < -1000) y = -1000; 
 if (y > Env->Water + 160) y = Env->Water + 160; //Pure virtual function call below map.
 if (x > GS->LevelSX + 1200) x = GS->LevelSX + 1200;
 if (x < -1200) x = -1200;
 
 SomethingActive = true;
 
 ParamExplosionX[i] = x;
 ParamExplosionY[i] = y;
 
 ParamDamage    [i] = EData->explosion.damage;
 ParamPushPower [i] = EData->explosion.pushPower;
 ParamDestroyRad[i] = EData->explosion.bias;       // Different purpose than in-game.     
 ParamFlags     [i] = EData->explosion.flags;
 ParamCooldown  [i] = EData->timeBeforeExplosion;
 ParamSound     [i] = sound;      
 DoEffects      [i] = bufferedEffect;   
 ParamTaze      [i] = taze;
 
 ParamAlive[i] = true; 
 if (i >= n_b_e) 
 {
        n_b_e = i + 1;
 }
    
 return i;
}

void CExplosionManager::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
    if (Type != M_FRAME) return;
    super;
    if (Type == M_FRAME)
    {
        Proceed();
    }
}

void CExplosionManager::Proceed()
{
   if (SomethingActive)
   {
      for (local i = 0; i < n_b_e; i++)
      {
         if (ParamAlive[i] == true)
         {
            if (ParamCooldown[i] > 0)
            {
               ParamCooldown[i]-=1;  
               StallEffect(i);
            }
            else if (ParamCooldown[i] <=0)
            {
               Detonate(i);
            }
         }
      }
   }
}

void CExplosionManager::StallEffect(int curr)
{            
   if (ParamAlive[curr] == false || DoEffects[curr] == NullObj) return;
   
   DoEffects[curr] -> explosionTimer = 0;
   DoEffects[curr] -> drawConventionalEffects = false;
   if (DoEffects[curr] -> freeAfter > 0)
   {
      DoEffects[curr] -> freeAfter++;  //Extend Free timer created by other methods.
   }
}

void CExplosionManager::ActivateEffect(int target)
{
   if (DoEffects[target]!=NullObj)
   {                                    
       DoEffects[target]->drawConventionalEffects = true;
       DoEffects[target]->targetObj = NullObj;  
       DoEffects[target]->ownerLess = false;    //Avoid accidental stalling
       DoEffects[target]->freeAfter = gframe + DoEffects[target]->expLimit;  //Unconditional Free
       DoEffects[target]->managerState = 1;
   }  
}

void CExplosionManager::Detonate(int curr)
{
   local i = curr;
   ParamAlive   [i] = false;  
   ParamCooldown[i] = 0;
   local shouldExplode = false;
   local doExpPar = true;
   if (DoEffects[i] != NullObj) doExpPar = false;
   if ( ParamDestroyRad[i] > 0) shouldExplode = true;
   
   do_custom_explosion(Root, ParamFlags[i],
                             ParamExplosionX[i],
                             ParamExplosionY[i],
                             ParamDamage[i],
                             ParamPushPower[i],
                             ParamDestroyRad[i],
                             shouldExplode,
                             doExpPar,  //Default explosion particles.
                             false,
                             ParamTaze[i],
                             0) ;
   
   if (ParamSound[i] == 0)
   PlaySound(RandomInt(69,71), 3.0, 1.0, 1.0);
   else              
   PlaySound(ParamSound[i], 3.0, 1.0, 1.0);
   
   if (DoEffects[curr]!=NullObj)
   {                                    
       DoEffects[curr]->lastKnownX = ParamExplosionX[i]; 
       DoEffects[curr]->lastKnownY = ParamExplosionY[i];
       DoEffects[curr]->drawConventionalEffects = true;
       DoEffects[curr]->targetObj = NullObj;  
       DoEffects[curr]->ownerLess = false;    //Avoid accidental stalling
       DoEffects[curr]->freeAfter = gframe + DoEffects[curr]->expLimit;  //Unconditional Free
       DoEffects[curr]->managerState = 1;
   }                          
   
   SomethingActive = CheckStates();
}

bool CExplosionManager::CheckStates()
{
    local activity = false;
    local highestPending = -1;

    for (local i = 0; i < 97; i++)
    {
        if (ParamAlive[i])
        {
            activity = true;
            highestPending = i;
        }   
    }

    n_b_e = highestPending + 1;

    return activity;
}

int CExplosionManager::FindFreeIndex()
{
    for (local i = 0; i < 97; i++)
    {
        if (ParamAlive[i] == false)
        {
            return i;
        }
    }
    return -1;   // no free slot available
}

void CExplosionManager::ForceExplosion(int target)
{
   Detonate(target);
   ActivateEffect(target);
}    

void CExplosionManager::DetonateRange(int targ1, int targ2)
{
    if (targ1 < 0 || targ1 > 98 || targ2 < 0 || targ2 > 98) return;
    if (targ1 > targ2)
    {
      target1 = targ2;
      targ2   = targ1;
      targ1   = target1;
    }
    
    for (i = targ1; i <= targ2; i++)
    {
        ForceExplosion(i);
    }
}