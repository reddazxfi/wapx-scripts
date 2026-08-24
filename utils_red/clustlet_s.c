require utils;
CWeapon clustlets;
                                                                     
void clustlets_s::FirstFrame()
{
    zero(&clustlets);   // Reset heap
 
    clustlets.NameA=16;  
    clustlets.NameB=16;
    clustlets.Row = 12;
    clustlets.Remember = true;
    clustlets.UseInCavern = false;
    clustlets.NShoots = 1;
    clustlets.EndsTurn = true;
    clustlets.RetreatTime = 3000;
    clustlets.IsWeaponCrate = true;
    clustlets.Probablity = 0;
    clustlets.Count = 1;
    clustlets.GlobalIndex = 81; ///out of the 80 index bounds
    clustlets.ActivationType =  AT_Crosshair;
    clustlets.Graphic = 6;          
    clustlets.WeaponType =  WT_Launcher;

    ////////////////////////ONLY THIS MATTERS////////////////////////
    clustlets.launch.spriteSize = 2;
    clustlets.launch.launchSpeed = 0; 
    clustlets.launch.runAway = true;
    
    clustlets.launch.explosion.damage = 20;
    clustlets.launch.explosion.bias = 0;
    clustlets.launch.explosion.pushPower = 100;   
    clustlets.launch.explosion.flags = 137342;
    
    clustlets.launch.unk = 1;
    clustlets.launch.anim.spriteIndex = 54; 
    clustlets.launch.anim.type = WAT_Rotation;    
    clustlets.launch.anim.trailIndex = 131;   
    clustlets.launch.anim.trailPower = 20;
    clustlets.launch.anim.trailSpeed = 100;
    clustlets.launch.anim.unk = 50;   
    clustlets.launch.speedMultipler = 100;
    clustlets.launch.windFactor = 0;
    clustlets.launch.gravityFactor = 100;  
    clustlets.launch.countdownFrom = 0;
    clustlets.launch.timeBeforeExplosion = 10000;      
    clustlets.launch.sound.soundIndex = 0;
    clustlets.launch.sound.useExpSound = false;
    clustlets.launch.explodeOnSpace = false;
    clustlets.launch.action = WAction_None;    
    clustlets.launch.explodeInto = WExplode_Nothing;    
    ////////////////////////ONLY THIS MATTERS////////////////////////    
}; 

CWeaponLaunch * getClustletsData()
{ //Modifying pointers also modify the main weapon, gotta reset variables here.
    clustlets.launch.spriteSize = 2;
    clustlets.launch.launchSpeed = 0; 
    clustlets.launch.runAway = true;
    
    clustlets.launch.explosion.damage = 20;
    clustlets.launch.explosion.bias = 0;
    clustlets.launch.explosion.pushPower = 100;   
    clustlets.launch.explosion.flags = 137342;
    
    clustlets.launch.unk = 1;
    clustlets.launch.anim.spriteIndex = 54; 
    clustlets.launch.anim.type = WAT_Rotation;    
    clustlets.launch.anim.trailIndex = 131;   
    clustlets.launch.anim.trailPower = 20;
    clustlets.launch.anim.trailSpeed = 100;
    clustlets.launch.anim.unk = 50;   
    clustlets.launch.speedMultipler = 100;
    clustlets.launch.windFactor = 0;
    clustlets.launch.gravityFactor = 100;  
    clustlets.launch.countdownFrom = 0;
    clustlets.launch.timeBeforeExplosion = 10000;      
    clustlets.launch.sound.soundIndex = 0;
    clustlets.launch.sound.useExpSound = false;
    clustlets.launch.explodeOnSpace = false;
    clustlets.launch.action = WAction_None;    
    clustlets.launch.explodeInto = WExplode_Nothing;   
    
    local verylocallaunchdatadata = &clustlets.launch;
    return verylocallaunchdatadata; 
}

//ShootMoreMissiles moved to p_sprite_builder because of global scope compilation problems
/*void ShootMoreMissiles
{


}
*/

void makeClustlets(float baseAngleDeg, float spreadAngleDeg, bool randomize, float speed, float x, float y, int quantity, int team, int worm)
{   
    local cldata = getClustletsData();
    Missindx = gframe;
    missindx = gframe;   
    local clustName = "Clustlets";
    
    if (cldata != NullObj && quantity > 0)
    {    
        int middle = int(quantity * 0.51);     
        if (middle == 0) middle = 1;
                 
        local startAngle = baseAngleDeg - (spreadAngleDeg / 2.0);
        
        local angleStep = 0.0;
        if (quantity > 1) 
        {
            angleStep = spreadAngleDeg / (quantity - 1.0);
        }

        for (int i = 0; i < quantity; i++)
        { 
            CShootDesc CDesc;
            zero(&CDesc);
            
            local currentDeg = startAngle + (i * angleStep);
            
            local currentRad = currentDeg * (MATH_PI / 180.0);
            
            CDesc.SpX = speed * cos(currentRad);
            CDesc.SpY = speed * sin(currentRad);
            if (i!=middle && randomize)
            {
               CDesc.SpX += RandomFloat(-1.8, 1.8);                   
               CDesc.SpY += RandomFloat(-1.7, 1.7);
            }
            else if (i == middle && randomize)
            {
               CDesc.SpX += RandomFloat(-0.3, 0.3);                   
               CDesc.SpY += RandomFloat(-1.7, 1.7);
            }
            CDesc.X = x;
            CDesc.Y = y;
            CDesc.Team = team;
            CDesc.Worm = worm;
            
            local clst = new CMissile(Root->GetObject(25, 0), cldata, &CDesc);
            if (clst!=NullObj) 
            {      
                missindx = Missindx + i ;
                clst->misIndex = missindx;       
                clst->WeaponName = clustName;  
                registeredMisIndex = Missindx;
            };
        };
    };  
                return Missindx; 
};
                                                                                                                                                            //just do NullObj for normal clustlets
int shootMoreMissilesCustomSprites(CGObject * sender,float FireanglE, int TurnsidE, float spreadAngleDeg, bool randomize, float speed, float x, float y, int quantity, int team, int worm, CWeaponLaunch *adcldata, int sprite1, int sprite2, int sprite3, int sprite4, int sprite5, int sprite6)
{   
    int Missindx = gframe; // if it lags it'll return ok
    int missindx = 0;     
            
    local clustName = "Clustlets";
    
    CWeaponLaunch* cldata;
    if (adcldata!=NullObj) cldata = adcldata;
    else cldata = getClustletsData();
    
    local baseAngleDeg = -FireAngleToDegrees(FireanglE, TurnsidE);
    
    if (cldata != NullObj && quantity > 0)
    {    
        int middle = int(quantity * 0.51);     
        if (middle == 0) middle = 1;
                 
        local startAngle = baseAngleDeg - (spreadAngleDeg / 2.0);
        
        local angleStep = 0.0;
        if (quantity > 1) 
        {
            angleStep = spreadAngleDeg / (quantity - 1.0);
        }

        for (int i = 0; i < quantity; i++)
        { 
            if (i == 0) {cldata->anim.spriteIndex = sprite1 ;}   
            if (i == 1) {cldata->anim.spriteIndex = sprite2 ;}
            if (i == 2) {cldata->anim.spriteIndex = sprite3 ;}
            if (i == 3) {cldata->anim.spriteIndex = sprite4 ;}
            if (i == 4) {cldata->anim.spriteIndex = sprite5 ;}  
            
            CShootDesc CDesc;                                    
            zero(&CDesc);   
            
            local currentDeg = startAngle + (i * angleStep);
            
            local currentRad = currentDeg * (MATH_PI / 180.0);
            
            CDesc.SpX = speed * cos(currentRad);
            CDesc.SpY = speed * sin(currentRad);
            if (i!=middle && randomize)
            {
               CDesc.SpX += RandomFloat(-1.8, 1.8);                   
               CDesc.SpY += RandomFloat(-1.7, 1.7);
            }
            else if (i == middle && randomize)
            {
               CDesc.SpX += RandomFloat(-0.3, 0.3);                   
               CDesc.SpY += RandomFloat(-1.7, 1.7);
            }
            CDesc.X = x;
            CDesc.Y = y;
            CDesc.Team = team;
            CDesc.Worm = worm;
            local clst = new CMissile(Root->GetObject(25, 0), cldata, &CDesc);
            if (clst!=NullObj)
            {
                if (sender->ClType == OC_Worm) {CWorm * Wsender = CWorm(sender); Wsender->childRange++; } 
                else if (sender is CMissile == true){ CMissile * Msender = CMissile(sender); Msender->childRange++;  }
                missindx = Missindx + i ;
                clst->misIndex = missindx;       
                clst->WeaponName = clustName;  
                registeredMisIndex = Missindx;
            };
        };
    };  
                if (sender->ClType == OC_Worm) {CWorm * Wsender = CWorm(sender); Wsender->childIndex = Missindx; } 
                else if (sender is CMissile == true){ CMissile * Msender = CMissile(sender); Msender->childIndex = Missindx;  }
                return Missindx; 
};

void CMissile::ApplyClustletHome(float clickX, float clickY, int delay, float speed, float turnsharpness, float homeAmount)
{
    for (local i = 0; i < Env->Objs.Count; i++) 
    {        
        CObject* obj = Env->Objs.Objs[i];  
        if (obj == NullObj) continue;

        if (obj->ClType == OC_Missile) 
        {
            CMissile* mis = CMissile(obj); 
            
            // Check if this missile belongs to the spawned batch
            if (mis->misIndex >= childIndex && mis->misIndex <= childIndex + childRange)
            {
                mis->HomeAfter(clickX, clickY, delay, speed, turnsharpness, homeAmount); 
            }
        }
    }
}

void CMissile::ApplyClustletExplosionEffect(float thickness, int radius, int Rgb, bool circle, bool ellipse, float vanish)
{
    for (local i = 0; i < Env->Objs.Count; i++) 
    {        
        CObject* obj = Env->Objs.Objs[i];  
        if (obj == NullObj) continue;

        if (obj->ClType == OC_Missile) 
        {
            CMissile* mis = CMissile(obj); 
            
            // Check if this missile belongs to the spawned batch
            if (mis->misIndex >= childIndex && mis->misIndex <= childIndex + childRange)
            {
                mis->expThicc = thickness;  mis->cusExpEff = true; mis->expR = radius; mis->expRGB = Rgb; mis->expCircle = circle; mis->expEllipse = ellipse ;
            }
        }
    }
}   

void CMissile::ApplyExplosionClustlet(int expFlags1, int expDmg1, int expPush1, int expDestroyR1, bool expShouldDestroy1, bool expParticles1, bool expSound1, bool expTaze1)
{
    for (local i = 0; i < Env->Objs.Count; i++) 
    {        
        CObject* obj = Env->Objs.Objs[i];  
        if (obj == NullObj) continue;

        if (obj->ClType == OC_Missile) 
        {
            CMissile* mis = CMissile(obj); 
            
            // Check if this missile belongs to the spawned batch
            if (mis->misIndex >= childIndex && mis->misIndex <= childIndex + childRange)
            {
                mis->applyCustomExplosionParams(expFlags1, expDmg1, expPush1, expDestroyR1, expShouldDestroy1, expParticles1, expSound1, expTaze1);
            
            }
        }
    }
}


/* ////////////////////////TEST////////////////////////
override void CWorm::FireFinal(CWeapon* Weap,CShootDesc* Desc)
{
  if (Weap->CheckName( "Holy Hand-Grenade" ) == true )
  {
    local spd = 0.75 * (sqrt(Desc->SpX * Desc->SpX + Desc->SpY * Desc->SpY)) ;
    shootMoreMissiles(FireAngle,TurnSide, 20, false, spd, PosX, PosY, 4, WormTeam, WormNumber, &Weap->launch, 0, NullObj);
  }
  else super;
}  
*/