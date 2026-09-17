///global res 

string fuckstring;
int stringfucker[1];

string stringintP(int targ)
{
   stringfucker[-1*1] = targ;
   return fuckstring;
};  //  [-1*1] = targ; - Returns the previous variable that was declared, in this case it's fuckstring, that is how PX works.

CTraceRes * spawnCheckRes;
CTraceArcRes * CheckArcRes;
CColMask * ColMask3s;    
CWeapon clustlets;

int CMASK_ALL;
int CMASK_EVERYTHING;
int CMASK_ALL_WORMS;     
int Ti_CGObject;
int New_Entry;
float RainbowGlobalHue;
float RainbowSpeed;


string NullString;

void _enums_::InitGraphic()
{    
    ColMask3s = new CColMask(3,3,MakeCircleMask(3)); 
};
void _enums_::Init()
{    
    RainbowGlobalHue = 0.0;
    RainbowSpeed     = 0.009;   // 0.003 = full cycle every 330 frames
    
    Ti_CGObject = 25;
    New_Entry;
    //Root->GetObject(Ti_CGObject, New_Entry);
    CMASK_ALL = -1;            
    CMASK_EVERYTHING = -1;
    CMASK_ALL_WORMS = 4 + 8 + 16 + 32 + 64 + 256 ;   
    NullString = "Null";  
    
    spawnCheckRes = new CTraceRes();  //Deleting and creating stuff mid frame is expensive, just let it be. Works well for several objs using same res at the same time (so far)
    CheckArcRes   = new CTraceArcRes(); //  
};
                                                                     
void _enums_::FirstFrame()
{
    zero(&clustlets);

    clustlets.NameA          = 16;
    clustlets.NameB          = 16;
    clustlets.Row             = 12;
    clustlets.Remember        = true;
    clustlets.UseInCavern     = false;
    clustlets.NShoots         = 1;
    clustlets.EndsTurn        = true;
    clustlets.RetreatTime     = 3000;
    clustlets.IsWeaponCrate   = true;
    clustlets.Probablity      = 0;
    clustlets.Count           = 1;
    clustlets.GlobalIndex     = 46;
    clustlets.ActivationType  = AT_Crosshair;
    clustlets.Graphic         = 6;
    clustlets.WeaponType      = WT_Launcher;

    ResetClustletLaunch();
} 