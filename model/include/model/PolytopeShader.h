#pragma once

#include "ImplicitFunction.h"

#include <algorithm>
#include <string>

enum class PolytopeShaderStyle
{
	Node,
	Edge,
	Face
};

inline std::string buildPolytopeFragmentShader(
	GLSLContext& ctx,
	const std::string& rootFunc,
	PolytopeShaderStyle style)
{
	std::string shader =
#include "PolytopeShader.glsl"
	;

	const std::string sdfEntry =
		"float sampleSDF(vec3 p)\n"
		"{\n"
		"    return " + rootFunc + "(p);\n"
		"}\n";

	std::string shadingFunction;
	switch (style)
	{
	case PolytopeShaderStyle::Face:
		shadingFunction = "shadeFace(p, rayDir)";
		break;
	case PolytopeShaderStyle::Edge:
		shadingFunction = "shadeEdge(p, rayDir)";
		break;
	case PolytopeShaderStyle::Node:
		shadingFunction = "shadeNode(p, rayDir)";
		break;
	}

	const auto replaceMarker = [&shader](const std::string& marker, const std::string& value)
	{
		size_t position = 0;
		while ((position = shader.find(marker, position)) != std::string::npos)
		{
			shader.replace(position, marker.size(), value);
			position += value.size();
		}
	};

	replaceMarker("/*__POLYTOPE_UNIFORMS__*/", ctx.uniforms);
	replaceMarker("/*__POLYTOPE_SDF_CODE__*/", ctx.code);
	replaceMarker("/*__POLYTOPE_SDF_ENTRY__*/", sdfEntry);
	replaceMarker("/*__POLYTOPE_HIT_SHADER__*/", shadingFunction);

	return shader;
}
