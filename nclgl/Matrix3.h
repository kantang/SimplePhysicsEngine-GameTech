/*
	Building up this Matrix3 only because when using matrix for inertia matrix,
	a Matrix 4 will be 7 more variables to store and deal with which is a waste
	of computational time.

	This class is quite identical to the Matrix4 class, but this class doesn't have
	the whole set of methods related to position manipulation and view matrix 
	manipulation.

	Kan Tang.
*/

#pragma once

#include <iostream>
#include "common.h"
#include "Vector3.h"

class Matrix3	{
public:
	Matrix3(void){ ToZero(); };
	Matrix3(float elements[9]){ memcpy(this->values, elements, 9 * sizeof(float)); };
	~Matrix3(void){ ToZero(); };


	//Set all matrix values to zero
	void	ToZero(){ memset(this->values, 0, 9 * sizeof(float)); };
	//Sets matrix to identity matrix (1.0 down the diagonal)
	void	ToIdentity()
	{
		ToZero();
		values[0] = 1.0f;
		values[4] = 1.0f;
		values[8] = 1.0f;
	};

	//Multiplies 'this' matrix by matrix 'a'. Performs the multiplication in 'OpenGL' order (ie, backwards)
	inline Matrix3 operator*(const Matrix3 &a) const{
		Matrix3 out;
		//Students! You should be able to think up a really easy way of speeding this up...
		for (unsigned int r = 0; r < 3; ++r) {
			for (unsigned int c = 0; c < 3; ++c) {
				out.values[c + (r * 3)] = 0.0f;
				for (unsigned int i = 0; i < 3; ++i) {
					out.values[c + (r * 3)] += this->values[c + (i * 3)] * a.values[(r * 3) + i];
				}
			}
		}
		return out;
	}

	inline Vector3 operator*(const Vector3 &v) const {
		return Vector3(v.x*values[0] + v.y*values[3] + v.z*values[6],
					   v.x*values[1] + v.y*values[4] + v.z*values[7],
					   v.x*values[2] + v.y*values[5] + v.z*values[8]);
	};

	//Handy string output for the matrix. Can get a bit messy, but better than nothing!
	inline friend std::ostream& operator<<(std::ostream& o, const Matrix3& m){
		o << "Mat3:" << std::endl;
		o << "\t" << m.values[0] << "," << m.values[1] << "," << m.values[2] << std::endl;
		o << "\t\t" << m.values[3] << "," << m.values[4] << "," << m.values[5] << std::endl;
		o << "\t\t" << m.values[6] << "," << m.values[7] << "," << m.values[8] << std::endl;
		return o;
	}

	//get the specific element from the matrix;
	float operator()(unsigned int x, unsigned int y) const{ return values[x * 3 + y]; };
	float& operator()(unsigned int x, unsigned int y){ return values[x * 3 + y]; };

protected:
		
	float	values[9];

};
