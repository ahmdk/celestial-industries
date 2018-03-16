#pragma once

#include "config.hpp"
#include "logger.hpp"
#include "objloader.hpp"

// stlib
#include <string>
#include <memory>
#include <ostream>


// glfw
#define NOMINMAX

#include <gl3w.h>
#include <GLFW/glfw3.h>

// glm
#include <glm/fwd.hpp>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

// Simple utility macros to avoid mistyping directory name, name has to be a string literal
// audio_path("audio.ogg") -> data/audio/audio.ogg
#ifdef _MSC_VER
#define data_path "../data"
#define shader_path(name) "../shaders/" name
#else
#define data_path "data"
#define shader_path(name) "shaders/" name
#endif
#define textures_path(name) data_path "/textures/" name
#define audio_path(name) data_path "/audio/" name
#define mesh_path(name) data_path "/meshes/" name


// Our stuff
extern Logger logger; //use extern so we share the logger in main.cpp with all other cpp files

std::vector<std::string> splitString(std::string input, const char separator);

char separator();

std::string pathBuilder(std::vector<std::string> parts);

std::string pathAppender(std::string base, std::vector<std::string> parts);

// OpenGL utilities
// cleans error buffer
void gl_flush_errors();

bool gl_has_errors();

// Because iterating over pointer is good
template<typename T>
auto inline begin(std::shared_ptr<T> ptr) -> typename T::iterator {
	return ptr->begin();
}

template<typename T>
auto inline end(std::shared_ptr<T> ptr) -> typename T::iterator {
	return ptr->end();
}

struct Coord {
	int rowCoord, colCoord;

	Coord() = default;

	Coord(int _rowCoord, int _colCoord) : rowCoord(_rowCoord), colCoord(_colCoord) {}

	bool operator==(const Coord& rhs) const {
		return rowCoord == rhs.rowCoord &&
			   colCoord == rhs.colCoord;
	}

	friend std::ostream& operator<<(std::ostream& os, const Coord& coord) {
		os << "rowCoord: " << coord.rowCoord << " colCoord: " << coord.colCoord;
		return os;
	}
};

template<class T>
T clamp(T lower, T value, T upper) {
	if (value > upper) {
		value = upper;
	}

	if (value < lower) {
		value = lower;
	}
	return value;
}