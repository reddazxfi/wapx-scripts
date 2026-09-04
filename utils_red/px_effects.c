require utils, utils_steps, weapon_tazer; 

CEffectManager :  CObject ;

CSprite * beam_glowSprite;  
    
#effects
 
float sawZapNoise [99] ;
float sawZapNoise2[99] ;                            

void GenerateZapNoise()
{                                  
    for (local i = 0; i < 16; i++)
        sawZapNoise[i] = RandomFloat(-1.0, 1.0);
}  
    
void GenerateZapNoise2()
{
    for (local i = 0; i < 36; i++)
        sawZapNoise2[i] = RandomFloat(-1.0, 1.0);
}   

float Master_LUT_Cos[361];
float Master_LUT_Sin[361];

void px_effects::FirstFrame()
{
    // Precompute a circle once
    for (int i = 0; i < 361; i++)
    {
        float ang = (float(i) / 360.0) * MATH_TWO_PI;
        Master_LUT_Cos[i] = cos(ang);
        Master_LUT_Sin[i] = sin(ang);
    }
    GenerateZapNoise ();
    GenerateZapNoise2();
}

void px_effects::InitGraphic()
{   
    beam_glowSprite = LoadSprite(GetAttachment("beam_glow.png"),1,0);
}

CEffectManager::CEffectManager(CGObject* target, float StartX, float StartY, int type, int segments, float radius, float noise, int red, int green, int blue)
{
    // --- Target & Position Setup ---
    targetObj  = target;
    lastKnownX = StartX;
    lastKnownY = StartY;                                             
    ownerLess  = false;
    if (target == NullObj)  ownerLess = true;

    // --- Core Parameters & Colors ---
    effectType     = type;
    numSegs        = segments;
    baseRadius     = radius;
    noiseIntensity = noise;
    sizeValue      = float(radius / 100);

    r = red; 
    g = green; 
    b = blue;
    rgb     = RGB(red, green, blue); //unused
    glowrgb = RGB(red * 0.72, green * 0.72, blue * 0.72);
    glow    = true;
    isRainbow = false;
    rainbowHue = RandomFloat(0.0,1.0);
    rainbowSpeed = 0.012;

    // --- State Flags & Frame Counters ---
    managerState   = 0;   //0 is active animation, 1 is explosion anim
    frameCount     = 0;
    nullFrameCount = 0;
    objIndex       = gframe;
    freeAfter      = 0;   //unconditional free.
    
    bufferChange   = false;
    bufferFrames   = 0;   //uses gframe

    reachedTarget  = true;
    shouldExplode  = false;
    shouldEllipse  = false;

    // --- Animation Timers & Fade Controls ---
    explosionTimer   = 0.0; //
    vanishSpeed      = 1.0; // No lower than 0.5 / -0.5
    expLimit         = 15;  // Extend explosion anim
    expandMultiplier = 3.0; // how far blastRadius grows beyond baseRadius, was hardcoded
    fadeInFrac       = 0.0; // 0.0 = no fade-in (old behavior). e.g. 0.15 = ramp up over first 15% of duration
    blastProgress    = 0.0; // computed once per frame, read by both draw functions
    envelope         = 1.0; // brightness/alpha multiplier after fade-in is applied
    circumference    = 1.0; // circumference multiplier. if both are the same, circle. if not, oval.  
    circumferenceSin = 1.0;

    // --- Explosion Buffer Defaults ---
    custExp    = false;
    //buffer values
    expType    = 4;
    expSeg     = segments;
    expRad     = baseRadius;
    expNoise   = noise;
    expR       = r;
    expG       = g;
    expB       = b;
    buffVanish = 0.0;

    // --- Scaled Visual Geometry ---
    beamThiccness = sizeValue * 10.0; //core
    if (beamThiccness < 2.3) beamThiccness = 2.3; //glow 
    beamGlowThicc = beamThiccness * 3.0;
    if (beamThiccness > 5.5) beamThiccness = 5.5; // Glow scales up, core doesn't to stay sharp.

    objectScale = sizeValue * 2.0; //For saw specifically

    gMult = sizeValue * 2.0; // FOR Loop Glow Multiplier.
    if (gMult > 1.2) gMult = 1.2;

    q_glow_s = float(radius / 100) * 3.5; // Single glow size

    // --- Rotation Angles ---
    currentAngle = 0.0;
    cosRot       = 0.0;
    sinRot       = 0.0;
    shiftAngle   = 0.14;
    linkedAngle  = 0.0;
    isLinked     = false;
    
    // --- ReachPoint ---
    curveType  = 0;      // 0 = straight, 1 = quadratic bezier (arc), 2 = circular arc
    curveBulge = 0.04;     // how far the curve bows out (bezier control point offset)
    arcRadius  = 5.0;      // used only for curveType == 2
    reachFromX = 0.0; reachFromY = 0.0;
    reachToX = 0.0;    reachToY = 0.0;
    hasReachTarget = false;
    reachExpand = 0.03;
    reachBow = 0.2;
    reachProgress = 0.0;
    reachSpeed = 0.2;
    rayThickness = 4.0;
    rayGlowThick = 6.5;
    raySegments = 16;      
    rayPersists = false;
    
    isTrail = false;
    trailWavy = false;
    StraightTrail = false;
    midThickEdgeThin = false;
    trailPastX = new int[9999];   
    trailPastY = new int[9999];
    trailAmount = 0;
    trailMaxL = 30;
    treshold = 3;
    drawConventionalEffects = true;
    trailSnapped = false;
    customTrail = false;
    
    //trailPastX[0] = StartX;     
    //trailPastY[0] = StartY;

    // --- Depth & Engine Init ---
    ZPlane  = 9.9;
    gZPlane = ZPlane - 0.1;   
    
    if (effectType == 4 || effectType == 5) shouldExplode = true;   

    super(Root, GS);       
}

void CEffectManager::Free(bool FreeMem)
{
 drawConventionalEffects = false;
 trailAmount = 0;
 delete trailPastX;
 delete trailPastY;
 super;
}

void CEffectManager::editParams(int type, int segments, float radius, float noise, int red, int green, int blue, float vanishspd)
{
    effectType = type;
    noiseIntensity = noise;
    numSegs    = segments;
    baseRadius = radius;  
      
    beamThiccness = sizeValue * 10.0;
    if (beamThiccness < 2.3) beamThiccness = 2.3;
    beamGlowThicc = beamThiccness * 3.0;
    if (beamThiccness > 5.5) beamThiccness = 5.5; // Glow scales up, core doesn't to stay sharp.
    
    sizeValue  = float(radius / 100);
    
    objectScale = sizeValue * 2.0;   //For saw specifically
    
    gMult = sizeValue * 2.0; // FOR Loop Glow Multiplier.
    if (gMult > 1.2) gMult = 1.2;
    
    q_glow_s = float(radius / 100) * 3.5; // Single glow size
    
    glowrgb = RGB(red * 0.72,green * 0.72,blue * 0.72);
    
    r = red; g = green; b = blue;
    
    vanishSpeed = vanishspd;
    
    if (effectType == 4 || effectType == 5) shouldExplode = true;
}

void CEffectManager::LinkTo(CGObject *target, bool lockRotation){  targetObj = target; ownerLess = false; isLinked = lockRotation; }
void CEffectManager::SetSizeValue(float sizevalue) { sizeValue = sizevalue;  gMult = sizeValue * 2.0; if (gMult > 1.2) gMult = 1.2; }   
void CEffectManager::SetBeamThickness(float beamthic) { beamThiccness = beamthic; }
void CEffectManager::SetBeamGlowThickness(float beamgt) { beamGlowThicc = beamgt; }
void CEffectManager::SetRadius(float radius) { baseRadius = radius; q_glow_s = float(radius / 100) * 3.5;  }
void CEffectManager::SetRGB(int red, int green, int blue) { glowrgb = RGB(red * 0.72,green * 0.72,blue * 0.72); r = red; g = green; b = blue; }
void CEffectManager::SetScale(float objscale) { objectScale = objscale; }
void CEffectManager::SetSegments(int segments) { numSegs = segments; }
void CEffectManager::SetType(int type) { effectType = type; }     
void CEffectManager::SetNoise(float noise) { noiseIntensity = noise; }
void CEffectManager::SetExplosionDuration(float duration) { expLimit = duration; }
void CEffectManager::SetExpandMultiplier(float mult)      { expandMultiplier = mult; }     
void CEffectManager::SetCircumferenceMultiplier(float onepointzero)    { circumference = onepointzero; }   
// If both are same value = circle, if one is different = oval. 
void CEffectManager::SetCircumferenceMultiplierSin(float onepointzero) { circumferenceSin = onepointzero; }
void CEffectManager::BufferChange(int type, int segments, float radius, float noise, int red, int green, int blue, int frames, float vanishspd)
{
    expType  = type      ;   
    expNoise = noise     ;
    expSeg   = segments  ;
    expRad   = baseRadius;
    expR     = red       ;        
    expG     = green     ;
    expB     = blue      ;
    bufferChange = true  ;   
    bufferFrames = gframe + frames; 
    buffVanish = vanishspd;
}
void CEffectManager::SetFadeIn(float frac)
{
    if (frac < 0.0) frac = 0.0;
    if (frac > 0.9) frac = 0.9;   // leave room for the fade-out tail
    fadeInFrac = frac;
}
void CEffectManager::SetVanishSpeedClamped(float speed)
{
    if (speed > 0.0 && speed < 0.05)  speed = 0.05;
    if (speed < 0.0 && speed > -0.05) speed = -0.05;
    vanishSpeed = speed;
}
void CEffectManager::SetRainbowMode(bool enable, float speed, float startOffset)
{
    isRainbow    = enable;
    rainbowSpeed = speed;
    rainbowHue   = startOffset;   // 0.0-1.0, lets two instances start at different colors
}
void CEffectManager::SetTrail(int maxLength, int delayTreshold, int type)
{
  isTrail = true;
  treshold =  delayTreshold;
  if (type == 1)  midThickEdgeThin = true;
  else if (type == 2)  trailWavy = true;
}

void CEffectManager::SetTarget(float fromX, float fromY, float toX, float toY, int type, float speed, float bulge)
{
    reachFromX     = fromX;
    reachFromY     = fromY;
    reachToX       = toX;
    reachToY       = toY;
    hasReachTarget = true;
    reachProgress  = 0.0;    
    curveType  = type;
    curveBulge = bulge;   
    reachSpeed = speed;
}
void CEffectManager::SetTargetNoTerrain(float fromX, float fromY, float toX, float toY, int type, float speed,  float bulge)
{
    int hitx; int hity;
    local obj = TraceLine(this,fromX,fromY,toX,toY, CMASK_TERRAIN, &hitx, &hity);
    reachFromX     = fromX;
    reachFromY     = fromY;
    reachToX       = hitx;
    reachToY       = hity;
    hasReachTarget = true;
    reachProgress  = 0.0;    
    curveType  = type;
    curveBulge = bulge;
    reachSpeed = speed;
}
void CEffectManager::ClearTarget()
{
    if (rayPersists) return;
    hasReachTarget = false;
    reachProgress  = 0.0;
}

void CEffectManager::UpdateType4Reach()
{
    if (!hasReachTarget) return;
    
    reachProgress += reachSpeed;
    if (reachProgress > 1.0) reachProgress = 1.0;
}

// Evaluates a point at parameter t (0.0 to 1.0) along the chosen curve shape
void CEffectManager::PointOnCurve(float t, float* outX, float* outY)
{
    float dx = reachToX - reachFromX;
    float dy = reachToY - reachFromY;
    float dist = sqrt(dx*dx + dy*dy);
    if (dist < 0.001) { *outX = reachFromX; *outY = reachFromY; return; }

    float perpX = 0.0 - dy / dist;
    float perpY =       dx / dist;

    if (curveType == 0)
    {
        // Straight line
        *outX = reachFromX + dx * t;
        *outY = reachFromY + dy * t;
        return;
    }

    if (curveType == 1)
    {
        // Quadratic bezier: control point offset perpendicular at the midpoint
        float midX = (reachFromX + reachToX) * 0.5 + perpX * curveBulge;
        float midY = (reachFromY + reachToY) * 0.5 + perpY * curveBulge;

        float oneMinusT = 1.0 - t;
        *outX = oneMinusT*oneMinusT*reachFromX + 2.0*oneMinusT*t*midX + t*t*reachToX;
        *outY = oneMinusT*oneMinusT*reachFromY + 2.0*oneMinusT*t*midY + t*t*reachToY;
        return;
    }

    if (curveType == 2)
    {
        // Circular arc: sweep around a center point derived from bulge
        // (bulge here acts as the arc's sagitta - how far it bows)
        float chordHalf = dist * 0.5;
        float sagitta = curveBulge;
        if (sagitta < 0.001) sagitta = 0.001;

        float radius = (chordHalf*chordHalf + sagitta*sagitta) / (2.0 * sagitta);

        float midX = (reachFromX + reachToX) * 0.5;
        float midY = (reachFromY + reachToY) * 0.5;

        float centerX = midX - perpX * (radius - sagitta);
        float centerY = midY - perpY * (radius - sagitta);

        float startAng = atan2(reachFromY - centerY, reachFromX - centerX);
        float endAng   = atan2(reachToY   - centerY, reachToX   - centerX);

        // Choose shortest sweep direction
        float diff = endAng - startAng;
        if (diff > MATH_PI)  diff -= MATH_TWO_PI;
        if (diff < -MATH_PI) diff += MATH_TWO_PI;

        float ang = startAng + diff * t;
        *outX = centerX + cos(ang) * radius;
        *outY = centerY + sin(ang) * radius;
        return;
    }

    // Fallback
    *outX = reachFromX + dx * t;
    *outY = reachFromY + dy * t;
}
                                                                   
void CEffectManager::drawGlow()
{
        CQuad q;
        FillSpriteQuad(&q, tazer_lightningGlowSprite->Index, 0, 0);
        TransformQuad(&q, 0, q_glow_s, q_glow_s, lastKnownX, lastKnownY);
        q.blend = 1;
        q.v.color = ARGB(190, r, g, b);      
            
        AddSpriteQ(gZPlane, &q, tazer_lightningGlowSprite->Index, 0, 64);
}
  
void CEffectManager::Message(CObject* sender, EMType Type, int MSize, CMessageData* MData)
{ 
    if (Type == M_DRAWQUEUE)
    {
    if (drawConventionalEffects)   {
        if (ZPlane < 1.1) ZPlane = 1.1;   
        if (hasReachTarget) DrawReachPoint();  
        if (effectType == 0)       UpdateAndDraw();
        else if (effectType == 1)  DrawSpinFX();              
        else if (effectType == 2)  DrawRingFX();
        else if (effectType == 3)  DrawTrailFX();
        else if (effectType == 4)  managerState = 1;  // Explosion    
        else if (effectType == 5)  managerState = 1;  // Pulsate
        else if (effectType == 6)  DrawThinSpinFX(); 
        else if (effectType == 99) {}   
        //else if (effectType == 7)  DrawTrailFX();
        else
                UpdateAndDraw();     
                
        
        if (managerState == 1) //Explosion
        {
                DrawExplosion();
                DrawEllipse();
        }
        if (glow && managerState != 1)
        {
                drawGlow();  
        }
        if (managerState == 1 && effectType == 5)
        {                                                                                                 
                if (glow) drawGlow();
        }   
    }    
    if (isTrail || customTrail) DrawTrail();    
     
    }
    if (Type == M_FRAME)
    {
        Calc();
        /*if (gframe % 3 == 0 )
        {
         GG->WriteToChat( 6, "-----------------------", false);
         GG->WriteToChat( 5, itoa(nullFrameCount), false);    
         GG->WriteToChat( 7, itoa(explosionTimer), false);
         GG->WriteToChat( 8, itoa(managerState), false);
        }*/     
        
        if (ownerLess && explosionTimer > expLimit * 2.5) Free(true);  //fallback
        
        if (vanishSpeed >= -0.5 && vanishSpeed <  0.0) vanishSpeed = -0.51;   
        if (vanishSpeed >=  0.0 && vanishSpeed <= 0.5) vanishSpeed =  0.51;
        
        if (bufferChange && gframe > bufferFrames )
        {
            editParams(expType, expSeg, expRad, expNoise, expR, expG, expB, buffVanish);
            bufferChange = false;
        }

        if (targetObj == NullObj) 
        {
            nullFrameCount+=1;
            
            if (effectType == 5 && nullFrameCount >= 5 )
            {
                managerState = 1;
                effectType = 4;
                explosionTimer = 0.0;
            }

            if (nullFrameCount >= 5 && !ownerLess)
            {
                managerState = 1; 
                if (custExp)
                {
                    editParams(effectType, expSeg, expRad, expNoise, expR, expG, expB, vanishSpeed);
                }
            }
        }         
        else if (nullFrameCount > 0 && targetObj != NullObj)
        {
            nullFrameCount-=1;
        }
    
        if (managerState == 1) 
        {
            explosionTimer = explosionTimer + vanishSpeed;
        }
        
        // Total frames the blast lasts
        if ((explosionTimer > expLimit || explosionTimer < -expLimit) && effectType != 5)
        {
            TryFree(); // Manager successfully cleans itself up
            return;
        }     
        else if (explosionTimer > expLimit && effectType == 5)
        {
            explosionTimer = 0;
        }
        if (freeAfter != 0 && frameCount > freeAfter)  //unconditional 
        {
            TryFree(); 
            return;
        }    
    }
    super;
    TryFree();
}

void CEffectManager::TryFree()
{
 local Condition1 = false;   
 local Condition2 = false;
 local Condition3 = false;
 
 
 if (explosionTimer > expLimit)
 Condition1 = true;

 if (frameCount > freeAfter)
 Condition2 = true;

 if (isTrail && trailSnapped)
 Condition3 = true;
 if (!isTrail)
 Condition3 = true;
 
 if (Condition1 || (Condition2 && freeAfter!=0)) drawConventionalEffects = false;
 
 if (Condition1 && Condition2 && Condition3) Free(true);
}

void CEffectManager::Calc()//SLANG for calculator by the way.
{    
     if (targetObj != NullObj)
     {
          lastKnownX = targetObj->PosX;
          lastKnownY = targetObj->PosY;            
     }
     frameCount++;     
     
     if (isRainbow)
     {
         CalculateRainbow();
     }
     if (isTrail && (gframe % treshold) == 0)
     {
          FillTrail();
     }
     if (managerState == 1 && effectType != 5)
     {
          DetractTrail();
     }
     //TrackMaster();   
     
     UpdateType4Reach();
     
     cosRot = cos(currentAngle);
     sinRot = sin(currentAngle);   
     
     if (effectType == 1 || effectType == 6) currentAngle += shiftAngle;
     currentAngle = NormalizeAngle(currentAngle);
     
     if (isLinked && targetObj != NullObj && (abs(targetObj->SpX) + abs(targetObj->SpY)) > 0) currentAngle = atan2(-targetObj->SpX, targetObj->SpY) - MATH_HALF_PI;

     // --- Unified explosion progress, driven by expLimit ---
     blastProgress = 0.0;
     if (expLimit > 0.001)
         blastProgress = explosionTimer / expLimit;

     if (blastProgress < 0.0) blastProgress = 0.0;
     if (blastProgress > 1.0) blastProgress = 1.0;

     // --- Envelope: fade-in ramp, then normal (1-progress) fade-out ---
     envelope = 1.0 - blastProgress;
     if (fadeInFrac > 0.001 && blastProgress < fadeInFrac)
     {
         float fadeInAmt = blastProgress / fadeInFrac;
         envelope = fadeInAmt;   // overrides the fade-out term while still ramping in
     }
     if (envelope < 0.0) envelope = 0.0;
     if (envelope > 1.0) envelope = 1.0;
}  

int CEffectManager::HueToRGBLocal(float hue)
{
    // Wrap hue into 0.0 .. 1.0
    hue = hue - float(int(hue));
    if (hue < 0.0) hue += 1.0;

    // Convert 0..1 to 0..2PI radians
    float rad = hue * 6.2831853; 

    // Center at 180, amplitude 75 -> Keeps RGB floor at 105 (no black zones)
    int rr = int(180.0 + 75.0 * sin(rad));
    int gg = int(180.0 + 75.0 * sin(rad + 2.0943951)); // +120 deg
    int bb = int(180.0 + 75.0 * sin(rad + 4.1887902)); // +240 deg

    return RGB(rr, gg, bb);
}

void CEffectManager::CalculateRainbow()
{
         rainbowHue += rainbowSpeed;
         if (rainbowHue > 1.0) rainbowHue -= 1.0;

         local packed = HueToRGBLocal(rainbowHue);
         r = (packed >> 16) & 255;
         g = (packed >> 8)  & 255;
         b = packed & 255;

         glowrgb = RGB(int(float(r) * 0.79), int(float(g) * 0.79), int(float(b) * 0.79));
}                                                                
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
void CEffectManager::UpdateAndDraw()
{
    if (managerState == 0)
    {
        float uX;
        float uY;

        float liveNoise;
        float totalRadius;

        float rotatedX;
        float rotatedY;

        float px;
        float py;
            
        float texCoord;    
            
        // Pass 1: soft outer glow — always present but kept subtle
            
        StartTexturedBeam(this, ZPlane, beam_glowSprite->Index, 0.0, 1);
        
        for (local j = 0; j <= numSegs; j++)
        {
            int lutIndex2 = (j * 360) / numSegs;
            if (lutIndex2 > 359) lutIndex2 = 359;

            uX = Master_LUT_Cos[lutIndex2] * circumference;
            uY = Master_LUT_Sin[lutIndex2] * circumferenceSin;

            // Use sawZapNoise2 so the core moves independently from the outer glow.
            // Multiply the intensity slightly to make the core more jagged.
            liveNoise = sawZapNoise2[j % 22] * noiseIntensity;
            totalRadius = baseRadius + liveNoise;

            rotatedX = (uX * cosRot) - (uY * sinRot);
            rotatedY = (uX * sinRot) + (uY * cosRot);

            px = lastKnownX + (rotatedX * totalRadius);
            py = lastKnownY + (rotatedY * totalRadius);

            texCoord = float(j) / float(numSegs);

            TexturedBeamPoint(px, py, beamGlowThicc, texCoord, glowrgb);
        }
        
        EndTexturedBeam();

        // Pass 2: Sharp, crackling rotating core
        StartTexturedBeam(this, ZPlane, tazer_lightningBeamSprite->Index, 0.0, 1);
        
        for (local j = 0; j <= numSegs; j++)
        {
            int lutIndex2 = (j * 360) / numSegs;
            if (lutIndex2 > 359) lutIndex2 = 359;

            uX = Master_LUT_Cos[lutIndex2] * circumference;
            uY = Master_LUT_Sin[lutIndex2] * circumferenceSin;

            liveNoise = sawZapNoise2[j % 22] * noiseIntensity;
            totalRadius = baseRadius + liveNoise;

            rotatedX = (uX * cosRot) - (uY * sinRot);
            rotatedY = (uX * sinRot) + (uY * cosRot);

            px = lastKnownX + (rotatedX * totalRadius);
            py = lastKnownY + (rotatedY * totalRadius);

            texCoord = float(j) / float(numSegs);

            TexturedBeamPoint(px, py, beamThiccness, texCoord, RGB(r, g, b));
        }
        
        EndTexturedBeam();
    }    
}

void CEffectManager::DrawSpinFX()
{       
    if (managerState == 1) return;
    float radius = baseRadius;
    int currentNumSegs = 16;
    // --- PASS 1: Soft Outer Glow ---
    StartTexturedBeam(this, ZPlane, beam_glowSprite->Index, 0.0, 1);
    for (local i = 0; i <= currentNumSegs; i++)
    {
        float t = float(i) / float(currentNumSegs);
    
        int lutIndex = (i * 360) / currentNumSegs;
        if (lutIndex > 359) lutIndex = 359; // clamp

        float uX = Master_LUT_Cos[lutIndex] * circumference;
        float uY = Master_LUT_Sin[lutIndex] * circumferenceSin;

        float dirX = uX * cosRot - uY * sinRot;
        float dirY = uX * sinRot + uY * cosRot;

        float totalRadius = radius + (sawZapNoise[i % 16] * (noiseIntensity * 0.5));

        float px = lastKnownX + dirX * totalRadius;
        float py = lastKnownY + dirY * totalRadius;

        float f = (4.0 * t * (1.0 - t) + 0.15);
        if (f > 1.0) f = 1.0;

        float r_col = r * f;
        float g_col = g * f;
        float b_col = b * f;

        TexturedBeamPoint(px, py, f * beamGlowThicc, t, RGB(r_col, g_col, b_col));

        // Glow dot at each point 
        if (i < currentNumSegs)
        {
            CQuad q;
            FillSpriteQuad(&q, tazer_lightningGlowSprite->Index, 0, 0);
            TransformQuad(&q, 0, sizeValue, sizeValue, px, py);
            q.blend = 1;
            for (int k = 0; k < 4; k++) q.v[k].color = RGB(r_col * gMult, g_col * gMult, b_col * gMult);
            AddSpriteQ(gZPlane, &q, tazer_lightningGlowSprite->Index, 0, 64);
        }
    }
    EndTexturedBeam();

    // --- PASS 2: Sharp Core ---
    StartTexturedBeam(this, ZPlane, tazer_lightningBeamSprite->Index, 0.0, 1);
    for (local i = 0; i <= currentNumSegs; i++)
    {
        float t = float(i) / float(currentNumSegs);

        float uX = Master_LUT_Cos[i] * circumference;
        float uY = Master_LUT_Sin[i] * circumferenceSin;

        float dirX = uX * cosRot - uY * sinRot;
        float dirY = uX * sinRot + uY * cosRot;

        float totalRadius = radius + (sawZapNoise2[i % 36] * noiseIntensity);

        float px = lastKnownX + dirX * totalRadius;
        float py = lastKnownY + dirY * totalRadius;

        float f = (4.0 * t * (1.0 - t) + 0.15);
        if (f > 1.0) f = 1.0;

        float r_col = r * f;
        float g_col = g * f;
        float b_col = b * f;

        TexturedBeamPoint(px, py, f * beamThiccness, t, RGB(r_col, g_col, b_col));
    }
    EndTexturedBeam();
}      

void CEffectManager::DrawRingFX() //double ring
{       
    if (managerState == 1) return;
    // --- PASS 1 Core
    StartTexturedBeam(this, ZPlane, tazer_lightningBeamSprite->Index, 0.0, 1);
    for (local i = 0; i <= numSegs; i++) 
    {
        int currentbeam = i; 
        
        int lutIndex2 = (i * 360) / numSegs;
        if (lutIndex2 > 359) lutIndex2 = 359;
        
          
        float noiseAmt = sawZapNoise[currentbeam % 16] * (noiseIntensity * 0.5);
        float radius   = (baseRadius + noiseAmt) * 0.85;

        float uX = Master_LUT_Cos[lutIndex2] * circumference;
        float uY = Master_LUT_Sin[lutIndex2] * circumferenceSin;

        float px = lastKnownX + (uX * cosRot - uY * sinRot) * radius;
        float py = lastKnownY + (uX * sinRot + uY * cosRot) * radius;
        float texCoord = float(i) / float(numSegs);

        TexturedBeamPoint(px, py, beamThiccness, texCoord, glowrgb);
    }
    EndTexturedBeam();

    // --- PASS 2: Core Beam ---
    StartTexturedBeam(this, ZPlane, tazer_lightningBeamSprite->Index, 0.0, 1);
    for (local i = 0; i <= numSegs; i++)
    {                                          
        int lutIndex2 = (i * 360) / numSegs;
        if (lutIndex2 > 359) lutIndex2 = 359;
            
        float noiseAmt = sawZapNoise2[i % 36] * noiseIntensity; 
        float radius   = baseRadius + noiseAmt;

        float uX = (Master_LUT_Cos[lutIndex2]  * circumference);
        float uY = (Master_LUT_Sin[lutIndex2]  * circumferenceSin);

        float px = lastKnownX + (uX * cosRot - uY * sinRot) * radius;
        float py = lastKnownY + (uX * sinRot + uY * cosRot) * radius;
        float texCoord = float(i) / float(numSegs);
        
        TexturedBeamPoint(px, py, beamThiccness, texCoord, RGB(r, g, b));
    }
    EndTexturedBeam(); 

    // --- PASS 3: Glow Beam ---
    StartTexturedBeam(this, ZPlane, beam_glowSprite->Index, 0.0, 1);
    for (local i = 0; i <= numSegs; i++)
    {                                          
        int lutIndex2 = (i * 360) / numSegs;
        if (lutIndex2 > 359) lutIndex2 = 359;
            
        float noiseAmt = sawZapNoise2[i % 36] * noiseIntensity; 
        float radius   = baseRadius + noiseAmt;

        float uX = Master_LUT_Cos[lutIndex2] * circumference;
        float uY = Master_LUT_Sin[lutIndex2] * circumference;

        float px = lastKnownX + (uX * cosRot - uY * sinRot) * radius;
        float py = lastKnownY + (uX * sinRot + uY * cosRot) * radius;
        float texCoord = float(i) / float(numSegs);
        
        TexturedBeamPoint(px, py, beamGlowThicc, texCoord, glowrgb);
    }
    EndTexturedBeam();
}
        
void CEffectManager::DrawExplosion()
{    
    if (!shouldExplode) return;

    float blastRadius = baseRadius + (baseRadius * expandMultiplier * blastProgress); 
    
    int fadeR = int(float(r) * envelope);
    int fadeG = int(float(g) * envelope);
    int fadeB = int(float(b) * envelope);

    StartTexturedBeam(this, ZPlane, tazer_lightningBeamSprite->Index, 0.0, 1);
    for (local i = 0; i <= numSegs; i++)
    {
        int lutIndex = (i * 360) / numSegs;
        if (lutIndex > 359) lutIndex = 359; 

        float noiseAmt = sawZapNoise2[i % 36] * blastRadius * (0.08 * noiseIntensity);

        float px = lastKnownX + ((Master_LUT_Cos[lutIndex] *  (blastRadius + noiseAmt))  * circumference);
        float py = lastKnownY + ((Master_LUT_Sin[lutIndex] *  (blastRadius + noiseAmt))  * circumferenceSin);
        
        float currentThickness = (beamGlowThicc * 0.90) * envelope;  
        if (currentThickness < 1.8) currentThickness = 1.8;
        
        TexturedBeamPoint(px, py, currentThickness, float(i)/float(numSegs), RGB(fadeR, fadeG, fadeB));
    }
    EndTexturedBeam();   
    
    StartTexturedBeam(this, ZPlane, beam_glowSprite->Index, 0.0, 1);
    for (local i = 0; i <= numSegs; i++)
    {
        int lutIndex = (i * 360) / numSegs;
        if (lutIndex > 359) lutIndex = 359; 

        float noiseAmt = sawZapNoise2[i % 36] * blastRadius * (0.08 * noiseIntensity);

        float px = lastKnownX + (Master_LUT_Cos[lutIndex] * (blastRadius + noiseAmt)) * circumference;
        float py = lastKnownY + (Master_LUT_Sin[lutIndex] * (blastRadius + noiseAmt)) * circumferenceSin;
        
        float currentThickness = beamGlowThicc * envelope;
        TexturedBeamPoint(px, py, currentThickness, float(i)/float(numSegs), RGB(fadeR, fadeG, fadeB));
    }
    EndTexturedBeam();
}

void CEffectManager::DrawEllipse()
{    
    if (!shouldEllipse) return;
    
    // Now uses the SAME blastProgress/envelope as DrawExplosion — no more mismatch
    float blastRadius = baseRadius + (baseRadius * expandMultiplier * blastProgress); 
    
    int fadeR = int(float(r) * envelope);
    int fadeG = int(float(g) * envelope);
    int fadeB = int(float(b) * envelope);

    StartTexturedBeam(this, ZPlane, tazer_lightningBeamSprite->Index, 0.0, 1);
    for (local i = 0; i <= numSegs; i++)
    {
        int lutIndex = (i * 360) / numSegs;
        if (lutIndex > 359) lutIndex = 359; 

        float noiseAmt = sawZapNoise2[i % 36] * blastRadius * (0.08 * noiseIntensity);

        float px = lastKnownX + (Master_LUT_Cos[lutIndex] * (blastRadius * 1.25 + noiseAmt)) * circumference;
        float py = lastKnownY + (Master_LUT_Sin[lutIndex] * (blastRadius * 0.45 + noiseAmt)) * circumferenceSin;
        
        float currentThickness = beamGlowThicc * envelope;  
        if (currentThickness < 1.4) currentThickness = 1.4;
        
        TexturedBeamPoint(px, py, currentThickness, float(i)/float(numSegs), RGB(fadeR, fadeG, fadeB));
    }
    EndTexturedBeam();   
    
    StartTexturedBeam(this, ZPlane, beam_glowSprite->Index, 0.0, 1);
    for (local i = 0; i <= numSegs; i++)
    {
        int lutIndex = (i * 360) / numSegs;
        if (lutIndex > 359) lutIndex = 359; 

        float noiseAmt = sawZapNoise2[i % 36] * blastRadius * (0.08 * noiseIntensity);

        float px = lastKnownX + (Master_LUT_Cos[lutIndex] * (blastRadius * 1.25 + noiseAmt));
        float py = lastKnownY + (Master_LUT_Sin[lutIndex] * (blastRadius * 0.45 + noiseAmt));
        
        float currentThickness = beamGlowThicc * envelope;
        TexturedBeamPoint(px, py, currentThickness, float(i)/float(numSegs), RGB(fadeR, fadeG, fadeB));
    }
    EndTexturedBeam();
}

void CEffectManager::DrawTrailFX()  
{                                          
                                           
} 

void CEffectManager::DrawThinSpinFX()
{
    // Sharp arc pass
    StartTexturedBeam(this, ZPlane, tazer_lightningBeamSprite->Index, 0.0, 1);
    
    for (int i = 0; i <= numSegs; i++)
    {
        float t = float(i) / float(numSegs);

        int lutIdx = (i * 360) / numSegs;
        if (lutIdx > 359) lutIdx = 359;
        float uX = Master_LUT_Cos[lutIdx] * circumference;
        float uY = Master_LUT_Sin[lutIdx] * circumferenceSin;

        // 2D rotation combining currentAngle with segment angle
        float dirX = uX * cosRot - uY * sinRot;
        float dirY = uX * sinRot + uY * cosRot;

        // Noise displacement 
        float totalRadius = baseRadius + (sawZapNoise[i % 16] * noiseIntensity);

        float px = lastKnownX + dirX * totalRadius;
        float py = lastKnownY + dirY * totalRadius;

        // Dome curve for thickness AND color
        float f = (4.0 * t * (1.0 - t) + 0.15);
        if (f > 1.0) f = 1.0;

        int dr = int(float(r) * f);
        int dg = int(float(g) * f);
        int db = int(float(b) * f);

        TexturedBeamPoint(px, py, f * 6.0 * objectScale, t, RGB(dr, dg, db));

        // Glow quad per point
        if (i < numSegs)
        {
            CQuad q;
            FillSpriteQuad(&q, tazer_lightningGlowSprite->Index, 0, 0);
            TransformQuad(&q, 0, 0.35, 0.35, px, py);
            q.blend = objectScale;
            
            for (int k = 0; k < 4; k++) q.v[k].color = RGB(dr * gMult, dg * gMult, db * gMult);
            
            AddSpriteQ(gZPlane, &q, tazer_lightningGlowSprite->Index, 0, 64);
        }
    }
    EndTexturedBeam();
}
void CEffectManager::DrawReachPoint()
{
    if (!hasReachTarget) return;
    
    int drawSegs = raySegments;   // more segments = smoother curve
    float tMax   = reachProgress;
    
    //Main Sharp Beam
    StartTexturedBeam(this, ZPlane, tazer_lightningBeamSprite->Index, 0.0, 1);
    for (local i = 0; i <= drawSegs; i++)
    {
        float t = (float(i) / float(drawSegs)) * tMax;
        
        float bx; float by;
        PointOnCurve(t, &bx, &by);
        
        // Noise still applies perpendicular to LOCAL tangent, not global perp
        // approximate tangent via nearby curve sample
        float t2 = t + 0.02;
        if (t2 > 1.0) t2 = 1.0;
        float bx2; float by2;
        PointOnCurve(t2, &bx2, &by2);
        
        float tanX = bx2 - bx;
        float tanY = by2 - by;
        float tanLen = sqrt(tanX*tanX + tanY*tanY);
        float nX = 0.0; float nY = 0.0;
        if (tanLen > 0.001) { nX = 0.0 - tanY/tanLen; nY = tanX/tanLen; }
        
        float noiseAmt = sawZapNoise[i % 16] * noiseIntensity;
        bx += nX * noiseAmt;
        by += nY * noiseAmt;
        
        float f = rayThickness * t * (1.0 - t) + 0.15;
        if (f > 1.0) f = 1.0;
        
        int dr = int(float(r) * f);
        int dg = int(float(g) * f);
        int db = int(float(b) * f);
        
        TexturedBeamPoint(bx, by, f * rayThickness, t, RGB(dr, dg, db));
        
       /* if (i % 2 != 0)// == drawSegs)
        {
            CQuad q;
            FillSpriteQuad(&q, tazer_lightningGlowSprite->Index, 0, 0);
            TransformQuad(&q, 0, 0.4, 0.4, bx, by);
            q.blend = 1;
            for (int k = 0; k < 4; k++) q.v[k].color = RGB(dr, dg, db);
            AddSpriteQ(9.9, &q, tazer_lightningGlowSprite->Index, 0, 64);
        }*/
    }
    EndTexturedBeam();
    
    //Glow
    StartTexturedBeam(this, ZPlane, beam_glowSprite->Index, 0.0, 1);
    for (local i = 0; i <= drawSegs; i++)
    {
        float t = (float(i) / float(drawSegs)) * tMax;
        
        float bx; float by;
        PointOnCurve(t, &bx, &by);
        
        // Noise still applies perpendicular to LOCAL tangent, not global perp
        // approximate tangent via nearby curve sample
        float t2 = t + 0.02;
        if (t2 > 1.0) t2 = 1.0;
        float bx2; float by2;
        PointOnCurve(t2, &bx2, &by2);
        
        float tanX = bx2 - bx;
        float tanY = by2 - by;
        float tanLen = sqrt(tanX*tanX + tanY*tanY);
        float nX = 0.0; float nY = 0.0;
        if (tanLen > 0.001) { nX = 0.0 - tanY/tanLen; nY = tanX/tanLen; }
        
        float noiseAmt = sawZapNoise[i % 16] * noiseIntensity;
        bx += nX * noiseAmt;
        by += nY * noiseAmt;
        
        float f = rayGlowThick * t * (1.0 - t) + 0.15;
        if (f > 1.0) f = 1.0;
        
        int dr = int(float(r) * f);
        int dg = int(float(g) * f);
        int db = int(float(b) * f);
        
        TexturedBeamPoint(bx, by, f * rayGlowThick, t, RGB(dr, dg, db));
        
       /* if (i % 2 != 0)// == drawSegs)
        {
            CQuad q;
            FillSpriteQuad(&q, tazer_lightningGlowSprite->Index, 0, 0);
            TransformQuad(&q, 0, 0.4, 0.4, bx, by);
            q.blend = 1;
            for (int k = 0; k < 4; k++) q.v[k].color = RGB(dr, dg, db);
            AddSpriteQ(9.9, &q, tazer_lightningGlowSprite->Index, 0, 64);
        }*/
    }
    
    EndTexturedBeam();
    
    if (reachProgress >= 1.0)
        hasReachTarget = false;
} 

void CEffectManager::FillTrail()
{     
    if (managerState == 1 && effectType != 5) return;
    if (trailAmount > 1200) trailAmount = 0; 
      
    if (targetObj!=NullObj && absFltn(targetObj->SpX) < 0.15 && absFltn(targetObj->SpY) < 0.15)  return;
    
    if (trailAmount > 0)
    {
        float dx = trailPastX[trailAmount - 1] - lastKnownX;
        float dy = trailPastY[trailAmount - 1] - lastKnownY;
    
        // Skip if position hasn't moved significantly
        if ((dx * dx + dy * dy) < 0.01) return; 
    }
    
    trailPastX[trailAmount] = lastKnownX;  
    trailPastY[trailAmount] = lastKnownY;
    trailAmount++;    
    if (trailAmount > trailMaxL)
    {
        DetractTrail();
    }
}

void CEffectManager::DetractTrail()
{
    if (trailAmount == 0) 
    {
        trailSnapped = true;
        return;
    }
    for (int i = 0; i < trailAmount - 1; i++)
    {
            trailPastX[i] = trailPastX[i + 1];
            trailPastY[i] = trailPastY[i + 1];
    }
    trailAmount--; 
} 

void CEffectManager::InsertInTrail(float x, float y)
{     
    if (managerState == 1 && effectType != 5) return;
    if (trailAmount > 1200) trailAmount = 0; 
      
    trailPastX[trailAmount] = x;  
    trailPastY[trailAmount] = y;
    trailAmount++;    
    if (trailAmount > trailMaxL)
    {
        DetractTrail();
    }
}

float CEffectManager::TrailConsistent(int i)
{
    float t = float(i) / float(trailAmount);
    if (t > 1.0) t = 1.0;
    if (t < 0.0) t = 0.0;

    // Dome curve: 0 at head/tail, peak in the middle
    // This is intentionally kept separate from thickness scaling
    float f = 4.0 * t * (1.0 - t) + 0.15;
    if (f > 1.0) f = 1.0;

    return f;
}

float CEffectManager::TrailEnvelope(int i)
{
    float t = float(i) / float(trailAmount - 1);
    if (t > 1.0) t = 1.0;
    if (t < 0.0) t = 0.0;

    // Parabolic dome curve: peak in middle, lower at ends
    float f = 4.0 * t * (1.0 - t) + 0.15;
    if (f > 1.0) f = 1.0;

    return f;
}

float absFltn(float f)
{
  if (f < 0.0) return -f;
  else
  return f;
}

void CEffectManager::DrawTrail()
{      //  trailProgressive = true;     
    StartTexturedBeam(this, ZPlane, tazer_lightningBeamSprite->Index, 0.0, 1);
    for (local i = 0; i < trailAmount; i++)
    {
        float t = float(i + 1) / float(trailAmount + 1);
        float f;

        if (trailWavy)
        {
            f = TrailEnvelope(i);
        }       
        else if (midThickEdgeThin)
        {                      
            f = TrailConsistent(i);
        }
        else
        {
            f = t; 
            if (f < 0.1) f = 0.1;
        } 
        if (StraightTrail)
        {
           t = rayThickness;
           f = t;
        }
        
        int dr = int(float(r) * f);
        int dg = int(float(g) * f);
        int db = int(float(b) * f);

        TexturedBeamPoint(trailPastX[i], trailPastY[i], f * rayThickness, t, RGB(dr, dg, db));
    }

    EndTexturedBeam();

    // --- Glow pass — same math, different sprite/thickness scale ---
    StartTexturedBeam(this, ZPlane, beam_glowSprite->Index, 0.0, 1);

    for (local i = 0; i < trailAmount; i++)
    {
        float t = float(i + 1) / float(trailAmount + 1);
        float f;
        
        if (trailWavy)
        {
            // Original wavy/thin-reversal look, but properly clamped this time
            f = TrailEnvelope(i);
        }   
        else if (midThickEdgeThin)
        {                      
            f = TrailConsistent(i);
        }
        else
        {
            f = t; 
            if (f < 0.1) f = 0.1;
        }
        if (StraightTrail)
        {
           t = rayGlowThick;
           f = t;
        }

        int dr = int(float(r) * f);
        int dg = int(float(g) * f);
        int db = int(float(b) * f);

        TexturedBeamPoint(trailPastX[i], trailPastY[i], f * rayGlowThick, t, RGB(dr, dg, db));
    }

    EndTexturedBeam();
}