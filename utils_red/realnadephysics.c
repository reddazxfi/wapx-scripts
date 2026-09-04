// BY REDDAZ - MAY 2026
// THIS SCRIPT TELLS THE GAME WHAT TO DO WITH SURFACE NORMALS AND SLOPES, NO MATTER HOW YOU GET THEM
// (had to do custom function for objects because TraceLine is erratic and innaccurate with them)
// If there's a better way to find them, do suggest it, as this 100% reliable, not even 70%

require p_sprite_builder, utils_red; 

CTraceRes * reflect;

script realnadephysics::InitGraphic()
{
    reflect = new CTraceRes();
} 

override void CMissile::CMissile(CObject* parent,CWeaponLaunch* ldata,CShootDesc* sdata)
{                       
 NewBounce = false;   
 nadeSize = 2;     
    CheckMisMask =  CColMask(NullObj);  
 if (ldata->explosion.damage >= 6767 && ldata->explosion.damage < (6767+300)) // No global variables nor custom class :)
 {
    ldata->explosion.damage = ldata->explosion.damage - 6767;          
    nadeSize  = 3.0;  // Use 3 if unsure.        
    ColMask = new CColMask(nadeSize,nadeSize,MakeCircleMask(nadeSize)); 
    CheckMisMask = new CColMask(nadeSize +2, nadeSize +2, MakeCircleMask(nadeSize +2));  
    NewBounce = true;  
 }
 attemptedBounces   = 0;
 FreeMe = false;            
 //reflect = new CTraceRes();  //gate behind bounciness (fuckass scarabs)
 
 bncExplosionCooldown = 1;
 
 staticFrames = 0;
 bouncesInFrame = 0;   
 nadeRestitution = 0.65;          
 Dead = false;
 WeaponName = GetName();   //Must be a static variable (if you pass GrenadeWP here, check in MFrame with GrenadeWP, not "My Grenade") (yes you gotta make a new global string variable)  
 //Did this to manage SPB and missile overrides, missiles spawned with new CMissile don't inherit name.
  
 nadeTeam  = sdata->Team;  
 
 super;  
 
 safeAirPosX = PosX;
 safeAirPosY = PosY;   
 RegPSpX  = sdata->SpX; RegPSpY  = sdata->SpY;
 RegPosX1 = PosX;       RegPosY1 = PosY;
 RegPSpX1 = sdata->SpX; RegPSpY1 = sdata->SpY;  
 pastNx = 0.0; pastNy = 0.0;     
 pastNx2 = 0.0; pastNy2 = 0.0;   
 snapBounces = 0; snapBounces1 = 0;                       
 lowSpdCount = 0; attemptedBounces = 0; //2nd Anticrash   
 slowframecount = 0.0;     
 failCount = 0;      
 bounceLimit = 200;         
 MaxAllowedBounces = 150;                    
                                                    
 gravity = GravityFactor;        
 
 ignoreCol = CGObject(NullObj);
 
 if (NewBounce)
 {
    timesExploded = 0;
    snapTimer = 0;        
                            
    nadeSound = 0;
                   
    bounceCounter = 0;
    bouncing = false;     
         
    doExplosionOnTouch = false;
    detectExplosion = false;
    queueExplosion  = false;
    
    nadePushPower =  ldata->explosion.pushPower;
    explosionDmg  =  ldata->explosion.damage;
    
    nadeFriction = 0.0;  // How much it bounces back, 0.65 For normal nades
    flags = 0; 
 }  
}
//No need for DoWeapBouncy with this func, but it's cleaner to use that function (because of SpawnCheck).
void CMissile::ActivateBouncePhysics(CObject* parent, CWeaponLaunch* ldata, CShootDesc* sdata, int flagss, float radius, float restitution, float friction, int maxBounces,int sound, bool exp)
{
    nadeSize  = radius;    
    nadeSound = sound;   
      
    nadePushPower =  launchdata.explosion.pushPower;
    explosionDmg  =  launchdata.explosion.damage;
    
    bounceLimit = maxBounces;
    if (bounceLimit == 0)
    {
       bounceLimit = 230; //Anticrash   
    }
    if (bounceLimit>MaxAllowedBounces)
    MaxAllowedBounces = bounceLimit-10; 
    
    doExplosionOnTouch = exp;
    
    nadeFriction = friction;  // How much it bounces back, 0.65 For normal nades.
    nadeRestitution = restitution;  // Friction, 0.02 For normal nades. subtracts from tangent in tracer.    
    
    flags = flagss;
    if (flagss == 0)
    flags =             CMASK_DEFAULT_COLLIDEABLE;
    else if (flagss == 1) 
    flags =             CMASK_TERRAIN;   
    ColMask = new CColMask(radius,radius,MakeCircleMask(radius)); 
    if (CheckMisMask==NullObj)CheckMisMask = new CColMask(radius +2, radius +2, MakeCircleMask(radius +2)) ;  
    if (reflect==NullObj) reflect = new CTraceRes();                                            
    snapBounces = 0; snapBounces1 = 0;                       
    lowSpdCount = 0; attemptedBounces = 0; //2nd Anticrash   
    slowframecount = 0.0;                                     
}

override void CMissile::Collide(CGObject* Obj, int type)
{
    super; 

    if (NewBounce && (launchdata.action == WAction_Dig || launchdata.action == WAction_Roam || launchdata.action == WAction_Homing))
    {
        NewBounce = false;
    }

    if (!NewBounce)
    {
        skipRest = true;    
        bounceCounter++;
        return;
    }

    if (!skipRest && (attemptedBounces >= 1000 || isStatic))
    {
        super;   //Engine behavior then replace its calc with my thing   
        attemptedBounces -= 1.0;  
        bounceCounter++;
        if (!doExplosionOnTouch) bounceCounter += 1;
        else queueExplosion = true;     //Exploding this frame will remove slope to detect (duh)
        isStatic = true;       //Give it a second chance by bouncing originally some times
        staticFrames = 20;
        skipRest = true;
    }

    if (!skipRest)
    {
        if (Obj != NullObj && bouncesInFrame == 0)  //Triggering explosion once this frame
        {
            super;
            bounceCounter++;    //Increment this only on explosions 
            if (bounceCounter == 55) { nadeRestitution -= 0.12; } //only once
            if (bounceCounter == 105) { nadeRestitution -= 0.10; }  //okay maybe twice
            if (nadeRestitution < 0.10) nadeRestitution = 0.10;   // but not thrice
            if (doExplosionOnTouch)
            {
                queueExplosion = true;
            }
            bouncesInFrame = 1;
        }

        if (Obj != NullObj && !isStatic && attemptedBounces < 1000 && failCount < 3)
        {
            attemptedBounces++;
            if ((Obj->ClType == OC_Worm || Obj->ClType == OC_OilDrum || Obj->ClType == OC_Crate || Obj->ClType == OC_Collideable) && (!isStatic || failCount < 3))
            {
                ignoreCol = PerformObjectBounce(Obj);
                if (reflect->Hit == false) failCount++;
                if (failCount <= 2)    //if it fails twice it will fail 1000 times lol
                {
                    bouncing = true;
                    bounceSound(1, 0);
                }
            }
            else if (Obj->ClType == OC_Landscape && (!isStatic || failCount < 3))
            {
                bool didBounce = PerformLandscapeBounce(true);
                attemptedBounces++;
                if (reflect->Hit == false) failCount++;
                if (failCount <= 2)  
                {
                    bouncing = true;
                    bounceSound(1, 0);
                }
            }
        }
    }
     // WriteToChat4(6,(itoa(bounceCounter))," bounces, " ,itoa(attemptedBounces), " attempted ones." ,false);
    // WriteToChat5(4, classtoString(Obj->ClType), " was the class, fail: ", StrCon(itoa(failCount), " times failed, that it hit it was "), BtoA(reflect->Hit), ftoa(slowframecount), false) ;  
   // WriteToChat2( 5, "Attempted Bounces: ",itoa(attemptedBounces), false) ;
}

void CMissile::bounceSound(int reason, int sound)
{         
  if ((reason == 2 && sound <= 0) || (reason == 1 && nadeSound <= 0)) return;
  pan=CalculateSoundPan(PosX,PosY);   //bugged             
  vol=CalculateSoundVolume(PosX,PosY);  
   
  if (reason == 1)   //if reason is 1, play sound loaded in class   
  PlayLocalSound(nadeSound, 3.0, vol, 1.0);
  if (reason == 2)       //if reason is 2, play sound from func  
  PlayLocalSound(sound, 3.0, vol, 1.0);          
} 
                                
void CMissile::DoBounceExplosion()
{
     //Trigger explosion exactly once, you know why.
     //Your modifiers here, do it however you want

     //explosionDmg =  explosionDmg - timesExploded * 3 ;                if (explosionDmg < 7) explosionDmg = 7;
    
     //nadePushPower= nadePushPower - timesExploded * 2.0 ;   //example  if (nadePushPower<15) nadePushPower = 15;
      
     if (launchdata.explosion.dmgVar>0)
     explosionDmg+=RandomInt(-launchdata.explosion.dmgVar, launchdata.explosion.dmgVar);
   
     DoExplosion( PosX, PosY, nadePushPower, explosionDmg, 5, nadeTeam);
     timesExploded++;
}

  ////////////////////////////////////////////////////////////////////    
 //////////////////////////////MESSAGE///////////////////////////////             
////////////////////////////////////////////////////////////////////        
override void CMissile::Message(CObject* sender, EMType Type, int MSize, CMessageData* MData)
{
    if (!NewBounce)
    {                                         
     super;
     return;
    }
    
    if (Type == M_FRAME)   
    {      
        //float clpX; float clpY;
        ///local clipping = CheckSpawnPoint(PosX,PosY,safeAirPosX, safeAirPosY, ColMask, 2, CMASK_TERRAIN, &clpX, &clpY, 5);
        //if (clipping){
        //PosX = clpX; PosY = clpY; }
        
        if (FreeMe){isStatic = true; Dead = true; Free(true); return;}
        //if (ignoreCol!=NullObj)GG->WriteToChat( 7, classtoString(ignoreCol->ClType), false);
        if ( CheckMaskAt(this, ColMask, int(PosX), int(PosY), -1) == NullObj )
        {
              safeAirPosX = PosX;
              safeAirPosY = PosY;
        }
        
        //if (gframe % 5 == 0) WriteToChat3(8,itoa(bounceCounter)," C <- bounces -> M", itoa(bounceLimit), false);
        if (IsStatic) Dead = true;   //If the game gets mad at us, dont call funcs
        if (bounceCounter >= bounceLimit) //The game's counter is bad
        {      
          isStatic = true;
          Dead = true;
          ExplodeAt(PosX,PosY);
          Free(true);
          return;
        }  
        if (bounceCounter>MaxAllowedBounces) { isStatic = true;  } //stop looping
        //Cap for positive restitution. Speed too high breaks the game.  
        if (SpX >  25.0) SpX =  25.0;
        if (SpX < -25.0) SpX = -25.0;
        if (SpY >  25.0) SpY =  25.0;
        if (SpY < -25.0) SpY = -25.0;
        // snapshots
        snapBounces = bounceCounter; snapBounces1 = snapBounces;  
        if (gframe % 2 == 0) 
       {RegPosX1 = PosX;    RegPosY1 = PosY; }
        RegPSpX1 = RegPSpX; RegPSpY1 = RegPSpY;
        RegPSpX  = SpX;     RegPSpY  = SpY; 
    }       
                              
    if (Type == M_EXPLOSION || Type == M_GUNEXP)  
    {
       detectExplosion = true; 
    }     
                         
    super;
    
    if (Type == M_FRAME)
    {   
        
        if (staticFrames>0) 
        {
          isStatic = true;
          staticFrames--;
          slowframecount = 3.9;  //lazy to check the other code so i add this as failsafe
        }
        local dx = PosX - RegPosX1;
        local dy = PosY - RegPosY1;           
        local moved = sqrt(dx*dx + dy*dy);   //Movement difference between frames

        local spd = sqrt(SpX*SpX + SpY*SpY); //Speed
        if (moved > 6.9)
        {
           if (slowframecount>=1.0) slowframecount-=1.0; 
        }
        else
        {
           if (!isStatic) slowframecount+=1.0;   //If mis moved low, go toward static
        }
        
        if (slowframecount >=65.0) 
        {
           slowframecount = 4.0;      
           isStatic = true;    
            //WriteToChat2(9,"Quiet ",ftoa(moved),false);
        }
        if (slowframecount < 1.0 && isStatic && bounceCounter<MaxAllowedBounces)
        { 
          isStatic = false;                
            //WriteToChat2(9,"Loud ",ftoa(moved),false);
        }
                
        if (queueExplosion && bncExplosionCooldown == 0)
        {
           DoBounceExplosion();
           queueExplosion = false;
           bncExplosionCooldown = 3;
           bounceCounter+=1;
        }   
        
        if (bncExplosionCooldown > 0) bncExplosionCooldown--;
        
        if (bouncing) bouncing = false;     //For ur code in M_FRAME override before super
        
        if (bouncesInFrame == 1)
          bouncesInFrame = 0; 
       
        if (failCount>2) 
          failCount =0;
        
        /*if ((bounceCounter - snapBounces) > 1)    //Only increment 1 bounce per frame
        bounceCounter = snapBounces + 1;
        if ((bounceCounter - snapBounces1) > 2 )
        bounceCounter = snapBounces1 + 2;     */
    }
    if (Type == M_FRAME && !Dead)
    {
          if (isStatic)
          {
               bouncing = false;
               int staticX; int staticY;
               // wake up
               if (detectExplosion)
               {                        
                   local terrainbelow = TraceLine(this, PosX, PosY,PosX, PosY + 7, CMASK_TERRAIN, &staticX, &staticY) != NullObj;
                   if (!terrainbelow)
                       {
                           isStatic = false;   
                           lowSpdCount = int(lowSpdCount *0.2) ;  
                           slowframecount = int(slowframecount * 0.2) + 0.0 ;
                       }
                   detectExplosion = false;
               }    
               return;       
          }
          if (detectExplosion) detectExplosion = false; 
    }
}

CGObject* CMissile::PerformObjectBounce(CGObject* obj)
{
    if (Dead || obj == NullObj || attemptedBounces > 1000) return;
    local fail = false;
    if (obj == ignoreCol) {fail = true; failCount++;} 
    if (failCount>2){ return obj; }
    
    local dx = (PosX - obj->PosX) + 0.0;
    local dy = (PosY - obj->PosY) + 0.0;
    local nx = 0.0;
    local ny = 0.0;
    local hw = 5.0;
    local hh = 5.0;         
    local ax = 0.0;
    local ay = 0.0;          
    if ((obj->ClType == OC_Worm || obj->ClType == OC_Mine || obj->ClType == OC_OldWorm) && obj->ObjState!=WS_FROZEN)
    {                                                                                                //its a square :)
        // Circle, normal points from object center outward to missile
        local dist = sqrt(dx*dx + dy*dy);
        if (dist < 0.001) { nx = 0.0; ny = -1.0; }
        else { nx = dx / dist; ny = dy / dist; }
    }
    else
    {
        // Rectangle, find which face was hit by comparing normalised penetration depth
        hw = 9.0;
        hh = 9.0;          
        if (obj->ClType == OC_Collideable) { hw = 10.0; hh = 10.0;  }
        if (obj->ClType == OC_OilDrum) hh = 12.0;
        
        ax = dx; if (ax < 0.0) ax = 0.0 - ax;
        ay = dy; if (ay < 0.0) ay = 0.0 - ay;

        if (ax / hw > ay / hh)
        {
            if (dx > 0.0) nx = 1.0; else nx = -1.0;
            ny = 0.0;
        }
        else
        {
            nx = 0.0;
            if (dy > 0.0) ny = 1.0; else ny = -1.0;
        }
    }
    local ratio_diff = (ax / hw) - (ay / hh);

    if (ratio_diff > 0.15)                                                    
    {
      if (dx > 0.0) nx = 1.0; else nx = -1.0;
      ny = 0.0;
    }
    else if (ratio_diff < -0.15)
    {
      nx = 0.0;
      if (dy > 0.0) ny = 1.0; else ny = -1.0;
    }
    else  // corner, velocity direction decides
    {
      local absVx = absfloat(RegPSpX1);
      local absVy = absfloat(RegPSpY1);
      if (absVx > absVy)
      {
          if (dx > 0.0) nx = 1.0; else nx = -1.0;
          ny = 0.0;
      }
      else
      {
          nx = 0.0;
          if (dy > 0.0) ny = 1.0; else ny = -1.0;
      }
    }
    if (nx == pastNx2 && ny == pastNy2)
    {
     local goToX = 0.0;   local goToY = 0.0;                                                                          
     local checkSP = CheckSpawnPointNoLOS(PosX, PosY, ColMask, 1, CMASK_TERRAIN | ObjToCMask(obj) , &goToX, &goToY, 4);
     if (checkSP == true)
     {
      PosX = goToX;
      PosY = goToY;  
     }
    }
    // Same reflection math as calculateReflectionOnSurface
    local vx = RegPSpX1 + 0.0;
    local vy = RegPSpY1 + 0.0;

    local dot = vx*nx + vy*ny;
    if (dot > 0.0) { nx = 0.0-nx; ny = 0.0-ny; dot = 0.0-dot; }

    local vn    = dot;
    local vt_x  = vx - vn * nx;
    local vt_y  = vy - vn * ny;
    local vn_out = (0.0 - vn) * nadeRestitution;

    vt_x = vt_x * (1.0 - nadeFriction);
    vt_y = vt_y * (1.0 - nadeFriction);

    local newSpX = vt_x + vn_out * nx;
    local newSpY = vt_y + vn_out * ny;
    
    local newSpd = sqrt(newSpX*newSpX + newSpY*newSpY);
    if (newSpd < 0.01) 
    {
      isStatic = true;
      SpX = 0.0;        
      SpY = 0.0;
      reflect->Hit = false;
      return obj;
    }  
    
    local cw = CWorm(NullObj);
    cw = GetCurrentWorm();
    local hitWorm = obj is CWorm || obj->ClType == OC_Worm ;
    
    if (newSpd <= 3.1 && !hitWorm && lowSpdCount <= 12)
    {
        lowSpdCount++;
    }
    else if (newSpd <= 3.1 && !hitWorm && lowSpdCount >= 12)
    {  
        isStatic = true; 
        SpX = SpX * 0.2;
        SpY = SpY * 0.2;
        lowSpdCount = 0;   
        slowframecount = 3.58;
        return obj;
    }
   if (!doExplosionOnTouch)
   {
    if (hitWorm && nadeRestitution<0.9 && obj!=cw && obj->ObjState!=WS_FROZEN)
    {
    // 1. Get the absolute values of our INCOMING velocity
    local absSpX = absfloat(RegPSpX1);
    local absSpY = absfloat(RegPSpY1);
    
    // 2. Evaluate the Normal (nx, ny) to know WHERE the worm was hit
    local isTopHit = (ny < -0.6);
    local isBottomHit = (ny > 0.6);
    
    // Check if the trajectory is "steep" (Vertical momentum is dominant)
    // trigger the dead-stop
    local isSteep = (absSpY > (absSpX * 0.78)); 
    local isSlow = (sqrt(RegPSpX1*RegPSpX1 + RegPSpY1*RegPSpY1)) < 4.2;     
    local isSlow2 = (sqrt(RegPSpX1*RegPSpX1 + RegPSpY1*RegPSpY1)) < 8;
    if ((isTopHit || isBottomHit) && isSteep && isSlow2)
    {   
        newSpX = 0.0; // Completely kill horizontal sliding
    }
    else
    {
        // It hit the side (chest/back), or came in at a shallow horizontal angle.
        // The less steep it is, the more it actually bounces.
        
        local verticality = absfloat(ny); // 0.0 (pure side hit) to 1.0 (pure top/bottom hit)
        
        // If it hits the exact side (ny = 0), it keeps 100% of its bounce.
        // If it hits a diagonal corner (ny = 0.5), it loses 25% of its speed.
        newSpX = newSpX * (1.0 - (verticality * 0.5));
        newSpY = newSpY * (1.0 - (verticality * 0.5));
        
        // Threshold check to completely stop horizontal micro-movements
        if (absfloat(newSpX) < 1.4) 
        {
            newSpX = 0.0;
        }
        if (abs(dx) < 10 && dx>0 && RegPSpY1 < 2.2)
        {
        PosX = PosX - 3.5;    //make sure its above its head  
        PosY-=2.8;   
        newSpY-=2.3;
        }
        else if (abs(dx) < 10 && dx<0 && RegPSpY1 < 2.2)   
        {
        PosX = PosX + 3.5;
        PosY-=3.3; 
        newSpY-=2.3;
        }
    }
    if (isSlow) newSpX = 0.0;
    
    if (newSpX == 0.0) staticFrames  = 30;
    }
   }           
    SpX = newSpX;
    SpY = newSpY;      
    if (pastNx == nx) pastNx2=pastNx;   
    if (pastNy == ny) pastNy2=pastNy;
    pastNx = nx;
    pastNy = ny;
    reflect->PreHitX = newSpX;
    reflect->PreHitY = newSpY;
    reflect->NormalX = nx;
    reflect->NormalY = ny;
    reflect->Hit = true;
                             
    slowframecount -= 0.5;
    attemptedBounces--;
    if (!fail && failCount>0)failCount -= 1;
    bouncing = true; 
    return obj;
}

void CMissile::PerformLandscapeBounce(bool success)
{
    if (Dead || reflect == NullObj || attemptedBounces > 600) return;

    local hit = calculateReflectionOnSurface(this, PosX, PosY, safeAirPosX, safeAirPosY, RegPSpX1, RegPSpY1, CMASK_TERRAIN, nadeRestitution, nadeFriction, reflect);

    if (pastNx == reflect->NormalX && pastNy == reflect->NormalY)
    {
        pastNx2 = pastNx;
        pastNy2 = pastNy;
    }

    local MayBeSolid = false;
    if (reflect->NormalX == pastNx2 && reflect->NormalY == pastNy2)
    {
        MayBeSolid = true;
        hit = calculateReflectionOnSurface(this, PosX-0.1, PosY-0.1, safeAirPosX, safeAirPosY, RegPSpX1, RegPSpY1, CMASK_TERRAIN, nadeRestitution-0.04, nadeFriction+0.01, reflect);
    }

    pastNx = reflect->NormalX;
    pastNy = reflect->NormalY;

    if (hit == false) return;
    local newSpd = sqrt(reflect->PreHitX * reflect->PreHitX + reflect->PreHitY * reflect->PreHitY);
    if (newSpd <= 3.1 && lowSpdCount < 10)
    {
        lowSpdCount++;
    }
    else if (newSpd <= 3.1 && lowSpdCount >= 10) 
    {          
        lowSpdCount++;
        isStatic = true; 
        SpX = SpX * 0.2;
        SpY = SpY * 0.2;
        lowSpdCount = 0;   
        slowframecount = 3.0;
        return;
    }
    if (lowSpdCount > 0) lowSpdCount--;

   // PosX = reflect->HitX;  //Optional, wonky    
   // PosY = reflect->HitY;

    SpX = reflect->PreHitX;  // Speed
    SpY = reflect->PreHitY;
    if (success)
    {                       
       slowframecount = 2.0;
       attemptedBounces--;
       failCount = 0;
    }  
   // if (reflect->HitObject!=NullObj) ignoreObj = reflect->HitObject;
    
    if (reflect->StartsSolid || MayBeSolid)
    {
        float goToX; float goToY;
        
        local checkSP = false;//CheckSpawnPointNoLOS(PosX, PosY, ColMask, 2, CMASK_DEFAULT_COLLIDEABLE, &goToX, &goToY, 6);
        if (checkSP == true)
        {
            PosX = goToX;
            PosY = goToY;
        }
    }
}  

CMissile *DoWeapBouncy(CWeapon* SWeap, CShootDesc* WPDesc, CGObject* parent, int flagss, float radius, float restitution, float friction, int maxBounces,int sound, bool exp, string wpname)
{                                                                                                                                                                         
     if ( SWeap == NullObj || parent == NullObj )  return NullObj;
     local EXPDMG = SWeap->launch.explosion.damage;         
     SWeap->launch.explosion.damage += 6767;
     
     local naded = new CMissile(Root->GetObject(25, 0), &SWeap->launch, WPDesc) ;      
     
     if (naded!=NullObj)
     {
        naded->ActivateBouncePhysics(Root->GetObject(25, 0), &SWeap->launch, WPDesc, flagss, radius, restitution, friction, maxBounces, sound, exp); 
        naded->WeaponName = wpname;  
        naded->ColMask = new CColMask(radius,radius,MakeCircleMask(radius));
        SWeap->launch.explosion.damage = EXPDMG; 
        
        float safeX; float safeY;
        local spawnOK = CheckSpawnPoint(parent->PosX, parent->PosY, WPDesc->X, WPDesc->Y, naded->ColMask, 1, CMASK_TERRAIN, &safeX, &safeY, 15);
        if (spawnOK)
        {
                naded->PosX = safeX;
                naded->PosY = safeY;
        }
        else if (parent!=NullObj && !spawnOK) 
        {
           if (parent is CWorm)
           {
              CWorm *worm = parent;
              naded->FreeMe = true;
              worm->nAvalShoots = 1;
              worm->nTotalShoots = 0;    
              return NullObj;
           }        
        } 
        naded->NewBounce = true;    
        return naded;
     }   
     return NullObj;                                                  
}                                      

//////////////////////////TEST///////////////////////////

override void CWorm::FireFinal(CWeapon* Weap,CShootDesc* Desc)
{
 if (Weap->CheckName("RealNade")) //Your Weapon Name (case sensitive)
 {                                                                     //Restituton = Bounciness, 1.0 returns the full power of the bounce.
    local realnade = DoWeapBouncy(Weap, Desc, this, CMASK_DEFAULT_COLLIDEABLE, 12, 0.6, 0.02, 0, 113, false, "RealNade"); 
                                                                   //Flags, 0 is default, Nade Size(radius), Restitution, Friction, Max Bounces, sound (0 is unlimited)                                                                  //Flags are used in tracer only, radius is used for SpawnPoint offset, leave at 15/20 if u arent using that.
    return;                                                            
 }                                                                
 super;
}                            