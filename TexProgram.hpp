#pragma once

#include "GL.hpp"
#include "Load.hpp"

//Shader program that draws transformed, vertices tinted with vertex colors:
struct TexProgram {
	TexProgram();
	~TexProgram();

	GLuint program = 0;
	//Attribute (per-vertex variable) locations:
	GLuint Position_vec4 = -1U;
	GLuint TexCoord_vec2 = -1U;
	//Uniform (per-invocation variable) locations:
	GLuint CLIP_FROM_LOCAL_mat4 = -1U;
	//Textures:
	//TEXTURE0 - texture that is accessed by TexCoord
};

extern Load< TexProgram > tex_program;
