#pragma once



class ShaderBuilder
{

public:

	static void replaceMarker(std::string& shader, const std::string& marker, const std::string& value)
	{
		size_t position = 0;
		while ((position = shader.find(marker, position)) != std::string::npos)
		{
			shader.replace(position, marker.size(), value);
			position += value.size();
		}
	};


	static std::string buildFragmentShader(	const std::string& dstFunction)
	{
		std::string shader =
#include "GeneralFragmentShader.glsl"
			;


		replaceMarker(shader, "/*__POLYTOPE_DST_FUNCTION__*/", dstFunction);

		return shader;
	}
};