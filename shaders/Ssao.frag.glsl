// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// SSAO fragment shader
#include "shaders/Common.glsl"
#include "shaders/Pack.glsl"
#include "shaders/Functions.glsl"

const float BIAS = 0.3;
const float STRENGTH = 3.0;
const float HISTORY_FEEDBACK = 1.0 / 8.0;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out float out_color;

layout(ANKI_UBO_BINDING(0, 0), std140, row_major) uniform _blk
{
	vec4 u_unprojectionParams;
	vec4 u_projectionMat;
	vec4 u_noiseLayerPad3;
	mat4 u_prevViewProjMatMulInvViewProjMat;
};

#define u_noiseLayer u_noiseLayerPad3.x

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_mMsDepthRt;
layout(ANKI_TEX_BINDING(0, 1)) uniform sampler2D u_msRt;
layout(ANKI_TEX_BINDING(0, 2)) uniform sampler2DArray u_noiseMap;
layout(ANKI_TEX_BINDING(0, 3)) uniform sampler2D u_prevSsaoRt;

// Get normal
vec3 readNormal(in vec2 uv)
{
	vec3 normal;
	readNormalFromGBuffer(u_msRt, uv, normal);
	return normal;
}

// Read the noise tex
vec3 readRandom(in vec2 uv)
{
	const vec2 tmp = vec2(float(WIDTH) / float(NOISE_MAP_SIZE), float(HEIGHT) / float(NOISE_MAP_SIZE));
	vec3 r = texture(u_noiseMap, vec3(tmp * uv, u_noiseLayer)).rgb;
	return r;
}

// Returns the Z of the position in view space
float readZ(in vec2 uv)
{
	float depth = texture(u_mMsDepthRt, uv).r;
	float z = u_unprojectionParams.z / (u_unprojectionParams.w + depth);
	return z;
}

// Read position in view space
vec3 readPosition(in vec2 uv)
{
	vec3 fragPosVspace;
	fragPosVspace.z = readZ(uv);

	fragPosVspace.xy = (2.0 * uv - 1.0) * u_unprojectionParams.xy * fragPosVspace.z;

	return fragPosVspace;
}

vec4 project(vec4 point)
{
	return projectPerspective(point, u_projectionMat.x, u_projectionMat.y, u_projectionMat.z, u_projectionMat.w);
}

void main(void)
{
	vec2 ndc = in_uv * 2.0 - 1.0;
	float depth = texture(u_mMsDepthRt, in_uv).r;

	vec3 origin;
	origin.z = u_unprojectionParams.z / (u_unprojectionParams.w + depth);
	origin.xy = ndc * u_unprojectionParams.xy * origin.z;

	vec3 normal = readNormal(in_uv);

	// Get rand factors
	vec3 randFactors = readRandom(in_uv);

	// Get prev SSAO
	vec4 clip = u_prevViewProjMatMulInvViewProjMat * vec4(vec3(ndc, UV_TO_NDC(depth)), 1.0);
	clip.xy /= clip.w;
	vec2 oldUv = NDC_TO_UV(clip.xy);
	float prevSsao = textureLod(u_prevSsaoRt, oldUv, 0.0).r;

	// Compute the history blend. If clip falls outside NDC then it's 1.0 (use only current SSAO term) and if it's
	// inside NDC then use the HISTORY_FEEDBACK value
	vec2 posNdc = abs(clip.xy);
	float historyFeedback = max(posNdc.x, posNdc.y);
	historyFeedback = min(floor(historyFeedback), 1.0 - HISTORY_FEEDBACK);
	historyFeedback += HISTORY_FEEDBACK;

	// Find the projected radius
	vec3 sphereLimit = origin + vec3(RADIUS, 0.0, 0.0);
	vec4 projSphereLimit = project(vec4(sphereLimit, 1.0));
	vec2 projSphereLimit2 = projSphereLimit.xy / projSphereLimit.w;
	float projRadius = length(projSphereLimit2 - ndc);

	// Compute rand cos and sin
	float randAng = randFactors[0] * PI * 2.0;
	float cosAng = cos(randAng);
	float sinAng = sin(randAng);

	// Rotate the disk point
	vec2 diskPoint = (randFactors.yz * 0.8 + 0.2) * projRadius;
	vec2 rotDiskPoint;
	rotDiskPoint.x = diskPoint.x * cosAng - diskPoint.y * sinAng;
	rotDiskPoint.y = diskPoint.y * cosAng + diskPoint.x * sinAng;
	vec2 finalDiskPoint = ndc + rotDiskPoint;

	// Compute factor
	vec3 s = readPosition(NDC_TO_UV(finalDiskPoint));
	vec3 u = s - origin;
	float ssao = max(dot(normal, u) + BIAS, 0.0) / max(dot(u, u), EPSILON);
	ssao = 1.0 - ssao * STRENGTH;

	// Blend
	out_color = mix(prevSsao, ssao, historyFeedback);
}
