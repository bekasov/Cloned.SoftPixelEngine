/*
 * Sparse voxel octree procedures file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

/* === Macros === */

#define AXIS_X_POSITIVE 0 // +x
#define AXIS_X_NEGATIVE 1 // -x
#define AXIS_Y_POSITIVE 2 // +y
#define AXIS_Y_NEGATIVE 3 // -y
#define AXIS_Z_POSITIVE 4 // +z
#define AXIS_Z_NEGATIVE 5 // -z


/* === Functions === */

int GetDominantAxis(float3 Vec)
{
    /* Get absolute vector */
    float3 AbsVec = abs(Vec);

    /* Find dominant axis */
    if (AbsVec.x >= AbsVec.y && AbsVec.x >= AbsVec.z)
        return (Vec.x > 0.0 ? AXIS_X_POSITIVE : AXIS_X_NEGATIVE);
    if (AbsVec.y >= AbsVec.x && AbsVec.y >= AbsVec.z)
        return (Vec.y > 0.0 ? AXIS_Y_POSITIVE : AXIS_Y_NEGATIVE);
    return (Vec.z > 0.0 ? AXIS_Y_POSITIVE : AXIS_Y_NEGATIVE);
}
