#pragma once

using vec_t = float;


class vec3_t {
public:
	vec_t x, y, z;

	vec3_t(vec_t x = 0, vec_t y = 0, vec_t z = 0) : x(x), y(y), z(z) {}

	inline vec3_t operator+(const vec3_t& v) const
	{
		return vec3_t(this->x + v.x, this->y + v.y, this->z + v.z);
	}

};



class VMatrix
{
public:

	VMatrix();
	VMatrix(
		vec_t m00, vec_t m01, vec_t m02, vec_t m03,
		vec_t m10, vec_t m11, vec_t m12, vec_t m13,
		vec_t m20, vec_t m21, vec_t m22, vec_t m23,
		vec_t m30, vec_t m31, vec_t m32, vec_t m33
	);

	// array access
	inline float* operator[](int i)
	{
		return m[i];
	}

	inline const float* operator[](int i) const
	{
		return m[i];
	}

	// Get a pointer to m[0][0]
	inline float* base()
	{
		return &m[0][0];
	}

	inline const float* base() const
	{
		return &m[0][0];
	}

public:
	// The matrix.
	vec_t		m[4][4];
};