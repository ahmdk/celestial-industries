#include "level.hpp"
#include <iostream>

bool Level::init(const std::vector<std::vector<TileType>>& levelArray,
				 const std::vector<std::pair<TileType, std::string>>& sources) {
	if (!initTileTypes(sources)) {
		std::cout << "Failed to init tile types!" << std::endl;
		return false;
	}

	// So that re initializing will be the same as first initialization
	tiles.clear();

	for (size_t i = 0; i < levelArray.size(); i++) {
		std::vector<TileType> row = levelArray[i];
		std::vector<Tile> tileRow;
		for (size_t j = 0; j < row.size(); j++) {
			TileType cell = row[j];
			Tile tile;
			bool success = tile.init(tileTypes[cell]);
			if (!success) {
				std::cout << "FAILED TO INITIALIZE TILE OF TYPE " << static_cast<int>(cell) << std::endl;
			}
			// TODO: Standardize tile size and resize the model to be the correct size
			tile.translate({j, 0, i});
			tileRow.push_back(tile);
		}
		tiles.push_back(tileRow);
	}
	return true;
}

std::vector<std::vector<TileType>> Level::levelLoader(const std::string& levelTextFile) {
	std::ifstream level(levelTextFile);
	std::string line;
	std::vector<std::vector<TileType>> levelData;

	if (level.is_open()) {
		int rowNumber = 0;
		while (getline(level, line)) {
			std::vector<TileType> row;
			std::vector<aStarPathState> tileData;
			int colNumber = 0;
			for (const char tile : line) {
				switch (tile) {
					case '#': {
						row.push_back(TileType::BRICK_CUBE);
						tileData.emplace_back(rowNumber, colNumber, 1000.0, INF);
						break;
					}

					case ' ': {
						row.push_back(TileType::SAND_1);
						tileData.emplace_back(rowNumber, colNumber, 10.0, INF);
						break;
					}
					default: {
						row.push_back(TileType::SAND_2);
						tileData.emplace_back(rowNumber, colNumber, 10.0, INF);
						break;
					}
				}
				colNumber++;
			}
			levelData.push_back(row);
			levelTraversalCostMap.push_back(tileData);
			rowNumber++;
		}
		level.close();
	} else fprintf(stderr, "Failed to open level data file");
	return levelData;
}

std::vector<std::vector<aStarPathState>> Level::getLevelTraversalCostMap() {
	return this->levelTraversalCostMap;
}

bool Level::initTileTypes(const std::vector<std::pair<TileType, std::string>>& sources) {
	// All the models come from the same place
	std::string path = pathBuilder({"data", "models"});
	for (auto source : sources) {
		TileType tileType = source.first;
		std::string filename = source.second;
		OBJ::Data obj;
		if (!OBJ::Loader::loadOBJ(path, filename, obj)) {
			// Failure message should already be handled by loadOBJ
			return false;
		}
		tileTypes[tileType] = obj;
	}
	return true;
}


bool Level::displayPath(const std::vector<aStarPathState>& path) {

	for (auto component : path) {
		std::vector<Tile> tileRow;

		Tile tile;
		bool success = tile.init(tileTypes[TileType::SAND_2]);
		if (!success) {
			std::cout << "FAILED TO INITIALIZE TILE OF TYPE " << static_cast<int>(TileType::SAND_2) << std::endl;
		}
		// TODO: Standardize tile size and resize the model to be the correct size
		tile.translate({component.colCoord, 0, component.rowCoord});
		tileRow.push_back(tile);
		tiles.push_back(tileRow);
	}

	return true;
}