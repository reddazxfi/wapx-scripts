require utils;

//Also AI Slop
                                                                                                       //fullCircle uses the full radian, maxTilt limits it (you manually turn the sprite with SIndex + 290524523)
float CalculateRotationFixed(float currentX, float currentY, float lookX, float lookY, float currentRot, float turnSpeed, bool fullCircle, float maxTilt)
{     //for still objects
    float dx = lookX - currentX;
    float dy = lookY - currentY;
    float dist = sqrt(dx*dx + dy*dy);

    float targetTilt = 0.0;

    if (dist > 1.0) 
    {
        if (fullCircle) 
        {
            targetTilt = atan2(dy, dx);
        }
        else 
        {
            targetTilt = atan2(dy, absfloat(dx));
            
            if (targetTilt > maxTilt) targetTilt = maxTilt;
            if (targetTilt < (0.0 - maxTilt)) targetTilt = 0.0 - maxTilt;
        }
    }
    else
    {
        targetTilt = currentRot;
    }

    float diff = targetTilt - currentRot;
    
    // Always take the shortest path toward the target
    if (fullCircle)
    {
        while (diff > 3.14159) diff = diff - 6.28318;
        while (diff < -3.14159) diff = diff + 6.28318;
    }

    float angleGap = absfloat(diff);
    float newRot = currentRot;

    // Smoothly apply rotation continuously
    if (angleGap <= turnSpeed) 
    {
        // This keeps the variable purely continuous.
        newRot = currentRot + diff; 
    } 
    else 
    {
        if (diff > 0.0) 
        {
            newRot = currentRot + turnSpeed;
        } 
        else 
        {
            newRot = currentRot - turnSpeed;
        }
    }

    return newRot;
}

float CalculateDynamicRotation(float currentX, float currentY, float targetX, float targetY, float currentRot, float smoothingFactor, bool fullCircle, float minTilt, float maxTilt)
{
    float dx = targetX - currentX;
    float dy = targetY - currentY;
    
    float instantAngle = 0.0;

    // 1. Calculate the instantaneous absolute angle
    if (fullCircle)
    {
        instantAngle = atan2(dy, dx);
    }
    else
    {
        // Right-side math only
        instantAngle = atan2(dy, abs(dx));
        
        // 2. Clamp the angle limits (only applies to non-full circle)
        if (instantAngle > maxTilt) instantAngle = maxTilt;
        if (instantAngle < minTilt) instantAngle = minTilt;
    }

    // 3. Find the difference between where we are and where we want to be
    float diff = instantAngle - currentRot;
    
    // 4. Prevent the 180-degree wrap-around snap for full 360-degree objects
    if (fullCircle)
    {
        while (diff > 3.14159) diff = diff - 6.28318;
        while (diff < -3.14159) diff = diff + 6.28318;
    }

    // 5. PROPORTIONAL INTERPOLATION
    // Multiply the difference by the smoothing factor (e.g., 0.25)
    float newRot = currentRot + (diff * smoothingFactor);
    
    return newRot;
}    

void BounceAgainstSphere(CGObject *sender, float x, float y, int force)
{
    if (sender == NullObj) return;

    // Normal Vector
    float dx = (sender->PosX) - x;
    float dy = (sender->PosY) - y;
    
    local dist = sqrt(dx * dx + dy * dy);
    float nx = 0.0;
    float ny = 0.0;
    
    // Safety check
    if (dist < 0.001) 
    { 
        nx = 0.0; 
        ny = -1.0; 
    }
    else 
    { 
        nx = dx / dist; 
        ny = dy / dist; 
    };

    // Incoming velocity
    float vx = sender->SpX;
    float vy = sender->SpY;

    // How direct was the impact?
    float dot = (vx * nx) + (vy * ny);

    // Prevent the math from pulling the object INWARD if it's already moving away
    if (dot > 0.0) 
    {
        nx = -nx;
        ny = -ny;
        dot = -dot;
    };

    // Standard Vector Reflection Formula
    // v_new = v - 2(v · n)n
    local newSpX = vx - (2.0 * dot * nx);
    local newSpY = vy - (2.0 * dot * ny);

    // Apply the custom Force kick directly outward along the normal
    newSpX = newSpX + (nx * force);
    newSpY = newSpY + (ny * force);
    
    local finalSpeed = sqrt(newSpX * newSpX + newSpY * newSpY);
    
    if (finalSpeed < 2.0) 
    {
        newSpX = nx * 2.0;
        newSpY = ny * 2.0;
    };

    sender->SpX = newSpX;
    sender->SpY = newSpY;
};

bool calculateReflectionOnSurface(CGObject* sender, fixed curPosX, fixed curPosY, float snapPosX, float snapPosY, float snapSpX, float snapSpY, int flags, float restitution, float friction, CTraceRes* res)
{
    if (sender == NullObj || res == NullObj) return false;
    
    local vx    = snapSpX + 0.0;
    local vy    = snapSpY + 0.0;
    local speed = sqrt(vx*vx + vy*vy);
    if (speed < 0.001) return false;
     // Direction of motion
    local dirX = vx / speed;
    local dirY = vy / speed;
    
    scanDist = 15.0;
    local endX = int(curPosX) + int(dirX * scanDist);
    local endY = int(curPosY) + int(dirY * scanDist);
        
    local gotNormal = detectSurfaceAverage(sender, dirX, dirY, curPosX, curPosY, snapPosX, snapPosY, res) ; 
    
    if (!gotNormal) gotNormal = TraceLineEx(sender, int(snapPosX), int(snapPosY), endX, endY, flags, res);  // Fallback 1, trace line
    
    if (!gotNormal)
    {
        // Fallback: trace along the incoming velocity direction from the snapshot point
        if (speed > 0.001)
        {
            fixed dirX = vx / speed;
            fixed dirY = vy / speed;
            local fallbackEndX = int(snapPosX) + int(dirX * scanDist);
            local fallbackEndY = int(snapPosY) + int(dirY * scanDist);
            gotNormal = TraceLineEx(sender, int(snapPosX), int(snapPosY), fallbackEndX, fallbackEndY, flags, res);
        }
    }
      
    local nx = 0.0;
    local ny = 0.0;  
    
    nx = res->NormalX;
    ny = res->NormalY;

    if (gotNormal && (res->NormalX * res->NormalX + res->NormalY * res->NormalY) > 0.01)
    {
        nx = res->NormalX;
        ny = res->NormalY;
    }
    else
    {
        nx = -dirX;
        ny = -dirY;
    }
                   
    // Ensure normal points toward the incoming projectile
    local dot = vx * nx + vy * ny;
    if (dot > 0.0) { nx = 0.0 - nx; ny = 0.0 - ny; dot = 0.0 - dot; }
    // Separate incoming velocity into normal and tangential components
    local vn    = dot;   // normal component (negative, into surface)
    local vt_x  = vx - vn * nx;
    local vt_y  = vy - vn * ny;
    local vn_out = (0.0 - vn) * restitution;  // Apply restitution to normal component (bounce back with reduced magnitude)

    vt_x = vt_x * (1.0 - friction);
    vt_y = vt_y * (1.0 - friction);

    local newSpX = vt_x + vn_out * nx;
    local newSpY = vt_y + vn_out * ny;

    local newSpd = sqrt(newSpX * newSpX + newSpY * newSpY);

    // Use a small push to clear the surface pixel
 
    local pushDist = 4;   //Default SpriteSize is 2
    if (sender is CMissile) 
    {
      CMissile *rlnd = CMissile(sender);
      pushDist = rlnd->nadeSize + 1.5; 
    }
    local spawnX = 0.0;
    local spawnY = 0.0;

    if (gotNormal)
    {
        spawnX = (res->HitX) + nx * pushDist;
        spawnY = (res->HitY) + ny * pushDist;
    }
    else
    {
        // Fallback: push from current position
        spawnX = (curPosX) + nx * pushDist;
        spawnY = (curPosY) + ny * pushDist;
    }
    //WriteToChat4(7,"Horiz. Speed: ", ftoa(newSpX), "Vertical Speed: ", ftoa(newSpY), false);
    res->HitX    = spawnX;
    res->HitY    = spawnY;
    res->PreHitX = newSpX;
    res->PreHitY = newSpY;
    res->NormalX = nx;
    res->NormalY = ny;
    res->Hit     = gotNormal;

    return gotNormal;
};

bool detectSurfaceAverage(CGObject *sender, float dirX, float dirY,fixed cPosX, fixed cPosY, float sPosX, float sPosY, CTraceRes* res)
{
    if (sender == NullObj)return;
    // First, ensure we have a valid starting point outside terrain
    fixed startX = sPosX;
    fixed startY = sPosY;
    
    // Optional: verify that start point is not inside terrain
    // If it is, move back along -dir until clear (safety)  
    // If even safePos ended up inside terrain (explosion moved it), back out
    local bail = 0;
    while (bail < 7)
    {
        if (CheckMaskAt(sender, ColMask3s, startX, startY, CMASK_TERRAIN) == NullObj)
               break;
        startX = startX - dirX * 2.0;
        startY = startY - dirY * 2.0;
        bail++;
    }
    
    // Compute ray length: travel distance from snapshot to current position + 30px guarantee
    float travelDx = cPosX - sPosX;
    float travelDy = cPosY - sPosY;
    float travelDist = sqrt(travelDx*travelDx + travelDy*travelDy);
    local scanDist = travelDist + 30.0;
    if (scanDist < 40.0) scanDist = 40.0;
    
    // End point along the velocity direction
    fixed endX = startX + dirX * scanDist;
    fixed endY = startY + dirY * scanDist;
    
    // Perpendicular directions for offset rays
    fixed perpX = -dirY;
    fixed perpY =  dirX;
    
    // Spread: scale with object size (if available)
    float spread = 6.0;
    if (sender is CMissile)
    {
        CMissile* rln = CMissile(sender);
        
        if (rln->nadeSize > 1)
            spread = rln->nadeSize * 1.5;
        else rln->nadeSize = 3.0;    
        
        if (spread>9.0) spread = 9.0;
    }
    float totalNX = 0.0; float totalNY = 0.0;
    int hitCount = 0;
    
    // Center ray – writes directly to 'res' so we get HitX/Y
    if (TraceLineEx(sender, startX, startY, endX, endY, CMASK_TERRAIN, res))
    {
        if ((res->NormalX*res->NormalX + res->NormalY*res->NormalY) > 0.01)
        {
            totalNX += res->NormalX;
            totalNY += res->NormalY;
            hitCount++;
        }
    }
    
    // Side rays (two pairs at 1x and 2x spread)
    for (int ray = 1; ray <= 2; ray++)
    {
        float offset = ray * spread;
        fixed offX = perpX * offset;
        fixed offY = perpY * offset;
        
        // Left side
        if (TraceLineEx(sender, startX + offX, startY + offY, endX + offX, endY + offY, CMASK_TERRAIN, spawnCheckRes))
        {
            if ((spawnCheckRes->NormalX*spawnCheckRes->NormalX + spawnCheckRes->NormalY*spawnCheckRes->NormalY) > 0.01)
            {
                totalNX += spawnCheckRes->NormalX;
                totalNY += spawnCheckRes->NormalY;
                hitCount++;
            }
        }
        
        // Right side
        if (TraceLineEx(sender, startX - offX, startY - offY, endX - offX, endY - offY, CMASK_TERRAIN, spawnCheckRes))
        {
            if ((spawnCheckRes->NormalX*spawnCheckRes->NormalX + spawnCheckRes->NormalY*spawnCheckRes->NormalY) > 0.01)
            {
                totalNX += spawnCheckRes->NormalX;
                totalNY += spawnCheckRes->NormalY;
                hitCount++;
            }
        }
    }
    
    if (hitCount == 0) return false;
    
    // Average normals
    float nx = totalNX / hitCount;
    float ny = totalNY / hitCount;
    float len = sqrt(nx*nx + ny*ny);
    if (len > 0.001) { nx = nx /len; ny = ny /len; }
    else { nx = -dirX; ny = -dirY; };
    
    // Ensure the normal points toward the incoming projectile
    //float dot = (sSpX * nx + sSpY * ny);  commented bc if speed is buggy this is pointless tbh
    //if (dot > 0) { nx = -nx; ny = -ny; }
    
    // If center ray missed but side rays hit, we need to create a Hit point
    if (res->Hit == false)
    {
        // Estimate hit point as current position minus half velocity
        res->HitX = cPosX - dirX * 5;
        res->HitY = cPosY - dirY * 5;
        res->Hit = true;
    }
    
    res->NormalX = nx;
    res->NormalY = ny;
    return true;
};   

bool CheckSpawnPointNoLOS(float spawnX, float spawnY, CColMask* colmask, float radius, int flags, float* outX, float* outY, int radiusLimit)
{
    if (colmask == NullObj) return false;
    *outX = spawnX;
    *outY = spawnY;
    if (radius < 1.0) radius = 2.0;

    float searchRadius = 0.0;
    while (searchRadius <= radiusLimit)
    {
        int degree = 0;
        int step;
        if (searchRadius == 0.0) step = 360;
        else step = int(maxfloat(15, 360.0 / (searchRadius / 2.0)));

        while (degree < 360)
        {
            float rad = degree * (MATH_PI / 180.0);
            int tx = int(spawnX + searchRadius * cos(rad) + 0.5);
            int ty = int(spawnY + searchRadius * sin(rad) + 0.5);

            if (CheckMaskAt(NullObj, colmask, tx, ty, flags) == NullObj)
            {
                *outX = tx + 0.0;
                *outY = ty + 0.0;
                return true;
            }
            degree = degree + step;
        }
        searchRadius = searchRadius + radius;
    };
    return false;
};