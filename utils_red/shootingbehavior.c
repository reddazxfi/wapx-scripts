// By reddaz, SPRITE BUILDER RECOMMENDED
// USE SPRITE BUILDER NEW PARAM OF Params->FSprite = YourSprite->Index; AND IT WILL WORK :)
require utils, utils_steps, weapon_bowlingball, weapon_tazer, pxeffects;
// utils, utils_steps entire lib, ball, weapon_tazer and pxeffects are for custom taze and explosion.

void CWorm::onCustomShoot(int shootNmb, int spreadNmb)
{
//override void CWorm::onCustomShoot(int shootNmb, int spreadNmb) {} //Your custom code 
}

void CWorm::OnFiringAnim(int shootNmb, int animFrame, bool isFiring)
{
 //empty on purpose
}

override CWorm::CWorm(CObject * Parent, int aTeam, int aIndex, CWormParams * params)
{
    super;
    shotCD = 0;
    shotsLeft = 0;
    ShotsCooldown = 0;
    ShotsFired = 0;
    SpreadNFired = 0;
    Shooting = false;
    ConstantFire = false;
    FireAnimation = false; 
    currentCooldown = 0;
    currentDLY = 0;       
    shootingSound = 0;
    
    burstSize = 0;   
    totalBursts = 0; 
    
    childIndex = 0;
    childRange = 0;
    
    animSHTFrames = 0;
    animDLYFrames = 0;
    
    drawCrosshair = false;
    drawGun       = false;      
    cusFireAngle  = -0.01;
}

// Call this to start the custom fire (ActivateFiring(3, 4, 0, 1) for 4 bursts of 3 bullets and 1 cooldown)
void CWorm::ActivateFiring(int bulletsPerBurst, int numBursts, int shotSound, int cooldown)
{
    ConstantFire = true;
    burstSize = bulletsPerBurst;
    totalBursts = numBursts;
    shootingSound = shotSound;
    
    shotsLeft = numBursts;
    ShotsFired = 0;
    SpreadNFired = 0;
    ShotsCooldown = cooldown;
    currentCooldown = 0;
    Shooting = false;
                                                                 
    SPBFiring = true;
    SPBCooldown = false;
}

/*override void CWorm::FireFinal(CWeapon* Weap, CShootDesc* Desc)
{
    if (Weap != NullObj && ConstantFire && cusFireAngle == -0.01 && !drawCrosshair)
    {     
        // Avoid hitting yourself
        Desc->Y -= 3.0;                 
        if (FireAngle < 0.167) { Desc->Y -= 2.95; Desc->X += 2 * TurnSide; }
        
        // Cooldown check at the top, gates everything below
        if (currentCooldown > 0)
        {
            currentCooldown--;
            SPBFiring  = false;
            SPBCooldown = true;
            return;
        }
        // Handle the bullets inside the current burst
        if (shootingSound > 0) 
            PlaySound(shootingSound, 3.0, CalculateSoundVolume(PosX, PosY), 1.0);
        
        super(Weap, Desc);
        
        onCustomShoot(ShotsFired, SpreadNFired);
        ShotsFired++;          // incremented BEFORE any re-entry can happen
        SPBFiring  = true;
        SPBCooldown = false;
        Shooting   = true;
        
        
        // Check for bursts left to fire
        if (ShotsFired >= burstSize)
        {
            ShotsFired = 0;
            Shooting   = false;
            currentCooldown = 0;
             
            if (SpreadNFired < (totalBursts - 1))
            {
                // More bursts remaining
                SpreadNFired++;
                shotsLeft--;
                ShowMessage2(itoa(shotsLeft), " bursts remaining");
                nAvalShoots  = 1;
                nTotalShoots = 0;
                SetState(WS_AIMING);
                return;
            }
             // Bursts done, end turn
            ConstantFire = false;
            SpreadNFired = 0;
            shotsLeft    = 0;
            nTotalShoots = 1;   
            SetState(WS_IDLE);   // Needed to get out of the loop
            return;
        }
        
        // More shots left in this burst
        currentCooldown = ShotsCooldown;
        nAvalShoots  = 1;
        nTotalShoots = 0;
        return;
    }        
    else if (Weap!=NullObj && !ConstantFire) 
        {
         super;
        } 
    else super;
}  */

override void CWorm::FireFinal(CWeapon* Weap, CShootDesc* Desc)
{
    if (Weap != NullObj && ConstantFire && cusFireAngle == -0.01 && !drawCrosshair)
    {     
        // Avoid hitting yourself
        Desc->Y -= 3.0;                  
        if (FireAngle < 0.167) { Desc->Y -= 2.95; Desc->X += 2 * TurnSide; }        
        
        // Cooldown check at the top, gates everything below
        if (currentCooldown > 0)
        {
            currentCooldown--;
            SPBFiring   = false;
            SPBCooldown = true;
            nAvalShoots  = 1;
            nTotalShoots = 0;
            return;
        }

        // Pre-shot delay / Charging buffer
        if (FireAnimation && currentDLY > 0)
        {
            // Compute current frame index (0 to animDLYFrames - 1)
            int currentFrame = animDLYFrames - currentDLY;
            
            // Trigger custom anim hook
            OnFiringAnim(ShotsFired, currentFrame, false);
            
            currentDLY--;
            
            nAvalShoots  = 1;
            nTotalShoots = 0;
            return;
        }
        
        // Handle the bullets inside the current burst
        if (shootingSound > 0) 
            PlaySound(shootingSound, 3.0, CalculateSoundVolume(PosX, PosY), 1.0);
        
        super(Weap, Desc); 
        
        onCustomShoot(ShotsFired, SpreadNFired);
        
        // Trigger anim hook for projectile release (isFiring = true)
        if (FireAnimation)
        {
            OnFiringAnim(ShotsFired, 0, true);
        }

        ShotsFired++;
        SPBFiring   = true;
        SPBCooldown = false;
        Shooting    = true;
        
        // Check for bursts left to fire
        if (ShotsFired >= burstSize)
        {
            ShotsFired = 0;
            Shooting   = false;
            currentCooldown = 0;
            currentDLY = animDLYFrames; // Reset wind-up for next burst
             
            if (SpreadNFired < (totalBursts - 1))
            {
                // More bursts remaining
                SpreadNFired++;
                shotsLeft--;
                ShowMessage2(itoa(shotsLeft), " bursts remaining");
                nAvalShoots  = 1;
                nTotalShoots = 0;
                SetState(WS_AIMING);
                return;
            }

             // Bursts done, end turn
            ConstantFire  = false;
            FireAnimation = false;
            SpreadNFired  = 0;
            shotsLeft     = 0;
            nTotalShoots  = 1;   
            SetState(WS_IDLE);
            return;
        }
        
        // More shots left in this burst
        currentDLY      = animDLYFrames; 
        currentCooldown = ShotsCooldown;
        nAvalShoots  = 1;
        nTotalShoots = 0;
        return;
    }        
    else if (Weap != NullObj && !ConstantFire) 
    {
        super;
    } 
    else super;
} 

override void CWorm::Message(CObject* sender, EMType Type, int MSize, CMessageData* MData)
{
    super;
    
    local cw = GetCurrentWorm();
    if (cw == NullObj || cw != this) return;
    
    if (Type == M_SETWEAPON)  
    {
        ShotsFired = 0;   
        currentCooldown = 0;
        currentDLY = 0;
        Shooting = false;
        ConstantFire = false;  // Failsafe 
        FireAnimation = false; 
        SPBFiring = true;
        SPBCooldown = false;
    }
    
    if (Type == M_FRAME)
    {        
        // persist animation.
        if (Shooting && ConstantFire)
        {
            nShoots = 1;
            nShooted = 0;
        }
    }
}
//Doesnt work somehow, placeholder, dont use.
void CWorm::ActivateFireAnim(int shootAmount, int shootBursts, int shotSound, int delayFrames, int shootFrames, int afterShotCD)
{
    FireAnimation   = true;
    shootingSound   = shotSound;
    animDLYFrames   = delayFrames;
    animSHTFrames   = shootFrames;
    currentDLY      = delayFrames; // Charges up first
    
    ConstantFire    = true;
    burstSize       = shootAmount;
    totalBursts     = shootBursts;
    
    shotsLeft       = shootBursts;
    ShotsFired      = 0;
    SpreadNFired    = 0;
    
    ShotsCooldown   = shootFrames + afterShotCD; // Total recovery time
    currentCooldown = 0;                         // No extra initial delay
    Shooting        = false;
    
    SPBFiring       = true;
    SPBCooldown     = false;
}