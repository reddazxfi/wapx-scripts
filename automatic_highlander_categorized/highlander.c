require utils, utils_steps;

// 1. Three distinct categories for perfect loadout balancing
int NormalWeapons[150];    
int UtilityWeapons[150];   
int MobilityWeapons[150];  

int CavernWeapons[150];    
int CavernUtility[150];    
int CavernMobility[150];

int HasWeapons[6400]; 

int NormalCount;
int UtilityCount;
int MobilityCount;

int CavernCount;
int CavernUtilCount;
int CavernMobCount;

CWorm* CurW;

void highlander::InitGraphic()    
{                                
}
   
bool IsInvalidWeapon(CWeapon * weap, int i)
{
    if (i <= 100 || i == 119 || i == 124 || i == 157 || 
        i == 159 || i == 161 || i == 166 || i == 167 || 
        i == 169 || i >= 170)  return true;  
    
    local chweap = weap;
    if (chweap != NullObj) 
    {
        if (chweap->CheckName("None") == true) return true;  
        else if (chweap->CheckName("Skip Go") == true) return true; 
        else if (chweap->CheckName("Surrender") == true) return true; 
        else if (chweap->CheckName("Skip Go/Surrender") == true) return true;     
        else if (chweap->CheckName("Select Worm") == true) return true; 
        else if (chweap->CheckName("Kamikaze Rocket") == true) return true;  
        else if (chweap->CheckName("Kamikaze") == true) return true;    
        else if (chweap->CheckName("Suicide Bomber") == true) return true;   
        else if (chweap->CheckName("Teleport") == true) return true;   
        else if (chweap->CheckName("Crate Spy") == true) return true;   
        else if (chweap->CheckName("Crate Shower") == true) return true;  
        else if (chweap->CheckName("Muahaha 3") == true) return true;  
        else if (chweap->CheckName("Double Turn Time") == true) return true;  
        else if (chweap->CheckName("Exp. Rain") == true) return true;   
        else if (chweap->CheckName("Damage*2") == true) return true;   
        else if (chweap->CheckName("None (Select Worm)") == true) return true;  
        else if (chweap->CheckName("Fire Pellet") == true) return true;    
        else if (chweap->CheckName("--------") == true) return true;       
        else if (chweap->CheckName("-------") == true) return true;       
        else if (chweap->CheckName("------") == true) return true;          
        else if (chweap->CheckName("-----") == true) return true;           
        else if (chweap->CheckName("----") == true) return true;           
        else if (chweap->CheckName("---") == true) return true;        
        else if (chweap->CheckName("--") == true) return true;           
        else if (chweap->CheckName("-") == true) return true;            
        else if (chweap->CheckName(" ") == true) return true;   
        else if (chweap->CheckName("") == true) return true;                                                            
    }
    else return true;                                                 
    
    return false;
} 

// UNIVERSAL MOBILITY CHECK (Traversal & Movement)
bool IsUniversalMobility(CWeapon* chweap)
{
    if (chweap == NullObj) return false;
    
    if (chweap->CheckName("Ninja Rope") || chweap->CheckName("Bungee")             ||
        chweap->CheckName("Jet Pack") ||  chweap->CheckName("Super Jet Pack")      ||  
        chweap->CheckName("Super Blowtorch") ||    chweap->CheckName("Super Drill")||
        chweap->CheckName("Jump") || chweap->CheckName("Double Jump")              ||
        chweap->CheckName("Hang-Glider") || chweap->CheckName("Super Tele")        ||
        chweap->CheckName("Tele Arrow") || chweap->CheckName("Portal Gun")         ||
        chweap->CheckName("Helicopter") || chweap->CheckName("Swim Suit")          ||
        chweap->CheckName("Hoverboard") || chweap->CheckName("Trampoline")         ||
        chweap->CheckName("Anti Fall Damage") || chweap->CheckName("Low Gravity")  ||
        chweap->CheckName("BlowTorch") ||chweap->CheckName("Gravity Control")      ||
        chweap->CheckName("Drill") || chweap->CheckName("Fire Jump")               ||
        chweap->CheckName("Siege ladder") || chweap->CheckName("Ghost Jet Pack")   || 
        chweap->CheckName("Climb") || chweap->CheckName("Parkour")                 ||
        chweap->CheckName("Drilling Crew") || chweap->CheckName("Anti Fall Damage") 
        )
    {
        return true;         
    }
    return false;
}                                                                                  

// UNIVERSAL UTILITY CHECK (Tools, Passives, Melee, Guns, nothing too powerful)
bool IsUniversalUtility(CWeapon* chweap)
{
    if (chweap == NullObj) return false;
    
    if (chweap->CheckName("Die")            || chweap->CheckName("EMP")             ||
        chweap->CheckName("Fan")            || chweap->CheckName("Uzi")             ||
        chweap->CheckName("Prod")           || chweap->CheckName("FAMAS")           ||
        chweap->CheckName("Stone")          || chweap->CheckName("Tazer")           ||
        chweap->CheckName("Endure")         || chweap->CheckName("Freeze")          ||
        chweap->CheckName("Girder")         || chweap->CheckName("Magnum")          ||
        chweap->CheckName("Pistol")         || chweap->CheckName("Ripper")          ||
        chweap->CheckName("Handgun")        || chweap->CheckName("Javelin")         ||
        chweap->CheckName("Lantern")        || chweap->CheckName("Longbow")         ||
        chweap->CheckName("MedPack")        || chweap->CheckName("Minigun")         ||
        chweap->CheckName("Shotgun")        || chweap->CheckName("Ice Bolt")        ||
        chweap->CheckName("Pillager")       || chweap->CheckName("Popkagun")        ||
        chweap->CheckName("Far Sight")      || chweap->CheckName("Fire-Bolt")       ||
        chweap->CheckName("Ice Arrow")      || chweap->CheckName("Laser Gun")       ||
        chweap->CheckName("Medkit 25")      || chweap->CheckName("Battle Axe")      ||
        chweap->CheckName("Blast Zone")     || chweap->CheckName("Fire Punch")      || 
        chweap->CheckName("Keeper Bee")     || chweap->CheckName("Mini-Torch")      ||
        chweap->CheckName("Multi-Bolt")     || chweap->CheckName("Ninja Bomb")      ||
        chweap->CheckName("Ninja Star")     || chweap->CheckName("Sentry Gun")      ||
        chweap->CheckName("Shield Gun")     || chweap->CheckName("Slowmotion")      ||
        chweap->CheckName("Spot Hider")     || chweap->CheckName("Burst Rifle")     ||
        chweap->CheckName("Dragon Ball")    || chweap->CheckName("EMP Limited")     ||
        chweap->CheckName("Gravity Gun")    || chweap->CheckName("Health Zone")     ||
        chweap->CheckName("Laser Sight")    || chweap->CheckName("Mine Turret")     ||
        chweap->CheckName("Shadow-Bolt")    || chweap->CheckName("Super Drill")     ||
        chweap->CheckName("Torch Flame")    || chweap->CheckName("Baseball Bat")    ||
        chweap->CheckName("Bowling Ball")   || chweap->CheckName("Crate Strike")    ||
        chweap->CheckName("Flamethrower")   || chweap->CheckName("Flame Turret")    ||
        chweap->CheckName("Homing Curse")   || chweap->CheckName("Real Shotgun")    ||
        chweap->CheckName("Sentry Laser")   || chweap->CheckName("Skin Changer")    ||
        chweap->CheckName("Sniper Rifle")   || chweap->CheckName("3 Girder Pack")   ||
        chweap->CheckName("Antifire Suit")  || chweap->CheckName("Auto-ballista")   ||
        chweap->CheckName("Crate Grabber")  || chweap->CheckName("Double Dragon")   ||
        chweap->CheckName("Electromagnet")  || chweap->CheckName("Flame Thrower")   ||
        chweap->CheckName("Friendly Mine")  || chweap->CheckName("Petrol Turret")   ||
        chweap->CheckName("Power Shotgun")  || chweap->CheckName("Reinforcement")   ||
        chweap->CheckName("Smoke Grenade")  || chweap->CheckName("Homing Longbow")  ||
        chweap->CheckName("Barrel Launcher")|| chweap->CheckName("Explosive Arrow") ||
        chweap->CheckName("Girder Launcher")|| chweap->CheckName("Marked For Death")||
        chweap->CheckName("Phoenix Blaster")|| chweap->CheckName("Pneumatic Drill") ||
        chweap->CheckName("Weap/Util Crate")|| chweap->CheckName("Homing Keeper Bee")||
        chweap->CheckName("Lightning Strike")||chweap->CheckName("Magnetic Missile") ||
        chweap->CheckName("Staff of Jolting")||chweap->CheckName("Automatic Shotgun")||
        chweap->CheckName("Health Crate (100)")||chweap->CheckName("Girder Starter-Pack")||
        chweap->CheckName("Reflective Laser Gun"))
    {
        return true;
    }
    return false;
}

void highlander::FirstFrame()    
{
    NormalCount = 0;
    UtilityCount = 0;
    MobilityCount = 0;
    
    CavernCount = 0;
    CavernUtilCount = 0;
    CavernMobCount = 0;

    for(local t = 0; t < 6400; t++) { HasWeapons[t] = 0; }

    for (int i = 99; i <= 210; i++)
    {   
        CWeapon* wep = GS->GetWeaponByIndex(i);
        local invalidW = IsInvalidWeapon(wep, i);
        
        if (!invalidW)
        {                 
            int idx = wep->GlobalIndex;
            
            // 1. Check if it's a Mobility Item
            if (IsUniversalMobility(wep))
            {
                MobilityWeapons[MobilityCount] = idx;
                MobilityCount++;
                if (wep->UseInCavern) { CavernMobility[CavernMobCount] = idx; CavernMobCount++; }
            }
            // 2. Check if it's a Tool / Utility / Melee
            else if (IsUniversalUtility(wep))
            {
                UtilityWeapons[UtilityCount] = idx;
                UtilityCount++;
                if (wep->UseInCavern) { CavernUtility[CavernUtilCount] = idx; CavernUtilCount++; }
            }
            // 3. If it's neither, it defaults to a Standard Attack Weapon!
            else 
            {
                NormalWeapons[NormalCount] = idx;
                NormalCount++;
                if (wep->UseInCavern) { CavernWeapons[CavernCount] = idx; CavernCount++; }
            }
        }
    }
}

override CWorm::CWorm(CObject* Parent,int aTeam,int aIndex,CWormParams* params)
{
    super;
    HasWeaponsNum = 0;
    teamN = 800 * WormTeam - 800;
    wormN = 100 * WormNumber - 100;
    multipl = teamN + wormN;
    dead = false;
}

override void CWorm::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
    if (Type == M_FRAME && gframe == 25)
    {
        local prodd = GS->GetWeaponByName( "Prod" );
        local wpCrate = GS->GetWeaponByName( "Weapon Crate" );   
        local fwlk = GS->GetWeaponByName( "Fast Walk" );
       
        if (prodd != NullObj)  GS->Info.SetWeaponEx(WormTeam, prodd->GetWeaponIndex(), prodd->GetPageIndex(), 1); 
        if (wpCrate != NullObj)  GS->Info.SetWeaponEx(WormTeam, wpCrate->GetWeaponIndex(), wpCrate->GetPageIndex(), 1);  
        if (fwlk != NullObj)  GS->Info.SetWeaponEx(WormTeam, fwlk->GetWeaponIndex(), fwlk->GetPageIndex(), -1); 
    }
    
    if(Type == M_TURNBEGIN)
    {
        if(CurW != NullObj){CurW->RemoveWeapons();}
        CurW = this;
        GiveWeapons();
    }
    
    if(Type == M_TURNEND)
    {
        RemoveWeapons();
    }
    
    if(Type == M_FRAME)
    {
        if (HasWeaponsNum == 0)
        {
            teamN = 800 * WormTeam - 800;
            wormN = 100 * WormNumber - 100;
            multipl = teamN + wormN; 
            HasWeaponsNum = 3;
            
            for (int burn = 0; burn < multipl % 17; burn++)
                RandomInt(0, 100);
            
            // Generate distinct RNG indices for the three categories
            local pickAttack = RandomInt(0, NormalCount - 1);
            local pickUtility = RandomInt(0, UtilityCount - 1);
            local pickMobility = RandomInt(0, MobilityCount - 1);
            
            if (GS->IsCavern)
            {
                // Assign 1 Cavern Weapon, 1 Cavern Tool, 1 Cavern Movement
                HasWeapons[multipl]   = CavernWeapons[pickAttack % CavernCount];
                HasWeapons[multipl+1] = CavernUtility[pickUtility % CavernUtilCount];
                HasWeapons[multipl+2] = CavernMobility[pickMobility % CavernMobCount];
            }
            else
            {
                // Assign 1 Standard Weapon, 1 Standard Tool, 1 Standard Movement
                HasWeapons[multipl]   = NormalWeapons[pickAttack];
                HasWeapons[multipl+1] = UtilityWeapons[pickUtility];
                HasWeapons[multipl+2] = MobilityWeapons[pickMobility];
            }
        }
           
        if(GS->Info.GetWormHealth(WormTeam, WormNumber) == 0 && dead == false)
        {
            dead = true;
            if(CurW != this && CurW != NullObj && CurW->dead != true)
            {                        
                StealWeapons(CurW);
            }
                
            if(CurW == this && CurW != NullObj && CurW->dead == true)
            {
                RemoveWeapons();
            }
        }
    }
    super;
}

void CWorm::GiveWeapons()
{
    for(local t = 0; t < HasWeaponsNum; t++)
    {
        int wepID = HasWeapons[multipl + t];
        
        if (wepID != 0) 
        {
            local wep = GS->GetWeaponByIndex(wepID);
            if (wep != NullObj)
            {
                GS->Info.SetWeaponEx(WormTeam, wep->GetWeaponIndex(), wep->GetPageIndex(), 1); 
            }
        }
    }
    
    local wc = GS->GetWeaponByName("Wind Changer");
    local hab = GS->GetWeaponByName("Hot Air Balloon"); 
    if (hab == NullObj) hab = GS->GetWeaponByName("Parachute");

    if (wc != NullObj)  GS->Info.SetWeaponEx(WormTeam, wc->GetWeaponIndex(), wc->GetPageIndex(), 1);
    if (hab != NullObj) GS->Info.SetWeaponEx(WormTeam, hab->GetWeaponIndex(), hab->GetPageIndex(), 3);
}

void CWorm::RemoveWeapons()
{
    for(local t = 0; t < HasWeaponsNum; t++)
    {
        int wepID = HasWeapons[multipl + t];
        
        if (wepID != 0)
        {                                               
            local wep = GS->GetWeaponByIndex(wepID);
            if (wep != NullObj)
            {
                GS->Info.SetWeaponEx(WormTeam, wep->GetWeaponIndex(), wep->GetPageIndex(), 0);
            }
        }
    }
}

void CWorm::StealWeapons(CWorm* stealer)
{              
    for(local t = 0; t < HasWeaponsNum; t++)
    {
        int wepID = HasWeapons[multipl + t];
        
        if (wepID != 0) 
        {
            local wep = GS->GetWeaponByIndex(wepID);
            if (wep != NullObj)
            {
                GS->Info.SetWeaponEx(stealer->WormTeam, wep->GetWeaponIndex(), wep->GetPageIndex(), 1);
            }
        }
    }

    for(local t = 0; t < HasWeaponsNum; t++)
    {
        HasWeapons[stealer->multipl + stealer->HasWeaponsNum + t] = HasWeapons[multipl + t];
    }
    
    stealer->HasWeaponsNum = stealer->HasWeaponsNum + HasWeaponsNum;
}