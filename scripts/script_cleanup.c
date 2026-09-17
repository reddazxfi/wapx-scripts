
require utils, utils_steps;

bool checkDone;

void sixmillion::Init()
{
 checkDone = false;
}

override void CTurnGame::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
   super;
   if (Type == M_FRAME)
   {
      if ((GameStartedToDestroy || GameOver) && !checkDone)
      {
         for (local i=0; i < Env->Objs.Count; i++)
         {
            CGObject *obj = CGObject(Env->Objs.Objs[i]);
            if (obj == NullObj) continue;
            if (obj->IsMaterial == false) continue;
      
            if (obj->ClType == OC_Missile || obj->ClType == OC_OilDrum || obj->ClType == OC_Mine || obj->ClType == OC_Crate || obj->ClType == OC_Saw || obj->ClType == OC_Bee || obj->ClType == OC_Portal || obj->ClType == OC_BowlingBall || obj->ClType == OC_Sentry ||obj->ClType == OC_CustomTurret )
            {
                  obj->Free(true);
            }
            if  (i >= Env->Objs.Count - 1) {checkDone = true;}
         }
         if (#Flowers)  if (FlowerMan!=NullObj) FlowerMan->Free(true);
         if (#EXPLODER) if (Exploder!=NullObj) Exploder->Free(true);
         if (#effects)  freeEffects = true;
      }
   }
}