require utils, utils_steps, pxeffects, weapon_bowlingball, utils_red, weapon_electric_mine;

#ELECTRIC_PLUGIN

bool TMSpawn;
int  TMChance;

void electric_plugin::Init()
{
 TMSpawn = true; // Change in Your_Script::FirstFrame() which should run after this.
 TMChance = 3;
}

override CMine::CMine(CObject* Parent,CMineParams* Params,CShootDesc* SDesc,bool Snap,int Unk)
{
     super;
     if (!TMSpawn) return;
     if (TMChance < 1) TMChance = 2;
     if (RandomInt(1,TMChance) ==1)
     tazed_m
     =
     true
     ;
}