//It's quite cool man
require weapon_tazer, pxeffects;

int   globalEleStunF;
float globalElePushX;
float globalElePushY;         
float hitOriginX;
float hitOriginY;         
float eleOriginX;
float eleOriginY;    
bool  electrifiedMessageM;    
bool  electrifiedMessage;    // False by default.
bool  retainspeedelectric;
int   eleFrame;             
int   eleFrameM;
bool  doEleDmg;
int   eleDmg;

void SetMessageElectric(int stunFrames, float pushX, float pushY, bool retainspd)
{        
        globalEleStunF = stunFrames;
	globalElePushX = pushX;
	globalElePushY = pushY;
	retainspeedelectric = retainspd;
	electrifiedMessage = true;
	doEleDmg = false;
	eleFrame = gframe;
}  

void SetMessageElectricDmg(int stunFrames, float pushX, float pushY, bool retainspd, int dmg)
{        
        globalEleStunF = stunFrames;
	globalElePushX = pushX;
	globalElePushY = pushY;
	retainspeedelectric = retainspd;
	electrifiedMessage = true;
	eleFrame = gframe;
	doEleDmg = true;
	eleDmg   = dmg;
}

void SetMessageElectricMissile(int stunFrames, float originX, float originY)
{
    globalEleStunF = stunFrames;
    electrifiedMessageM = true;
    doEleDmg = false;
    eleFrameM = gframe;
    eleOriginX = originX;   
    eleOriginY = originY;  
}

override void CWorm::CWorm(CObject* Parent,int aTeam,int aIndex,CWormParams* params)
{
	super;
	TazeFrames = 0;
	ArtificialTaze = false;
	FlyTheFuckToX = 0.0;        
	FlyTheFuckToY = 0.0;
	ReceiveDmg = 0;
	
	hitFrame = 0; //For Saw
}

//Damn, proper indentation makes the code look good
void CWorm::TazeArtificial(int TazeFrameAmount, float FlyToX, float FlyToY)
{
	/*CEffectManager* zapFX = new CEffectManager(this, PosX, PosY, 4, 30, 6, 0.45, 160, 220, 255) ;
        if (zapFX!=NullObj)
        {
                zapFX->ZPlane = 3.1;  
                zapFX->gZPlane = 3.09;
                zapFX->shouldExplode = true;
                zapFX->expLimit = 25;    
                zapFX->vanishSpeed = 0.55;
        }*/ 
        
        createElectricImpact(this, PosX, PosY, 5.5);
        
        ArtificialTaze = true;
    
	FlyTheFuckToX = FlyToX;        
	FlyTheFuckToY = FlyToY; 
	
	if (doEleDmg) ReceiveDmg = eleDmg;
	
	flyEToX = FlyToX;  
	flyEToY = FlyToY;
	
	TazeFrames = TazeFrameAmount;
	
	if (RandomInt(0, 1) == 0)
        {
         	TazedAnim = wormTazedSprite1->Index;
        }
        else
        {
        	TazedAnim = wormTazedSprite2->Index;
        }
	
	TazedAnimCycle = 0.0;
        TazedAnimAngle = 3.141593 + RandomFloat(-0.2, 0.2); // Gives it that jittery fuckass shock effect
        
        tazerHitSound->Play(1.0, 0.0, false);
}

void CWorm::TazeArtificialRet(int TazeFrameAmount)   // Retain SpX and SpY
{
        TazeArtificial(TazeFrameAmount, SpX, SpY);
}

void CWorm::UnTazeArtificial()
{
       if (ReceiveDmg != 0)
       {
                CMessageData msg;
                msg.params[0]	= 0;
                msg.fparams[1]	= PosX;
 	        msg.fparams[2]	= PosY;
	        msg.fparams[3]	= 0;
	        msg.fparams[4]	= 0;
	        msg.params[5]	= ReceiveDmg;
	        msg.params[6]	= 0;
				
		Message(Root, M_GUNEXP, 1032, &msg);  
	}
		SpX = FlyTheFuckToX;    
		SpY = FlyTheFuckToY;
		
        ArtificialTaze = false;
	FlyTheFuckToX = 0.0;  
	FlyTheFuckToY = 0.0;  
        TazeFrames = 0;  
        ReceiveDmg = 0;
        
        if (GS->Info.GetWormHealth(WormTeam, WormNumber) <=0) return; // ObjState isn't dead, but worm is dead.
        
        if (ObjState!=WS_DEAD && ObjState!=WS_DEATH && ObjState!=WS_SINKING && GS->Info.GetWormHealth( WormTeam, WormNumber) != 0)
        {
                if (TazedAnim == wormTazedSprite1->Index)
                {
                        SetState(WS_FLYING2);
                }        
                else 
                {
                        SetState(WS_POWERFLY); //Unfreeze, Undrill.
                }        
        }
}

override void CWorm::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
        // Main Taze Logic.
 	if (Type == M_FRAME && ArtificialTaze)
 	{
		// Makes the worm invisible
		if (ObjState!=WS_DEAD && ObjState!=WS_DEATH && ObjState!=WS_SINKING && GS->Info.GetWormHealth(WormTeam, WormNumber) != 0)
                {
                	SetState(WS_IDLE);
			Anim->Reset();
			Anim->SetAnimEx(136, 0.0);
		}
                TazedAnimCycle = TazedAnimCycle + 0.1;
		if(TazedAnimCycle >= 1.0) TazedAnimCycle -= 1.0;
		
		if (TazeFrames < 1)
		{
			UnTazeArtificial();
			hitFrame = 0;       // Avoid infinite Saw hits
		}
		else
		{
			TazeFrames--;
			SpX = 0.0;
			SpY = 0.0;
	                hitFrame = 0;    
		}
	}
        if (Type == M_DRAWQUEUE)
        {		
		if(ArtificialTaze)
		{
                        if(TurnSide < 0)
			{
				AddSpriteEx(7, PosX, PosY, TazedAnim + 262144, TazedAnimCycle, TazedAnimAngle, 1.0);
			}
			else
			{
				AddSpriteEx(7, PosX, PosY, TazedAnim, TazedAnimCycle, TazedAnimAngle, 1.0);
			}
		}
        }  
        local health = GS->Info.GetWormHealth(WormTeam, WormNumber);
        
 	super; // TazeRet needs current Speed values.
 	//////////////////////////// Gun / Melee Hits ////////////////////////////
 	if (Type == M_GUNEXP)
 	{
         	if (electrifiedMessage && (gframe - eleFrame) <= 8) 
         	{
                 	if (!retainspeedelectric) // Account for lag too
                 	{
                         	 SpX = 0;
                         	 SpY = 0;
                                 TazeArtificial(globalEleStunF, globalElePushX, globalElePushY);
                  	}
                  	else if (retainspeedelectric)  // Only for single hits.
                  	{                                                           
                  	        TazeArtificialRet(globalEleStunF);
                  	}
          	} 
                else if (electrifiedMessage && (gframe - eleFrame) > 8)
      	        {
                   	electrifiedMessage = false;
                   	retainspeedelectric = false;
                   	eleFrame = 0;
         	}
 	}
 	//////////////////////////// Explosions ////////////////////////////
 	if (Type == M_EXPLOSION)
 	{  
           	if (electrifiedMessageM) 
 	  	{
                      	if (gframe - eleFrameM <= 10)
 	  	      	{          
                                float ox = MData->params[1] / 65536;
                                float oy = MData->params[2] / 65536;
                                float dx = ox - eleOriginX;
                                float dy = oy - eleOriginY;
                                float originDistSqr = dx*dx + dy*dy;
            
                                if (originDistSqr < 12.0)  // Compare with origin explosion.
                                {
                                   	if (health != GS->Info.GetWormHealth(WormTeam, WormNumber))
                                   	{
                                   	   	TazeArtificialRet(globalEleStunF);
                                   	}
                                }   	
                                /*  
                                local dmg = 0;
                                local shouldHit = CalculateElectricShock(MData->fparams[1], MData->fparams[2], MData->params[4], &dmg);

 	  	   	        if (shouldHit && !ArtificialTaze && abs(SpX + SpY) > 5 && dmg > 1)      // Default, just dont get tazed for random explosions on the other side of the map 
 	  	   	        {
                         	 	TazeArtificialRet(globalEleStunF);
 	  	   	        }
 	  	   	        else if (shouldHit && !ArtificialTaze && abs(SpX + SpY) < 5 && dmg > 10) // Nudge 
 	  	   	        {
                         	 	TazeArtificial(globalEleStunF, dmg * 0.75, RandomInt(4,5));
 	  	   	        } 
                                else if (shouldHit && !ArtificialTaze && abs(SpX + SpY) < 5 && dmg < 10) // More artificial nudge 
 	  	   	        {
                         	 	TazeArtificial(globalEleStunF, RandomFloat(-2.3,2.3), RandomInt(4,5));
 	  	   	        }       
 	  	   	        else if (shouldHit && ArtificialTaze && abs(SpX + SpY) < 5) // Stunned by Taze
                                {
                                 	//js do nothing bruh
                                }    
 	  	   	        else if (shouldHit && ArtificialTaze && abs(SpX + SpY) > 5) // This is impossible btw (Unless same frame bs happens)
                                {
                                 	TazeArtificialRet(globalEleStunF);
                                };  */
 	  	   	}
 	  	   	else
 	  	   	{
 	  	   	   	electrifiedMessageM = false;
                   	   	eleFrameM = 0;
 	  	   	}
 	  	}
 	}
}

bool CWorm::CalculateElectricShock(fixed x, fixed y, int dmg, int* retdmg)  // Sigh unused.
{   	
        if(dmg == 0)
        {
         	return false;
	}    
	float fdmg = dmg;
	float dx = PosX - x;
	float dy = PosY - y;
	
	float distsqr = dx*dx + dy*dy;
	float fdmgsqr = fdmg*fdmg*4.0;
	
	fdmg = (1.0 - distsqr/fdmgsqr) * fdmg;
	dmg = int(fdmg);
	if (dmg > 0) 
        {     
                *retdmg = dmg;
         	return true;
        }
        else return false;
} 

override void CMissile::DoExplosion(fixed x,fixed y,int PushPower,int Damage,int unkB,int Team)
{   
        if (isMisElectric)
 
        SetMessageElectricMissile(25, x, y);  // i COULD stun them here, would be a bit more complex though
 
        super;    
}

override void CMissile::CMissile(CObject* parent,CWeaponLaunch* ldata,CShootDesc* sdata)
{
        isMisElectric = false;
  
        super;
}

void do_cus_exp_electric(CGObject * sender, int flags, float x, float y, int dmg, int pushPower, float destroyRadius, bool destroy)
{
        local tier = 0;              
        
        tier = do_custom_explosion(sender, flags, x, y, dmg, pushPower, destroyRadius, destroy, false, false, true); //actual explosion
        
        local radius = float((dmg + 2) / 2);
        createElectricExplosion(x, y, radius);    //effect
        
        do_explosion_particles( x, y, tier, false, tier > 1, false, false, true, false, 100, 120, 190);      
}