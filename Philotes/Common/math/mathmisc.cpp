


#include "mathmisc.h"
#include "asm_math.h"
#include "vector2.h"
#include "vector3.h"
#include "vector4.h"
#include "ray.h"
#include "sphere.h"
#include "axisalignedbox.h"
#include "plane.h"

#include <xutility>

#pragma warning(disable:4305)

namespace Philo
{
	const scalar 	Math::PI = 3.141592653f;
	const scalar 	Math::POS_INFINITY = std::numeric_limits<scalar>::infinity();
	const scalar 	Math::NEG_INFINITY = -std::numeric_limits<scalar>::infinity();
	const scalar 	Math::TWO_PI = scalar( 2.0 * PI );
	const scalar 	Math::HALF_PI = scalar( 0.5 * PI );
	const scalar 	Math::fDeg2Rad = PI / scalar(180.0);
	const scalar 	Math::fRad2Deg = scalar(180.0) / PI;
	const scalar 	Math::LOG2 = log(scalar(2.0));
	const scalar	Math::INFINITE_FAR_PLANE_ADJUST = 0.00001f;

    int				Math::mTrigTableSize;
	Math::AngleUnit Math::msAngleUnit;

    scalar  		Math::mTrigTableFactor;
    scalar*			Math::mSinTable = NULL;
    scalar*			Math::mTanTable = NULL;

    //-----------------------------------------------------------------------
	Math::Math( uint trigTableSize )
    {
        msAngleUnit = AU_DEGREE;

        mTrigTableSize = trigTableSize;
        mTrigTableFactor = mTrigTableSize / Math::HALF_PI;

        //mSinTable = PH_ALLOC_T(scalar, mTrigTableSize, Memory::ObjectHeap);
		mSinTable = ph_new_array(scalar,mTrigTableSize);
        //mTanTable = PH_ALLOC_T(scalar, mTrigTableSize, Memory::ObjectHeap);
		mTanTable = ph_new_array(scalar,mTrigTableSize);

        buildTrigTables();
    }

    //-----------------------------------------------------------------------
    Math::~Math()
    {
        //PH_FREE(mSinTable, Memory::ObjectHeap);
        //PH_FREE(mTanTable, Memory::ObjectHeap);
		ph_delete_array(mSinTable);
		ph_delete_array(mTanTable);
    }

    //-----------------------------------------------------------------------
    void Math::buildTrigTables(void)
    {
        // Build trig lookup tables
        // Could get away with building only PI sized Sin table but simpler this 
        // way. Who cares, it'll ony use an extra 8k of memory anyway and I like 
        // simplicity.
        scalar angle;
        for (int i = 0; i < mTrigTableSize; ++i)
        {
            angle = Math::HALF_PI * i / mTrigTableSize;
            mSinTable[i] = sin(angle);
            mTanTable[i] = tan(angle);
        }	
    }
	//-----------------------------------------------------------------------	
	scalar Math::SinTable (scalar fValue)
    {
        // Convert range to index values, wrap if required
        int idx;
        if (fValue >= 0)
        {
            idx = int(fValue * mTrigTableFactor) % mTrigTableSize;
        }
        else
        {
            idx = mTrigTableSize - (int(-fValue * mTrigTableFactor) % mTrigTableSize) - 1;
        }

        return mSinTable[idx];
    }
	//-----------------------------------------------------------------------
	scalar Math::TanTable (scalar fValue)
    {
        // Convert range to index values, wrap if required
		int idx = int(fValue *= mTrigTableFactor) % mTrigTableSize;
		return mTanTable[idx];
    }
    //-----------------------------------------------------------------------
    int Math::ISign (int iValue)
    {
        return ( iValue > 0 ? +1 : ( iValue < 0 ? -1 : 0 ) );
    }
    //-----------------------------------------------------------------------
    Radian Math::ACos (scalar fValue)
    {
        if ( -1.0 < fValue )
        {
            if ( fValue < 1.0 )
                return Radian(acos(fValue));
            else
                return Radian(0.0);
        }
        else
        {
            return Radian(PI);
        }
    }
    //-----------------------------------------------------------------------
    Radian Math::ASin (scalar fValue)
    {
        if ( -1.0 < fValue )
        {
            if ( fValue < 1.0 )
                return Radian(asin(fValue));
            else
                return Radian(HALF_PI);
        }
        else
        {
            return Radian(-HALF_PI);
        }
    }
    //-----------------------------------------------------------------------
    scalar Math::Sign (scalar fValue)
    {
        if ( fValue > 0.0 )
            return 1.0;

        if ( fValue < 0.0 )
            return -1.0;

        return 0.0;
    }
	//-----------------------------------------------------------------------
	scalar Math::InvSqrt(scalar fValue)
	{
		return scalar(asm_rsq(fValue));
	}
    //-----------------------------------------------------------------------
    scalar Math::UnitRandom ()
    {
        return asm_rand() / asm_rand_max();
    }
    
    //-----------------------------------------------------------------------
    scalar Math::RangeRandom (scalar fLow, scalar fHigh)
    {
        return (fHigh-fLow)*UnitRandom() + fLow;
    }

    //-----------------------------------------------------------------------
    scalar Math::SymmetricRandom ()
    {
		return 2.0f * UnitRandom() - 1.0f;
    }

   //-----------------------------------------------------------------------
    void Math::setAngleUnit(Math::AngleUnit unit)
   {
       msAngleUnit = unit;
   }
   //-----------------------------------------------------------------------
   Math::AngleUnit Math::getAngleUnit(void)
   {
       return msAngleUnit;
   }
    //-----------------------------------------------------------------------
    scalar Math::AngleUnitsToRadians(scalar angleunits)
    {
       if (msAngleUnit == AU_DEGREE)
           return angleunits * fDeg2Rad;
       else
           return angleunits;
    }

    //-----------------------------------------------------------------------
    scalar Math::RadiansToAngleUnits(scalar radians)
    {
       if (msAngleUnit == AU_DEGREE)
           return radians * fRad2Deg;
       else
           return radians;
    }

    //-----------------------------------------------------------------------
    scalar Math::AngleUnitsToDegrees(scalar angleunits)
    {
       if (msAngleUnit == AU_RADIAN)
           return angleunits * fRad2Deg;
       else
           return angleunits;
    }

    //-----------------------------------------------------------------------
    scalar Math::DegreesToAngleUnits(scalar degrees)
    {
       if (msAngleUnit == AU_RADIAN)
           return degrees * fDeg2Rad;
       else
           return degrees;
    }

    //-----------------------------------------------------------------------
	bool Math::pointInTri2D(const Vector2& p, const Vector2& a, 
		const Vector2& b, const Vector2& c)
    {
		// Winding must be consistent from all edges for point to be inside
		Vector2 v1, v2;
		scalar dot[3];
		bool zeroDot[3];

		v1 = b - a;
		v2 = p - a;

		// Note we don't care about normalisation here since sign is all we need
		// It means we don't have to worry about magnitude of cross products either
		dot[0] = v1.crossProduct(v2);
		zeroDot[0] = Math::RealEqual(dot[0], 0.0f, 1e-3);


		v1 = c - b;
		v2 = p - b;

		dot[1] = v1.crossProduct(v2);
		zeroDot[1] = Math::RealEqual(dot[1], 0.0f, 1e-3);

		// Compare signs (ignore colinear / coincident points)
		if(!zeroDot[0] && !zeroDot[1] 
		&& Math::Sign(dot[0]) != Math::Sign(dot[1]))
		{
			return false;
		}

		v1 = a - c;
		v2 = p - c;

		dot[2] = v1.crossProduct(v2);
		zeroDot[2] = Math::RealEqual(dot[2], 0.0f, 1e-3);
		// Compare signs (ignore colinear / coincident points)
		if((!zeroDot[0] && !zeroDot[2] 
			&& Math::Sign(dot[0]) != Math::Sign(dot[2])) ||
			(!zeroDot[1] && !zeroDot[2] 
			&& Math::Sign(dot[1]) != Math::Sign(dot[2])))
		{
			return false;
		}


		return true;
    }
	//-----------------------------------------------------------------------
	bool Math::pointInTri3D(const Vector3& p, const Vector3& a, 
		const Vector3& b, const Vector3& c, const Vector3& normal)
	{
        // Winding must be consistent from all edges for point to be inside
		Vector3 v1, v2;
		scalar dot[3];
		bool zeroDot[3];

        v1 = b - a;
        v2 = p - a;

		// Note we don't care about normalisation here since sign is all we need
		// It means we don't have to worry about magnitude of cross products either
        dot[0] = v1.crossProduct(v2).dotProduct(normal);
		zeroDot[0] = Math::RealEqual(dot[0], 0.0f, 1e-3);


        v1 = c - b;
        v2 = p - b;

		dot[1] = v1.crossProduct(v2).dotProduct(normal);
		zeroDot[1] = Math::RealEqual(dot[1], 0.0f, 1e-3);

		// Compare signs (ignore colinear / coincident points)
		if(!zeroDot[0] && !zeroDot[1] 
			&& Math::Sign(dot[0]) != Math::Sign(dot[1]))
		{
            return false;
		}

        v1 = a - c;
        v2 = p - c;

		dot[2] = v1.crossProduct(v2).dotProduct(normal);
		zeroDot[2] = Math::RealEqual(dot[2], 0.0f, 1e-3);
		// Compare signs (ignore colinear / coincident points)
		if((!zeroDot[0] && !zeroDot[2] 
			&& Math::Sign(dot[0]) != Math::Sign(dot[2])) ||
			(!zeroDot[1] && !zeroDot[2] 
			&& Math::Sign(dot[1]) != Math::Sign(dot[2])))
		{
			return false;
		}


        return true;
	}
    //-----------------------------------------------------------------------
    bool Math::RealEqual( scalar a, scalar b, scalar tolerance )
    {
        if (fabs(b-a) <= tolerance)
            return true;
        else
            return false;
    }

    //-----------------------------------------------------------------------
    std::pair<bool, scalar> Math::intersects(const Ray& ray, const Plane& plane)
    {

        scalar denom = plane.normal.dotProduct(ray.getDirection());
        if (Math::Abs(denom) < std::numeric_limits<scalar>::epsilon())
        {
            // Parallel
            return std::pair<bool, scalar>(false, 0);
        }
        else
        {
            scalar nom = plane.normal.dotProduct(ray.getOrigin()) + plane.d;
            scalar t = -(nom/denom);
            return std::pair<bool, scalar>(t >= 0, t);
        }
        
    }
    //-----------------------------------------------------------------------
    std::pair<bool, scalar> Math::intersects(const Ray& ray, 
        const std::vector<Plane>& planes, bool normalIsOutside)
    {
		std::list<Plane> planesList;
		for (std::vector<Plane>::const_iterator i = planes.begin(); i != planes.end(); ++i)
		{
			planesList.push_back(*i);
		}
		return intersects(ray, planesList, normalIsOutside);
    }
    //-----------------------------------------------------------------------
    std::pair<bool, scalar> Math::intersects(const Ray& ray, 
        const std::list<Plane>& planes, bool normalIsOutside)
    {
		std::list<Plane>::const_iterator planeit, planeitend;
		planeitend = planes.end();
		bool allInside = true;
		std::pair<bool, scalar> ret;
		std::pair<bool, scalar> end;
		ret.first = false;
		ret.second = 0.0f;
		end.first = false;
		end.second = 0;


		// derive side
		// NB we don't pass directly since that would require Plane::Side in 
		// interface, which results in recursive includes since Math is so fundamental
		Plane::Side outside = normalIsOutside ? Plane::POSITIVE_SIDE : Plane::NEGATIVE_SIDE;

		for (planeit = planes.begin(); planeit != planeitend; ++planeit)
		{
			const Plane& plane = *planeit;
			// is origin outside?
			if (plane.getSide(ray.getOrigin()) == outside)
			{
				allInside = false;
				// Test single plane
				std::pair<bool, scalar> planeRes = 
					ray.intersects(plane);
				if (planeRes.first)
				{
					// Ok, we intersected
					ret.first = true;
					// Use the most distant result since convex volume
					ret.second = Math::Max(ret.second, planeRes.second);
				}
				else
				{
					ret.first =false;
					ret.second=0.0f;
					return ret;
				}
			}
			else
			{
				std::pair<bool, scalar> planeRes = 
					ray.intersects(plane);
				if (planeRes.first)
				{
					if( !end.first )
					{
						end.first = true;
						end.second = planeRes.second;
					}
					else
					{
						end.second = Math::Min( planeRes.second, end.second );
					}

				}

			}
		}

		if (allInside)
		{
			// Intersecting at 0 distance since inside the volume!
			ret.first = true;
			ret.second = 0.0f;
			return ret;
		}

		if( end.first )
		{
			if( end.second < ret.second )
			{
				ret.first = false;
				return ret;
			}
		}
		return ret;
    }
    //-----------------------------------------------------------------------
    std::pair<bool, scalar> Math::intersects(const Ray& ray, const Sphere& sphere, 
        bool discardInside)
    {
        const Vector3& raydir = ray.getDirection();
        // Adjust ray origin relative to sphere center
        const Vector3& rayorig = ray.getOrigin() - sphere.getCenter();
        scalar radius = sphere.getRadius();

        // Check origin inside first
        if (rayorig.squaredLength() <= radius*radius && discardInside)
        {
            return std::pair<bool, scalar>(true, 0);
        }

        // Mmm, quadratics
        // Build coeffs which can be used with std quadratic solver
        // ie t = (-b +/- sqrt(b*b + 4ac)) / 2a
        scalar a = raydir.dotProduct(raydir);
        scalar b = 2 * rayorig.dotProduct(raydir);
        scalar c = rayorig.dotProduct(rayorig) - radius*radius;

        // Calc determinant
        scalar d = (b*b) - (4 * a * c);
        if (d < 0)
        {
            // No intersection
            return std::pair<bool, scalar>(false, 0);
        }
        else
        {
            // BTW, if d=0 there is one intersection, if d > 0 there are 2
            // But we only want the closest one, so that's ok, just use the 
            // '-' version of the solver
            scalar t = ( -b - Math::Sqrt(d) ) / (2 * a);
            if (t < 0)
                t = ( -b + Math::Sqrt(d) ) / (2 * a);
            return std::pair<bool, scalar>(true, t);
        }


    }
    //-----------------------------------------------------------------------
	std::pair<bool, scalar> Math::intersects(const Ray& ray, const AxisAlignedBox& box)
	{
		if (box.isNull()) return std::pair<bool, scalar>(false, 0);
		if (box.isInfinite()) return std::pair<bool, scalar>(true, 0);

		scalar lowt = 0.0f;
		scalar t;
		bool hit = false;
		Vector3 hitpoint;
		const Vector3& min = box.getMinimum();
		const Vector3& max = box.getMaximum();
		const Vector3& rayorig = ray.getOrigin();
		const Vector3& raydir = ray.getDirection();

		// Check origin inside first
		if ( rayorig > min && rayorig < max )
		{
			return std::pair<bool, scalar>(true, 0);
		}

		// Check each face in turn, only check closest 3
		// Min x
		if (rayorig.x <= min.x && raydir.x > 0)
		{
			t = (min.x - rayorig.x) / raydir.x;
			if (t >= 0)
			{
				// Substitute t back into ray and check bounds and dist
				hitpoint = rayorig + raydir * t;
				if (hitpoint.y >= min.y && hitpoint.y <= max.y &&
					hitpoint.z >= min.z && hitpoint.z <= max.z &&
					(!hit || t < lowt))
				{
					hit = true;
					lowt = t;
				}
			}
		}
		// Max x
		if (rayorig.x >= max.x && raydir.x < 0)
		{
			t = (max.x - rayorig.x) / raydir.x;
			if (t >= 0)
			{
				// Substitute t back into ray and check bounds and dist
				hitpoint = rayorig + raydir * t;
				if (hitpoint.y >= min.y && hitpoint.y <= max.y &&
					hitpoint.z >= min.z && hitpoint.z <= max.z &&
					(!hit || t < lowt))
				{
					hit = true;
					lowt = t;
				}
			}
		}
		// Min y
		if (rayorig.y <= min.y && raydir.y > 0)
		{
			t = (min.y - rayorig.y) / raydir.y;
			if (t >= 0)
			{
				// Substitute t back into ray and check bounds and dist
				hitpoint = rayorig + raydir * t;
				if (hitpoint.x >= min.x && hitpoint.x <= max.x &&
					hitpoint.z >= min.z && hitpoint.z <= max.z &&
					(!hit || t < lowt))
				{
					hit = true;
					lowt = t;
				}
			}
		}
		// Max y
		if (rayorig.y >= max.y && raydir.y < 0)
		{
			t = (max.y - rayorig.y) / raydir.y;
			if (t >= 0)
			{
				// Substitute t back into ray and check bounds and dist
				hitpoint = rayorig + raydir * t;
				if (hitpoint.x >= min.x && hitpoint.x <= max.x &&
					hitpoint.z >= min.z && hitpoint.z <= max.z &&
					(!hit || t < lowt))
				{
					hit = true;
					lowt = t;
				}
			}
		}
		// Min z
		if (rayorig.z <= min.z && raydir.z > 0)
		{
			t = (min.z - rayorig.z) / raydir.z;
			if (t >= 0)
			{
				// Substitute t back into ray and check bounds and dist
				hitpoint = rayorig + raydir * t;
				if (hitpoint.x >= min.x && hitpoint.x <= max.x &&
					hitpoint.y >= min.y && hitpoint.y <= max.y &&
					(!hit || t < lowt))
				{
					hit = true;
					lowt = t;
				}
			}
		}
		// Max z
		if (rayorig.z >= max.z && raydir.z < 0)
		{
			t = (max.z - rayorig.z) / raydir.z;
			if (t >= 0)
			{
				// Substitute t back into ray and check bounds and dist
				hitpoint = rayorig + raydir * t;
				if (hitpoint.x >= min.x && hitpoint.x <= max.x &&
					hitpoint.y >= min.y && hitpoint.y <= max.y &&
					(!hit || t < lowt))
				{
					hit = true;
					lowt = t;
				}
			}
		}

		return std::pair<bool, scalar>(hit, lowt);

	} 
    //-----------------------------------------------------------------------
    bool Math::intersects(const Ray& ray, const AxisAlignedBox& box,
        scalar* d1, scalar* d2)
    {
        if (box.isNull())
            return false;

        if (box.isInfinite())
        {
            if (d1) *d1 = 0;
            if (d2) *d2 = Math::POS_INFINITY;
            return true;
        }

        const Vector3& min = box.getMinimum();
        const Vector3& max = box.getMaximum();
        const Vector3& rayorig = ray.getOrigin();
        const Vector3& raydir = ray.getDirection();

        Vector3 absDir;
        absDir[0] = Math::Abs(raydir[0]);
        absDir[1] = Math::Abs(raydir[1]);
        absDir[2] = Math::Abs(raydir[2]);

        // Sort the axis, ensure check minimise floating error axis first
        int imax = 0, imid = 1, imin = 2;
        if (absDir[0] < absDir[2])
        {
            imax = 2;
            imin = 0;
        }
        if (absDir[1] < absDir[imin])
        {
            imid = imin;
            imin = 1;
        }
        else if (absDir[1] > absDir[imax])
        {
            imid = imax;
            imax = 1;
        }

        scalar start = 0, end = Math::POS_INFINITY;

#define _CALC_AXIS(i)                                       \
    do {                                                    \
        scalar denom = 1 / raydir[i];                         \
        scalar newstart = (min[i] - rayorig[i]) * denom;      \
        scalar newend = (max[i] - rayorig[i]) * denom;        \
        if (newstart > newend) std::swap(newstart, newend); \
        if (newstart > end || newend < start) return false; \
        if (newstart > start) start = newstart;             \
        if (newend < end) end = newend;                     \
    } while(0)

        // Check each axis in turn

        _CALC_AXIS(imax);

        if (absDir[imid] < std::numeric_limits<scalar>::epsilon())
        {
            // Parallel with middle and minimise axis, check bounds only
            if (rayorig[imid] < min[imid] || rayorig[imid] > max[imid] ||
                rayorig[imin] < min[imin] || rayorig[imin] > max[imin])
                return false;
        }
        else
        {
            _CALC_AXIS(imid);

            if (absDir[imin] < std::numeric_limits<scalar>::epsilon())
            {
                // Parallel with minimise axis, check bounds only
                if (rayorig[imin] < min[imin] || rayorig[imin] > max[imin])
                    return false;
            }
            else
            {
                _CALC_AXIS(imin);
            }
        }
#undef _CALC_AXIS

        if (d1) *d1 = start;
        if (d2) *d2 = end;

        return true;
    }
    //-----------------------------------------------------------------------
    std::pair<bool, scalar> Math::intersects(const Ray& ray, const Vector3& a,
        const Vector3& b, const Vector3& c, const Vector3& normal,
        bool positiveSide, bool negativeSide)
    {
        //
        // Calculate intersection with plane.
        //
        scalar t;
        {
            scalar denom = normal.dotProduct(ray.getDirection());

            // Check intersect side
            if (denom > + std::numeric_limits<scalar>::epsilon())
            {
                if (!negativeSide)
                    return std::pair<bool, scalar>(false, 0);
            }
            else if (denom < - std::numeric_limits<scalar>::epsilon())
            {
                if (!positiveSide)
                    return std::pair<bool, scalar>(false, 0);
            }
            else
            {
                // Parallel or triangle area is close to zero when
                // the plane normal not normalised.
                return std::pair<bool, scalar>(false, 0);
            }

            t = normal.dotProduct(a - ray.getOrigin()) / denom;

            if (t < 0)
            {
                // Intersection is behind origin
                return std::pair<bool, scalar>(false, 0);
            }
        }

        //
        // Calculate the largest area projection plane in X, Y or Z.
        //
        size_t i0, i1;
        {
            scalar n0 = Math::Abs(normal[0]);
            scalar n1 = Math::Abs(normal[1]);
            scalar n2 = Math::Abs(normal[2]);

            i0 = 1; i1 = 2;
            if (n1 > n2)
            {
                if (n1 > n0) i0 = 0;
            }
            else
            {
                if (n2 > n0) i1 = 0;
            }
        }

        //
        // Check the intersection point is inside the triangle.
        //
        {
            scalar u1 = b[i0] - a[i0];
            scalar v1 = b[i1] - a[i1];
            scalar u2 = c[i0] - a[i0];
            scalar v2 = c[i1] - a[i1];
            scalar u0 = t * ray.getDirection()[i0] + ray.getOrigin()[i0] - a[i0];
            scalar v0 = t * ray.getDirection()[i1] + ray.getOrigin()[i1] - a[i1];

            scalar alpha = u0 * v2 - u2 * v0;
            scalar beta  = u1 * v0 - u0 * v1;
            scalar area  = u1 * v2 - u2 * v1;

            // epsilon to avoid float precision error
            const scalar EPSILON = 1e-6f;

            scalar tolerance = - EPSILON * area;

            if (area > 0)
            {
                if (alpha < tolerance || beta < tolerance || alpha+beta > area-tolerance)
                    return std::pair<bool, scalar>(false, 0);
            }
            else
            {
                if (alpha > tolerance || beta > tolerance || alpha+beta < area-tolerance)
                    return std::pair<bool, scalar>(false, 0);
            }
        }

        return std::pair<bool, scalar>(true, t);
    }
    //-----------------------------------------------------------------------
    std::pair<bool, scalar> Math::intersects(const Ray& ray, const Vector3& a,
        const Vector3& b, const Vector3& c,
        bool positiveSide, bool negativeSide)
    {
        Vector3 normal = calculateBasicFaceNormalWithoutNormalize(a, b, c);
        return intersects(ray, a, b, c, normal, positiveSide, negativeSide);
    }
    //-----------------------------------------------------------------------
    bool Math::intersects(const Sphere& sphere, const AxisAlignedBox& box)
    {
        if (box.isNull()) return false;
        if (box.isInfinite()) return true;

        // Use splitting planes
        const Vector3& center = sphere.getCenter();
        scalar radius = sphere.getRadius();
        const Vector3& min = box.getMinimum();
        const Vector3& max = box.getMaximum();

		// Arvo's algorithm
		scalar s, d = 0;
		for (int i = 0; i < 3; ++i)
		{
			if (center.ptr()[i] < min.ptr()[i])
			{
				s = center.ptr()[i] - min.ptr()[i];
				d += s * s; 
			}
			else if(center.ptr()[i] > max.ptr()[i])
			{
				s = center.ptr()[i] - max.ptr()[i];
				d += s * s; 
			}
		}
		return d <= radius * radius;

    }
    //-----------------------------------------------------------------------
    bool Math::intersects(const Plane& plane, const AxisAlignedBox& box)
    {
        return (plane.getSide(box) == Plane::BOTH_SIDE);
    }
    //-----------------------------------------------------------------------
    bool Math::intersects(const Sphere& sphere, const Plane& plane)
    {
        return (
            Math::Abs(plane.getDistance(sphere.getCenter()))
            <= sphere.getRadius() );
    }
    //-----------------------------------------------------------------------
    Vector3 Math::calculateTangentSpaceVector(
        const Vector3& position1, const Vector3& position2, const Vector3& position3,
        scalar u1, scalar v1, scalar u2, scalar v2, scalar u3, scalar v3)
    {
	    //side0 is the vector along one side of the triangle of vertices passed in, 
	    //and side1 is the vector along another side. Taking the cross product of these returns the normal.
	    Vector3 side0 = position1 - position2;
	    Vector3 side1 = position3 - position1;
	    //Calculate face normal
	    Vector3 normal = side1.crossProduct(side0);
	    normal.normalise();
	    //Now we use a formula to calculate the tangent. 
	    scalar deltaV0 = v1 - v2;
	    scalar deltaV1 = v3 - v1;
	    Vector3 tangent = deltaV1 * side0 - deltaV0 * side1;
	    tangent.normalise();
	    //Calculate binormal
	    scalar deltaU0 = u1 - u2;
	    scalar deltaU1 = u3 - u1;
	    Vector3 binormal = deltaU1 * side0 - deltaU0 * side1;
	    binormal.normalise();
	    //Now, we take the cross product of the tangents to get a vector which 
	    //should point in the same direction as our normal calculated above. 
	    //If it points in the opposite direction (the dot product between the normals is less than zero), 
	    //then we need to reverse the s and t tangents. 
	    //This is because the triangle has been mirrored when going from tangent space to object space.
	    //reverse tangents if necessary
	    Vector3 tangentCross = tangent.crossProduct(binormal);
	    if (tangentCross.dotProduct(normal) < 0.0f)
	    {
		    tangent = -tangent;
		    binormal = -binormal;
	    }

        return tangent;

    }
    //-----------------------------------------------------------------------
    Matrix4 Math::buildReflectionMatrix(const Plane& p)
    {
        return Matrix4(
            -2 * p.normal.x * p.normal.x + 1,   -2 * p.normal.x * p.normal.y,       -2 * p.normal.x * p.normal.z,       -2 * p.normal.x * p.d, 
            -2 * p.normal.y * p.normal.x,       -2 * p.normal.y * p.normal.y + 1,   -2 * p.normal.y * p.normal.z,       -2 * p.normal.y * p.d, 
            -2 * p.normal.z * p.normal.x,       -2 * p.normal.z * p.normal.y,       -2 * p.normal.z * p.normal.z + 1,   -2 * p.normal.z * p.d, 
            0,                                  0,                                  0,                                  1);
    }
    //-----------------------------------------------------------------------
    Vector4 Math::calculateFaceNormal(const Vector3& v1, const Vector3& v2, const Vector3& v3)
    {
        Vector3 normal = calculateBasicFaceNormal(v1, v2, v3);
        // Now set up the w (distance of tri from origin
        return Vector4(normal.x, normal.y, normal.z, -(normal.dotProduct(v1)));
    }
    //-----------------------------------------------------------------------
    Vector3 Math::calculateBasicFaceNormal(const Vector3& v1, const Vector3& v2, const Vector3& v3)
    {
        Vector3 normal = (v2 - v1).crossProduct(v3 - v1);
        normal.normalise();
        return normal;
    }
    //-----------------------------------------------------------------------
    Vector4 Math::calculateFaceNormalWithoutNormalize(const Vector3& v1, const Vector3& v2, const Vector3& v3)
    {
        Vector3 normal = calculateBasicFaceNormalWithoutNormalize(v1, v2, v3);
        // Now set up the w (distance of tri from origin)
        return Vector4(normal.x, normal.y, normal.z, -(normal.dotProduct(v1)));
    }
    //-----------------------------------------------------------------------
    Vector3 Math::calculateBasicFaceNormalWithoutNormalize(const Vector3& v1, const Vector3& v2, const Vector3& v3)
    {
        Vector3 normal = (v2 - v1).crossProduct(v3 - v1);
        return normal;
    }
	//-----------------------------------------------------------------------
	scalar Math::gaussianDistribution(scalar x, scalar offset, scalar scale)
	{
		scalar nom = Math::Exp(
			-Math::Sqr(x - offset) / (2 * Math::Sqr(scale)));
		scalar denom = scale * Math::Sqrt(2 * Math::PI);

		return nom / denom;

	}
	//---------------------------------------------------------------------
	Matrix4 Math::makeViewMatrix(const Vector3& position, const Quaternion& orientation, 
		const Matrix4* reflectMatrix)
	{
		Matrix4 viewMatrix;

		// View matrix is:
		//
		//  [ Lx  Uy  Dz  Tx  ]
		//  [ Lx  Uy  Dz  Ty  ]
		//  [ Lx  Uy  Dz  Tz  ]
		//  [ 0   0   0   1   ]
		//
		// Where T = -(Transposed(Rot) * Pos)

		// This is most efficiently done using 3x3 Matrices
		Matrix3 rot;
		orientation.ToRotationMatrix(rot);

		// Make the translation relative to new axes
		Matrix3 rotT = rot.Transpose();
		Vector3 trans = -rotT * position;

		// Make final matrix
		viewMatrix = Matrix4::IDENTITY;
		viewMatrix = rotT; // fills upper 3x3
		viewMatrix[0][3] = trans.x;
		viewMatrix[1][3] = trans.y;
		viewMatrix[2][3] = trans.z;

		// Deal with reflections
		if (reflectMatrix)
		{
			viewMatrix = viewMatrix * (*reflectMatrix);
		}

		return viewMatrix;

	}
	//---------------------------------------------------------------------
	scalar Math::boundingRadiusFromAABB(const AxisAlignedBox& aabb)
	{
		Vector3 max = aabb.getMaximum();
		Vector3 min = aabb.getMinimum();

		Vector3 magnitude = max;
		magnitude.makeCeil(-max);
		magnitude.makeCeil(min);
		magnitude.makeCeil(-min);

		return magnitude.length();
	}

	void Math::makeProjectionMatrix( const Radian& fovy, scalar aspectRatio, scalar nearPlane, scalar farPlane, Matrix4& dest )
	{
		Radian theta ( fovy * 0.5 );
		scalar h = 1 / Math::Tan(theta);
		scalar w = h / aspectRatio;
		scalar q, qn;
		if (farPlane == 0)
		{
			q = 1 - Math::INFINITE_FAR_PLANE_ADJUST;
			qn = nearPlane * (Math::INFINITE_FAR_PLANE_ADJUST - 1);
		}
		else
		{
			q = farPlane / ( farPlane - nearPlane );
			qn = -q * nearPlane;
		}

		dest = Matrix4::ZERO;
		dest[0][0] = w;
		dest[1][1] = h;

		dest[2][2] = -q;
		dest[3][2] = -1.0f;

		dest[2][3] = qn;
	}

	void Math::makeProjectionMatrix( scalar left, scalar right, scalar bottom, scalar top, scalar nearPlane, scalar farPlane, Matrix4& dest )
	{
		scalar width = right - left;
		scalar height = top - bottom;
		scalar q, qn;
		if (farPlane == 0)
		{
			q = 1 - Math::INFINITE_FAR_PLANE_ADJUST;
			qn = nearPlane * (Math::INFINITE_FAR_PLANE_ADJUST - 1);
		}
		else
		{
			q = farPlane / ( farPlane - nearPlane );
			qn = -q * nearPlane;
		}
		dest = Matrix4::ZERO;
		dest[0][0] = 2 * nearPlane / width;
		dest[0][2] = (right+left) / width;
		dest[1][1] = 2 * nearPlane / height;
		dest[1][2] = (top+bottom) / height;
		dest[2][2] = -q;
		dest[3][2] = -1.0f;

		dest[2][3] = qn;
	}

	void Math::buildProjectMatrix( float *dst, const Matrix4 &proj, const Matrix4 &view )
	{
		Matrix4 projView = Matrix4(view * proj).inverse();
		memcpy(dst, projView[0], sizeof(float)*16);
	}

	void Math::buildUnprojectMatrix( float *dst, const Matrix4 &proj, const Matrix4 &view )
	{
		Matrix4 invProjView = Matrix4(view * proj).inverse();
		memcpy(dst, invProjView[0], sizeof(float)*16);
	}

}
