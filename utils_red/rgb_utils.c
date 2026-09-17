override void CTurnGame::Message(CObject* sender, EMType Type, int MSize, CMessageData* MData)
{
    super;
    if (Type == M_FRAME)
    {
        RainbowGlobalHue += RainbowSpeed;
        if (RainbowGlobalHue > 1.0) RainbowGlobalHue -= 1.0;
        if (gframe == 3) { SecondFrame(); };                       
    }
}

void CTurnGame::SecondFrame() {RainbowGlobalHue = 0.009; }

// Fast, smooth 3-phase sine rainbow (No black dips, continuous curves)
int FastSmoothRainbow(float phase)
{
    // Convert 0.0..1.0 range into radians (2 * PI)
    float rad = phase * 6.2831853; 

    // Center at 180, amplitude 75 -> range [105 .. 255]
    // Phase offsets: Red = 0, Green = 120 deg (2.094 rad), Blue = 240 deg (4.188 rad)
    int rr = int(180.0 + 75.0 * sin(rad));
    int gg = int(180.0 + 75.0 * sin(rad + 2.0943951));
    int bb = int(180.0 + 75.0 * sin(rad + 4.1887902));

    return RGB(rr, gg, bb);
}

int RRGB(float phaseOffset)
{
    return FastSmoothRainbow(RainbowGlobalHue + phaseOffset);
}

// FIX: Changed int offset to float offset
void SetRainbowMod(float offset, int blendType)
{
    SetColorMod(RRGB(offset), blendType);
}

void SetRainbowMod(int offset, int blendType)
{
   SetColorMod(RRGB(offset), blendType);
};

void r_g_b_Brightness(int amount, int *r, int *g, int *b)
{
     *r += amount;
     *g += amount;     
     *b += amount;
     if (*r <0) *r = 0;   
     if (*g <0) *g = 0;
     if (*b <0) *b = 0; 
     if (*r >255) *r = 255;   
     if (*g >255) *g = 255;
     if (*b >255) *b = 255;
};

int RGB_Brightness(int amount, int RgB)
{
     int r; int g; int b;
     stripRGB(RgB, &r, &g, &b);
     
     r_g_b_Brightness(amount, &r, &g, &b);
     
     return RGB(r,g,b);
}; 

int ARGB_Brightness(int amount, int aRGB)
{
     int a; int r; int g; int b;
     stripARGB(aRGB, &a, &r, &g, &b);
     
     r_g_b_Brightness(amount, &r, &g, &b);
     
     return ARGB(a,r,g,b);
}; 

void mult_r_g_b(float amount, int *r, int *g, int *b) 
{
     * r = *r * amount;        
     * g = *g * amount;
     * b = *b * amount;
     if (*r <0) *r = 0;   
     if (*g <0) *g = 0;
     if (*b <0) *b = 0; 
     if (*r >255) *r = 255;   
     if (*g >255) *g = 255;
     if (*b >255) *b = 255;
}; 

int mult_RGB(float amount, int RgB) 
{
     int r; int g; int b;
     stripRGB(RgB, &r, &g, &b);                     
     
     mult_r_g_b(amount, &r, &g, &b); 
     
     return RGB(r,g,b);
}; 

void stripRGB(int RGB, int *r, int *g, int *b)
{
     // Remove the Alpha channel
     int pRGB = RGB & 16777215; 
     
     *r = (pRGB & 16711680)   >> 16; // Red channel   (0x00FF0000)
     *g = (pRGB & 65280)      >>  8; // Green channel (0x00FF00)
     *b =  pRGB               & 255; // Blue channel  (0x00FF)
};                                                                                                                          

void stripARGB(int ARGB, int* a, int * r, int *g, int *b)
{
     *r = (ARGB & 16711680)   >> 16; // Red channel   (0x00FF0000)
     *g = (ARGB & 65280)      >>  8; // Green channel (0x00FF00)
     *b =  ARGB               & 255; // Blue channel  (0x00FF)
     
     //Alpha channel (0xFF000000), overflows to negative (int is 4278190080) 
     *a = (ARGB >> 24) & 127;
     if   (ARGB < 0)
     {
         *a = *a + 128; // Restores the top bit if it was set
     }
};