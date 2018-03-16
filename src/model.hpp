#pragma once
#include <vector>
#include <map>
#include <utility>
#include "objloader.hpp"
#include "renderer.hpp"
#include "collisiondetector.hpp"

// glfw
#define NOMINMAX
#include <gl3w.h>
#include <GLFW/glfw3.h>

namespace Model {

    enum class MeshType {
        SAND_1,
        SAND_2,
        SAND_3,
        WALL,
        BRICK_CUBE,
        MINING_TOWER,
        PHOTON_TOWER,
        TREE,
        GUN_TURRET,
        BALL,
        GEYSER,
        PARTICLE,
    };

    extern std::vector<std::pair<Model::MeshType, std::vector<SubObjectSource>>> meshSources;
    extern std::vector<std::shared_ptr<Renderer>>meshRenderers;
    extern CollisionDetector collisionDetector;
    Renderable createRenderable(MeshType type);
}
