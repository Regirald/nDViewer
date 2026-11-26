#pragma once

#include "ShaderBuilder.h"

#include <algorithm>
#include <string>


class PolytopeShaderBuilder : public ShaderBuilder
{
public:
	enum class PolytopeShaderStyle
	{
		Node,
		Edge,
		Face
	};

	static std::string buildVertexShader()
	{
		std::string shader =
#include "PolytopeVertexShader.glsl"
			;
		return shader;
	}

	static std::string buildFragmentShader()
	{
		std::string shader =
#include "PolytopeFragmentShader.glsl"
			;

		//std::string shadingFunction;
		//switch (style)
		//{
		//case PolytopeShaderStyle::Face:
		//	shadingFunction = "shadeFace(p, rayDir)";
		//	break;
		//case PolytopeShaderStyle::Edge:
		//	shadingFunction = "shadeEdge(p, rayDir)";
		//	break;
		//case PolytopeShaderStyle::Node:
		//	shadingFunction = "shadeNode(p, rayDir)";
		//	break;
		//}



		//replaceMarker(shader, "/*__POLYTOPE_UNIFORMS__*/", uniforms);
		//replaceMarker(shader, "/*__POLYTOPE_DST_FUNCTION__*/", dstFunction);
		//replaceMarker(shader, "/*__POLYTOPE_HIT_SHADER__*/", shadingFunction);

		return shader;
	}
};