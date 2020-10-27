#include "Animation.h"

#include <iostream>
#include <glm/gtx/quaternion.hpp>

using namespace glm;
using namespace std;

Animation::Animation(){}

void Animation::storeFrame(const Frame& f)
{
	this->frames.push_back(f);
}

void Animation::removeFrame()
{
	this->frames.pop_back();
}

int Animation::num_frames()
{
	return this->frames.size();
}

Frame Animation::play(double m_time)
{
	return interpolate(this->frames, m_time);
}

void Animation::resetFrames()
{
	this->frames.clear();
}

void Animation::decompose(const mat4& m, vec3& T, quat& Rquat, vec3& S)
{
	//Extract the translation
	T.x = m[3][0];
	T.y = m[3][1];
	T.z = m[3][2];

	//Extract the scale
	S.x = length(vec3(m[0][0], m[0][1], m[0][2]));
	S.y = length(vec3(m[1][0], m[1][1], m[1][2]));
	S.z = length(vec3(m[2][0], m[2][1], m[2][2]));

	//Compute new transformation matrix M without translation

	mat M = m;
	for (int i = 0; i < 3; i++)
	{
		M[i][3] = M[3][i] = 0.0f;
	}
	M[3][3] = 1.0f;

	//Extract rotation R from transformation matrix

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			M[i][j] /= S[i];
		}
	}

	Rquat = quat_cast(M);

}


mat4 Animation::compose(const vec3& T, const quat& Rquat, const vec3& S)
{
	mat4 m = mat4(1.0f);

	m = translate(m, T);
	m = m * toMat4(Rquat);
	m = scale(m, S);

	return m;
}


template<typename genTypeT>
genTypeT Animation::interpolate(const vector<genTypeT>& p, const float& a)
{
	if (p.size() == 1)
	{
		return p[0];
	}
	else
	{
		vector<Frame> nextInterp;
		for (int i = 0; i < p.size() - 1; i++)
		{
			nextInterp.push_back(interpolate(p[i], p[i + 1], a));
		}
		return interpolate(nextInterp, a);
	}
}


float Animation::interpolate(const float& p0, const float& p1, const float& a)
{
	return p0 * (1.0f - a) + p1 * a;
}

quat Animation::interpolate(const quat& q1, const quat& q2, const float & a)
{
	/* Another way that I found to do the slerp interpolation
	float cosTheta = dot(q1, q2);
	if (cosTheta > .9995f)
		return normalize((1 - a) * q1 + a * q2);
	else {
		float theta = acos(clamp(cosTheta, -1.0f, 1.0f));
		float thetap = theta * a;
		quat qperp = normalize(q2 - q1 * cosTheta);
		return q1 * std::cos(thetap) + qperp * std::sin(thetap);
	}*/
	return slerp(q1, q2, a);
}

vec3 Animation::interpolate(const vec3& v0, const vec3& v1, const float& a)
{
	return v0 * (1.0f - a) + v1 * a;
}


mat4 Animation::interpolate(const mat4& p0, const mat4& p1, const float& a)
{
	vec3 T0, T1;
	quat R0, R1;
	vec3 S0, S1;
	decompose(p0, T0, R0, S0);
	decompose(p1, T1, R1, S1);

	vec3 qT = interpolate(T0, T1, a);
	quat qR = interpolate(R0, R1, a);
	vec3 qS = interpolate(S0, S1, a);

	return compose(qT, qR, qS);
}


Frame Animation::interpolate(const Frame& f0, const Frame& f1, const float& a)
{
	Frame f_ret;
	f_ret.backgroundColor = interpolate(f0.backgroundColor, f1.backgroundColor, a);
	f_ret.explosionOffset = interpolate(f0.explosionOffset, f1.explosionOffset, a);
	f_ret.lightTransform = interpolate(f0.lightTransform, f1.lightTransform, a);
	f_ret.viewTransform = interpolate(f0.viewTransform, f1.viewTransform, a);

	return f_ret;
}