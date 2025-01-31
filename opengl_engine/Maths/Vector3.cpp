#include "vector3.h"
#include "matrix4.h"
#include "quaternion.h"
#include "vector2.h"
#include <Utils/color.h>
#include <FMod/fmod_common.h> //  fmod vectors

const Vector3 Vector3::zero(0.0f, 0.0f, 0.f);
const Vector3 Vector3::one(1.0f, 1.0f, 1.0f);
const Vector3 Vector3::unitX(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::unitY(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::unitZ(0.0f, 0.0f, 1.0f);
const Vector3 Vector3::negUnitX(-1.0f, 0.0f, 0.0f);
const Vector3 Vector3::negUnitY(0.0f, -1.0f, 0.0f);
const Vector3 Vector3::negUnitZ(0.0f, 0.0f, -1.0f);
const Vector3 Vector3::infinity(Maths::infinity, Maths::infinity, Maths::infinity);
const Vector3 Vector3::negInfinity(Maths::negInfinity, Maths::negInfinity, Maths::negInfinity);

Vector3::Vector3(const Vector2 vec2)
{
	x = vec2.x;
	y = vec2.y;
	z = 0.0f;
}

Vector3::Vector3(const Vector2 vec2, float zP)
{
	x = vec2.x;
	y = vec2.y;
	z = zP;
}

Vector3::Vector3(const Color color)
{
	Vector3 color_to_vector = color.toVector();
	x = color_to_vector.x;
	y = color_to_vector.y;
	z = color_to_vector.z;
}

void Vector3::set(float xP, float yP, float zP)
{
	x = xP;
	y = yP;
	z = zP;
}

float Vector3::lengthSq() const
{
	return (x * x + y * y + z * z);
}

float Vector3::length() const
{
	return (Maths::sqrt(lengthSq()));
}

void Vector3::normalize()
{
	if (x == 0.0f && y == 0.0f && z == 0.0f) return;
	float len = length();
	x /= len;
	y /= len;
	z /= len;
}

void Vector3::clampMagnitude(float magnitude)
{
	if (lengthSq() <= magnitude * magnitude) return;
	setMagnitude(magnitude);
}

void Vector3::setMagnitude(float magnitude)
{
	normalize();
	x *= magnitude;
	y *= magnitude;
	z *= magnitude;
}

void Vector3::clampToOne()
{
	float highest_abs = Maths::max(Maths::abs(x), Maths::max(Maths::abs(y), Maths::abs(z)));
	if (highest_abs == 0.0f)
	{
		x = y = z = 1.0f;
		return;
	}

	float scale = 1.0f / highest_abs;
	x *= scale;
	y *= scale;
	z *= scale;
}

Vector3& Vector3::operator+=(const Vector2& right)
{
	x += right.x;
	y += right.y;
	return *this;
}

Vector3& Vector3::operator-=(const Vector2& right)
{
	x -= right.x;
	y -= right.y;
	return *this;
}

Vector3 Vector3::transform(const Vector3& vec, const Matrix4& mat, float w)
{
	Vector3 retVal;
	retVal.x = vec.x * mat.mat[0][0] + vec.y * mat.mat[1][0] +
		vec.z * mat.mat[2][0] + w * mat.mat[3][0];
	retVal.y = vec.x * mat.mat[0][1] + vec.y * mat.mat[1][1] +
		vec.z * mat.mat[2][1] + w * mat.mat[3][1];
	retVal.z = vec.x * mat.mat[0][2] + vec.y * mat.mat[1][2] +
		vec.z * mat.mat[2][2] + w * mat.mat[3][2];
	//ignore w since we aren't returning a new value for it...
	return retVal;
}

Vector3 Vector3::transformWithPerspDiv(const Vector3& vec, const Matrix4& mat, float w)
{
	Vector3 retVal;
	retVal.x = vec.x * mat.mat[0][0] + vec.y * mat.mat[1][0] +
		vec.z * mat.mat[2][0] + w * mat.mat[3][0];
	retVal.y = vec.x * mat.mat[0][1] + vec.y * mat.mat[1][1] +
		vec.z * mat.mat[2][1] + w * mat.mat[3][1];
	retVal.z = vec.x * mat.mat[0][2] + vec.y * mat.mat[1][2] +
		vec.z * mat.mat[2][2] + w * mat.mat[3][2];
	float transformedW = vec.x * mat.mat[0][3] + vec.y * mat.mat[1][3] +
		vec.z * mat.mat[2][3] + w * mat.mat[3][3];
	if (!Maths::nearZero(Maths::abs(transformedW)))
	{
		transformedW = 1.0f / transformedW;
		retVal *= transformedW;
	}
	return retVal;
}

Vector3 Vector3::transform(const Vector3& v, const Quaternion& q)
{
	// v + 2.0*cross(q.xyz, cross(q.xyz,v) + q.w*v);
	Vector3 qv(q.x, q.y, q.z);
	Vector3 retVal = v;
	retVal += 2.0f * Vector3::cross(qv, Vector3::cross(qv, v) + q.w * v);
	return retVal;
}

float Vector3::Distance(const Vector3& a, const Vector3& b)
{
	Vector3 c = b - a;
	return c.length();
}

FMOD_VECTOR Vector3::toFMOD() const
{
	return FMOD_VECTOR{ x, y, z };
}

Vector3 Vector3::FromFMOD(const FMOD_VECTOR fmod)
{
	return Vector3{ fmod.x, fmod.y, fmod.z };
}

Vector3 operator+(const Vector3& a, const Vector2& b)
{
	return Vector3(a.x + b.x, a.y + b.y, a.z);
}

Vector3 operator+(const Vector2& a, const Vector3& b)
{
	return b + a;
}

Vector3 operator-(const Vector3& a, const Vector2& b)
{
	return Vector3(a.x - b.x, a.y - b.y, a.z);
}

Vector3 operator-(const Vector2& a, const Vector3& b)
{
	return b - a;
}

Vector3 operator*(const Vector3& left, const Vector2& right)
{
	return Vector3(left.x * right.x, left.y * right.y, left.z);
}

Vector3 operator*(const Vector2& left, const Vector3& right)
{
	return right * left;
}
