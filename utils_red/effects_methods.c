require weapon_bowlingball;
CEffectManager * createElectricImpact(CGObject * target, float x, float y, float radius)
{
        CEffectManager* zapFX = new CEffectManager(NullObj, x, y, 4, 30, radius, 0.45, 160, 220, 255) ;
        if (zapFX!=NullObj)
        {
                zapFX->ZPlane = 3.1;  
                zapFX->gZPlane = 3.09;
                zapFX->shouldExplode = true;
                zapFX->expLimit = 15;    
                zapFX->freeAfter = 16;
                zapFX->vanishSpeed = 0.55;
                zapFX->beamThiccness = 14.0; //Everlapping quads in a small radius give a star shape that looks cool        
                zapFX->beamGlowThicc = 25.0;     
                return zapFX;
        } 
        return NullObj;
}

CEffectManager * attachPulseEffect(CGObject * target, float radius, int r, int g, int b)
{ 
     if (target == NullObj) return NullObj;
     CEffectManager* zapFX = new CEffectManager(target, target->PosX, target->PosY, 5, 20, radius, 0.001, r, g, b) ;   
     if (zapFX!=NullObj)
     {
          zapFX->ZPlane  = 3.1;  
          zapFX->gZPlane = 3.09;
          zapFX->shouldExplode = true;                 
          return zapFX;
     }
     return NullObj;
}

void drawSparkEle(fixed Posx, fixed Posy, fixed zPlane, float Rotation, float size)   //from bowling ball
{		                                                                                                
		if(gSparkCycle <= 1.0)
		{      
		        CQuad q;
			// Spark effect                                                               
			FillSpriteQuad(&q, bowlingballsparkSprite->Index, 0, 0);
			TransformQuad(&q, Rotation, q.v.tx * size, q.v.ty * size, Posx, Posy);
			q.blend = 1;
			for(local j=0 ; j<4 ; j+=1) q.v[j].color = RGB(255,255,255);
			Root->AddSpriteQ(zPlane, &q, bowlingballsparkSprite->Index, gSparkCycle, 64);
		}
}

void drawGlowCircle(fixed x, fixed y, fixed zplane, float size int rgb)
{
		CQuad q;
		local j;
	
		//Glow effect 
	        GlowSize = size;
		FillSpriteQuad(&q, bowlingballglowSprite->Index, 0, 0);
		TransformQuad(&q, 0, GlowSize, GlowSize, x, y);
		q.blend = 1;
		for(j=0 ; j<4 ; j+=1) q.v[j].color = rgb;
		
		for(j=0 ; j<3 ; j+=1)
			Root->AddSpriteQ(zplane, &q, bowlingballglowSprite->Index, 0, 64);
}

void drawEleTrail(fixed PosX, fixed PosY)
{
	GlowSize = RandomFloat(0.3, 0.4);
	
        p0 = new PxParticle(bowlingballglowSprite->Index, PosX, PosY);
	p0->SetBlendMode(1);
	p0->SetStartColor(80,110,128);
	p0->SetEndColor(0,0,0);
	p0->SetLifeTime(30);
	p0->SetRandomVelocity(0, 0.3);
	p0->SetAirResistance(0.0);
	p0->SetAlpha(255);
	p0->SetStartSize(0.3,0.3);
	p0->SetEndSize(0.5,0.5);
}

CEffectManager* createElectricExplosion(fixed ePosX, fixed ePosY, float radius)
{
      CEffectManager* newEffect = new CEffectManager(NullObj, ePosX, ePosY, 4, int(radius * 1.20), radius, 1.1, 100, 150, 220) ;
      newEffect->vanishSpeed = 0.55;
      newEffect->explosionTimer = 0;     
      newEffect->expLimit = 18; 
      newEffect->shouldExplode = true;
      newEffect->shouldEllipse = true;    
            
      newEffect->custExp = false; 
      newEffect->managerState = 1;
      newEffect->effectType = 4; 
      
      newEffect->BufferChange(4, int(radius * 1.30), radius * 0.85, 1.8, 120, 170, 240, 16, -0.85);  
      newEffect->freeAfter = newEffect->frameCount + 24;       
      newEffect->SetBeamThickness(radius * 0.70);  
      newEffect->SetBeamGlowThickness(radius * 0.90);  
      return  newEffect;
}   

CEffectManager* createEffectExplosion(fixed ePosX, fixed ePosY, float thickness, float radius, float noise, int r, int g, int b, bool particles, int duration, float fadeOut, float fadeIn)
{
      CEffectManager* newEffect = new CEffectManager(NullObj, ePosX, ePosY, 4, int(radius * 1.20), radius, noise, r, g, b) ;
      newEffect->vanishSpeed = fadeOut;
      newEffect->explosionTimer = 0;     
      newEffect->expLimit = duration; 
      newEffect->shouldExplode = true;
      newEffect->shouldEllipse = true;    
            
      newEffect->custExp = false; 
      newEffect->managerState = 1;
      newEffect->effectType = 4; 
      if (fadeIn != 0.0)
      newEffect->BufferChange(4, int(radius * 1.15), radius * 0.85, noise * 1.5, r, g, b, 16, fadeIn);  
      else  
      newEffect->BufferChange(4, int(radius * 1.15), radius * 0.85, 0.01, 0, 0, 0, 16, 0.0);  
      
      newEffect->freeAfter = newEffect->frameCount + duration;       
      newEffect->SetBeamThickness(thickness);  
      newEffect->SetBeamGlowThickness(thickness * 3);  
      
      if (particles)
      {
            //drawExpSmokeParticles(ePosX, ePosY, radius / 2.4, 150, 170, 190, 1);
            local smokeCount = radius / 2.4;
            local tier = (radius / 15) ;
            for (local type = 0; type < 2; type++) 
            {
            for (local i = 0; i < smokeCount; i++)
            {
                int smokeSprite;
                if (type == 0) 
                {
                    smokeSprite = RandomInt(609, 612);
                } 
                else 
                {
                    smokeSprite = RandomInt(613, 615);
                }
                
                if (tier < 2 &&  smokeSprite == 615)  smokeSprite = RandomInt(613,614);
                if (tier < 2 && (smokeSprite == 612 || smokeSprite == 613)) smokeSprite = RandomInt(609, 610);
                
                local pSmoke = new PxParticle(smokeSprite, ePosX, ePosY);
                if (pSmoke != NullObj)
                {
                    if (smokeSprite == 612)
                    pSmoke->SetLifeTime(RandomInt(40,50));      
                    else if (smokeSprite == 611)
                    pSmoke->SetLifeTime(RandomInt(35,42)); 
                    else if (smokeSprite == 610)
                    pSmoke->SetLifeTime(RandomInt(30,36));  
                    else if (smokeSprite == 609)
                    pSmoke->SetLifeTime(RandomInt(22,28)); 
                    
                    pSmoke->SetAnimSpeed(0.0);
                    
                    pSmoke->SetStartSize(1.0, 1.0); 
                    pSmoke->SetEndSize(1.0, 1.0);
                    local spread = tier / 1.4;
                    float vx = RandomFloat(-4.0 * spread, 4.0 * spread);
                    float vy = RandomFloat(-6.0 * spread, 2.0 * spread); 
                    pSmoke->SetVelocity(vx, vy);
                    
                    pSmoke->SetAirResistance(0.12); 
                    pSmoke->SetStartAlpha(255);
                    pSmoke->SetEndAlpha(255);
                    pSmoke->SetColor( r, g, b);
                }
            }
        }
        }
        return newEffect;
}

CEffectManager *CEffectManager::CreateRainbowExplosion(fixed ePosX, fixed ePosY, float thickness, float radius, float noise, int duration, float fadeOut, float fadeIn)
{
local expEff = createEffectExplosion( ePosX,  ePosY,  thickness,  radius,  noise,  255,  255,  255,  false,  duration,  fadeOut,  fadeIn) ;
expEff->isRainbow = true;

expEff->rainbowSpeed = 0.15;        

return expEff;
}

void effects::FirstFrame()
{
	gSparkAnimSpeed = 0.02;
	gSparkPauseDuration = 0.1;
	gSparkCycle = 0.0;
	gSparkCycleDuration = 1.0 + gSparkPauseDuration;
}
float gSparkAnimSpeed;
float gSparkCycleDuration;
float gSparkCycle;

override void CTurnGame::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{   
  super;
  if (Type == M_FRAME)
  {     
      if (gframe %  2 == 0) GenerateZapNoise ();  // 
      if (gframe %  3 == 0) GenerateZapNoise2(); // Static circle noise.              
 		gSparkCycle = gSparkCycle + gSparkAnimSpeed;
		if(gSparkCycle > gSparkCycleDuration) gSparkCycle = gSparkCycle - gSparkCycleDuration; 
  }
}