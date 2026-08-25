require utils, p_sprite_builder, utility_endure, utils_red;

int wormCount;
int wormsTeleported;
CWorm* wormList[120];
bool sliceOccupied[120];  // one per possible worm plus respawned ones
int  sliceAssigned[120]; 
bool fuckeryDisplayed;

override CWorm::CWorm(CObject* Parent,int aTeam,int aIndex,CWormParams* params)
{     
 wormList[wormCount] = this;
 wormNum = wormCount;     
 
 super;
         
 wormCount = wormCount + 1;
 
 teleportsAtt = 0.0;
 
 windfactor = WindFactor;
 
 M_TELEPORTBEGIN = false; // Let me fantasize
 teleported = false;
 
 pendingTeleFrame = 0;
 fixed zero = 0.0;
 pendingTeleX = zero; // Can't do fixed()
 pendingTeleY = zero;
}

void CWorm::TeleportRandom()
{
    if (teleported) return;
    teleportsAtt+=1.0;

    local mySlice     = sliceAssigned[wormNum];
    local sliceWidth  = GS->LevelSX / wormCount;
    local overlap     = sliceWidth / 5;
    local sliceStart  = (mySlice * sliceWidth) - overlap;
    local sliceEnd    = sliceStart + sliceWidth + overlap;

    if (sliceStart < 30)             sliceStart = 30;
    if (sliceEnd > GS->LevelSX - 30) sliceEnd = GS->LevelSX - 30;

    local teleAtX = RandomInt(sliceStart, sliceEnd);
    local teleAtY = RandomInt(30, Env->Water - 100);
    
    local teleFlags = CMASK_TERRAIN | CMASK_OILDRUM | CMASK_CRATE;  
    
    local hitX = 0; local hitY = 0;

    local tracingtp = TraceLine(this, float(teleAtX), float(teleAtY), float(teleAtX), float(Env->Water - 50), CMASK_TERRAIN, &hitX, &hitY);

    local spawnOK = false;
    float spawnX = float(hitX);
    float spawnY = float(hitY);
        
    if (tracingtp!=NullObj && !IsTooCloseToWorms(spawnX, spawnY, 60.0))
    {
        spawnOK = CheckSpawnPoint(spawnX, spawnY - 4.0, spawnX, spawnY - 4.0, ColMask, 1.0, teleFlags, &spawnX, &spawnY, 11.0);
    }
    else { teleportsAtt-=0.6; return; } // Cheap cull, return attempt.
    
    //CheckSpawnPoint function goes in circles and may find available slot on top of bullshit
    tracingtp = TraceLine(this, spawnX, spawnY-5, spawnX, spawnY+30, CMASK_TERRAIN, &hitX, &hitY);

    if (spawnOK && tracingtp!=NullObj)
    {
        sliceOccupied[mySlice] = true;
        teleportsAtt = 0.0;
        pendingTeleX = spawnX;
        pendingTeleY = spawnY - 2.5;
        pendingTeleFrame = gframe + 51;
        if (ObjState != WS_FROZEN && ObjState != WS_DEAD && ObjState != WS_DEATH)
            SetState(WS_TELEPORTING);
        WindFactor = windfactor;    // I read somewhere SetState changes wind factor
        teleported = true;
    }
}

bool CWorm::IsTooCloseToWorms(float x, float y, float minDist)
{
    float minDistSqr = minDist * minDist;
    for (int i = 0; i < wormCount; i++)
    {
        if (wormList[i] == NullObj) continue;
        if (wormList[i] == this) continue;
        if (!wormList[i]->teleported) continue; // not placed yet  
        //if (!wormList[i]->M_TELEPORTBEGIN) continue; // not placed yet  
        //if (wormList[i]->teleportsAtt == 0) continue;
        float dx = wormList[i]->pendingTeleX  - x;
        float dy = wormList[i]->pendingTeleY  - y;
        if (dx*dx + dy*dy < minDistSqr) return true;
    }
    return false;
}

bool fuckery()
{
    local rndmngmfr = RandomInt(1,5);
    if (rndmngmfr == 1) 
       ShowMessage("Pray.") ;
    else if (rndmngmfr == 2)     
       ShowMessage("Fuck it");
    else if (rndmngmfr == 3)   
       ShowMessage("Good luck lol");
    else if (rndmngmfr == 4) 
    {
      ShowMessage("Fahhh!");
      fahh->Play(CalculateSoundVolume(GS->LevelSX/2,GS->LevelSY/2), 0.0, false);
      return false;
    }
    else if (rndmngmfr == 5)  
       ShowMessage("Executing Tactical Move No. 3302: Maneuvered Frenzy Escape");
    return true;
}

override void CTurnGame::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
 super;
 if (gframe == 2) fuckeryDisplayed = false;
 if (Type == M_TURNBEGIN && gframe > 190)
 {   
    wormsTeleported = 0;
    for (local i = 0; i < wormCount; i++)
    sliceOccupied[i] = false;

    // Shuffle slice assignment
    int availableSlices[120];
    local availCount = wormCount;
   
    for (local i = 0; i < wormCount; i++) 
    {
        availableSlices[i] = i;
    }

    // Fisher-Yates shuffle
    for (local i = availCount - 1; i > 0; i--)
    {
        local j = RandomInt(0, i); 
    
        local tmp = availableSlices[i];
        availableSlices[i] = availableSlices[j]; 
        availableSlices[j] = tmp;                 
    }

    for (local i = 0; i < wormCount; i++)
    {
        sliceAssigned[i] = availableSlices[i];
    }
    
    for (local i = 0; i < wormCount; i++)
    {
      if (wormList[i] != NullObj)
      {
        wormList[i] -> M_TELEPORTBEGIN = true;
      }
    }
    //if (wormList[0] != NullObj)  //wtf 
    //    wormList[0] -> M_TELEPORTBEGIN = true;
    PlaySound(68,0,0,1.3);  
    PlaySound(67,0,0,1.3);
 } 
 if (Type == M_PRETURNSTART)
 {
    if (!fuckeryDisplayed)
    {
       local soundPlayed = fuckery();
       fuckeryDisplayed = true;
    }      
 }   
}

override void CWorm::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{ 
 super;         
 if (Type == M_PRETURNSTART)
 {
    teleportsAtt = 0;
    fuckeryDisplayed = false;
 }
 if (Type == M_FRAME)
 {
  if ((wormCount - wormsTeleported) <= 4)
  {    
    wormsTeleported = 0;
    for (local i = 0; i < wormCount; i++)
    sliceOccupied[i] = false;
  }
  if (teleportsAtt >= 20.0 && M_TELEPORTBEGIN)
  {
    teleportsAtt-=0.2;
  }
  if (teleported && pendingTeleFrame > 0 && gframe == pendingTeleFrame)
  {
    PosX = pendingTeleX;
    PosY = pendingTeleY;
    //GG->land->ApplyMask(4, int(pendingTeleX), int(pendingTeleY));
    //GG->land->ApplyMask(4, int(pendingTeleX), int(pendingTeleY) - 5);
    //GG->land->ApplyMask(4, int(pendingTeleX), int(pendingTeleY) - 22);      
    pendingTeleFrame = 0; 
    M_TELEPORTBEGIN = false; 
    teleported = false;
    wormsTeleported++;
  }
 }
 if (Type == M_FRAME && M_TELEPORTBEGIN && !teleported && teleportsAtt < 20.0)
 {
   local myMod = 3 + (wormNum % 5);  // spreads across 3,4,5,6,7
   if ((gframe % myMod) == 0)
   {
       TeleportRandom();
       if (teleported) M_TELEPORTBEGIN = false;
   }
 }
}