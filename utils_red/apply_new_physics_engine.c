require utils, script_newphysics;

void betterbounce::FirstFrame()
{}
   
override void CMissile::CMissile(CObject* parent,CWeaponLaunch* ldata,CShootDesc* sdata)
{ 
  //NewBounce = true;
    super;     
    appliedBounce = false;
  //NewBounce = true;
}    

override void CMissile::Message(CObject* sender, EMType Type, int MSize, CMessageData* MData)
{  
  super;
  if (Type == M_FRAME && !appliedBounce && !NewBounce && GravityFactor > 0 )
  {                      
        NewBounce = true;    
        CShootDesc sdesc;
        zero(&sdesc);
        sdesc.X = PosX;
        sdesc.Y = PosY; 
        sdesc.SpX = SpX;
        sdesc.SpY = SpY;
        sdesc.Team = nadeTeam;
        sdesc.Worm = 0;
        sdesc.Delay = 0;
        appliedBounce = true;
        ActivateBouncePhysics(NullObj, &launchdata, &sdesc, 0, 2, 0.60, 0.027, 0, 0, false); 
        if ( weap->CheckName( "Dynamite" ) == true  ||  launchdata.anim.spriteIndex == 78 ) nadeRestitution = 0.102 ;  //Dynamite
  }
  else appliedBounce = true;
}

require utils; 
//Custom CMASK

 
override void CMissile::Message(CObject* sender,EMType Type,int MSize,CMessageData* MData)
{
super;

if (Type == M_FRAME && NewBounce && !fixedMask)
{
 if (weap->CheckName("Nuke Rocket") == true) 
 {
   fixedMask = true;
   ColMask = new CColMask(3,3,MakeCircleMask(3));
 }
 
 if (weap->CheckName("Bunker Buster") == true) 
 {   
   fixedMask = true;
   NewBounce = false;
   nadeRestitution = 0.0;
   ColMask = new CColMask(2,2,MakeCircleMask(2));
 }

 if (weap->CheckName("Super Bunker Buster") == true) 
 {
   fixedMask = true;    
   NewBounce = false;
   nadeRestitution = 0.0;
   ColMask = new CColMask(2,2,MakeCircleMask(2));
 }
 if (weap->CheckName("Exp. Rain") == true || GravityFactor == -1) 
 {
   fixedMask = true;    
   NewBounce = false;
   nadeRestitution = 0.0;
   ColMask = new CColMask(2,2,MakeCircleMask(2));
 }
  /*
 if (weap->CheckName("Ripper") == true) 
 {
   fixedMask = true;
   ColMask = new CColMask(2,2,MakeCircleMask(2));
 }
 if (weap->CheckName("Shadow Bolt") == true) 
 {
   fixedMask = true;
   ColMask = new CColMask(2,2,MakeCircleMask(2));
 }      
 if (weap->CheckName("Super Shadow Bolt") == true) 
 {
   fixedMask = true;
   ColMask = new CColMask(2,2,MakeCircleMask(2));
 }   */
 fixedMask = true;
}
} 