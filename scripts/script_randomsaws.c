require utils, p_sprite_builder, utils_steps, utils_red, map_object_saw;

bool spawnedSaw;
CSawRandSpawn : CObject;

void randomsaws::FirstFrame()
{
   spawnedSaw = false;
}

override void CTurnGame::Message(CObject* sender, EMType Type, int MSize, CMessageData* MData)
{
    super;
    
    if (gframe >= 1 && !spawnedSaw) 
    {
        local sawspawn = new CSawRandSpawn();
        if (sawspawn != NullObj) 
        {
            spawnedSaw = true;
        };
    };
}

CSawRandSpawn::CSawRandSpawn()
{
    super(Root, GS);
    currentSlice = 0; // Start at the first slice  
    currentSliceAttempts = 0;
    currentSawSize = RandomFloat(0.17, 0.65);
    SawRadius = currentSawSize * 100;
    ColMaskSaw = new CColMask(SawRadius + 3,SawRadius + 3,MakeCircleMask(SawRadius + 3));
    
    if (sawQuantity <= 0) sawQuantity = 4; // Fallback
}
/// Saw 
void CSawRandSpawn::Message(CObject* sender, EMType Type, int MSize, CMessageData* MData)
{
    if (currentSlice < sawQuantity)
    {
        if (Type == M_FRAME && gframe > 1)
        {
            currentSliceAttempts = currentSliceAttempts + 1;
            local tryY = RandomInt(0 + 100, Env->Water - 100);
            local sliceWidth = GS->LevelSX / sawQuantity;
            local sliceStart = currentSlice * sliceWidth;
            local sliceEnd = sliceStart + sliceWidth;

            if (sliceStart < 30) sliceStart = 30;
            if (sliceEnd > GS->LevelSX - 30) sliceEnd = GS->LevelSX - 30;

            local tryX = RandomInt(sliceStart, sliceEnd);
            local hitX = 0; local hitY = 0; local hitY2 = 0;

            local tracing = TraceLine(this, float(tryX), tryY, float(tryX), float(Env->Water) - 70, CMASK_TERRAIN, &hitX, &hitY);
            local tracing2 = TraceLine(this, float(tryX), tryY, float(tryX), float(0) - 70, CMASK_TERRAIN, &hitX, &hitY2);
            
            if (tracing !=NullObj && tracing2!=NullObj)
            {
                hitY = chooseNum(hitY, hitY2);
            }
            else if (tracing == NullObj && tracing2!=NullObj)
            {
                hitY = hitY2;
                tracing = tracing2;
            }
            
            if (tracing != NullObj)
            {
                float spawnX = float(hitX);
                float spawnY = float(hitY);
                sawcheckmask = CMASK_ALL_WORMS | CMASK_MINE | CMASK_OILDRUM;
                // Hazard Check
                local spawnOK = CheckSpawnPoint(spawnX, spawnY - 5.0, spawnX, spawnY - 5.0, ColMaskSaw, (currentSawSize * 100.0) * 0.32, sawcheckmask, &spawnX, &spawnY, SawRadius * 1.8);
                
                local trace2 = TraceLine(this, spawnX - 8, spawnY - 8, spawnX + 8, spawnY + 8, CMASK_TERRAIN | CMASK_OILDRUM, &hitX, &hitY);   //not inside terrain
                
                if (trace2 == NullObj && spawnOK)
                {
                    CMineParams MParams;
                    zero(&MParams);
                    MParams.Prefuse  = 0;
                    MParams.Fuse     = 0; 
                    MParams.Radius   = 70;    
                    MParams.Flags    = 0;
                    MParams.Bias     = 0;
                    MParams.Damage   = 0;
                    MParams.BlastPower = 0;      
                    
                    CShootDesc SDesc; 
                    zero(&SDesc);
                    SDesc.Team = 0;      
                    SDesc.Worm = 0;     
                    SDesc.Delay = 0;
                    SDesc.X = spawnX;
                    SDesc.Y = spawnY - 1.0; 
                    
                    Rndmtgn = RandomInt(1,3);
                    if (Rndmtgn == 1 || Rndmtgn == 2 )
                    {
                         SDesc.Delay = 25;
                    };
                 
                    CSaw * spwnbk = new CSaw(Root->GetObject(25, 0), &SDesc, currentSawSize); 
                    if (spwnbk!=NullObj)
                    {
                        electricornot = RandomInt(1,7);
                        if  (electricornot == 5)  
                            spwnbk->supercharged = true;
                            
                        currentSlice = currentSlice + 1;     
                        currentSawSize = RandomFloat(0.17, 0.65);
                        SawRadius = currentSawSize * 100;
                        ColMaskSaw = new CColMask(SawRadius +3,SawRadius + 3,MakeCircleMask(SawRadius + 3));  
                    // Move on to the next slice for the next frame
                    };
                }
                else if ((tracing != NullObj || !spawnOK) && currentSliceAttempts < 10)
                { 
                        currentSawSize = RandomFloat(0.162,0.642); //maybe it was too big
                        SawRadius = currentSawSize * 100;        
                        ColMaskSaw = new CColMask(SawRadius +3,SawRadius + 3,MakeCircleMask(SawRadius + 3));  
                }  
                else if ((tracing != NullObj || !spawnOK) && currentSliceAttempts >= 10)   
                { 
                        currentSlice = currentSlice + 1;     
                        currentSliceAttempts = 0;
                };
            }
            else if (tracing != NullObj && currentSliceAttempts >= 10 )     
            { 
                        currentSlice = currentSlice + 1;     
                        currentSliceAttempts = 0;
            };
        };
    }
    else
    {
    delete ColMaskSaw;
    Free(true);
    };
};