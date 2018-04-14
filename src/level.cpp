#include <iostream>
#include "global.hpp"
#include "level.hpp"
#include "logger.hpp"
#include "particle.hpp"
#include "coord.hpp"

std::map<Model::MeshType, std::pair<int, float>> Level::tileToCost{
		{Model::MeshType::HROAD,      {Config::OBSTACLE_COST,            INF}},
		{Model::MeshType::SAND_1,     {Config::DEFAULT_TRAVERSABLE_COST, INF}},
		{Model::MeshType::SAND_2,     {Config::DEFAULT_TRAVERSABLE_COST, INF}},
		{Model::MeshType::SAND_3,     {Config::DEFAULT_TRAVERSABLE_COST, INF}},
		{Model::MeshType::SAND_4,     {Config::DEFAULT_TRAVERSABLE_COST, INF}},
		{Model::MeshType::SAND_5,     {Config::DEFAULT_TRAVERSABLE_COST, INF}},
		{Model::MeshType::TREE,       {Config::OBSTACLE_COST,            INF}},
		{Model::MeshType::YELLOWTREE, {Config::OBSTACLE_COST,            INF}},
		{Model::MeshType::REDTREE,    {Config::OBSTACLE_COST,            INF}},
		{Model::MeshType::WATER,      {Config::OBSTACLE_COST,            INF}},
		{Model::MeshType::GRASS,      {Config::DEFAULT_TRAVERSABLE_COST, INF}},
		{Model::MeshType::HROAD,      {Config::DEFAULT_TRAVERSABLE_COST, INF}},
		{Model::MeshType::VROAD,      {Config::DEFAULT_TRAVERSABLE_COST, INF}},
		{Model::MeshType::GEYSER,     {Config::OBSTACLE_COST,            INF}}
};

std::map<char, Model::MeshType> Level::charToType{
		{'#',  Model::MeshType::HROAD},
		{' ',  Model::MeshType::SAND_1},
		{'\\', Model::MeshType::SAND_2},
		{'.',  Model::MeshType::SAND_3},
		{';',  Model::MeshType::SAND_4},
		{',',  Model::MeshType::SAND_5},
		{'T',  Model::MeshType::TREE},
		{'Y',  Model::MeshType::YELLOWTREE},
		{'R',  Model::MeshType::REDTREE},
		{'W',  Model::MeshType::WATER},
		{'G',  Model::MeshType::GRASS},
		{'H',  Model::MeshType::HROAD},
		{'V',  Model::MeshType::VROAD},
		{'P',  Model::MeshType::GEYSER},
		{'X',  Model::MeshType::GUN_TURRET}
};


bool Level::init(const std::vector<std::shared_ptr<Renderer>>& meshRenderers) {
	// So that re initializing will be the same as first initialization
	tiles.clear();
	
	for (size_t i = 0; i < Global::levelArray.size(); i++) {
		std::vector<Model::MeshType> row = Global::levelArray[i];
		for (size_t j = 0; j < row.size(); j++) {
            Model::MeshType type = row[j];
			std::shared_ptr<Tile> tilePointer = tileFromMeshType(type);
			// TODO: Standardize tile size and resize the model to be the correct size
            tilePointer->setPosition({ j, 0, i });
			tilePointer->position = { j, 0, i };	// Because the tile needs its position on its own as well
            tiles.push_back(tilePointer);
		}
	}
	tileCursor = std::make_shared<Tile>(Model::MeshType::TILE_CURSOR);

	return true;
}

// Takes in full path
void Level::save(std::string filename)
{
	std::map<short, char> typeToChar;
	for (const auto& pair : charToType) {
		typeToChar[pair.second] = pair.first;
	}
	std::ofstream fs(filename);
	if (!fs) {
		logger(LogLevel::ERR) << "Failed to save level with name " << filename;
		throw "Failed to open file for writing";
	}
	for (const auto& row : Global::levelTraversalCostMap) {
		for (const auto& cell : row) {
			fs << typeToChar[cell.type];
		}
		fs << '\n';
	}
	fs.close();
}

void Level::update(float ms)
{
    for (auto& tile : tiles) {
		tile->update(ms);
    }
}

AStarNode Level::nodeFromCost(int row, int col, Model::MeshType type) {
	std::pair<int, float> cost = tileToCost[type];
	// TODO: Why are we giving this stuff doubles instead of ints?
	return AStarNode(col, row, cost.first, cost.second, (short)type);
}

std::vector<std::vector<Model::MeshType>> Level::levelLoader(const std::string& levelTextFile) {
	std::ifstream level(levelTextFile);
	std::string line;
	std::vector<std::vector<Model::MeshType>> levelData;
	std::vector<Model::MeshType> row;
	std::vector<AStarNode> tileData;

	if (!level.is_open()) {
		logger(LogLevel::ERR) << "Failed to open level data file '" << levelTextFile << "'\n";
		return {};
	}

	for (int rowNumber = 0; getline(level, line); rowNumber++) {
		row.clear();
		tileData.clear();
		int colNumber = 0;
		for (const char tile : line) {
			// Actually add the tiles
			if (charToType.find(tile) == charToType.end()) {
				// Not in map
				row.push_back(Model::MeshType::SAND_2);
				tileData.push_back(nodeFromCost(rowNumber, colNumber, Model::MeshType::SAND_2));
			}
			else {
				row.push_back(charToType[tile]);
				tileData.push_back(nodeFromCost(rowNumber, colNumber, charToType[tile]));
			}
			colNumber++;
		}
		levelData.push_back(row);
		Global::levelTraversalCostMap.push_back(tileData);
	}
	level.close();

	return levelData;
}

std::vector<std::vector<AStarNode>> Level::getLevelTraversalCostMap() {
	return Global::levelTraversalCostMap;
}

inline bool tilesOverlap(glm::vec3 locA, glm::vec3 sizeA, glm::vec3 locB, glm::vec3 sizeB) {
	int aLowerCornerX = locA.x;
	int aLowerCornerZ = locA.z - sizeA.z;
	int bLowerCornerX = locB.x;
	int bLowerCornerZ = locB.z - sizeB.z;
	int aUpperCornerX = locA.x + sizeA.x;
	int aUpperCornerZ = locA.z;
	int bUpperCornerX = locB.x + sizeB.x;
	int bUpperCornerZ = locB.z;

	int abx = bLowerCornerX - aUpperCornerX;
	int bax = aLowerCornerX - bUpperCornerX;
	int aby = bLowerCornerZ - aUpperCornerZ;
	int bay = aLowerCornerZ - bUpperCornerZ;

	if (abx >= 0 || aby >= 0)
		return false;

	if (bax >= 0 || bay >= 0)
		return false;

	return true;
}

std::shared_ptr<Tile> Level::placeTile(Model::MeshType type, glm::vec3 location, GamePieceOwner owner, unsigned int width, unsigned int height, int extraArg, Model::MeshType replacingMesh)
{
	// Update graphics
	glm::vec3 size = { width, 0, height };
	for (auto& tile : tiles) {
		if(!tile->isDeleted && tilesOverlap(tile->position, tile->size, location, size)) {
			// Get rid of old tile
			tile->removeSelf();

			// Handle case in which we would end up with empty tiles
			int minX = tile->position.x;
			int maxX = (tile->position + tile->size).x;
			int minZ = (tile->position - tile->size).z;
			int maxZ = tile->position.z;
			for (int x = minX; x < maxX; x++) {
				for (int z = maxZ; z > minZ; z--) {
					if (z <= minZ) {
					}
					if (!tilesOverlap({ x,0,z }, { 1,0,1 }, location, size)) {
						placeTile(replacingMesh, { x, 0, z }, GamePieceOwner::NONE);
					}
				}
			}
			// TODO: Actually remove the tiles lol (memory is still allocated)
		}
	}

	// Update AI info
	for (unsigned int x = location.x; x < location.x + width; x++) {
		for (unsigned int y = location.z; y < location.z + height; y++) {
			Global::levelTraversalCostMap[y][x] = nodeFromCost(x,y, type);
		}
	}

	std::shared_ptr<Tile> newTile = tileFromMeshType(type, extraArg);
	newTile->setPosition(location);
	newTile->position = location;
	setupAiCompForTile(newTile, owner);
	tiles.push_back(newTile);
	return newTile;
}

int Level::numTilesOfTypeInArea(Model::MeshType type, glm::vec3 location, unsigned int height, unsigned int width)
{
	int total = 0;
	glm::vec3 size = { width, 0, height };
	for (auto& tile : tiles) {
		if (!tile->isDeleted && tilesOverlap(tile->position, tile->size, location, size)) {
			if (tile->type == type)total++;
		}
	}
	return total;
}

int Level::numTilesOfOwnerInArea(GamePieceOwner owner, glm::vec3 location, unsigned int height, unsigned int width)
{
	int total = 0;
	glm::vec3 size = { width, 0, height };
	for (auto& tile : tiles) {
		if (!tile->isDeleted && tilesOverlap(tile->position, tile->size, location, size)) {
			if (tile->aiComp.owner == owner)total++;
		}
	}
	return total;
}

std::shared_ptr<Tile> Level::tileFromMeshType(Model::MeshType type, int extraArg)
{
	switch (type) {
		case Model::MeshType::GUN_TURRET:
			return std::make_shared<GunTowerTile>();
		case Model::MeshType::REFINERY:
			return std::make_shared<RefineryTile>(extraArg);
		case Model::MeshType::GEYSER:
			return std::make_shared<GeyserTile>(particleShader, particleTexture);
		default:
			return std::make_shared<Tile>(type);
	}
}

void Level::setupAiCompForTile(std::shared_ptr<Tile> tile, GamePieceOwner owner)
{
	tile->aiComp.owner = owner;
	switch (tile->type)	{
		case (Model::SUPPLY_DEPOT): {
			tile->aiComp.totalHealth = 400;
			tile->aiComp.visionRange = 10;
			tile->aiComp.type = GamePieceClass::BUILDING_NON_ATTACKING;
			tile->aiComp.currentHealth = tile->aiComp.totalHealth;
			tile->aiComp.value = 100;
			tile->unitComp.state = UnitState::NONE;
			break;
		}
		case (Model::MINING_TOWER): {
			tile->aiComp.totalHealth = 500;
			tile->aiComp.visionRange = 10;
			tile->aiComp.type = GamePieceClass::BUILDING_NON_ATTACKING;
			tile->aiComp.currentHealth = tile->aiComp.totalHealth;
			tile->aiComp.value = 75;
			tile->unitComp.state = UnitState::NONE;
			break;
		}
		case (Model::GUN_TURRET): {
			tile->aiComp.totalHealth = 250;
			tile->aiComp.visionRange = 10;
			tile->aiComp.type = GamePieceClass::BUILDING_DEFENSIVE_ACTIVE;
			tile->aiComp.currentHealth = tile->aiComp.totalHealth;
			tile->aiComp.value = 100;
			tile->unitComp.state = UnitState::NONE;
			break;
		}
		case (Model::PHOTON_TOWER): {
			tile->aiComp.totalHealth = 250;
			tile->aiComp.visionRange = 10;
			tile->aiComp.type = GamePieceClass::BUILDING_DEFENSIVE_ACTIVE;
			tile->aiComp.currentHealth = tile->aiComp.totalHealth;
			tile->aiComp.value = 100;
			tile->unitComp.state = UnitState::NONE;
			break;
		}
		default:
			break;
		}
}

void Level::displayPath(const std::vector<Coord>& path) {
	// TODO: Replace tiles in question instead of just adding 2 tiles in one spot
	for (const Coord& component : path) {
		auto tile = std::make_shared<Tile>(Model::MeshType::SAND_2);
		tile->translate({component.colCoord, 0, component.rowCoord});
		tiles.push_back(tile);
	}
}
