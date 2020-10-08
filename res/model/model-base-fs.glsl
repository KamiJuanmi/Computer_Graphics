#version 400
#extension GL_ARB_shading_language_include : require
#include "/model-globals.glsl"

uniform vec3 worldCameraPosition;
uniform vec3 worldLightPosition;
uniform vec3 m_diffuse;

uniform vec3 m_ambient;
uniform vec3 m_specular;
uniform float shininess;

uniform sampler2D diffuseTexture;
uniform bool wireframeEnabled;
uniform vec4 wireframeLineColor;

uniform float smoothValue;
uniform vec3 light_d;
uniform vec3 light_a;
uniform vec3 light_s;

in fragmentData
{
	vec3 position;
	vec3 normal;
	vec2 texCoord;
	noperspective vec3 edgeDistance;
} fragment;

out vec4 fragColor;

void main()
{
	vec3 lightVector = fragment.position-worldLightPosition;
	vec3 viewerVector = fragment.position-worldCameraPosition;
	vec3 reflectedVector = 2*dot(lightVector,fragment.normal)*fragment.normal-lightVector;
	vec4 result = vec4(m_ambient*light_a + m_diffuse*dot(lightVector,fragment.normal)*light_d + m_specular*pow(dot(reflectedVector,viewerVector),shininess)*light_s,1.0)*smoothValue;

	if (wireframeEnabled)
	{
		float smallestDistance = min(min(fragment.edgeDistance[0],fragment.edgeDistance[1]),fragment.edgeDistance[2]);
		float edgeIntensity = exp2(-1.0*smallestDistance*smallestDistance);
		result.rgb = mix(result.rgb,wireframeLineColor.rgb,edgeIntensity*wireframeLineColor.a);
	}

	fragColor = result;
}