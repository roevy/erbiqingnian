
#pragma once

#include "core/types.h"
#include <iostream>

namespace Philo {

    typedef uint32 RGBA;
    typedef uint32 ARGB;
    typedef uint32 ABGR;
	typedef uint32 BGRA;


    class _PhiloCommonExport Colour
    {
    public:
        static const Colour ZERO;
        static const Colour Black;
        static const Colour White;
        static const Colour Red;
        static const Colour Green;
        static const Colour Blue;

	    explicit Colour( float red = 1.0f,
				    float green = 1.0f,
				    float blue = 1.0f,
				    float alpha = 1.0f ) : r(red), g(green), b(blue), a(alpha)
        { }

	    bool operator==(const Colour& rhs) const;
	    bool operator!=(const Colour& rhs) const;

        float r,g,b,a;

	    /** Retrieves colour as RGBA.
	    */
	    RGBA getAsRGBA(void) const;

	    /** Retrieves colour as ARGB.
	    */
	    ARGB getAsARGB(void) const;

		/** Retrieves colour as BGRA.
		*/
		BGRA getAsBGRA(void) const;

		/** Retrieves colours as ABGR */
	    ABGR getAsABGR(void) const;

	    /** Sets colour as RGBA.
	    */
        void setAsRGBA(const RGBA val);

	    /** Sets colour as ARGB.
	    */
        void setAsARGB(const ARGB val);

		/** Sets colour as BGRA.
		*/
		void setAsBGRA(const BGRA val);

	    /** Sets colour as ABGR.
	    */
        void setAsABGR(const ABGR val);

        /** Clamps colour value to the range [0, 1].
        */
        void saturate(void)
        {
            if (r < 0)
                r = 0;
            else if (r > 1)
                r = 1;

            if (g < 0)
                g = 0;
            else if (g > 1)
                g = 1;

            if (b < 0)
                b = 0;
            else if (b > 1)
                b = 1;

            if (a < 0)
                a = 0;
            else if (a > 1)
                a = 1;
        }

        /** As saturate, except that this colour value is unaffected and
            the saturated colour value is returned as a copy. */
        Colour saturateCopy(void) const
        {
            Colour ret = *this;
            ret.saturate();
            return ret;
        }

		/// Array accessor operator
		inline float operator [] ( const size_t i ) const
		{
			ph_assert( i < 4 );

			return *(&r+i);
		}

		/// Array accessor operator
		inline float& operator [] ( const size_t i )
		{
			ph_assert( i < 4 );

			return *(&r+i);
		}

		/// Pointer accessor for direct copying
		inline float* ptr()
		{
			return &r;
		}
		/// Pointer accessor for direct copying
		inline const float* ptr() const
		{
			return &r;
		}

		
		// arithmetic operations
        inline Colour operator + ( const Colour& rkVector ) const
        {
            Colour kSum;

            kSum.r = r + rkVector.r;
            kSum.g = g + rkVector.g;
            kSum.b = b + rkVector.b;
            kSum.a = a + rkVector.a;

            return kSum;
        }

        inline Colour operator - ( const Colour& rkVector ) const
        {
            Colour kDiff;

            kDiff.r = r - rkVector.r;
            kDiff.g = g - rkVector.g;
            kDiff.b = b - rkVector.b;
            kDiff.a = a - rkVector.a;

            return kDiff;
        }

        inline Colour operator * (const float fScalar ) const
        {
            Colour kProd;

            kProd.r = fScalar*r;
            kProd.g = fScalar*g;
            kProd.b = fScalar*b;
            kProd.a = fScalar*a;

            return kProd;
        }

        inline Colour operator * ( const Colour& rhs) const
        {
            Colour kProd;

            kProd.r = rhs.r * r;
            kProd.g = rhs.g * g;
            kProd.b = rhs.b * b;
            kProd.a = rhs.a * a;

            return kProd;
        }

        inline Colour operator / ( const Colour& rhs) const
        {
            Colour kProd;

            kProd.r = rhs.r / r;
            kProd.g = rhs.g / g;
            kProd.b = rhs.b / b;
            kProd.a = rhs.a / a;

            return kProd;
        }

        inline Colour operator / (const float fScalar ) const
        {
            ph_assert( fScalar != 0.0 );

            Colour kDiv;

            float fInv = 1.0f / fScalar;
            kDiv.r = r * fInv;
            kDiv.g = g * fInv;
            kDiv.b = b * fInv;
            kDiv.a = a * fInv;

            return kDiv;
        }

        inline _PhiloCommonExport friend Colour operator * (const float fScalar, const Colour& rkVector )
        {
            Colour kProd;

            kProd.r = fScalar * rkVector.r;
            kProd.g = fScalar * rkVector.g;
            kProd.b = fScalar * rkVector.b;
            kProd.a = fScalar * rkVector.a;

            return kProd;
        }

        // arithmetic updates
        inline Colour& operator += ( const Colour& rkVector )
        {
            r += rkVector.r;
            g += rkVector.g;
            b += rkVector.b;
            a += rkVector.a;

            return *this;
        }

        inline Colour& operator -= ( const Colour& rkVector )
        {
            r -= rkVector.r;
            g -= rkVector.g;
            b -= rkVector.b;
            a -= rkVector.a;

            return *this;
        }

        inline Colour& operator *= (const float fScalar )
        {
            r *= fScalar;
            g *= fScalar;
            b *= fScalar;
            a *= fScalar;
            return *this;
        }

        inline Colour& operator /= (const float fScalar )
        {
            ph_assert( fScalar != 0.0 );

            float fInv = 1.0f / fScalar;

            r *= fInv;
            g *= fInv;
            b *= fInv;
            a *= fInv;

            return *this;
        }

		/** Set a colour value from Hue, Saturation and Brightness.
		@param hue Hue value, scaled to the [0,1] range as opposed to the 0-360
		@param saturation Saturation level, [0,1]
		@param brightness Brightness level, [0,1]
		*/
		void setHSB(float hue, float saturation, float brightness);

		/** Convert the current colour to Hue, Saturation and Brightness values. 
		@param hue Output hue value, scaled to the [0,1] range as opposed to the 0-360
		@param saturation Output saturation level, [0,1]
		@param brightness Output brightness level, [0,1]
		*/
		void getHSB(float* hue, float* saturation, float* brightness) const;



		/** Function for writing to a stream.
		*/
		inline friend std::ostream& operator <<
			( std::ostream& o, const Colour& c )
		{
			o << "ColourValue(" << c.r << ", " << c.g << ", " << c.b << ", " << c.a << ")";
			return o;
		}

    };

} // namespace