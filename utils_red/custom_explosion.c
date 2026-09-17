require utils, pxparticles;          

void CMissile::applyCustomExplosionParams(int expFlags1, int expDmg1, int expPush1, int expDestroyR1, bool expShouldDestroy1, bool expParticles1, bool expSound1, bool expTaze1, int expSoundNum1)
{
 customExplosion   = true;
 expFlags    = expFlags1;
 expDmg      = expDmg1;
 expPush     = expPush1;
 expDestroyR = expDestroyR1;
 expShouldDestroy = expShouldDestroy1;
 expParticles     = expParticles1;
 expSound         = expSound1;
 expTaze          = expTaze1;
 expSoundNum      = expSoundNum1;
}

int do_custom_explosion(CGObject * sender, int flags, fixed x, fixed y, int dmg, int pushPower, fixed destroyRadius, bool destroy, bool particles, bool defSound, bool taze, int esound)
{      
      if (defSound) 
      {
            PlayGlobalSound(RandomInt(69,71), 5, 1.0, 1.0);
      }
      else   
            PlayGlobalSound(esound, 5, 1.0, 1.0);
       
      ePosX = x;
      ePosY = y;
      local targetsHit = 0; 
      local radiusss = (dmg + 2) * 2;
      local tier = 0;  
                  
      if (dmg > 0 && radiusss > 0)
      {
      if (#Flowers)
      {
            if(FlowerMan!=NullObj)
            {
                  local fdCheck = (radiusss - 2) * 0.75;
                  if (destroyRadius > radiusss)
                        fdCheck = destroyRadius;
                  if (fdCheck > 0)      
                  FlowerMan->CheckFlowerDamage( x, y, fdCheck); 
            }
      }
      for (local i = 0; i < Env->Objs.Count; i++)
      {
            local obj = CGObject(Env->Objs.Objs[i]);
            if (obj == NullObj) continue;
            if (obj is PxDeadWorm == true) continue;
            
            if (obj != NullObj && (flags == -1 || MatchCollisionGroup(obj, flags) ) )
            {
                  float oPosX = float(obj->PosX);
                  float oPosY = float(obj->PosY);
                  float dx = oPosX - ePosX;
                  float dy = oPosY - ePosY;     
                 
                  float dist = sqrt(dx * dx + dy * dy);

                  // if dist > r: nothing
                  if (dist >= radiusss) continue;

                  // falloff = (r - dist) / r
                  float falloff = (radiusss - dist) / radiusss;
            
                  // hp = damage * falloff
                  int dmgDone = int(float(dmg) * falloff);

                  // push = (hp / 5) * dir * (strength / 100) / mass
                  // Assuming default worm mass is 1.0, strength is pushPower
                  fixed pushX = 0.0;
                  fixed pushY = 0.0;

                  if (dist > 0.001 && dmgDone > 0) 
                  {
                        float pushForce = (float(dmgDone) / 5.0) * (float(pushPower) / 100.0);

                        pushX = (dx / dist) * pushForce;
                        pushY = (dy / dist) * pushForce;
                  } 
                  if (obj->ClType == OC_Cross || (obj->ObjState == WS_FROZEN && !taze)) { obj->SpY = pushY; continue; }
                  
                  if (int(dmgDone) == 0) continue;

                  CMessageData msg;
                  msg.params[0]  = 0;
                  msg.fparams[1] = ePosX; 
                  msg.fparams[2] = ePosY; 
                  msg.fparams[3] = pushX;
                  msg.fparams[4] = pushY;
                  msg.params[5]  = dmgDone;
                  msg.params[6]  = 0;
                  if (#ELECTRIC_PLUGIN)
                  {
                      if (taze && obj->ClType == OC_Mine)
                      {
                         amine = CMine(obj);
                         amine->TazeMine();
                         amine->linkedEffect2->SetTargetDynamic(ePosX,ePosY, 2,0.079, RandomFloat(-5.0,5.0));
             
                      }
                  }
                  if (obj is CWorm == true)  
                  {
                        local worm = CWorm(obj);
                        if (worm == NullObj) continue; //lol
                        if (worm != NullObj)
                        {      
                              if (taze)                   
                              {            
                                    msg.fparams[3] = 0;
                                    msg.fparams[4] = 0;          
                                    if (worm->ObjState == WS_FROZEN) worm->SetState(WS_IDLE);
                                    worm->Message(sender, M_GUNEXP, 1032, &msg);
                                    worm->TazeArtificial(20 + 11 * tier * 1.5 , pushX, pushY);
                              } 
                              else
                              worm->Message(sender, M_GUNEXP, 1032, &msg);
                              targetsHit++;   
                        };            
                  }
                  else 
                  { 
                              obj->Message(sender, M_GUNEXP, 1032, &msg);   
                              targetsHit++;   
                  }
            }
      }
      
      }
      if (destroy)
      {
            GG->land->MakeHole(destroyRadius, ePosX, ePosY);
      }  
      if (sender->ClType == OC_Missile || sender is CMissile == true || sender is CMine == true ||  sender is COilDrum == true)
      {
            local mis = CMissile(sender); 
            mis->OnCustomExplosion(dmg, pushPower, targetsHit);   
      }                                            
      if (particles)
      {
        //Explosion Tier (0 to 3 max)
        int tier = (dmg / 22) - 1; 
        if (tier > 3) tier = 3;
        if (tier < 0) tier = 0;

        int flareCount = (dmg / 15) + 1; 

        // Fast fading flare
        for (local i = 0; i < flareCount; i++)
        {
            local pFlare = new PxParticle(85, ePosX, ePosY);
            if (pFlare != NullObj)
            {
                pFlare->SetLifeTime(24); 
                pFlare->SetAnimSpeed(0.0);
                pFlare->SetVelocity(RandomFloat(-5.0, 5.0), RandomFloat(-6.0, 2.0));
                pFlare->SetStartSize(1.0, 1.0);
                pFlare->SetEndSize(1.0, 0.8);
                pFlare->SetStartAlpha(255);
                pFlare->SetEndAlpha(255);
            }
        }
        flareCount -= 1;
        // CMissile flare
        if (flareCount != 0)
        for (local i = 0; i < flareCount; i++)
        {
            local flare = new CFlare(ePosX, ePosY, 84, RandomInt(50,120), RandomFloat(0.7,1.0));
            if (flare!=NullObj) flare->hitFrame = -1000;   //avoid saw collissions
        }                    

       
        // Smokes
        int smokeCount = int(float(dmg) * 0.3);
        
        local tierMultiplier = tier;
        if (tier == 0) tierMultiplier = 1;
        
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
                
                int sizeIndex = 1;

                if (smokeSprite <= 612) 
                {
                    sizeIndex = smokeSprite - 608; 
                } 
                else 
                {
                    sizeIndex = smokeSprite - 612; 
                }
                
                local pSmoke = new PxParticle(smokeSprite, ePosX, ePosY);
                if (pSmoke != NullObj)
                {     
                    float upVel = -7.0 + (float(sizeIndex) * 0.5);  
                    
                    //pSmoke->SetVelocity(RandomFloat(-1.5, 1.5), RandomFloat(-5.5, -3.5));
                    pSmoke->SetVelocity(RandomFloat(-1.5, 1.5), RandomFloat(upVel - 1.0, upVel));
                    pSmoke->SetLifeTime(RandomInt(15 + (sizeIndex * 7), 20 + (sizeIndex * 8))); 

                    pSmoke->SetAnimSpeed(0.0);
                    pSmoke->SetStartSize(1.0, 1.0); 
                    pSmoke->SetEndSize(1.0, 1.0);

                    pSmoke->SetMotionRandomness(0.35);

                    pSmoke->SetAirResistance(0.04 + (float(sizeIndex) * 0.02));            

                    pSmoke->SetStartAlpha(255);
                    pSmoke->SetEndAlpha(200);   
                    if (smokeSprite >= 613)
                    {       
                        pSmoke->SetVelocity(RandomFloat(-1.5, 1.5), RandomFloat(-2.5, 2.5)); 
                        pSmoke->SetAirResistance(0.08);    
                    }
                }
            }
        }
        
        //Ellipse (604+tier)
        local pEll = new PxParticle(604 + tier, ePosX, ePosY);
        if (pEll != NullObj)
        {
            int ellFrames = 10;
            if (tier == 0) ellFrames = 22;
            else if (tier == 1) ellFrames = 20;
            
            pEll->SetLifeTime(ellFrames * 1.5);
            pEll->SetAnimSpeed(0.0);
            pEll->SetStartSize(1.0, 1.0);
            pEll->SetEndSize(1.0, 1.0);
            pEll->SetStartAlpha(255);
            pEll->SetEndAlpha(255);
        }
        
        local pCirc = new PxParticle(600 + tier, ePosX, ePosY);
        if (pCirc != NullObj)
        {
            int circFrames;
            if (tier <= 1)
            {
                circFrames = 8;
            }
            else 
            {
                circFrames = 4;
            }
            pCirc->SetLifeTime(circFrames * 1.5); 
            
            pCirc->SetAnimSpeed(0.0);
            pCirc->SetStartSize(1.0, 1.0);
            pCirc->SetEndSize(1.0, 1.0); 
            pCirc->SetStartAlpha(255);
            pCirc->SetEndAlpha(255); 
        }
        
        if (tier >= 1)
        {
            int kachowSprite = RandomInt(596, 599);
            local pComic = new PxParticle(kachowSprite, ePosX, ePosY);
            if (pComic != NullObj)
            {
                // Poof(18), Foom(20), Pow/Biff(12)
                int comicFrames = 12;
                if (kachowSprite == 598) comicFrames = 18; 
                else if (kachowSprite == 596) comicFrames = 20; 
                
                pComic->SetLifeTime(comicFrames * 1.5);
                pComic->SetAnimSpeed(0.0);
                pComic->SetStartSize(1.0, 1.0);
                pComic->SetEndSize(1.0, 1.0);
                pComic->SetVelocity(0.0, 0.0); 
                pComic->SetStartAlpha(255);
                pComic->SetEndAlpha(255);
            }
      }

      }
      return tier;
}  

void do_explosion_particles(float x, float y, int tier, bool defSound, bool kachow, bool circle, bool ellipse, bool smoke, bool flare, int r, int g, int b)
{   
        local ePosX = x;
        local ePosY = y;
                  
        //Explosion Tier (0 to 3 max)    
        tierMultiplier = tier;
        if (tier > 3) tier = 3;
        if (tier < 0) tier = 0;
        tierMultiplier = tier;
        
        int red = 255; int blue = 255; int green = 255;
              
        if (r != 0 && g != 0 && b != 0)
        {
                red   = r;
                green = g;  
                blue  = b;          
        }
        
        int testA = 250;  
        int ogR = red;
        int ogG = green;
        int ogB = blue;      
        
        int ARGBtest = ARGB(250,ogR,ogG,ogB);
        
        stripARGB(ARGBtest,&testA,&ogR,&ogG,&ogB);
        
        int smokeR = ogR;   int smokeG = ogG;   int smokeB = ogB;
        mult_r_g_b(1.0 - float(tierMultiplier) / 12.0, &smokeR, &smokeG, &smokeB);    
        
        int dsmokeR = ogR;  int dsmokeG = ogG;  int dsmokeB = ogB;
        mult_r_g_b(0.9 + float(tierMultiplier) / 10.0, &dsmokeR, &dsmokeG, &dsmokeB);  
        
        int expR = ogR;     int expG = ogG;     int expB = ogB;
        mult_r_g_b(1.1 + float(tierMultiplier) / 10.0, &expR,&expG,&expB);
        
        if (defSound) 
        {
                PlayGlobalSound(RandomInt(69,71), 5, 1.0, 1.0);
        }    
        
        if (tierMultiplier <= 0) tierMultiplier = 1;
        
        if (flare)
        {
        int flareCount = tierMultiplier * 1.5 + 1; 

        // Fast fading flare
        for (local i = 0; i < flareCount; i++)
        {
            local pFlare = new PxParticle(85, ePosX, ePosY);
            if (pFlare != NullObj)
            {
                pFlare->SetLifeTime(24); 
                pFlare->SetAnimSpeed(0.0);
                pFlare->SetVelocity(RandomFloat(-5.0, 5.0), RandomFloat(-6.0, 2.0));
                pFlare->SetStartSize(1.0, 1.0);
                pFlare->SetEndSize(1.0, 0.8);
                pFlare->SetStartAlpha(255);
                pFlare->SetEndAlpha(255);
            }
        }
        flareCount -= 1;
        // CMissile flare
        if (flareCount != 0)
        for (local i = 0; i < flareCount; i++)
        {
            local xflare = new CFlare(ePosX, ePosY, 84, RandomInt(50,120), RandomFloat(0.8,1.0));
        }                    
        }
        if (smoke)
        {
        // Smokes
        int smokeCount = 14 * (tierMultiplier * 0.5);
        
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
                
                int sizeIndex = 1;

                if (smokeSprite <= 612) 
                {
                    sizeIndex = smokeSprite - 608; 
                } 
                else 
                {
                    sizeIndex = smokeSprite - 612; 
                }
                
                local pSmoke = new PxParticle(smokeSprite, ePosX, ePosY);
                if (pSmoke != NullObj)
                {     
                    float upVel = -7.0 + (float(sizeIndex) * 0.5);  
                    
                    //pSmoke->SetVelocity(RandomFloat(-1.5, 1.5), RandomFloat(-5.5, -3.5));
                    pSmoke->SetVelocity(RandomFloat(-2.0, 2.0), RandomFloat(upVel - 1.0, upVel));
                    pSmoke->SetLifeTime(RandomInt(20.0 + (sizeIndex * 7), 20 + (sizeIndex * 8))); 

                    pSmoke->SetAnimSpeed(0.0);
                    pSmoke->SetStartSize(1.0, 1.0); 
                    pSmoke->SetEndSize(1.0, 1.0);


                    pSmoke->SetAirResistance(0.04 + (float(sizeIndex) * 0.02));            

                    pSmoke->SetStartAlpha(255);
                    pSmoke->SetEndAlpha(200);   
                    if (smokeSprite < 613)
                    {
                        pSmoke->SetColor(smokeR, smokeG, smokeB); 
                        pSmoke->SetMotionRandomness(1.0); 
                    }
                    else
                    {       
                        pSmoke->SetVelocity(RandomFloat(-1.5, 1.5), RandomFloat(-2.3, 2.3)); 
                        pSmoke->SetAirResistance(0.04 + (float(sizeIndex) * 0.02)); 
                        pSmoke->SetColor(dsmokeR, dsmokeG, dsmokeB);   
                        pSmoke->SetMotionRandomness(3.0);  
                    }
                }
            }
        }
        }
        if (ellipse)
        {
        //Ellipse (604+tier)
        local pEll = new PxParticle(604 + tier, ePosX, ePosY);
        if (pEll != NullObj)
        {
            int ellFrames = 10;
            if (tier == 0) ellFrames = 22;
            else if (tier == 1) ellFrames = 20;
            
            pEll->SetLifeTime(ellFrames * 1.5);
            pEll->SetAnimSpeed(0.0);
            pEll->SetStartSize(1.0, 1.0);
            pEll->SetEndSize(1.0, 1.0);
            pEll->SetStartAlpha(255);
            pEll->SetEndAlpha(255);
            pEll->SetColor(expR, expG, expB);
        }
        }
        if (circle)
        {
        local pCirc = new PxParticle(600 + tier, ePosX, ePosY);
        if (pCirc != NullObj)
        {
            int circFrames;
            if (tier <= 1)
            {
                circFrames = 8;
            }
            else 
            {
                circFrames = 4;
            }
            pCirc->SetLifeTime(circFrames * 1.5); 
            
            pCirc->SetAnimSpeed(0.0);
            pCirc->SetStartSize(1.0, 1.0);
            pCirc->SetEndSize(1.0, 1.0); 
            pCirc->SetStartAlpha(255);
            pCirc->SetEndAlpha(255);  
            pCirc->SetColor(expR, expG, expB);
        }
        }
        
        if (kachow)
        {
            int kachowSprite = RandomInt(596, 599);
            local pComic = new PxParticle(kachowSprite, ePosX, ePosY);
            if (pComic != NullObj)
            {
                // Poof(18), Foom(20), Pow/Biff(12)
                int comicFrames = 12;
                if (kachowSprite == 598) comicFrames = 18; 
                else if (kachowSprite == 596) comicFrames = 20; 
                
                pComic->SetLifeTime(comicFrames * 1.5);
                pComic->SetAnimSpeed(0.0);
                pComic->SetStartSize(1.0, 1.0);
                pComic->SetEndSize(1.0, 1.0);
                pComic->SetVelocity(0.0, 0.0); 
                pComic->SetStartAlpha(255);
                pComic->SetEndAlpha(255);
            }
      }
}

//
void drawCustomExplosionParticles(float ePosX, float ePosY, int tier, int flareCount, int smokeCount, int expIndex, int ellIndex, int flareIndex, int smokeIndex, int smokeIndex2, int darksmIndex, int darksmIndex2, bool puff)
{        
        //Explosion Tier (0 to 3 max)
        if (tier > 3) tier = 3;
        if (tier < 0) tier = 0;

        if (flareIndex != 0)
        for (local i = 0; i < flareCount; i++)
        {
            local pFlare = new PxParticle(flareIndex, ePosX, ePosY);
            if (pFlare != NullObj)
            {
                // Run fast: 1 tick per frame
                pFlare->SetLifeTime(24); 
                pFlare->SetAnimSpeed(0.0);
                pFlare->SetVelocity(RandomFloat(-5.0, 5.0), RandomFloat(-6.0, 2.0));
                pFlare->SetStartSize(1.0, 1.0);
                pFlare->SetEndSize(1.0, 1.0);
                pFlare->SetStartAlpha(255);
                pFlare->SetEndAlpha(255);
            }
        }
        flareCount -= 1;
        // CMissile flare
        if (flareCount != 0 && flareIndex != 0) 
        for (local i = 0; i < flareCount; i++)
        {
            
            local zflare = new CFlare(ePosX, ePosY, flareIndex, RandomInt(180,290), RandomFloat(0.7,1.0));
        }                    
       
        // Smokes
        local tierMultiplier = tier;
        if (tier == 0) tierMultiplier = 1;
        
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
                
                int sizeIndex = 1;

                if (smokeSprite <= 612) 
                {
                    sizeIndex = smokeSprite - 608; 
                } 
                else 
                {
                    sizeIndex = smokeSprite - 612; 
                }
                
                local pSmoke = new PxParticle(smokeSprite, ePosX, ePosY);
                if (pSmoke != NullObj)
                {     
                    float upVel = -7.0 + (float(sizeIndex) * 0.5);  
                    
                    //pSmoke->SetVelocity(RandomFloat(-1.5, 1.5), RandomFloat(-5.5, -3.5));
                    pSmoke->SetVelocity(RandomFloat(-1.5, 1.5), RandomFloat(upVel - 1.0, upVel));
                    pSmoke->SetLifeTime(RandomInt(15 + (sizeIndex * 7), 20 + (sizeIndex * 8))); 

                    pSmoke->SetAnimSpeed(0.0);
                    pSmoke->SetStartSize(1.0, 1.0); 
                    pSmoke->SetEndSize(1.0, 1.0);

                    pSmoke->SetMotionRandomness(0.35);

                    pSmoke->SetAirResistance(0.04 + (float(sizeIndex) * 0.02));            

                    pSmoke->SetStartAlpha(255);
                    pSmoke->SetEndAlpha(200);   
                }
            }
        }
        
        //Ellipse (604+tier)
        if (ellIndex != 0)
        local pEll = new PxParticle(ellIndex, ePosX, ePosY);
        if (pEll != NullObj)
        {
            int ellFrames = 28;
            if (tier == 0) ellFrames = 20;
            else if (tier == 1) ellFrames = 22;
            
            pEll->SetLifeTime(ellFrames * 1.5);
            pEll->SetAnimSpeed(0.0);
            pEll->SetStartSize(1.0, 1.0);
            pEll->SetEndSize(1.0, 1.0);
            pEll->SetStartAlpha(255);
            pEll->SetEndAlpha(255);
        }
        
        if (expIndex != 0)
        local pCirc = new PxParticle(expIndex, ePosX, ePosY);
        if (pCirc != NullObj)
        {
            int circFrames;
            if (tier <= 1)
            {
                circFrames = 8;
            }
            else 
            {
                circFrames = 4;
            }
            pCirc->SetLifeTime(circFrames * 1.5); 
            
            pCirc->SetAnimSpeed(0.0);
            pCirc->SetStartSize(1.0, 1.0);
            pCirc->SetEndSize(1.0, 1.0); 
            pCirc->SetStartAlpha(255);
            pCirc->SetEndAlpha(255); 
        }
        
        if (tier >= 1 && puff)
        {
            int kachowSprite = RandomInt(596, 599);
            local pComic = new PxParticle(kachowSprite, ePosX, ePosY);
            if (pComic != NullObj)
            {
                // Poof(18), Foom(20), Pow/Biff(12)
                int comicFrames = 12;
                if (kachowSprite == 598) comicFrames = 18; 
                else if (kachowSprite == 596) comicFrames = 20; 
                
                pComic->SetLifeTime(comicFrames * 1.5);
                pComic->SetAnimSpeed(0.0);
                pComic->SetStartSize(1.0, 1.0);
                pComic->SetEndSize(1.0, 1.0);
                pComic->SetVelocity(0.0, 0.0); 
                pComic->SetStartAlpha(255);
                pComic->SetEndAlpha(255);
            }
      }

}

void CMissile::OnCustomExplosion(int dmg, int pushPower, int nTargetsHit)
{ /*override void CMissile etc and run ur bs here*/ } 
void CMine::OnCustomExplosion(int dmg, int pushPower, int nTargetsHit)
{}
void COilDrum::OnCustomExplosion(int dmg, int pushPower, int nTargetsHit)
{}

/*override void CMissile::ExplodeAt(fixed x,fixed y)
{                                                   // (sender, flags, x, y, dmg, pushPower, destroyRadius, destroy, false, false);
      if (launchdata.anim.spriteIndex != 84 && launchdata.anim.spriteIndex != 85) do_custom_explosion(this,-1, PosX, PosY, launchdata.explosion.damage, launchdata.explosion.pushPower, 20, true, true,true);   //test
}  */
          

//old
void drawExpSmokeParticles(fixed ePosX, fixed ePosY, int smokeCount, int r, int g, int b, int tier)
{
        //int smokeCount = int(float(dmg) * 0.3);
        
        local tierMultiplier = tier;
        if (tier == 0) tierMultiplier = 1;
        
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
                
                int sizeIndex = 1;

                if (smokeSprite <= 612) 
                {
                    sizeIndex = smokeSprite - 608; 
                } 
                else 
                {
                    sizeIndex = smokeSprite - 612; 
                }
                
                local pSmoke = new PxParticle(smokeSprite, ePosX, ePosY);
                if (pSmoke != NullObj)
                {     
                    float upVel = -7.0 + (float(sizeIndex) * 0.5);  
                    
                    //pSmoke->SetVelocity(RandomFloat(-1.5, 1.5), RandomFloat(-5.5, -3.5));
                    pSmoke->SetVelocity(RandomFloat(-1.5, 1.5), RandomFloat(upVel - 1.0, upVel));
                    pSmoke->SetLifeTime(RandomInt(15 + (sizeIndex * 7), 20 + (sizeIndex * 8)));  

                    pSmoke->SetAnimSpeed(0.0);
                    pSmoke->SetStartSize(1.0, 1.0); 
                    pSmoke->SetEndSize(1.0, 1.0);

                    pSmoke->SetMotionRandomness(4.0);

                    pSmoke->SetAirResistance(0.04 + (float(sizeIndex) * 0.02));            

                    pSmoke->SetStartAlpha(255);
                    pSmoke->SetEndAlpha(200);   
                    if (smokeSprite >= 613)
                    {       
                        pSmoke->SetVelocity(RandomFloat(-1.5, 1.5), RandomFloat(-2.5, 2.5)); 
                        pSmoke->SetAirResistance(0.08);  
                        pSmoke->SetMotionRandomness(0.2);  
                    }
                }
            }
        }
}

void drawKachaw(fixed ePosX, fixed ePosY)
{
            int kachowSprite = RandomInt(596, 599);
            local pComic = new PxParticle(kachowSprite, ePosX, ePosY);
            if (pComic != NullObj)
            {
                // Poof(18), Foom(20), Pow/Biff(12)
                int comicFrames = 14;
                if (kachowSprite == 598) comicFrames = 18; 
                else if (kachowSprite == 596) comicFrames = 20; 
                
                pComic->SetLifeTime(comicFrames * 1.5);
                pComic->SetAnimSpeed(0.0);
                pComic->SetStartSize(1.0, 1.0);
                pComic->SetEndSize(1.0, 1.0);
                pComic->SetVelocity(0.0, 0.0); 
                pComic->SetStartAlpha(255);    
                pComic->SetEndAlpha(180);
                //pComic->SetEndColor(0,0, 255);
            } 
}

CFlare : CMissile;

CFlare::CFlare(fixed x, fixed y, int sprite, int duration, float size)
{
    CWeaponLaunch* ldata = getClustletsData();
    ldata->anim.spriteIndex = 0;   
    ldata->anim.trailIndex  = 0;
    ldata->anim.trailSpeed  = 100;    
    ldata->anim.trailPower  = 10;   
    ldata->anim.unk         = 50;
    ldata->explosion.flags  = 16;   
    ldata->explosion.damage  = 0;     
    ldata->explosion.pushPower  = 0;          
    ldata->action           = WAction_Bounce;
    ldata->explodeInto      = WExplode_Nothing;
    ldata->timeBeforeExplosion = 5300;
    ldata->spriteSize       = 2; 
    
    flareSprite = sprite; 

    CShootDesc SDesc;
    zero(&SDesc);
    SDesc.SpX = RandomFloat(-2.0, 2.0);     
    SDesc.SpY = RandomFloat(-2.0, 2.5);
    SDesc.X   = x;
    SDesc.Y   = y;

    super(Root->GetObject(25, 0), ldata, &SDesc);

    if (PosX != SDesc.X && PosY!= SDesc.Y)
    {
       PosX = SDesc.X; PosY = SDesc.Y; IsStatic = false;
    } 
    fsize        = size;
    fduration    = duration;
    fdurationMax = duration;   
    fdead        = false;     
    flareRot     = RandomFloat(0.0, 6.28318);
    flareSpin    = 0.0;
    if (RandomInt(1,2) == 1) flareSpin = 0.16; else flareSpin = -0.16;
}

void CFlare::Message(CObject* sender, EMType Type, int MSize, CMessageData* MData)
{
    super;

    if (Type == M_FRAME)
    {
        if (fdead) { Free(true); return; }
        if (PosY > GS->LevelSY && !fdead){ PlayLocalSound(55,5.0,1.0,1.0); fdead = true;}// this.PlaySound( SIndex, UnkB, UnkC, Pan)
        fduration--;
        if (fduration <= 0)
        {
            fdead = true;
            Free(true);
            return;
        }

        float fadeFrac = float(fduration) / float(fdurationMax);
        if (fadeFrac < 0.0) fadeFrac = 0.0;
        if (fadeFrac > 1.0) fadeFrac = 1.0;

        currentSize = fsize * fadeFrac; 
        flareRot   += flareSpin;

        if ((gframe % 2) == 0)
        {
            local p = new PxParticle(615, PosX, PosY);
            if (p != NullObj)
            {
                p->SetLifeTime(20);
                p->SetAnimSpeed(0.0);
                p->SetVelocity(RandomFloat(-0.5, 0.5), RandomFloat(-3.5, -2.8));
                p->SetAirResistance(0.05);
                
                p->SetStartSize(currentSize * 0.9, currentSize * 0.9);
                p->SetEndSize(currentSize * 0.8, currentSize * 0.8);
                
                p->SetStartAlpha(int(200.0 * fadeFrac));
                p->SetEndAlpha(0);
                p->SetMotionRandomness(0.3);
            }
        }
    }
    // 3. Custom rendering hooked directly into the draw queue
    else if (Type == M_DRAWQUEUE)
    {
        if (fdead) return;

        float fadeFrac = float(fduration) / float(fdurationMax);
        if (fadeFrac < 0.0) fadeFrac = 0.0;
        if (fadeFrac > 1.0) fadeFrac = 1.0;

        float flicker = 1.0 + RandomFloat(-0.08, 0.08);

        AddSpriteEx(5.0, PosX, PosY, flareSprite, 0, flareRot, currentSize * flicker);         
    }
}