//Homing, name return, weap cmissile check, exp check.
CWeapon * gu_cweap; 

void ApplyConstantSpeed(CGObject *sender, float speedcap)
{       
    local spdd  = sqrt(sender->SpX * sender->SpX + sender->SpY * sender->SpY);
    if (spdd > 0.001)
    {
        sender->SpX = (sender->SpX / spdd) * speedcap;
        sender->SpY = (sender->SpY / spdd) * speedcap;
    }
}

override void CMissile::CMissile(CObject* parent,CWeaponLaunch* ldata,CShootDesc* sdata)
{ 
  // Whether or not the missile explodes on impact.
  // Check the truth table before bothering me =)
  // gframe based missileIndex (clustlets, etc)
  misIndex = gframe + 1;  
  // From string fucker  
  backup_weap = CWeapon(NullObj);
  
  homeTargX = 0.0;
  homeTargY = 0.0;
  homeMCD = 2;
  defaultHomeCooldown = 1;
  homeMArc = 0.25;
  homeMSpeed = 10;
  homeMAmount = gframe + 4000;
  
  childIndex = 0;
  childRange = 0;
 
  super;  
  
  WeaponName = NullString;  
  
  ExpOnImpact = MissileExplodesOnImpact(&this->launchdata);
  CWeaponLaunch* ldata2 = new CWeaponLaunch; 
 
  customExplosion = false;
  expDmg = ldata->explosion.damage;
  expDestroyR = expDmg;
  expShouldDestroy = true;
  expPush = ldata->explosion.pushPower;
  expFlags = -1;
  expParticles = true;
  expSound = true;   
  expSoundNum = 0;
  expTaze = false;
  
  isMisElectric = false;
  nocrashss = 0;
  
  cusExpEff = false;
  expR = 0.0;  expRGB = RGB(255,255,255);
  expCircle = true;
  expEllipse = true;
  expVanish = 1.0;
  expThicc = 2.0;
  expNoise = 0.01;         
  
 SAWTEAM = sdata->Team;
 hitFrame = 5;
 sawhitlimit = 0;
  
 if (weap == NullObj && gu_cweap != NullObj)
 {
   // This works better than utils' solution somehow   
   // I Suppose that not resetting the variable in FireFinal is better.     
   backup_weap = gu_cweap;
   gu_cweap = CWeapon(NullObj); 
 };    
}     

override void CWorm::FireFinal(CWeapon* Weap,CShootDesc* Desc)
{
// bool is_CMissile = Weap.WeaponType = false 
 if (Weap!=NullObj && IsClassMissileLauncher(Weap))
 {
   gu_cweap = Weap;
 };
 super;
};
                                                   // missileSpeed, up to 35. turnSharpness, up to 1.0 (overshoots after prolly, still cool ig)
void chaseTarget(CGObject* targtt, CGObject * sender, float missileSpeed, float turnSharpness)
{                 
      if (sender == NullObj) return;
      if (targtt == NullObj) return;           
      // DIRECT GUIDANCE LOGIC
      float missX = sender->PosX;
      float missY = sender->PosY;  
      // Get the direction vector pointing straight at the target.
      float dirX = targtt->PosX - missX;
      float dirY = targtt->PosY - missY; 
                  
      ArrivePointX = targtt->PosX;
      ArrivePointY = targtt->PosY;

      //  Normalize the vector to get a pure direction (length of 1).
      float distanceToTarget = sqrt(dirX * dirX + dirY * dirY);
      if (distanceToTarget > 0) // Avoid division by zero
      {
          dirX = dirX / distanceToTarget;
          dirY = dirY / distanceToTarget;
      }
      //float missileSpeed = missileSpeed;   // How fast the missile tries to fly.
      //float homingStrength = turnSharpness; // How sharply it can turn (0.0 to 1.0)

      //Calculate the ideal velocity (direction * speed).
      float requiredSpX = dirX * missileSpeed;
      float requiredSpY = dirY * missileSpeed;

      // Gently steer the current velocity towards the ideal velocity.
      // This prevents the zig-zagging and creates a smooth turn.
      sender->SpX += (requiredSpX - sender->SpX) * turnSharpness;
      sender->SpY += (requiredSpY - sender->SpY) * turnSharpness;
                
      //Manage your cooldown in your own M_FRAME
};
                 
void homeToPlace(float PointX, float PointY, CGObject * sender, float missileSpeed, float turnSharpness) 
{               //more reliable because of no target pointer, call with (yourTarget->PosX, yourTarget->PosY)
                //Also useable for custom CObject classes (like PXParticle) (as long as its SpX and not spx)
                if (sender == NullObj) return;
                float missX = sender->PosX;                                                                    
                float missY = sender->PosY;  
                float dirX = PointX - missX;
                float dirY = PointY - missY;

                float distanceToTarget = sqrt(dirX * dirX + dirY * dirY);
                if (distanceToTarget > 0) // Avoid division by zero
                {
                    dirX = dirX / distanceToTarget;
                    dirY = dirY / distanceToTarget;
                }
                //float missileSpeed = RandomFloat(1.15, 2.35);   // How fast the missile tries to fly.
                //float homingStrength = turnSharpness; // How sharply it can turn (0.0 to 1.0)
                float requiredSpX = dirX * missileSpeed;
                float requiredSpY = dirY * missileSpeed;
                sender->SpX += (requiredSpX - sender->SpX) * turnSharpness;
                sender->SpY += (requiredSpY - sender->SpY) * turnSharpness;
                
               //Manage your cooldown in your own M_FRAME
};

void CMissile::HomeAfter(float homeX, float homeY, int frames, float speed, float arcsharpness, int homeAmount, int homeSound)
{
     shouldHomeCustom = true;
     homeTargX = homeX;    
     homeTargY = homeY;
     HomeMCD = frames;
     homeMArc = arcsharpness;
     homeMSpeed = speed;
     if (homeAmount != 0) homeMAmount = gframe + homeAmount;
     homeMSound = homeSound;
}

void CMissile::doEffectExp(fixed x, fixed y)
{
    // Scale duration UP with damage.
    int scaledDuration = 10 + int(launchdata.explosion.damage * 0.125);
    
    outVanish = expVanish * 0.25;
    
    int r; int g; int b;   
    stripRGB(expRGB, &r, &g, &b);
    
    expRadius = launchdata.explosion.damage * 0.80;
    if (expR!=0.0)  { expRadius = expR; }
    
    CEffectManager* wowExplosion = createEffectExplosion(x, y, expThicc, launchdata.explosion.damage, expNoise, r, g, b, false, scaledDuration, expVanish, outVanish);
        wowExplosion->numSegs = int(launchdata.explosion.damage * 0.40) + 5;
        wowExplosion->shouldExplode = expCircle;
        wowExplosion->shouldEllipse = expEllipse;  
}

override void CMissile::DoExplosion(fixed x,fixed y,int PushPower,int Damage,int unkB,int Team)
{  
    if (!customExplosion) super;  
    if (cusExpEff)
    {
        doEffectExp(x,y);
    }  
    if (customExplosion) {  do_custom_explosion(this, expFlags, x, y, expDmg, expPush, expDestroyR, expShouldDestroy, expParticles, expSound, expTaze, expSoundNum); }        
} 

void CMissile::ApplyExplosionEffect(float thickness, int radius, int Rgb, bool circle, bool ellipse, float vanish)
{
   cusExpEff = true; 
   expR = radius; 
   expRGB = Rgb; 
   expCircle = circle; 
   expEllipse = ellipse ;
   expVanish = vanish;
}

void CMissile::ON_FRAME()
{  
 if (hitFrame<5) hitFrame++;
   if (gframe > homeMAmount) shouldHomeCustom = false;                                                                 
   if (shouldHomeCustom)
   {
      if (HomeMCD == 3) PlayLocalSound(homeMSound, 1.0, 1.0, 1.0);
      if (HomeMCD <=0 && gframe < homeMAmount) 
      {
         homeToPlace(homeTargX, homeTargY,this,homeMSpeed,homeMArc); HomeMCD = defaultHomeCooldown;      //CMissile::ApplyClustletHome(float clickX,float clickY,int delay,float speed,float turnsharpness,float homeAmount)
      }
      else if (HomeMCD > 0 && gframe < homeMAmount) { HomeMCD--;  }
   }  
} 

override void CMissile::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData){ super; if (Type == M_FRAME) {ON_FRAME();}}// override void CWorm::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData){ super;  if (Type == M_FRAME) ON_FRAME();} override void CMine::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData){  super; if (Type == M_FRAME) ON_FRAME();}

string CWeapon::GetName()
{   
    if (this == NullObj){ return " "; };    
    if (NameA < 400000 || NameA > 1900000000) {return ItoA(NameA);} 
    else { return  stringintP(NameA); };
};

string CMissile::GetName()
{                                                    
    if (weap == NullObj && backup_weap!=NullObj)
    {
       return backup_weap->GetName();
    }
    else if (weap == NullObj && backup_weap==NullObj)
    { 
       return " "; 
    };
    
    if (WeaponName != "Null" || WeaponName != NullString) return WeaponName;
     
    return weap->GetName();
};

string GetWeaponName(CWeapon* wep)
{
    if (wep == NullObj){ return " "; }
    else { return wep->GetName();  };
};//End;

bool IsClassMissileLauncher(CWeapon* Weap)
{
    if (Weap == NullObj) return false;
    //if (Weap->launch == NullObj) return false; //Weird but possible lol
    
    if (Weap->WeaponType == WT_Gun)    return false;
    if (Weap->WeaponType == WT_Bow)    return false;
    if (Weap->WeaponType == WT_Flamer) return false;
    if (Weap->WeaponType == WT_None)   return false;
    if (Weap->ActivationType == AT_None) return false;

    // CMissile launchers that use Airstrikes / Click / other
    if (Weap->CheckName("9/11"))          return true;

    // Check for garbage values
    
    if (Weap->launch.spriteSize < 1 || 
        Weap->launch.spriteSize > 15)  return false;

    else if (Weap->launch.launchSpeed < -10 ||
        Weap->launch.launchSpeed > 250)  return false;
        
    else if (Weap->launch.speedMultipler < -100 ||
        Weap->launch.speedMultipler > 170) return false;

    if (Weap->launch.anim == NullObj) return false;
    
    else
    {
    
    if (Weap->launch.anim.spriteIndex < 0 || 
        Weap->launch.anim.spriteIndex > 1024) return false;

    else if (Weap->launch.anim.trailIndex < 0 || 
        Weap->launch.anim.trailIndex > 1024) return false;

    else if (Weap->launch.gravityFactor < -100 || 
        Weap->launch.gravityFactor > 1400) return false;      
        
    else if (Weap->launch.windFactor < -50 ||
        Weap->launch.windFactor > 300) return false;
    }

    if  (Weap->launch.explosion == NullObj){ return false; }        
         
    else  
    {
    
    if (Weap->launch.explosion.damage < 0 || 
    Weap->launch.explosion.damage > 500) return false;   

    else if (Weap->launch.explosion.pushPower < 0 || 
        Weap->launch.explosion.pushPower > 1000) return false; 
        
    else if (Weap->launch.explosion.bias < -50 || 
        Weap->launch.explosion.bias > 400) return false;

    else if (Weap->launch.explosion.dmgVar < 0 || 
       
        Weap->launch.explosion.dmgVar > 200) return false;
    };
        
    return true; 
};

bool MissileExplodesOnImpact(CWeaponLaunch* ldata)
{
    if (ldata == NullObj) return false;
    
    // flag == 0 = nothing 
    // flag == 4194304 = only skimming
    // meaning prolly has collission flags in bounciness or we shouldnt mess with it anyways
    local flagsExplode = (ldata->explosion.flags == 0   || 
                          ldata->explosion.flags == 4194304);
    
    local actionBounces = (ldata->action == WAction_Dig || 
                           ldata->action == WAction_Roam);
    
    return (flagsExplode && actionBounces) == false;
};