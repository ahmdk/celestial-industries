#pragma once

#include <cassert>
#include <cmath>
#include <sstream>
#include <cstring>

// glm
#define GLM_ENABLE_EXPERIMENTAL

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/quaternion.hpp"

// custom headers
#include "objrenderable.hpp"
#include "objloader.hpp"

// collision geometries
struct bounding_box {

//               	 @ + + + + + + + + @						|
//               	 +\                +\						|
//               	 + \               + \						|
//               	 +  \              +  \         y			|
//           width   +   \             +   \        |			|
//               	 +    @ + + + + + +++ + @       |______ x	|
//               	 +    +            +    +        \			|
//               	 +    +            +    +         \			|
//               	 +    +            +    +          z		|
//  corner position  @ + +++ + + + + + @    +					|
//               	  \   +             \   +					|
//               	   \  +              \  +					|
//               height \ +               \ +					|
//               	     \+    length      \+					|
//               	      @ + + + + + + + + @					|

	double length, width, height;
	glm::vec3 corner_position;
};

struct bounding_sphere {
	double radius;
	glm::vec3 center;
};

struct bounding_cylinder {
	double radius, height;
	glm::vec3 center;
};

enum collision_geometry_type {
	cg_bounding_box,
	cg_bounding_sphere,
	cg_bounding_cylinder,
};

class Entity : public OBJRenderable {
public:
	virtual void update(float ms) = 0;

	void set_velocity(glm::vec3);

	void set_gravity(glm::vec3);

	void set_force(glm::vec3);

	void set_geometryId(long);

	void set_rotation(glm::vec3);

	void set_translation(glm::vec3);

	void set_scale(glm::vec3);

	glm::mat4 model_matrix();

	void set_collision_geometry_type(collision_geometry_type);

	collision_geometry_type get_collision_geometry_type();

	glm::vec3 get_velocity();

	long get_geometryId();

	bool is_textured();

protected:
	// physical properties
	double density;
	double volume;
	glm::vec3 velocity;
	glm::vec3 gravity;
	glm::vec3 applied_force;
	glm::vec3 translation;
	glm::vec3 scale;
	glm::vec3 rotation;

	// collision geometry type
	collision_geometry_type cg_type;

	// graphics attributes
	size_t indices;
	bool texture_flag;

	// id number of the geometry
	long geometry_id;
};