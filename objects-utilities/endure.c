 require utils, utility_no_fall_damage;

 CSoundFile * fahh ; 
 CSoundFile * vine ;
 CSoundFile * ahh;      
 CSoundFile * rizz;

 void endure::Init()
 {
    fahh = new CSoundFile(GetAttachment("fahhh.wav"));
    vine = new CSoundFile(GetAttachment("vine.wav"));
    ahh = new CSoundFile(GetAttachment("ahh.wav"));
    rizz = new CSoundFile(GetAttachment("rizz.wav"));
 }

 void memeSound(float x,float y)
 { 
                  vol = CalculateSoundVolume(x,y);
                  vol = vol * 0.7;
    			  pan = CalculateSoundPan(x,y);

			    local i = RandomInt(1, 4);
    			if      (i == 1) { fahh->Play(vol * 1.8, pan, false);  }
    			else if (i == 2) vine->Play(vol * 1.2, pan, false);
    			else if (i == 3) ahh->Play(vol * 0.8, pan, false);
    			else if (i == 4) rizz->Play(vol * 1.2, pan, false);
 }

 override void CWorm::Free(bool FreeMem)
 {
   if (FreeMem) Endure = false;
   super;
 }

 override void CWorm::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
 { 
 if (Type == M_FRAME)
 {
     if (Endure && GS->Info.GetWormHealth(WormTeam,WormNumber) == 0 && ObjState!=WS_DEAD )//BEFORE super; for fall damage actually
     {
       
          GS->Info.SetWormHealth(WormTeam, WormNumber, int(15));  //Set HP to 15       
          thehp = 15;   
          if (ObjState == WS_DEATH) SetState(WS_IDLE);
          isStatic = false;
          memeSound(PosX,PosY);
     
     }
     if (lostEndure && endured)
     {
         GS->Info.SetWormHealth(WormTeam, WormNumber, int(15));  //Somehow losing endure sets HP to 0
         lostEndure = false;
         endured = false;
     }
     if  (lostEndure) lostEndure = false;
 
 }
 super;
  if (Type == M_FRAME)
  {  
     if (ObjState == WS_SINKING) {Endure = false;}      
     if (ObjState == WS_DEAD) {Endure = false;}
     if (!WasEHit && Endure && GS->Info.GetWormHealth(WormTeam,WormNumber) >0 && GS->Info.GetWormHealth(WormTeam,WormNumber) <15  ) //other cases
     {          
          GS->Info.SetWormHealth(WormTeam, WormNumber, int(15));  //Set HP to 15       
          thehp = 15;              
          Endured = true; 
          if (ObjState == WS_DEATH && PosY>Env->Water){ SetState(WS_IDLE); dead = false; IsStatic = false;}
          memeSound(PosX,PosY);    
     }
     if (WasEHit && Endure)
     {           
          if (GS->Info.GetWormHealth(WormTeam,WormNumber) <=0 )   //If it's dying
          {              
          memeSound(PosX,PosY);    
          GS->Info.SetWormHealth(WormTeam, WormNumber, int(15));  //Set HP to 15
          WasEHit = false;
          Endured = true; 
          if (ObjState == WS_DEATH && PosY>Env->Water) { this->SetState( WS_IDLE );  isStatic = false; dead = false; } //Revive
          thehp = 15;
          }
     }
     
     if (Endure)   //CopyPaste from NoFallDamage
     { 
       if (GS->Info.GetWormHealth(WormTeam,WormNumber) >0 && GS->Info.GetWormHealth(WormTeam,WormNumber) <15)  //redundant
          { memeSound(PosX,PosY); GS->Info.SetWormHealth(WormTeam, WormNumber, int(15)); } //Set HP to 15 
       
       /*if (SpY > 13.5 || SpY < -13.5) //MAX FALL DAMAGE IS LIKE 13 OR SOMETHING  
       //int spy = SpY ** 0;  //forcing float to one to determine direction
       SpY = SpY / 1.25; 
       if (SpX > 15.0 || SpX < -15.0 )
       SpX = SpX / 1.18; */    
       local rem;
       if ((SpY > 8.0 || SpX > 8.0 || SpX < -8.0 || SpY < -8.0) && (ObjState == WS_JUMP || ObjState == WS_FLYING2 || ObjState == WS_SLIDING || ObjState == WS_FLYING || ObjState == WS_AFTERROPE || ObjState == WS_POWERFLY)) 
       {
          if (rem == 1) this->SetState(WS_JUMP);
          else if (rem == 2) this->SetState(WS_FLYING2);
          else if (rem == 3) this->SetState(WS_AFTERROPE);
          else if (rem == 4) this->SetState(WS_SLIDING);
          else this->SetState(WS_IDLE);
       }          
       else if ((SpY < 8.0 || SpX < 8.0 || SpX > -8.0 || SpY > -8.0))
       {
          if (ObjState == WS_JUMP) rem = 1;
          else if (ObjState == WS_FLYING2) rem = 2;
          else if (ObjState == WS_AFTERROPE) rem = 3;
          else if (ObjState == WS_SLIDING) rem = 4;
          else 
       {
          rem = 5;
          if (remote_fd_switch) this->SetState(WS_IDLE);
       }
      } 
     }
     if (Endure && ECount <=0)
     {
         Endure = false;
         endured = false;
         lostEndure = true; 
     }
  }
     
 if (Type == M_GUNEXP && Endure)
 {
   if ((GS->Info.GetWormHealth(WormTeam,WormNumber) -  MData->params[5]) > 0 && (GS->Info.GetWormHealth(WormTeam,WormNumber) -  MData->params[5]) < 15) 
   memeSound(PosX,PosY);
 }  
 
 if (Type == M_EXPLOSION || Type == M_GUNEXP)
 { 
   if(Endure) WasEHit = true;       //Check hits
 }
                          
 if (Type == M_DRAWQUEUE && Endure)
 {
   AddSpriteEx(14, PosX, PosY, 81, 7, 0, 1.4);    
 }
   if (Type == M_FRAME)   //When turn begins, reduce cooldown or remove mark.
  { 
     if (Root->IsTimerActive() == true && !turnBeganEx && GetCurrentWorm()!=NullObj)
     {
     local cwm = GetCurrentWorm();
     if (cwm !=NullObj)
     {
        if (cwm->WormTeam == WormTeam)
        {
            if (ECount>0) ECount--;
            turnBeganEx = true;
        }                                         
     }
     }
  }
  if (Type == M_PRETURNSTART) turnBeganEx = false;   
}


 override void CWorm::FireFinal(CWeapon* Weap,CShootDesc* Desc)
 {
 if (Weap->CheckName( "Endure" ))
    {    
        memeSound(PosX,PosY);
        Endure = true;
        ECount = 1;    //
    }
 super;
 }

 override void CWorm::CWorm(CObject* Parent,int aTeam,int aIndex,CWormParams* params)
 {
    Endure = false;
    turnBeganEx = false;
    ECount = 0;
    super;
 }