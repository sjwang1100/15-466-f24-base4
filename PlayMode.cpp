#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"
#include "TexProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <string>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <hb.h>
#include <hb-ft.h>
#include <iostream>
#include <fstream>

// NOTE: most of the FT and harfbuzz (text rendering) related codes are assisted by ChatGPT, some referenced Sasha's note

FT_Library ft;
FT_Face face1;

//TexProgram texProgram;

//void RenderText(const std::string& text, FT_Face face, hb_font_t* hb_font, float x, float y, float scale, glm::vec3 color);

hb_font_t* hb_font1;
hb_buffer_t* hb_buffer1;


GLuint VAO, VBO;

const float word_interval = 0.2f;
float time_record = 0;

bool SteadyText(const std::string& text, DrawLines& line, float aspect, float ofs, bool force_out = false);
bool displayChoice(DrawLines& line, float aspect, float ofs);

std::string storyLine;
std::string choice1;
std::string choice2;
bool isChoice = false;

std::ifstream storyFile(data_path("storyline.txt"));


GLuint steam_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > steam_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("steam.pnct"));
	steam_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

GLuint table_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > table_meshes(LoadTagDefault, []() -> MeshBuffer const* {
	MeshBuffer const* ret = new MeshBuffer(data_path("table.pnct"));
	table_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

GLuint bao_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > bao_meshes(LoadTagDefault, []() -> MeshBuffer const* {
	MeshBuffer const* ret = new MeshBuffer(data_path("bao.pnct"));
	bao_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("empty.scene"), [&](Scene& scene, Scene::Transform* transform, std::string const& mesh_name) {
		});
});

//Load< Sound::Sample > dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
//	return new Sound::Sample(data_path("dusty-floor.opus"));
//});

PlayMode::PlayMode() : scene(*hexapod_scene) {
	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
	camera->transform->position = glm::vec3(6.0f, -0.5f, 0.0f);
	camera->transform->rotation *= glm::angleAxis(
		glm::radians(-15.0f),
		glm::vec3(1.0f, 0.0f, 0.0f));

	std::getline(storyFile, storyLine);

	Mesh mesh = table_meshes->lookup("Cube.001");

	auto newTrans = new Scene::Transform();
	scene.drawables.emplace_back(newTrans);
	Scene::Drawable& drawable = scene.drawables.back();

	drawable.pipeline = lit_color_texture_program_pipeline;
	drawable.pipeline.vao = table_meshes_for_lit_color_texture_program;

	drawable.pipeline.type = mesh.type;
	drawable.pipeline.start = mesh.start;
	drawable.pipeline.count = mesh.count;
	drawable.transform->position = glm::vec3(1.2f, 0.0f, -1.5f);

	/*mesh = steam_meshes->lookup("Cylinder");

	newTrans = new Scene::Transform();
	scene.drawables.emplace_back(newTrans);
	drawable = scene.drawables.back();

	drawable.pipeline = lit_color_texture_program_pipeline;
	drawable.pipeline.vao = steam_meshes_for_lit_color_texture_program;

	drawable.pipeline.type = mesh.type;
	drawable.pipeline.start = mesh.start;
	drawable.pipeline.count = mesh.count;
	drawable.transform->position = glm::vec3(1.2f, 0.0f, 1.5f);*/

	//if (FT_Init_FreeType(&ft)) {
	//	std::cerr << "Could not init FreeType Library" << std::endl;
	//	return;
	//}

	//if (FT_New_Face(ft, data_path("GentiumBookPlus-Regular.ttf").c_str(), 0, &face1)) {
	//	std::cerr << "Failed to load font" << std::endl;
	//	return;
	//}

	//FT_Set_Pixel_Sizes(face1, 0, 48); // Set font size, 0 for width means use the best guess

	//hb_font1 = hb_ft_font_create(face1, NULL);
	//hb_buffer1 = hb_buffer_create();


	//
	//glGenVertexArrays(1, &VAO); // Generate VAO
	//glGenBuffers(1, &VBO); // Generate VBO
	//glBindVertexArray(VAO); // Bind VAO

	//glBindBuffer(GL_ARRAY_BUFFER, VBO); // Bind VBO
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Upload data to VBO

	//// Set attribute pointers
	//glVertexAttribPointer(tex_program->Position_vec4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
	//glEnableVertexAttribArray(tex_program->Position_vec4);

	//glVertexAttribPointer(tex_program->TexCoord_vec2, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
	//glEnableVertexAttribArray(tex_program->TexCoord_vec2);

	//// Unbind VAO (optional but good practice)
	//glBindVertexArray(0);
	//GL_ERRORS();
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			if (space.downs == 0) space.downs = 1;
			space.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			camera->transform->rotation = glm::normalize(
				camera->transform->rotation
				* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
			);
			return true;
		}
	}

	return false;
}

bool temp = false;	// used to control the SteadyText
void PlayMode::update(float elapsed) {

	//move camera:
	{

		//combine inputs into a move:
		//constexpr float PlayerSpeed = 30.0f;
		//glm::vec2 move = glm::vec2(0.0f);
		//if (left.pressed && !right.pressed) move.x =-1.0f;
		//if (!left.pressed && right.pressed) move.x = 1.0f;
		///*if (down.pressed && !up.pressed) move.y =-1.0f;
		//if (!down.pressed && up.pressed) move.y = 1.0f;*/

		if (space.pressed && temp) {
			std::getline(storyFile, storyLine);
			temp = false;
		}

		////make it so that moving diagonally doesn't go faster:
		//if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		//glm::mat4x3 frame = camera->transform->make_local_to_parent();
		//glm::vec3 frame_right = frame[0];
		////glm::vec3 up = frame[1];
		//glm::vec3 frame_forward = -frame[2];

		//camera->transform->position += move.x * frame_right + move.y * frame_forward;
	}

	{ //update listener to camera position:
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		glm::vec3 frame_at = frame[3];
		Sound::listener.set_position_right(frame_at, frame_right, 1.0f / 60.0f);
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	space.downs = 0;
	down.downs = 0;

	time_record += elapsed;
}


void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	//RenderText("Hello, HarfBuzz & FreeType!", face1, hb_font1, 25.0f, 300.0f, 1.0f, glm::vec3(0.5f, 0.8f, 0.2f));
	GL_ERRORS();
	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		//constexpr float H = 0.09f;
		/*lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));*/
		float ofs = 2.0f / drawable_size.y;
		/*lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));*/
		temp = SteadyText(storyLine, lines, aspect, ofs, temp);
		//displayChoice(lines, aspect, ofs);
	}
	GL_ERRORS();
}

//void RenderText(const std::string& text, FT_Face face, hb_font_t* hb_font, float x, float y, float scale, glm::vec3 color) {
//	// Bind shader
//	glUseProgram(tex_program->program);
//
//	// Set the color uniform
//	GLuint colorLoc = glGetUniformLocation(tex_program->program, "textColor");
//	glUniform3f(colorLoc, color.x, color.y, color.z);
//
//	// Create HarfBuzz buffer and shape the text
//	hb_buffer_t* hb_buffer = hb_buffer_create();
//	hb_buffer_add_utf8(hb_buffer, text.c_str(), -1, 0, -1);
//	hb_buffer_guess_segment_properties(hb_buffer);
//	hb_shape(hb_font, hb_buffer, NULL, 0);
//
//	// Get glyph information
//	unsigned int len = hb_buffer_get_length(hb_buffer);
//	hb_glyph_info_t* info = hb_buffer_get_glyph_infos(hb_buffer, NULL);
//	hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(hb_buffer, NULL);
//
//	// Load and render each glyph
//	glActiveTexture(GL_TEXTURE0);
//	glBindVertexArray(0);
//	GL_ERRORS();
//	for (unsigned int i = 0; i < len; ++i) {
//		// Load the glyph from FreeType
//		if (FT_Load_Glyph(face, info[i].codepoint, FT_LOAD_RENDER)) {
//			continue;
//		}
//
//		FT_GlyphSlot g = face->glyph;
//
//		// Generate a texture for the glyph
//		GLuint texture;
//		glGenTextures(1, &texture);
//		glBindTexture(GL_TEXTURE_2D, texture);
//		glTexImage2D(
//			GL_TEXTURE_2D,
//			0,
//			GL_RED,
//			g->bitmap.width,
//			g->bitmap.rows,
//			0,
//			GL_RED,
//			GL_UNSIGNED_BYTE,
//			g->bitmap.buffer
//		);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//		
//		// Calculate position and size
//		float xpos = x + pos[i].x_offset / 64.0f * scale;
//		float ypos = y - (g->bitmap.rows - pos[i].y_offset / 64.0f) * scale;
//
//		float w = g->bitmap.width * scale;
//		float h = g->bitmap.rows * scale;
//
//		// Update VBO for each character
//		GLfloat vertices[6][4] = {
//			{ xpos,     ypos + h,   0.0f, 0.0f },
//			{ xpos,     ypos,       0.0f, 1.0f },
//			{ xpos + w, ypos,       1.0f, 1.0f },
//
//			{ xpos,     ypos + h,   0.0f, 0.0f },
//			{ xpos + w, ypos,       1.0f, 1.0f },
//			{ xpos + w, ypos + h,   1.0f, 0.0f }
//		};
//
//		// Render glyph texture over quad
//		glBindTexture(GL_TEXTURE_2D, texture);
//
//		// Update content of VBO memory
//		//GLuint VBO;
//		glBindVertexArray(VAO);
//		glGenBuffers(1, &VBO);
//		glBindBuffer(GL_ARRAY_BUFFER, VBO);
//		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
//		
//		// Set attributes
//		glVertexAttribPointer(tex_program->Position_vec4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
//		glEnableVertexAttribArray(tex_program->Position_vec4);
//		//GL_ERRORS();
//		GLenum error = glGetError();
//		if (error != GL_NO_ERROR) {
//			std::cerr << "OpenGL error: " << error << std::endl;
//		}
//		// Render quad
//		glDrawArrays(GL_TRIANGLES, 0, 6);
//		GL_ERRORS();
//		// Advance cursor to next glyph position
//		x += (pos[i].x_advance / 64.0f) * scale;
//		y -= (pos[i].y_advance / 64.0f) * scale;
//		GL_ERRORS();
//		// Cleanup
//		glBindBuffer(GL_ARRAY_BUFFER, 0);
//		glDeleteBuffers(1, &VBO);
//		glDeleteTextures(1, &texture);
//		GL_ERRORS();
//	}
//
//	// Unbind shader
//	glUseProgram(0);
//	hb_buffer_destroy(hb_buffer);
//	GL_ERRORS();
//}



size_t word_pos = 1;
// Draw text, word by word, return true when string is finished
bool SteadyText(const std::string& text, DrawLines & line, float aspect, float ofs, bool force_out) {
	std::string tempstr = text;
	if (force_out) {
		word_pos = std::string::npos;
		tempstr = tempstr + " ...[SPACE]";
	}
	else if (time_record > word_interval) {
		time_record = 0;
		size_t pos = tempstr.find(' ', word_pos);
		
		constexpr float H = 0.09f;
		line.draw_text(tempstr.substr(0, pos),
			glm::vec3(-aspect + 0.1f * H + ofs, -0.80f, 0.0f),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x0f, 0x0f, 0x0f, 0x00));

		if (tempstr.length() > 50) {
			size_t pos1 = tempstr.find(' ', pos+1);

			line.draw_text(tempstr.substr(pos+1, pos1),
				glm::vec3(-aspect + 0.1f * H + ofs, -0.90f, 0.0f),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0x0f, 0x0f, 0x0f, 0x00));
			pos = pos1;
		}
		word_pos = pos + 1;

		if (pos == std::string::npos) {
			word_pos = 1;
			return true;
		}
		return false;
	}

	if (force_out) {
		word_pos = std::string::npos + 1;
	}
	constexpr float H = 0.09f;
	line.draw_text(tempstr.substr(0, word_pos - 1),
		glm::vec3(-aspect + 0.1f * H + ofs, -0.80f, 0.0f),
		glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
		glm::u8vec4(0x0f, 0x0f, 0x0f, 0x00));
	return force_out;
}

bool displayChoice(DrawLines& line, float aspect, float ofs) {
	constexpr float H = 0.09f;
	line.draw_text(choice1,
		glm::vec3(-aspect + 0.1f * H + ofs, 0.1f, 0.0f),
		glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
		glm::u8vec4(0xf, 0xf, 0xf, 0x00));
	line.draw_text(choice2,
		glm::vec3(-aspect + 0.1f * H + ofs, -0.05f, 0.0f),
		glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
		glm::u8vec4(0xf, 0xf, 0xf, 0x00));
	return true;
}