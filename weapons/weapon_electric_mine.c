require utils, pxeffects, utils_red;

override CMine::CMine(CObject* Parent,CMineParams* Params,CShootDesc* SDesc,bool Snap,int Unk)
{
    tazed_m = false;
    rayCooldown1 = RandomInt(6,22);
    rayCooldown2 = rayCooldown1 + 12;  
    linkedEffect1 = CEffectManager(NullObj);    
    linkedEffect2 = CEffectManager(NullObj);   
 
    super;    
}                                              

void CMine::TazeMine()
{
    tazed_m = true;  
    
    if (linkedEffect1 != NullObj && linkedEffect2 != NullObj) return;
    if (ClType!=OC_Mine) return; // Only taze actual mines.              
    if (linkedEffect1 == NullObj)linkedEffect1 = new CEffectManager(this, PosX, PosY, 99, 15, 15, 4.0, 130, 180, 255);  
    linkedEffect1->glow = false;        
    linkedEffect1->SetTrail( 12, 2, 0);
    
    if (linkedEffect2 == NullObj) linkedEffect2 = new CEffectManager(this, PosX, PosY, 99, 15, 15, 4.0, 130, 180, 255);  
    //linkedEffect2->AttachHeartShape(25, 2.0);       
    linkedEffect2->glow = false;        
}

override void CMine::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
    super;
    
    if (!tazed_m) return;
    if (ClType != OC_Mine) return;
                              
    if (Type == M_DRAWQUEUE)
    {
         if (tazed_m)
         {
              drawSparkEle(PosX, PosY, 4.0, 0.0, 0.85);
              drawGlowCircle(PosX, PosY, 14.4, 0.18, RGB(20,40,70));
         }
    }
    
    if (Type == M_FRAME && gframe > 2)
    {
        if (linkedEffect1 == NullObj)
        { 
            TazeMine();  
        }
        if (rayCooldown1 > 0 && linkedEffect1->hasReachTarget == false) 
        {
            rayCooldown1 -=1;
        }
        if (rayCooldown1 == 0)
        {
            linkedEffect1->SetTargetNoTerrain(PosX,PosY,PosX + RandomInt(-35,35),PosY + RandomInt(-35,35), 2, RandomFloat(0.08, 0.18), float(RandomInt(-130,130)));
            rayCooldown1 = RandomInt(13,51);
        }
        if (linkedEffect2 == NullObj)
        { 
            TazeMine();
        }
        if (rayCooldown2 > 0 && linkedEffect2->hasReachTarget == false) 
        {
            rayCooldown2 -=1;
        }
        if (rayCooldown2 == 0)
        {                 
            linkedEffect2->SetTargetNoTerrain(PosX,PosY,PosX + RandomInt(-35,35),PosY + RandomInt(-35,35), 2, RandomFloat(0.08, 0.18), float(RandomInt(-130,130)));
            rayCooldown2 = RandomInt(13,51);
        }
    }
}; 

/*override void CTurnGame::SecondFrame()
{
   super;
   for (local i = 0; i < Env->Objs.Count; i++)
   {
 
      CGObject *obj = Env->Objs.Objs[i];
      if (obj == NullObj) continue;
      if (obj is CGObject == false) continue;
      if (obj->ClType == OC_Mine )
      {
         local amine = *CMine(obj);
         amine->TazeMine();
      }
   }
} */    

override void CMine::DoExplosion(fixed x,fixed y,int PushPower,int Damage,int unkB,int Team)
{
if (tazed_m && ClType == OC_Mine)
{
local explos = do_cus_exp_electric(Root, -1, x, y, Damage * 1.25, PushPower * 1.25, Damage * 1.25, true);      //not sure why this doesnt dmg lol
local explxoe = do_custom_explosion(Root, -1, x, y, Damage * 1.25, PushPower * 1.25, Damage * 1.25, true, false, true, true, 0);
//do_custom_explosion(this, CMASK_DEFAULT_COLLIDEABLE, PosX,PosY)
}
else super;
}

override void CWorm::FireFinal(CWeapon* Weap, CShootDesc* Desc)
{
 if (Weap->CheckName("Electric Mine") == true)
 {
    local objint = Env->Objs.Count;
    super;
    CMine * mine = Env->Objs.GetObject(objint);
    mine->TazeMine();
 }
  else super;
}