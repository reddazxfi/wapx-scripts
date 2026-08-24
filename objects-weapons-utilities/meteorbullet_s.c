require utils, p_sprite_builder, utils_red, utils_steps, sprite_aim_guns, sprite_sheet_red;

CMeteoriteBullet : CMissile;
CSoundFile * item_41;    //from terraria

CColMask*PUSH_COLMASK_METEOR; void meteorbullet_s::Init(){}void meteorbullet_s::FirstFrame(){PUSH_COLMASK_METEOR=new CColMask(10,10,MakeCircleMask(10));}

override void CWorm::CWorm(CObject* Parent,int aTeam,int aIndex,CWormParams* params)
{
 super;
 MPBShots = 3; //4
 MshotsRemaining = MPBShots; 
 meteorHitCooldown = 0;
}

override void CWorm::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
 if (Type == M_FRAME)
 {
      if (meteorHitCooldown>0)  meteorHitCooldown--;
 }
 if (Type == M_SETWEAPON) MshotsRemaining = MPBShots;
 super;
}

CTraceRes * meteorRes;

void meteorbullet_s::InitGraphic()
{
 CFile *f;
 meteorRes = new CTraceRes();
 
 item_41 = new CSoundFile(GetAttachment("item_41.wav"));
    
}

CMeteoriteBullet::CMeteoriteBullet(CObject* parent, CWeaponLaunch* ldata, CShootDesc* sdata, int inheritedBounces, float inheritedWC)
{
    sawhitlimit = 0;
    fixed zzzero = 0.0;
    simulatedHitX = zzzero;
    simulatedHitY = zzzero;
    
    super(parent, ldata, sdata);
    mteam = sdata->Team;
    meteor_anim = true;    
    OwnerWorm = CWorm(NullObj);    
    FreeMe = false;
    Mbouncing = false;
    maxBounces      = 10;
    curWormCollissions = inheritedWC;
    maxWormCollissions = 8.0;
    currentBounces  = inheritedBounces;
    meteorSpeed     = 11.0;
    GravityFactor   = 0.0;
    snapTimer       = 0;
    // Seed both snapshots at spawn position
    snapPosX0 = sdata->X;
    snapPosY0 = sdata->Y;
    snapPosX1 = sdata->X;
    snapPosY1 = sdata->Y;
    snapSpX   = sdata->SpX;
    fuckCounter = 0;
    snapSpY   = sdata->SpY;
    dead = false;
    local fSpX = sdata->SpX + 0.0;
    local fSpY = sdata->SpY + 0.0;
    local spd  = sqrt(fSpX * fSpX + fSpY * fSpY);
    if (spd > 0.001)
    {
        SpX = (fSpX / spd) * meteorSpeed;
        SpY = (fSpY / spd) * meteorSpeed;
    }
}
   
void CMeteoriteBullet::Collide(CGObject* Obj,int type)
{   
    super;
        
    if (dead) return;   
        
    bool returnEarly = false;
    currentBounces++;
    
    if (Obj != NullObj && Obj is CSaw == true) returnEarly = true; // Let the saw run the calculation.
    
    if (currentBounces >= maxBounces)
    {
        Mbouncing = false;
        dead = true;
        DoExplosion( PosX, PosY, 50, 10, 10, mteam);
        FreeMe = true; dead = true;
        return;
    }
    
    if (returnEarly) return;   
    
    local x = 0.0; local y = 0.0;
    
    if (simulatedHitX == 0 && simulatedHitY == 0)
    {   
        x = PosX;  y = PosY;
    }
    else    
    {
        x = simulatedHitX;  y = simulatedHitY;
    }
    simulatedHitX = 0.0;
    simulatedHitY = 0.0;

    // --- Pick best history snapshot ---
    local useX = snapPosX0 + 0.0;
    local useY = snapPosY0 + 0.0;

    local dx0 = (snapPosX0 + 0.0) - (x + 0.0);
    local dy0 = (snapPosY0 + 0.0) - (y + 0.0);
    local dx1 = (snapPosX1 + 0.0) - (x + 0.0);
    local dy1 = (snapPosY1 + 0.0) - (y + 0.0);
    local d0  = sqrt(dx0 * dx0 + dy0 * dy0);
    local d1  = sqrt(dx1 * dx1 + dy1 * dy1);

    if (d1 > d0 && d1 < 120.0)
    {
        useX = snapPosX1 + 0.0;
        useY = snapPosY1 + 0.0;
    }

    // --- Extend endpoint 60px PAST the impact point ---
    // This guarantees the ray enters terrain and TraceMaskEx
    // can properly walk around the hit pixel to find the normal.
    local fSpX = snapSpX + 0.0;
    local fSpY = snapSpY + 0.0;
    local spd  = sqrt(fSpX * fSpX + fSpY * fSpY);
    local dirX = 0.0;
    local dirY = 1.0;
    if (spd > 0.001)
    {
        dirX = fSpX / spd;
        dirY = fSpY / spd;
    }
    local scanDist = 60.0;
    local endX = int(x) + int(dirX * scanDist);
    local endY = int(y) + int(dirY * scanDist);
    
    //CTraceRes *res = new CTraceRes();
    local gotNormal = TraceLineEx(this, int(useX), int(useY), endX, endY, CMASK_TERRAIN | CMASK_MINE | CMASK_OILDRUM, meteorRes);

    local nx = 0.0;
    local ny = 0.0;

    if (gotNormal && (meteorRes->NormalX * meteorRes->NormalX + meteorRes->NormalY * meteorRes->NormalY) > 0.01)
    {
        nx = meteorRes->NormalX;
        ny = meteorRes->NormalY;
    }
    else
    {
        // Fallback: reverse incoming direction
        if (spd > 0.001)
        {
            nx = 0.0 - dirX;
            ny = 0.0 - dirY;
        }
        else
        {
            nx = 0.0;
            ny = -1.0;
        }
    }
    // --- Reflect ---
    local dot = fSpX * nx + fSpY * ny;
    if (dot > 0.0)
    {
        nx  = 0.0 - nx;
        ny  = 0.0 - ny;
        dot = 0.0 - dot;
    }

    local newSpX = fSpX - 2.0 * dot * nx;
    local newSpY = fSpY - 2.0 * dot * ny;

    local newSpd = sqrt(newSpX * newSpX + newSpY * newSpY);
    if (newSpd > 0.001)
    {
        newSpX = (newSpX / newSpd) * meteorSpeed;
        newSpY = (newSpY / newSpd) * meteorSpeed;
    }

    // --- Spawn position: hit pixel + push along normal ---
    local pushDist = 10.0;
    local spawnX = 0.0;
    local spawnY = 0.0;

    if (gotNormal)
    {
        spawnX = (meteorRes->HitX + 0.0) + nx * pushDist;
        spawnY = (meteorRes->HitY + 0.0) + ny * pushDist;
    }
    else
    {
        spawnX = (x + 0.0) + nx * pushDist;
        spawnY = (y + 0.0) + ny * pushDist;
    }
    
  /*  local wp = GetWeaponByName("Phoenix Blaster");   
    
    if (wp != NullObj)
    ldata = wp->launch;
    CShootDesc SDesc;
    zero(&SDesc);
    SDesc.X     = spawnX;
    SDesc.Y     = spawnY;
    SDesc.SpX   = newSpX;
    SDesc.SpY   = newSpY;
    SDesc.Team  = 0;
    SDesc.Worm  = 0;
    SDesc.Delay = 0;
    SDesc.AddX  = 0;
    SDesc.AddY  = 0;

    if (ldata != NullObj)
        new CMeteoriteBullet(Root->GetObject(25, 0), &ldata, &SDesc, currentBounces + 1, curWormCollissions);
        
    Free(true);*/   
    local spnwptn = CheckSpawnPoint(PosX, PosY, PosX, PosY, ColMask, 1, CMASK_TERRAIN, &spawnX, &spawnY, 8);
    int hitXX; int hitYY;
    if (spnwptn == false)
    {
        if (TraceLine(this, PosX, PosY, spawnX, spawnY, CMASK_TERRAIN, &hitXX, &hitYY)!= NullObj || meteorRes->StartsSolid)
        {                          
            fuckCounter++;
            DoExplosion(spawnX, spawnY, 0, 0, 0, mteam) ;
            GG->land->MakeHole( 10, spawnX, spawnY);
        }
    } 
    if (fuckCounter>4)
    {
        Mbouncing = false;
        dead = true;
        DoExplosion( PosX, PosY, 50, 10, 10, mteam);
        if (Obj is CSaw == false) Free(true);
        else fuckCounter = 4;
        return;
    }
    PosY = spawnY;
    PosX = spawnX;
    SpX = newSpX; 
    SpY = newSpY;     
    ApplyConstantSpeedMeteorite(SpX,SpY, meteorSpeed); 
    Mbouncing = true;
    return;
}
       
bool CMeteoriteBullet::IsStuck()
{  
    if (dead) return;     
 if (SpX > -1 && SpX < 1)
        {if (SpY > -1 && SpY < 1)
             return true;}
 if(PosX < -400           ||
    PosY < -400           ||
    PosY > Env->Water+100 ||
    PosX > GS->LevelSX + 400)             
             return true;
              
 return false;
}

void CMeteoriteBullet::ApplyConstantSpeedMeteorite(fixed SpX, fixed SpY, float speedcap)
{       
    local spdd  = sqrt(SpX * SpX + SpY * SpY);
    if (spdd > 0.001)
    {
        SpX = (SpX / spdd) * speedcap;
        SpY = (SpY / spdd) * speedcap;
    }
}

void CMeteoriteBullet::Message(CObject* sender, EMType Type, int MSize, CMessageData* MData)
{      
       super;
       if (Type == M_FRAME)
       {           
              if (dead)
              {
                     Free(true);
                     return; 
              }   
              if (gframe % 10 == 0) //Limit to gframe to avoid bad arcs.
              {
                     ApplyConstantSpeedMeteorite(SpX, SpY , meteorSpeed);
              }
              if (IsStuck())
              {
                     currentBounces = maxBounces;
                     ExplodeAt(PosX,PosY);
                     Free(true);
              }
        
              snapTimer = snapTimer + 1;
        
              if (snapTimer >= 3)
              {
                     snapTimer  = 0;
                     // Rotate: old "recent" becomes "far", current pos becomes "recent"
                     snapPosX1  = snapPosX0;
                     snapPosY1  = snapPosY0;
                     snapPosX0  = PosX;
                     snapPosY0  = PosY;
                     snapSpX    = SpX;
                     snapSpY    = SpY;
              }
              ProcessCollission();    
       }                       
       if(Type==M_FRAME
       ){if(!IsStuck()){if(meteor_anim){local sprite
       =                                            //Spri
       redWeapSheet->Index;WeapSpriteParams* Params= new WeapSpriteParams;Params->animate=false;Params-> //teb
       anim_speed=1.50; Params             //uild
       ->ZPlane = 5.0; Params->scale = 1.32; Params->numSpritesInSheet = 12; Params->useSpriteSystem=true; new WeapSprite(this,sprite,Params);meteor_anim=false;}//er
       }}
}

void CMeteoriteBullet::ProcessCollission()
{
    if (dead) return;     
        CGObject *obj;
	for(int i = 0; i < Env->Objs.Count; i += 1)
	{
		obj = CGObject(Env->Objs.Objs[i]);     
		if(obj == NullObj) continue;
		if(obj == this)continue;
		if(GetCurrentWorm() != NullObj)
                {
                      if(obj == GetCurrentWorm())
                          continue;
                }
		if ( obj->Layer == LAYER_OILDRUM || obj->Layer == LAYER_MINE || obj->ClType == OC_OilDrum || obj->ClType == OC_Mine || obj->ClType == OC_Crate || obj->ClType == OC_Fire || obj->ClType == OC_Flame && (gframe % 2) == 0)
		{   
		  if(PUSH_COLMASK_METEOR->Check(PosX, PosY, obj->ColMask, obj->PosX, obj->PosY))
		  {
		     float ForceX = obj->PosX - PosX;
	             float ForceY = obj->PosY - PosY;
		     
		     float dist = sqrt(ForceX*ForceX + ForceY*ForceY);
		     if(dist > 0)
		     {            
                           if (SpX > 0) 
                                 ForceY = -SpX * 0.295;
                           else 
                                 ForceY =  SpX * 0.295;
                                 
                           if (ForceY>-1.50) ForceY = -1.50 ;
                           
                           ForceX = SpX * 0.145;
                           
                           CMessageData msg;
		           msg.params[0]  = 0;
		           msg.fparams[1] = PosX;
		           msg.fparams[2] = PosY;
		           msg.fparams[3] = ForceX;
		           msg.fparams[4] = ForceY;
		           msg.params[5]  = 10;
                           msg.params[6]  = 0;  
                           
                           curWormCollissions+=0.676767;
                           obj->Message(this, M_GUNEXP, 1032, &msg);    
	                   if (curWormCollissions>=maxWormCollissions)
	                   {
                                 currentBounces = maxBounces;
                                 ExplodeAt(PosX,PosY);
                                 Free(true);
                                 return;
                           } 
                           
                     if (obj is CPlatform == true)
                     {
                          Collide(obj,1);
                          return;
                     } 
                     }
                     continue;
                  }
		}
		if(obj->ClType == OC_Worm)
		{
		        local w = CWorm(obj);
		        if (OwnerWorm!=NullObj)
                        { if (w == OwnerWorm) continue; }
                        
			if(PUSH_COLMASK_METEOR->Check(PosX, PosY, obj->ColMask, obj->PosX, obj->PosY) && w->meteorHitCooldown<=0)
			{
                                float ForceX = obj->PosX - PosX;
				float ForceY = obj->PosY - PosY;
				float dist = sqrt(ForceX*ForceX + ForceY*ForceY);
				
				if(dist > 0)
				{
				   if (SpX > 0) 
                                      ForceY = -SpX * 0.265;
				   else 
                                      ForceY =  SpX * 0.265;
                                      
                                   if (ForceY>-1.65) ForceY = -1.65 ;
                                      
				   ForceX = SpX * 0.125;
				   
                                   CMessageData msg;
		                   msg.params[0]  = 0;
		                   msg.fparams[1] = PosX;
		                   msg.fparams[2] = PosY;
		                   msg.fparams[3] = ForceX; 
		                   msg.fparams[4] = ForceY;
		                   msg.params[5]  = 15;    //Damage
                                   msg.params[6]  = 0;   
                                   w->Message(this, M_GUNEXP, 1032, &msg);
                                   w->meteorHitCooldown = 4;		
				   curWormCollissions+=1.0;
				   if (curWormCollissions>=maxWormCollissions)
				   {
				       currentBounces = maxBounces;
                                       DoExplosion( PosX, PosY, 50, 10, 10, mteam) ;
                                       dead = true;
                                       Free(true);
                                       return;
				   }  
				   DoExplosion(PosX, PosY, 0, 1, 5, mteam); //funky effect
				}
				continue;
			}
		}
		else continue;
	}
}

override void CWorm::FireFinal(CWeapon* Weap, CShootDesc* Desc)
{
    if (Weap->CheckName("Phoenix Blaster"))  //AIM anim Fixer
    {              
       vol = CalculateSoundVolume(PosX,PosY);
       pan = CalculateSoundPan(PosX,PosY);   
       
       item_41->Play(vol, pan, false);
       
       Desc->Y -= 3.7;           
       if (FireAngle < 0.165) {Desc->Y -=2.44;  Desc->X +=2*TurnSide; }      
       cmtblt = new CMeteoriteBullet(Root->GetObject(25, 0), &Weap->launch, Desc, 0, 0.0);
       if (cmtblt!=NullObj)cmtblt->OwnerWorm = this;
       if (MshotsRemaining>0)
       {
          nAvalShoots = 1;
          nTotalShoots = 0;    
          ShowMessage2(itoa(MshotsRemaining), " Shots Remaining.") ;      
          MshotsRemaining--;
          SetState(WS_AIMING);   //WS_FIRED Bugs Out with custom firing anim, which is the default state for worms, this fixes the bug
          return;
       } 
        nTotalShoots = 1;   
        MshotsRemaining = MPBShots;
        SetState(WS_FIRED); 
    }        
    else {MshotsRemaining = MPBShots; //Shouldn't be neccessary, however i'm insecure.
    super; }
}                                         

override void CTurnGame::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData) { 
 super;                                                                             
  if (Type == M_FRAME) {                                                         
     if (gframe == 2) {                                                           
      local weapName; local sprite;

      CWormAnimParams* Params = new CWormAnimParams;
      weapName = "Phoenix Blaster";                                               
      sprite = gunAimSheet->Index;   
      Params->FSprite = sprite;    
                                        
      Params->hand_radius = -6.0;                                                         
      Params->animate = false;
      Params->hand_radial_rotation = 1.0;
      Params->fix_flip_rotation = true;
      Params->hand_rotation = -0.5;                                                                                                                                                             
      Params->weap_radial_rotation = 1.0;
      Params->weap_rotation = 1.0;
      Params->weap_radius = 0.0;
      Params->draw_hand = false;          
      Params->zplane = 5;       
      
      Params->useSpriteSystem = true;
      
      Params->subSpriteNumber = 4;
      Params->subSpriteFiring = 4;
      Params->numSpritesInSheet = 12;
                                    
      local WA = new WormSprite(weapName, sprite, Params);              
  }                              
  else if (Type==M_TURNBEGIN){  
  super;
  }                                                                                
 }
} 