//Asked claude free what functions could be useful, fair enough i guess

// asin/acos via identity — engine only has atan2
// asin(x) = atan2(x, sqrt(1 - x*x))
// acos(x) = atan2(sqrt(1 - x*x), x)
float asin_rx(float x) //asin_r
{
    if (x >  1.0) x =  1.0;
    if (x < -1.0) x = -1.0;
    return atan2(x, sqrt(1.0 - x*x));
};

float acos_rx(float x) //acos_r
{
    if (x >  1.0) x =  1.0;
    if (x < -1.0) x = -1.0;
    return atan2(sqrt(1.0 - x*x), x);
};

// Angle between two vectors — useful for cone detection, turret FOV checks
float AngleBetween(float ax, float ay, float bx, float by)
{
    local dot = ax*bx + ay*by;
    local lenA = sqrt(ax*ax + ay*ay);
    local lenB = sqrt(bx*bx + by*by);
    if (lenA < 0.001 || lenB < 0.001) return 0.0;
    local cosVal = dot / (lenA * lenB);
    return acos_rx(cosVal);
};

// Is point inside cone — useful for sentry FOV, homing missile acquisition
bool IsInCone(float originX, float originY,
              float dirX, float dirY,
              float targetX, float targetY,
              float halfAngleRad)
{
    local dx = targetX - originX;
    local dy = targetY - originY;
    local dist = sqrt(dx*dx + dy*dy);
    if (dist < 0.001) return true;
    dx = dx / dist; dy = dy / dist;
    local dot = dirX*dx + dirY*dy;
    return (dot >= cos(halfAngleRad));
};

// Reflect a direction vector off a normal — pure math, no tracing
// Useful for laser previews, predictive bounce indicators
void ReflectVector(float vx, float vy, float nx, float ny,
                   float* outX, float* outY)
{
    local dot = vx*nx + vy*ny;
    if (dot > 0.0) { nx = 0.0-nx; ny = 0.0-ny; dot = 0.0-dot; }
    *outX = vx - 2.0 * dot * nx;
    *outY = vy - 2.0 * dot * ny;
};

// Clamp
float ClampValue(float val, float minVal, float maxVal)
{
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
};

// Sign — returns 1, -1, or 0
int SignOf(float val)
{
    if (val > 0.0) return 1;
    if (val < 0.0) return -1;
    return 0;
};

// Normalize a 2D vector, returns false if degenerate
bool NormalizeVector(float* x, float* y)
{
    local len = sqrt(*x * *x + *y * *y);
    if (len < 0.001) return false;
    *x = *x / len;
    *y = *y / len;
    return true;
};