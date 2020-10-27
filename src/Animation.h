#include "Interactor.h"
#include <glm/glm.hpp>
#include <vector>

using namespace glm;

struct Frame
{
	vec3 backgroundColor;
	mat4 viewTransform;
	mat4 lightTransform;
	float explosionOffset;
};

class Animation
{
public:
	Animation();
	void storeFrame(const Frame& newFrame);
	void removeFrame();
	int num_frames();
	Frame play(double m_time);
	void resetFrames();
private:
	std::vector<Frame> frames;

	void decompose(const mat4& m, vec3& T, quat& Rquat, vec3& S);
	mat4 compose(const vec3& T, const quat& Rquat, const vec3& S);

	template<typename genTypeT>
	genTypeT interpolate(const std::vector<genTypeT>& p, const float& a);
	
	float interpolate(const float& p0, const float& p1, const float& a);

	quat interpolate(const quat& q0, const quat& p1, const float& a);

	mat4 interpolate(const mat4& p0, const mat4& p1, const float& a);

	vec3 interpolate(const vec3& v0, const vec3& v1, const float& a);
	
	Frame interpolate(const Frame& f0, const Frame& f1, const float& a);
};