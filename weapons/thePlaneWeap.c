// Also 9-11
require utils, p_sprite_builder;

CWeapon airplaneWEAP;
 
 void thePlaneWeap::FirstFrame()
 { 
 zero(&airplaneWEAP);
 
 airplaneWEAP.NameA="AirPlane";  
 airplaneWEAP.NameB="AirPlane";
 airplaneWEAP.Row = 12;
 airplaneWEAP.Remember = true;
 airplaneWEAP.UseInCavern = false;
 airplaneWEAP.NShoots = 1;
 airplaneWEAP.EndsTurn = true;
 airplaneWEAP.RetreatTime = 3000;
 airplaneWEAP.IsWeaponCrate = true;
 airplaneWEAP.Probablity = 0;
 airplaneWEAP.Count = 1;
 airplaneWEAP.GlobalIndex = 32;///IDK
 airplaneWEAP.ActivationType =  AT_Crosshair;
 airplaneWEAP.Graphic = 6;          
 airplaneWEAP.WeaponType =  WT_Launcher;
 ////////////////////////ONLY THIS MATTERS////////////////////////
 airplaneWEAP.launch.spriteSize = 4;
 airplaneWEAP.launch.launchSpeed = 0; 
 airplaneWEAP.launch.runAway = true;
 airplaneWEAP.launch.explosion.flags=137342 + CMASK_OTHERWEAPON; //ALL Worms, Terrain, Oildrums, Crates, Dnor Cards, Skimming.
 airplaneWEAP.launch.explosion.bias = 30;
 airplaneWEAP.launch.explosion.pushPower = 100;
 airplaneWEAP.launch.explosion.damage = 60;
 airplaneWEAP.launch.explosion.dmgVar = 0;
 airplaneWEAP.launch.unk = 0;
 airplaneWEAP.launch.anim.spriteIndex = 0; 
 airplaneWEAP.launch.anim.type = WAT_TrackMovement;    
 airplaneWEAP.launch.anim.trailIndex = 131;   
 airplaneWEAP.launch.anim.trailPower = 4;
 airplaneWEAP.launch.anim.trailSpeed = 200;
 airplaneWEAP.launch.anim.unk = 0; 
 airplaneWEAP.launch.speedMultipler = 100;
 airplaneWEAP.launch.windFactor = 0;
 airplaneWEAP.launch.gravityFactor = 0;
 airplaneWEAP.launch.countdownFrom = 0;
 airplaneWEAP.launch.timeBeforeExplosion = 10000;
 airplaneWEAP.launch.sound.soundIndex = 0;
 airplaneWEAP.launch.sound.useExpSound = false;
 airplaneWEAP.launch.sound.expSoundIndex = 0;
 airplaneWEAP.launch.sound.expSoundDelay = 0; 
 airplaneWEAP.launch.explodeOnSpace = false;
 airplaneWEAP.launch.action = WAction_None;    
 airplaneWEAP.launch.explodeInto = WExplode_Nothing; 
 ////////////////////////ONLY THIS MATTERS////////////////////////  
 }
 

//CSprite* GrayplaneFlyS;  
 
CSprite* WhiteplaneFly;

int WhitePlaneSPR;

void thePlaneWeap::InitGraphic()
{ 
CFile *f;

f = GetAttachment("whitejet.png");
WhiteplaneFly = LoadSprite(f, 1, 0);  

//f = GetAttachment("grayjet.png");
//GrayplaneFlyS = LoadSprite(f, 1, 0);  

WhitePlaneSPR = WhiteplaneFly->Index;
}
                                                            
override planeCrash::planeCrash(CObject* parent,CWeaponLaunch* ldata,CShootDesc* sdata){
super;                                                                           
  planeFly_anim = true;                                                   
}                                                                                                                              