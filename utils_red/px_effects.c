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
        float ang = (float(i) / 360.0) * 6.28318;
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
    ownerLess  = (targetObj == NullObj);

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
    explosionTimer   = 0.0; // Coded so that 15 is max, itll look weird after
    vanishSpeed      = 1.0; // No lower than 0.5 / -0.5
    expLimit         = 15;  // Extend explosion anim
    expandMultiplier = 3.0; // how far blastRadius grows beyond baseRadius, was hardcoded
    fadeInFrac       = 0.0; // 0.0 = no fade-in (old behavior). e.g. 0.15 = ramp up over first 15% of duration
    blastProgress    = 0.0; // computed once per frame, read by both draw functions
    envelope         = 1.0; // brightness/alpha multiplier after fade-in is applied

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

    // --- Depth & Engine Init ---
    ZPlane  = 9.9;
    gZPlane = ZPlane - 0.1;

    super(Root, GS);
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
}

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
        if (ZPlane < 1.1) ZPlane = 1.1;
        if (effectType == 0)       UpdateAndDraw();
        else if (effectType == 1)  DrawSpinFX();              
        else if (effectType == 2)  DrawRingFX();
        else if (effectType == 3)  DrawTrailFX();
        else if (effectType == 4)  managerState = 1;  // Explosion    
        else if (effectType == 5)  managerState = 1;  // Pulsate
        else if (effectType == 6)  DrawThinSpinFX();    
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
        else if (managerState == 1 && effectType == 5)
        {                                                                                                 
                drawGlow();
        }
    }
    if (Type == M_FRAME)
    {
        Calc();
        
        if (bufferChange && gframe > bufferFrames )
        {
            editParams(expType, expSeg, expRad, expNoise, expR, expG, expB, buffVanish);
            bufferChange = false;
        }

        if (targetObj == NullObj) 
        {
            nullFrameCount++;
            
            if (effectType == 5 && nullFrameCount == 5 )
            {
                managerState = 1;
                effectType = 4;
                explosionTimer = 0;
            }

            if (nullFrameCount == 5 && !ownerLess)
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
            nullFrameCount--;
        }
    
        if (managerState == 1) 
        {
            explosionTimer = explosionTimer + vanishSpeed;
        }
        
        // Total frames the blast lasts
        if (explosionTimer > expLimit && effectType != 5 && frameCount > freeAfter)
        {
            Free (true); // Manager successfully cleans itself up
            return;
        }
        if (freeAfter != 0 && frameCount > freeAfter)  //unconditional 
        {
            Free (true); 
            return;
        }    
        else if (explosionTimer > expLimit && effectType == 5)
        {
            explosionTimer = 0;
        }
    }
    super;
}

void CEffectManager::Calc()//SLANG for calculator by the way.
{    
     if (targetObj != NullObj)
     {
          lastKnownX = targetObj->PosX;
          lastKnownY = targetObj->PosY;            
     }
     frameCount++;
     cosRot = cos(currentAngle);
     sinRot = sin(currentAngle);   
     
     if (effectType == 1 || effectType == 6) currentAngle += shiftAngle;
     currentAngle = NormalizeAngle(currentAngle);

     // --- Unified explosion progress, driven by expLimit instead of hardcoded 15 ---
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

            uX = Master_LUT_Cos[lutIndex2];
            uY = Master_LUT_Sin[lutIndex2];

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

            uX = Master_LUT_Cos[lutIndex2];
            uY = Master_LUT_Sin[lutIndex2];

            liveNoise = sawZapNoise2[j % 22] * noiseIntensity;
            totalRadius = baseRadius + liveNoise;

            rotatedX = (uX * cosRot) - (uY * sinRot);
            rotatedY = (uX * sinRot) + (uY * cosRot);

            px = lastKnownX + (rotatedX * totalRadius);
            py = lastKnownY + (rotatedY * totalRadius);

            texCoord = float(j) / float(numSegs);
            
            // 1. Extremely thin, sharp line instead of a smoothed dome curve 

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

        float uX = Master_LUT_Cos[lutIndex];
        float uY = Master_LUT_Sin[lutIndex];

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

        float uX = Master_LUT_Cos[i];
        float uY = Master_LUT_Sin[i];

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

        float uX = Master_LUT_Cos[lutIndex2];
        float uY = Master_LUT_Sin[lutIndex2];

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

        float uX = Master_LUT_Cos[lutIndex2];
        float uY = Master_LUT_Sin[lutIndex2];

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

        float uX = Master_LUT_Cos[lutIndex2];
        float uY = Master_LUT_Sin[lutIndex2];

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

        float px = lastKnownX + (Master_LUT_Cos[lutIndex] * (blastRadius + noiseAmt));
        float py = lastKnownY + (Master_LUT_Sin[lutIndex] * (blastRadius + noiseAmt));
        
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

        float px = lastKnownX + (Master_LUT_Cos[lutIndex] * (blastRadius + noiseAmt));
        float py = lastKnownY + (Master_LUT_Sin[lutIndex] * (blastRadius + noiseAmt));
        
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

        float px = lastKnownX + (Master_LUT_Cos[lutIndex] * (blastRadius * 1.25 + noiseAmt));
        float py = lastKnownY + (Master_LUT_Sin[lutIndex] * (blastRadius * 0.45 + noiseAmt));
        
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
        float uX = Master_LUT_Cos[lutIdx];
        float uY = Master_LUT_Sin[lutIdx];

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

   
override void CMissile::CMissile(CObject* parent,CWeaponLaunch* ldata,CShootDesc* sdata)
{
  super;
 
  //CEffectManager* zapFX = new CElectricSpriteManager(this, PosX, PosY, 2, 30, 60, 0.001, 110, 190, 255) ;
 // if (zapFX!=NullObj)
 // zapFX->shouldExplode = true;
    //createEffectExplosion(PosX, PosY, 30);
    //createEffectExplosion(PosX, PosY, 30, 30, 0.1, 200, 150, 90, true, 16, 0.51, 0.0) ;
}
  