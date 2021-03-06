#pragma once

// internal
#include "camera.hpp"
#include "common.hpp"
#include "level.hpp"
#include "skybox.hpp"
#include "tile.hpp"
#include "model.hpp"

// stdlib
#include <memory> //for shared_ptr
#include <random>
#include <vector>

//sdl stuff
#define SDL_MAIN_HANDLED

#include <SDL.h>

// glm
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/constants.hpp"

// glfw
#include "GLFW/glfw3.h"

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
namespace World {
	//members
	// Window handle
	extern GLFWwindow* m_window;

	extern glm::vec2 m_screen;

	// Camera stuff
	extern Camera camera;

	// Selection
	extern Coord selectedTileCoordinates;
	extern bool shiftBeingHeld;

	// Game entities
	extern std::shared_ptr<Shader> objShader;
	extern Level level;
	extern Skybox m_skybox;

	// Particle things
	extern std::shared_ptr<Shader> particleShader;

	// C++ rng
	extern std::default_random_engine m_rng;
	extern std::uniform_real_distribution<float> m_dist; // default 0..1

	extern double gameElapsedTime;

	//funcs
	// Creates a window, sets up events and begins the game
	bool init();

	// Releases all associated resources
	void destroy();

	// Steps the game ahead by ms milliseconds
	void update(double ms);

	// Renders our scene
	void draw();

	// game termination stuff
	bool gameCloseDetected();

	bool isGameLose();

	bool isGameWin();

	//funcs
	//init stuff
	bool initMeshTypes(const std::vector<std::pair<Model::MeshType, std::vector<SubObjectSource>>>& sources);

	bool loadSkybox(const std::string& skyboxFilename, const std::string& skyboxTextureFolder);

	//other
	std::pair<bool, glm::vec3> getTileCoordFromWindowCoords(double xpos, double ypos);

	void updateBoolFromKey(int action, int key, bool& toUpdate, const std::vector<int>& targetKeys);

	// !!! INPUT CALLBACK FUNCTIONS
	void on_key(GLFWwindow* window, int key, int scancode, int action, int mods);

	void on_mouse_move(GLFWwindow* window, double xpos, double ypos);

	void on_mouse_scroll(GLFWwindow* window, double xoffset, double yoffset);

	void on_mouse_button(GLFWwindow* window, int button, int action, int mods);

	void on_window_resize(GLFWwindow* window, int width, int height);
}
