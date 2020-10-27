#version 400
#extension GL_ARB_shading_language_include : require
#include "/model-globals.glsl"

uniform vec3 worldCameraPosition;
uniform vec3 worldLightPosition;

uniform vec3 diffuseColor, ambientColor, specularColor;

uniform float shininess;

uniform vec3 light_A, light_S, light_D;

uniform bool diff_txt, ambn_txt, spec_txt;

uniform bool objSpace, tangSpace, bumpMapping;

uniform sampler2D diffuseTexture, ambientTexture, specularTexture;

uniform float amp,freq;

uniform sampler2D objectSpaceNormals, tangentSpaceNormals;
uniform bool wireframeEnabled;
uniform vec4 wireframeLineColor;

in fragmentData
{
	vec3 position;
	vec3 normal;
	vec2 texCoord;
	noperspective vec3 edgeDistance;
	mat3 TBN;
} fragment;

float bump_func(vec2 txCoord)
{
	return amp*pow(sin(freq*txCoord.x),2)*pow(sin(freq*txCoord.y),2);
	//return amp*exp(pow(sin(freq*txCoord.x),2)*pow(cos(freq*txCoord.x),2))*(pow(sin(freq*txCoord.y),2)*pow(cos(freq*txCoord.y),2));
}

out vec4 fragColor;

void main()
{
	vec3 normal= fragment.normal;

	if(objSpace)
	{
		normal = texture(objectSpaceNormals, fragment.texCoord).xyz;
		normal = normalize(normal * 2.0 - 1.0);
	}else if(tangSpace)
	{
		normal = texture(tangentSpaceNormals, fragment.texCoord).xyz;
		normal = normalize(normal * 2.0 - 1.0);

		if(bumpMapping)
		{
			//Pu = tangent; Pv=bitangent
			//N' = N + dy(Pu x n) + dx(n x Pv)
			normal = normal + dFdy(bump_func(fragment.texCoord))*(cross(fragment.TBN[0],normalize(normal))) + dFdx(bump_func(fragment.texCoord))*(cross(normalize(normal),fragment.TBN[1]));
		}
		
		normal = normalize(fragment.TBN * normal);
	}

	

	vec3 viewer =  normalize(worldCameraPosition - fragment.position);
	vec3 light =  normalize(worldLightPosition - fragment.position);
	vec3 reflected = normalize(2*dot(light,normal)*normal-light);
	vec3 total = ambientColor*light_A + diffuseColor*max(dot(light, normalize(normal)),0.0)*light_D + specularColor*(pow(max(dot(reflected,viewer),0.0), shininess))*light_S;
	vec4 result = vec4(total,1.0);

	if(diff_txt)
	{
		result = result*texture(diffuseTexture,fragment.texCoord);
	}

	if(ambn_txt)
	{
		result = result*texture(ambientTexture,fragment.texCoord);
	}

	if(spec_txt)
	{
		result = result*texture(specularTexture,fragment.texCoord);
	}

	if (wireframeEnabled)
	{
		float smallestDistance = min(min(fragment.edgeDistance[0],fragment.edgeDistance[1]),fragment.edgeDistance[2]);
		float edgeIntensity = exp2(-1.0*smallestDistance*smallestDistance);
		result.rgb = mix(result.rgb,wireframeLineColor.rgb,edgeIntensity*wireframeLineColor.a);
	}

	fragColor = result;
}