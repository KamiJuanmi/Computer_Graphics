#version 400
#extension GL_ARB_shading_language_include : require
#include "/model-globals.glsl"

uniform vec3 worldCameraPosition;
uniform vec3 worldLightPosition;

uniform vec3 diffuseColor, ambientColor, specularColor;

uniform float shininess;

uniform vec3 light_A, light_S, light_D;

uniform sampler2D diffuseTexture;
uniform bool wireframeEnabled;
uniform vec4 wireframeLineColor;

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
	vec3 viewer =  normalize(worldCameraPosition - fragment.position);
	vec3 light =  normalize(worldLightPosition - fragment.position);
	vec3 reflected = normalize(2*dot(light,fragment.normal)*fragment.normal-light);
	vec3 total = ambientColor*light_A + diffuseColor*dot(light, normalize(fragment.normal))*light_D + specularColor*(pow(dot(reflected,viewer), shininess))*light_S;
	vec4 result = vec4(total,1.0);

	if (wireframeEnabled)
	{
		float smallestDistance = min(min(fragment.edgeDistance[0],fragment.edgeDistance[1]),fragment.edgeDistance[2]);
		float edgeIntensity = exp2(-1.0*smallestDistance*smallestDistance);
		result.rgb = mix(result.rgb,wireframeLineColor.rgb,edgeIntensity*wireframeLineColor.a);
	}

	fragColor = result;
}