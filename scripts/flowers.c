require utils, utils_steps, pxparticles;

#Flowers //:D

int   FlowerData[99] ;
float FlowerPosX[99] ;
float FlowerPosY[99] ;

int Flower_Amount;
int Flower_Empty;
int Flower_RED;  int Flower_PINK; int Flower_BLUE; int Flower_YELLOW;  int Flower_ORANGE; int Flower_PURPLE;    
int Flower_Sunflower; int Flower_Rose; int Flower_Lily; int Flower_Tulip; int Flower_Orchid; 

CTraceRes *FlowerRes;

CFlowerManager : CObject;

void flowers_uwu::FirstFrame()
{
 Flower_Empty  = 0;
 Flower_Rose   = 2;  
 Flower_Tulip  = 3;  
 Flower_Orchid = 4;    
 Flower_Lily   = 5;   
 Flower_Sunflower = 1;   //only has one color
 
 Flower_RED    = RGB(245, 120, 120);
 Flower_PINK   = RGB(245, 155, 245);
 Flower_BLUE   = RGB(55,  110, 215);
 Flower_YELLOW = RGB(245, 245, 150);
 Flower_ORANGE = RGB(235, 130, 90);
 Flower_PURPLE = RGB(115, 225, 235);    //actually Light Blue
 
 if (Flower_Amount == 0) Flower_Amount = RandomInt(15,20);
}; 

CSprite * flowers;

void flowers_uwu::InitGraphic()
{
 flowers = LoadSprite(GetAttachment("flowersheet.png"),10,0);
};

int flower_offset_x(int flower)
{
  if      (flower == Flower_Lily)      return 4;  
  else if (flower == Flower_Orchid)    return 13;
  else if (flower == Flower_Sunflower) return -4;
  else
  return 0;
};

int flower_offset_y(int flower)
{
  if      (flower == Flower_Lily || flower == Flower_Orchid)    return 11;  
  else if (flower == Flower_Tulip)                              return 7;
  else if (flower == Flower_Rose || flower == Flower_Sunflower) return 6;
};
  
int checkFlowerType(int flower)
{
    if (flower == 0) return 0;
    if (flower == 1) return 1;  // sunflower already has color

    local base;
    base = flower - Flower_RED;    if (base >= 2 && base <= 5) return base;
    base = flower - Flower_PINK;   if (base >= 2 && base <= 5) return base;
    base = flower - Flower_BLUE;   if (base >= 2 && base <= 5) return base;
    base = flower - Flower_YELLOW; if (base >= 2 && base <= 5) return base;
    base = flower - Flower_ORANGE; if (base >= 2 && base <= 5) return base;
    base = flower - Flower_PURPLE; if (base >= 2 && base <= 5) return base;
    return 0;
};

override void CTurnGame::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
super;
 if (Type == M_FRAME && gframe == 2)
 {
    CFlowerManager * rndflowers = new CFlowerManager(Flower_Amount);
 }
}

int CheckFlowerProximity(CGObject*sender, float scanRadius)
{
    cPosX = sender->PosX;
    cPosY = sender->PosY;
    int closestIndex = 98;
    float closestDistSqr = scanRadius * scanRadius;

    // Assuming you have access to the global flower arrays or a pointer to the manager
    for (local i = 0; i < Flower_Amount; i++)
    {
        // Ignore empty/dead flowers
        if (FlowerData[i] == 0) continue; 

        float dx = FlowerPosX[i] - cPosX;
        float dy = FlowerPosY[i] - cPosY;
        float distSqr = dx * dx + dy * dy;

        if (distSqr < closestDistSqr)
        {
            closestDistSqr = distSqr;
            closestIndex = i;
        };
    };

    return closestIndex; // Returns -1 if nothing is close, or the array index [i] if found
};

CFlowerManager::CFlowerManager(int amount)
{
        currentSlice = 0;
        currentSliceAttempts = 0;
        firstSpawned  = false;
        placedFlowers = false;
        failCount = 0;
 
        flowerQuantity = amount;
        
        customSize = 1.0;
    	animSeed = GS->Tick;
 
        super (Root,GS);
};

void CFlowerManager::DrawFlowers()
{
    customSize =  1.0 + ((sin((GS->Tick/10)+animSeed)) * 0.05);
    
    for (local i = 0; i < flowerQuantity; i++)
        {
                int shift = 0;
                local flipped = false;
                if (i % 2 == 0) { shift = 262144; flipped = true; }
                local cflower = checkFlowerType(FlowerData[i]);
                local hasColorMod = true;
                if (cflower == 1) hasColorMod = false;
                if (cflower <= 0 || cflower > 5) continue;
                float flowerSpr = subSprIndex(9,cflower-1);
                local minusone = -1;
                if (flipped == false) minusone = 1;    
                if (hasColorMod)
                SetColorMod(FlowerData[i] - cflower,8);
                AddSpriteEx( 5.0 + 0.01, FlowerPosX[i] + flower_offset_x(cflower) * minusone, FlowerPosY[i] - flower_offset_y(cflower), flowers->Index + shift, flowerSpr, 0, customSize);
                if (hasColorMod) ClearColorMod();         
                flowerSpr = subSprIndex(9, (cflower-1)+5); //Green stem
                AddSpriteEx( 5.0 + 0.02, FlowerPosX[i] + flower_offset_x(cflower) * minusone, FlowerPosY[i] - flower_offset_y(cflower), flowers->Index + shift, flowerSpr, 0, customSize);
                
                /*PxParticle *p;

               	p = new PxParticle(129, FlowerPosX[i] + flower_offset_x(cflower) * minusone + RandomInt(-8, 8), FlowerPosY[i] - flower_offset_y(cflower) + RandomInt(-30, -15));

               	p->SetLifeTime(50);
               	p->SetMotionRandomness(0.1);
               	p->SetAirResistance(0.5);
               	float s = RandomFloat(0.6, 0.9);
               	p->SetStartSize(s, s);
               	p->SetEndSize(s-0.3, s-0.3);
               	//p->SetStartAlpha(255);
               	//p->SetEndAlpha(255);
               	p->SetBlendMode(8);
               	//p->SetStartColor(255, 255, 160);
               	//p->SetEndColor(255, 255, 70);
               	p->SetRandomVelocity(0.0,0.0);
               	p->GravityFactor(-0.5);
               	p->SetWindFactor(0.3); */
        }
}

void CFlowerManager::CheckFlowerDamage(float expX, float expY, int expDmg)
{
    if (expDmg <= 0) return;
    
    float fdmg = expDmg;
    float fdmgsqr = fdmg * fdmg * 4.0;

    for (local i = 0; i < flowerQuantity; i++)
    {
        // Skip already dead/empty flowers
        if (FlowerData[i] == 0) continue; 

        float dx = FlowerPosX[i] - expX;
        float dy = FlowerPosY[i] - expY;
        float distsqr = dx * dx + dy * dy;

        // Check if it's within the blast radius
        if (distsqr < fdmgsqr)
        {
            float calculatedDmg = (1.0 - distsqr / fdmgsqr) * fdmg;
            if (calculatedDmg > 0.05)
            {
                // Destroy the flower
                FlowerData[i] = 0; 
            }
        }
    }
}

void CFlowerManager::Message(CObject* sender, EMType Type, int MSize, CMessageData* MData)
{
    super;
    if (currentSlice < flowerQuantity)
    {
        if (Type == M_FRAME && gframe > 2)
        {
            currentSliceAttempts = currentSliceAttempts + 1;
            
            local sliceWidth = GS->LevelSX / flowerQuantity;
            local sliceStart = currentSlice * sliceWidth;
            local sliceEnd = sliceStart + sliceWidth;

            // Keep them away from the extreme map edges
            if (sliceStart < 30) sliceStart = 30;
            if (sliceEnd > GS->LevelSX - 30) sliceEnd = GS->LevelSX - 30;

            local tryX = RandomInt(sliceStart, sliceEnd);
            local tryY = RandomInt(-350, Env->Water - 100);
            local hitX = 0; local hitY = 0;

            // Trace straight down to find the terrain surface
            local tracing = TraceLine(this, float(tryX), tryY, float(tryX), float(Env->Water) - 70, CMASK_TERRAIN, &hitX, &hitY);

            if (tracing != NullObj)
            {
                float spawnX = float(hitX);
                float spawnY = float(hitY);
                
                local dirt = TraceLine(this, spawnX, spawnY - 15, spawnX, spawnY, CMASK_TERRAIN, &hitX, &hitY);
                
                if (dirt == NullObj) // Space above is clear
                {
                    // Store the coordinates globally
                    FlowerPosX[currentSlice] = spawnX;
                    FlowerPosY[currentSlice] = spawnY;
                    
                    FlowerData[currentSlice] = RandomInt(1,5);
                    
                    if (FlowerData[currentSlice] != 1)
                    {
                        FlowerData[currentSlice] += chooseBetweenNums(
                        Flower_RED,
                        Flower_PINK,
                        Flower_BLUE,
                        Flower_YELLOW,
                        Flower_ORANGE,
                        Flower_PURPLE, 6);
                    }
                    // Move to the next slice
                    currentSlice = currentSlice + 1;
                    currentSliceAttempts = 0;
                }
                else if (currentSliceAttempts >= 25)
                {
                    // Too many fails in this slice (e.g., complex terrain). Mark as empty and move on.
                    FlowerData[currentSlice] = 0;
                    currentSlice = currentSlice + 1;
                    currentSliceAttempts = 0;
                }
            }
            else if (currentSliceAttempts >= 25)
            {
                // No terrain found below this X coordinate (e.g., a pit). Mark empty.
                FlowerData[currentSlice] = 0;
                currentSlice = currentSlice + 1;
                currentSliceAttempts = 0;
            }
        }
    }
    else
    {
        if (Type == M_EXPLOSION) 
        {
                CheckFlowerDamage(MData->fparams[1], MData->fparams[2], MData->params[4]);
        }
        if (Type == M_DRAWQUEUE) 
        {
                DrawFlowers();
        }
        
    }
}