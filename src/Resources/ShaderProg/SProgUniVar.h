#ifndef S_PROG_UNI_VAR_H
#define S_PROG_UNI_VAR_H

#include "SProgVar.h"
#include "Math.h"


class Texture;


/// Uniform shader variable
class SProgUniVar: public SProgVar
{
	public:
		SProgUniVar(int loc_, const char* name_, GLenum glDataType_, const ShaderProg& fatherSProg_);

		/// @name Set the var
		/// @{
		void setFloat(float f) const;
		void setFloatVec(float f[], uint size = 1) const;
		void setVec2(const Vec2 v2[], uint size = 1) const;
		void setVec3(const Vec3 v3[], uint size = 1) const;
		void setVec4(const Vec4 v4[], uint size = 1) const;
		void setMat3(const Mat3 m3[], uint size = 1) const;
		void setMat4(const Mat4 m4[], uint size = 1) const;
		void setTexture(const Texture& tex, uint texUnit) const;
		/// @}
};


inline SProgUniVar::SProgUniVar(int loc_, const char* name_, GLenum glDataType_, const ShaderProg& fatherSProg_):
	SProgVar(loc_, name_, glDataType_, SVT_UNIFORM, fatherSProg_)
{}


#endif
