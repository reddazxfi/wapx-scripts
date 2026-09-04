require utils, utils_steps;

////////////////////ACTUAL UTILS////////////////////
void WriteToChat2(int findex, string part1, string part2, bool unknwn)
{
   GG->WriteToChat(findex, StrCon(part1,part2), unknwn) ;
}; 
 
void WriteToChat3(int findex, string part1, string part2,string part3, bool unknwn)
{
   GG->WriteToChat(findex, StrCon3(part1,part2,part3), unknwn) ;
};  
void WriteToChat4(int findex, string part1, string part2,string part3,string part4, bool unknwn)
{
   GG->WriteToChat(findex, StrCon4(part1,part2,part3,part4), unknwn) ;
}; 
void WriteToChat5(int findex, string part1, string part2,string part3,string part4,string part5, bool unknwn)
{
   GG->WriteToChat(findex, StrCon5(part1,part2,part3,part4,part5), unknwn) ;
};

int WormAimSprNum(float FireanglE)  //0 Pointing down, 31 pointing up.
{
  return int((1.0 - FireanglE) * 31.0); 
};  

float WormAimSprIdx(float FireanglE)
{
  return subSprIndex(31, WormAimSprNum(FireanglE)); 
};

float absfloat(float n)
{
  if (n<0.0) n = -n;
  return n;
};

float maxfloat(float a, float b)       //if u need int just do int(maxfloat(a,b)) 
{
    if (a > b) 
    {
        return a;
    };
    return b;
}; 

//minfloat and more in ai_slop 

float max3float(float a, float b, float c) 
{
    return maxfloat(maxfloat(a, b), c);
}; 

float max4float(float a, float b, float c, float d) 
{
    return maxfloat(maxfloat(a, b),  maxfloat(c, d));
};

int chooseNum(int a, int b)
{
  if (RandomInt(1,2) == 1) 
       return a;
  else 
       return b;                   
};  

float chooseFloat(float a, float b)
{
  if (RandomInt(1,2) == 1) 
       return a;
  else 
       return b;
};  

float chooseBetweenFloat(float a, float b, float c, float d, float e, float f, int qnt)
{  
    if (qnt < 1 || qnt > 6) return a;
    
    local i = RandomInt(1, qnt); 
    
    if      (i == 1) return a;
    else if (i == 2) return b;
    else if (i == 3) return c;
    else if (i == 4) return d;
    else if (i == 5) return e;
    else             return f;
};

override void CTurnGame::Message(CObject* sender, EMType Type, int MSize, CMessageData* MData)
{
    super;
    if (Type == M_FRAME)
    {
        RainbowGlobalHue += RainbowSpeed;
        if (RainbowGlobalHue > 1.0) RainbowGlobalHue -= 1.0;
        if (gframe == 3) { SecondFrame(); };                       
    }
}

void CTurnGame::SecondFrame() {RainbowGlobalHue = 0.009; }

// Fast, smooth 3-phase sine rainbow (No black dips, continuous curves)
int FastSmoothRainbow(float phase)
{
    // Convert 0.0..1.0 range into radians (2 * PI)
    float rad = phase * 6.2831853; 

    // Center at 180, amplitude 75 -> range [105 .. 255]
    // Phase offsets: Red = 0, Green = 120 deg (2.094 rad), Blue = 240 deg (4.188 rad)
    int rr = int(180.0 + 75.0 * sin(rad));
    int gg = int(180.0 + 75.0 * sin(rad + 2.0943951));
    int bb = int(180.0 + 75.0 * sin(rad + 4.1887902));

    return RGB(rr, gg, bb);
}

int RRGB(float phaseOffset)
{
    return FastSmoothRainbow(RainbowGlobalHue + phaseOffset);
}

// FIX: Changed int offset to float offset
void SetRainbowMod(float offset, int blendType)
{
    SetColorMod(RRGB(offset), blendType);
}

void SetRainbowMod(int offset, int blendType)
{
   SetColorMod(RRGB(offset), blendType);
};

void r_g_b_Brightness(int amount, int *r, int *g, int *b)
{
     *r += amount;
     *g += amount;     
     *b += amount;
     if (*r <0) *r = 0;   
     if (*g <0) *g = 0;
     if (*b <0) *b = 0; 
     if (*r >255) *r = 255;   
     if (*g >255) *g = 255;
     if (*b >255) *b = 255;
};

int RGB_Brightness(int amount, int RgB)
{
     int r; int g; int b;
     stripRGB(RgB, &r, &g, &b);
     
     r_g_b_Brightness(amount, &r, &g, &b);
     
     return RGB(r,g,b);
}; 

int ARGB_Brightness(int amount, int aRGB)
{
     int a; int r; int g; int b;
     stripARGB(aRGB, &a, &r, &g, &b);
     
     r_g_b_Brightness(amount, &r, &g, &b);
     
     return ARGB(a,r,g,b);
}; 

void mult_r_g_b(float amount, int *r, int *g, int *b) 
{
     * r = *r * amount;        
     * g = *g * amount;
     * b = *b * amount;
     if (*r <0) *r = 0;   
     if (*g <0) *g = 0;
     if (*b <0) *b = 0; 
     if (*r >255) *r = 255;   
     if (*g >255) *g = 255;
     if (*b >255) *b = 255;
}; 

int mult_RGB(float amount, int RgB) 
{
     int r; int g; int b;
     stripRGB(RgB, &r, &g, &b);                     
     
     mult_r_g_b(amount, &r, &g, &b); 
     
     return RGB(r,g,b);
}; 

void stripRGB(int RGB, int *r, int *g, int *b)
{
     // Remove the Alpha channel
     int pRGB = RGB & 16777215; 
     
     *r = (pRGB & 16711680)   >> 16; // Red channel   (0x00FF0000)
     *g = (pRGB & 65280)      >>  8; // Green channel (0x00FF00)
     *b =  pRGB               & 255; // Blue channel  (0x00FF)
};                                                                                                                          

void stripARGB(int ARGB, int* a, int * r, int *g, int *b)
{
     *r = (ARGB & 16711680)   >> 16; // Red channel   (0x00FF0000)
     *g = (ARGB & 65280)      >>  8; // Green channel (0x00FF00)
     *b =  ARGB               & 255; // Blue channel  (0x00FF)
     
     //Alpha channel (0xFF000000), overflows to negative (int is 4278190080) 
     *a = (ARGB >> 24) & 127;
     if   (ARGB < 0)
     {
         *a = *a + 128; // Restores the top bit if it was set
     }
};

void PlayGlobalSound(int SIndex,int UnkB,fixed UnkC,fixed Pan)
{    
     local playedSound = false;
     for (i = 0; i < Env->Objs.Count; i++)
     {
          if (playedSound) break;
          local obj = CGObject(Env->Objs.Objs[i]);
          if (obj == NullObj) 
          {
               continue;
          }
          if (obj is CGObject == true)
          {
               if (!playedSound)
               {
                    obj->PlaySound(SIndex, UnkB, UnkC, Pan);   
                    i = Env->Objs.Count - 1; //Just in case, had break; crash the game in FOR loops.
                    playedSound = true;
               }
          } 
          continue;
     }; if (!playedSound)
     Root->PlaySound(SIndex, UnkB, UnkC, Pan); 
};

float FireAngleToRadians(float FireAngleF, int TurnsidE) 
{
    local fRadians = MATH_PI * (FireAngleF - 0.5);
                 
    if (TurnsidE == -1) 
    {
        fRadians = MATH_PI - fRadians;
    };

    if (fRadians > MATH_PI) fRadians -= 2 * MATH_PI;
    if (fRadians < -MATH_PI) fRadians += 2 * MATH_PI;

    return fRadians ;  
};

float FireAngleToDegrees(float FireAngleF, int TurnsidE) 
{
    float fDegrees = 180.0 * (FireAngleF - 0.5);
                 
    if (TurnsidE == -1) 
    {
        fDegrees = 180.0 - fDegrees;
    }
    
    if (fDegrees > 180.0) fDegrees -= 360.0;
    if (fDegrees < -180.0) fDegrees += 360.0;

    return fDegrees;  
};
	
int ObjToCMask(CGObject *obj)
{
	if(obj->ClType == OC_Landscape)	{ return 2; } // Land
	else if(obj->ClType == OC_Worm)
	{
	                // Standing worms		
			if(obj->ObjState == WS_IDLE)		return 4;   
			if(obj->ObjState == WS_WALKING)		return 4;
			if(obj->ObjState == WS_AIMING)		return 4;
			if(obj->ObjState == WS_SETPOWER)	return 4;
			if(obj->ObjState == WS_JUMPPREPARE)	return 4;
			
			// Worms with weapon
			if(obj->ObjState == WS_FIRED)		return 8;
			if(obj->ObjState == WS_FIRECONT)	return 8;
			if(obj->ObjState == WS_DRILLING)	return 8;
			if(obj->ObjState == WS_TELEPORTING)	return 8;
			if(obj->ObjState == WS_CONTROL_WEAPON)	return 8;
			if(obj->ObjState == WS_BLOWTORCH)	return 8;
			
                 	// Worms in mid-air
			if(obj->ObjState == WS_POING)		return 16;
			if(obj->ObjState == WS_JUMP)		return 16;
			if(obj->ObjState == WS_PARACHUTE)	return 16;
			if(obj->ObjState == WS_SLIDING)		return 16;
			if(obj->ObjState == WS_POWERFLY)	return 16;
			if(obj->ObjState == WS_FLYING)		return 16;
			if(obj->ObjState == WS_FLYING2)		return 16;
			if(obj->ObjState == WS_JETPACK)		return 16;
			if(obj->ObjState == WS_AFTERROPE)	return 16;
                        		
                        // Worms on rope/bungee		
			if(obj->ObjState == WS_ROPING)	return 32;
			if(obj->ObjState == WS_BUNGEE)	return 32;
                        		
			// Frozen worms
			if(obj->ObjState == WS_FROZEN)	return 64;
                        		
			// Suicide bomber/kamikaze/shooting anim (only briefly though)		
			if(obj->ObjState == WS_FIREPUNCH)	return 256;
			if(obj->ObjState == WS_SUICIDEBOMBER)	return 256;
		
	}              // Objects
	else if(obj->ClType == OC_Mine)		{ return 1024; }       // Mines
	else if(obj->ClType == OC_Crate)	{ return 2048; }       // Crates
	else if(obj->ClType == OC_Cross)	{ return 8192; }       // Gravestones
	else if(obj->ClType == OC_Missile)	{ return 32768; }      // Weapons
	else if(obj->ClType == OC_Arrow)	{ return 65536; }      // Arrows
	else if(obj->ClType == OC_OilDrum)	{ return 131072; };    // Oil drums   
	
	                // Custom objects    
        local flagszs = CMASK_DEFAULT_COLLIDEABLE;   
	if(obj->Layer == LAYER_OILDRUM || obj->Layer == LAYER_MINE)	{ return 1; } 
        else if(flagszs & (1 << obj->Layer) == 0) { return 1; };  
	
//	if (obj->PosY >= Env->Water) return 4194304; //Must be the water...
//	else
        return obj->ColGroup;
};
//One of the most useful things i have here                                      //if u got no colmask just make a temporal one and delete after     
bool CheckSpawnPoint(float spawnX, float spawnY, float targetX, float targetY, CColMask* colmask, float radius, int flags, float* outX, float* outY, int radiusLimit)
{
    if (colmask == NullObj) return false;

    *outX = spawnX;
    *outY = spawnY;
                         
    // Safety fallback if a bad radius is passed
    if (radius < 1.0) radius = 2.0;
                               
    // Expand outward from the spawn point up to the maximum
    float searchRadius = 0.0;
    while (searchRadius <= radiusLimit)
    {
        int degree = 0;
        int step;
        if (searchRadius == 0.0)
            step = 360;
        else
            step = int(maxfloat(15, 360.0 / (searchRadius / 2.0)));

        while (degree < 360)
        {
            float rad = degree * (MATH_PI / 180.0);

            // Round to nearest int instead of truncating, avoids the -0.0001 ? 0 shave
            int tx = int(spawnX + searchRadius * cos(rad) + 0.5);
            int ty = int(spawnY + searchRadius * sin(rad) + 0.5);

            default_offset = 3;
            if (radius > 10)
            default_offset = int(radius * 0.4);
            // 1. Mask Check: Does the projectile fit here without clipping?
            local trcmsk = TraceMaskEx(NullObj, colmask, tx, ty, tx, ty + default_offset, flags, spawnCheckRes);
            
            if (spawnCheckRes->StartsSolid == false || trcmsk)
            {                           
                // 2. Line of Sight Check: Is there a wall between this safe spot and the target?
                int hitX; int hitY;
                CGObject* wall = TraceLine(NullObj, tx, ty, int(targetX), int(targetY), flags, &hitX, &hitY);
                                 
                // Clear line of sight!
                if (wall == NullObj)
                {
                    *outX = tx + 0.0;
                    *outY = ty + 0.0;
                    return true;
                }
            }
            degree = degree + step;
        }
        searchRadius = searchRadius + radius;  // Increase search radius safely
    };

    return false;
};     
                                        //own col flags
void ForceCollission(CGObject * sender, int flags, float x, float y, int force, bool bounce)
{       
        CGObject *hitLand = CGObject(NullObj);   
        CGObject *hitMine = CGObject(NullObj); 
        CGObject *hitDrum = CGObject(NullObj);
        CGObject *hitObj  = CGObject(NullObj);    
        CGObject *hitWorm = CGObject(NullObj);
        
        for(int i = 0; i < Env->Objs.Count; i += 1)
	{
		obj = CGObject(Env->Objs.Objs[i]);
		
		if (obj == NullObj) continue;  
		if ((flags & obj->ColGroup) != 0) continue;
		//if (obj is CGObject == false) continue;
		
                if(obj->ClType == OC_Landscape && (flags & obj->ColGroup) == 0) 
                {
                     hitLand = CGObject(obj); break; 
                } 
                else if(obj->ClType == OC_Collideable && (flags & obj->ColGroup) == 0) 
                {
                     hitLand = CGObject(obj); break; 
                }
                else if(obj->ClType == OC_Mine && (flags & obj->ColGroup) == 0) 
                {
                     hitMine = CGObject(obj); break; 
                }
		else if(sender == obj && (flags & obj->ColGroup == 0))
                {
		     hitObj = sender; break; // Collide with self works best for exploding stuff
                }
                else if(obj->ClType != OC_Worm && obj->ClType != OC_OilDrum && (flags & obj->ColGroup) == 0) 
                {
                     hitObj = CGObject(obj);  break;    //Prolly a crate or some bs
                }  
                else if(obj->ClType == OC_OilDrum && (flags & obj->ColGroup) == 0) 
                {
                     hitDrum = CGObject(obj); break; 
                }
                else if(obj->ClType == OC_Worm && (flags & obj->ColGroup) == 0) // Worm last to avoid potential weird shit
                {
                     hitWorm = CGObject(obj); break; 
                } 
                else {continue;};
   
       };
       
       local hitobj = CGObject(NullObj);
       
       if      (hitLand!=NullObj) hitobj = hitLand;    
       else if (hitMine!=NullObj) hitobj = hitMine;    
       else if (hitObj!=NullObj)  hitobj = hitObj;
       else if (hitDrum!=NullObj) hitobj = hitDrum;  
       else if (hitWorm!=NullObj) hitobj = hitWorm;
       
       if (hitobj!=NullObj)
       sender->Collide(hitobj, 1); // Play sound or do stuff idk
       
       if (bounce) // Bounce anyways if hitobj is NullObj (shouldnt happen tbh), u called the function for a reason i imagine 
       {   
            BounceAgainstSphere(sender, sender->PosX, sender->PosY, force);
       };
};

bool TeleportToRandom(CGObject * sender, CColMask *colMask, int flags)
{     
  if (sender == NullObj) return false;
  if (sender->ObjState==WS_DEAD || sender->ObjState==WS_DEATH || sender->ObjState==WS_SINKING || sender->ObjState==WS_SUICIDEBOMBER)
  {
    return false;
  }
  if (flags == 0) flags = CMASK_DEFAULT_COLLIDEABLE - 4194304; //cmon
  
  local teleAtX = RandomInt((gframe / 7) * 0.5, GS->LevelSX); // Prime multiplier for RNG seed
  local teleAtY = RandomInt((gframe / 11) * 0.5, Env->Water - 200);
  local hitX = 0; local hitY = 0;
  
  local land = (TraceLine(sender, float(teleAtX), float(teleAtY), float(teleAtX), float(Env->Water), flags, &hitX, &hitY) == NullObj) == false;
         
  local spawnOK = false;

  float spawnX = float(hitX);
  float spawnY = float(hitY);  

  if (land)
  {                                        //not too narrow
      if (colMask == NullObj)
      spawnOK = CheckSpawnPoint(spawnX, spawnY-3.0, spawnX, spawnY-12.0, sender->ColMask, 1, flags, &spawnX, &spawnY, 11);     
      else
      spawnOK = CheckSpawnPoint(spawnX, spawnY-3.0, spawnX, spawnY-12.0, colMask, 1, flags, &spawnX, &spawnY, 11);
  }
  else
  return false;
  
  //land = (TraceLine(sender, spawnX, spawnY, spawnX, float(Env->Water), flags, &hitX, &hitY) == NullObj) == false; // Sometimes it likes to put you on top of water
  if (spawnOK)
  {            
     sender->PosX = spawnX;
     sender->PosY = spawnY-1.5;
     return true;
  }
  return false;
}

bool IsTooCloseToObjs(CGObject * sender, float x, float y, float minDist)
{
    float minDistSqr = minDist * minDist;
    for (local i = 0; i < Env->Objs.Count; i += 1)
    {
		CGObject * obj = CGObject( Env->Objs.Objs[i] );

		if(obj == sender) continue;
		if(obj == NullObj) continue;
		if(obj->IsMaterial == false) continue;
                float dx = obj->PosX - x;
                float dy = obj->PosY - y;
                if (dx*dx + dy*dy < minDistSqr) return true;
    }
    return false;
}

bool IsTooCloseToCWorms(CGObject * sender, float x, float y, float minDist)
{
    float minDistSqr = minDist * minDist;
    for (local i = 0; i < Env->Objs.Count; i += 1)
    {
		CGObject * obj = CGObject( Env->Objs.Objs[i] );

		if(obj == sender) continue;
		if(obj == NullObj) continue;
		if(obj->ClType != OC_Worm) continue;
                float dx = obj->PosX - x;
                float dy = obj->PosY - y;
                if (dx*dx + dy*dy < minDistSqr) return true;
    }
    return false;
}

///global res 
CTraceRes * spawnCheckRes;
CTraceArcRes * CheckArcRes;
CColMask * ColMask3s ;

int CMASK_ALL;
int CMASK_EVERYTHING;
int CMASK_ALL_WORMS;     
float RainbowGlobalHue;
float RainbowSpeed;

string NullString;

void redutils::InitGraphic()
{    
    ColMask3s = new CColMask(3,3,MakeCircleMask(3)); 
};
void redutils::Init()
{    
    RainbowGlobalHue = 0.0;
    RainbowSpeed     = 0.009;   // 0.003 = full cycle every 330 frames
    
    CMASK_ALL = -1;            
    CMASK_EVERYTHING = -1;
    CMASK_ALL_WORMS = 4 + 8 + 16 + 32 + 64 + 256 ;   
    NullString = "Null";  
    
    spawnCheckRes = new CTraceRes();  //Deleting and creating stuff mid frame is expensive, just let it be. Works well for several objs using same res at the same time (for now)
    CheckArcRes   = new CTraceArcRes(); //  
}; 