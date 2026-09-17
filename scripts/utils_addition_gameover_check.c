bool isTeamEqual;
int GlobalTC;
bool GameOver;

void match_over::Init()
{
   GlobalTC = 0;
   isTeamEqual = true;
   GameOver = false;
} 

override CWorm::CWorm(CObject* Parent,int aTeam,int aIndex,CWormParams* params)
{
   skipCheck = false;
   Zombie = false;
   super;
   if (GlobalTC == 0)
   {
    if (!Zombie) GlobalTC = GetTeamColor(WormTeam);
   }
   else
   {
       if (GlobalTC != GetTeamColor(WormTeam) && !Zombie)
       {
           isTeamEqual = false;  
       }
   }  
}

bool AreWormsNull()
{
    for (local i = 0; i < Env->Objs.Count; i++)
    {
        CGObject* obje = Env->Objs.Objs[i];       
        if (obje == NullObj) continue;
        if (obje->ClType != OC_Worm) continue;  // Skip OC_Cross or other custom Worm classes.
        
        CWorm *worm = CWorm(obje);    
        if (worm->Zombie == true || worm->skipCheck == true) continue;
        if (worm != NullObj) return false;
    } 
    return true;   
}

bool isGameOver()
{
    if (isTeamEqual)
    {
        return AreWormsNull(); // If match begins with just one color, game goes on.
    }

    local teamColor = -1;
    
    for (local i = 0; i < Env->Objs.Count; i++)
    {
        CGObject* obj = Env->Objs.Objs[i];       
        if (obj == NullObj) continue;
        if (obj->ClType != OC_Worm) continue;  // Skip OC_Cross or other custom Worm classes.
        
        CWorm *worm = CWorm(obj);    
        if (worm->Zombie == true || worm->skipCheck == true) continue;
        
        if (worm == NullObj) continue;
        if (worm->ClType != OC_Worm) continue;  // Skip OC_Cross or other custom Worm classes.
        if (worm->TargHealth <= 0 || worm->ObjState == WS_DEAD || worm->ObjState == WS_DEATH) continue; // Skip dead worms
        if (GS->Info.GetWormHealth( worm->WormTeam, worm->WormNumber) <= 0) continue;
        
        if (worm->skipCheck || worm->Zombie) continue; // Skip surrendered/zombie worms

        local wormColor = GetTeamColor(worm->WormTeam);
        
        if (teamColor == -1)
        {
            teamColor = wormColor;
        }
        else if (teamColor != wormColor)
        {
            return false; // Found at least two different active colors 
        }
    } 
    
    return true; // Only 1 team color or all worms are Null
}     

override void CWorm::Message(CObject* sender, EMType Type, int MSize, CMessageData* MData)
{
    super;
    if (Type == M_TEAMVICTORY) GameOver = true;
    if (Type == M_SURRENDER)   skipCheck = true;
    if (Zombie)                skipCheck = true; 
}   

override void CTurnGame::Message(CObject* sender, EMType Type, int MSize, CMessageData* MData)
{
    super;
    if (Type == M_GAMEOVER || Type == M_TEAMVICTORY) 
    {
        GameOver = true;
    }
    if (Type == M_TURNEND || Type == M_TURNFINISHED)
    {
        GameOver = isGameOver();
    }      
}    