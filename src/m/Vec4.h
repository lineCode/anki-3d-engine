#ifndef M_VEC4_H
#define M_VEC4_H

#include "Common.h"


namespace m {


/// 4D vector
class Vec4
{
	public:
		/// @name Constructors
		/// @{
		explicit Vec4();
		explicit Vec4(float x, float y, float z, float w);
		explicit Vec4(float f);
		explicit Vec4(float arr[]);
		explicit Vec4(const Vec2& v2, float z, float w);
		explicit Vec4(const Vec3& v3, float w);
		         Vec4(const Vec4& b);
		explicit Vec4(const Quat& q);
#if defined(MATH_INTEL_SIMD)
		explicit Vec4(const __m128& mm);
#endif
		/// @}

		/// @name Accessors
		/// @{
		float& x();
		float x() const;
		float& y();
		float y() const;
		float& z();
		float z() const;
		float& w();
		float w() const;
		float& operator[](uint i);
		float operator[](uint i) const;
#if defined(MATH_INTEL_SIMD)
		__m128& getMm();
		const __m128& getMm() const;
#endif
		/// @}

		/// @name Operators with same
		/// @{
		Vec4& operator=(const Vec4& b);
		Vec4 operator+(const Vec4& b) const;
		Vec4& operator+=(const Vec4& b);
		Vec4 operator-(const Vec4& b) const;
		Vec4& operator-=(const Vec4& b);
		Vec4 operator*(const Vec4& b) const;
		Vec4& operator*=(const Vec4& b);
		Vec4 operator/(const Vec4& b) const;
		Vec4& operator/=(const Vec4& b);
		Vec4 operator-() const;
		bool operator==(const Vec4& b) const;
		bool operator!=(const Vec4& b) const;
		/// @}

		/// @name Operators with float
		/// @{
		Vec4 operator+(float f) const;
		Vec4& operator+=(float f);
		Vec4 operator-(float f) const;
		Vec4& operator-=(float f);
		Vec4 operator*(float f) const;
		Vec4& operator*=(float f);
		Vec4 operator/(float f) const;
		Vec4& operator/=(float f);
		/// @}

		/// @name Operators with other
		/// @{
		Vec4 operator*(const Mat4& m4) const;
		/// @}

		/// @name Misc methods
		/// @{
		float getLength() const;
		Vec4 getNormalized() const;
		void normalize();
		float dot(const Vec4& b) const;
		/// @}

	private:
		/// @name Data
		/// @{
		union
		{
			struct
			{
				float x, y, z, w;
			} vec;

			boost::array<float, 4> arr;

#if defined(MATH_INTEL_SIMD)
			__m128 mm;
#endif
		};
		/// @}
};


/// @name Global operators with Vec4 and float
/// @{
extern Vec4 operator+(float f, const Vec4& v4);
extern Vec4 operator-(float f, const Vec4& v4);
extern Vec4 operator*(float f, const Vec4& v4);
extern Vec4 operator/(float f, const Vec4& v4);
/// @}


extern std::ostream& operator<<(std::ostream& s, const Vec4& v);


} // end namespace


#include "Vec4.inl.h"


#endif