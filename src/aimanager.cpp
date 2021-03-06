//
// Created by eric on 2/25/18.
//

#include "aimanager.hpp"
#include "unit.hpp"

namespace AI {
	namespace Manager {
		const double AI_VISIBLE_THRESHOLD = 0.4; //scout only if we've seen less than this value
		const int AI_RUN_THRESHOLD = 1600; //run every 1600ms
		const int TIMED_SPAWN_THRESHOLD = 20000; //spawn a unit every 20s
		const int UNSEEN_RADIUS_THRESHOLD = 6;
		const int FOG_OF_WAR_TIME_THRESHOLD = 10;
		int aiManagerRunIterations = 0;
		int playerUnitValue = 0;
		int aiUnitValue = 0;
		int playerBuildingValue = 0;
		double percentVisible = 0;
		double lastRunTimestamp = AI_RUN_THRESHOLD;
		double timedSpawner = 0;
		std::vector<Coord> aiSpawn;

		void init(size_t levelHeight, size_t levelWidth) {
			for (size_t z = 0; z < Global::levelArray.size(); z++) {
				for (size_t x = 0; x < Global::levelArray[z].size(); x++) {
					if (Global::levelArray[z][x] == Model::MeshType::ENEMY_PORTAL) {
						aiSpawn.emplace_back(x, z);
					}
				}
			}

		}

		void updateVisibilityMap() {
			int currentUnixTime = (int) getUnixTime();
			int cellsVisible = 0;
			for (const auto& row: Global::aiVisibilityMap) {
				for (const auto& cellLastSceneTime : row) {
					if (currentUnixTime - cellLastSceneTime < FOG_OF_WAR_TIME_THRESHOLD) {
						cellsVisible++;
					}
				}
			}

//			std::cout << "time: " << currentUnixTime << " cells seen recently: " << cellsVisible << '\n';
			percentVisible = (double) cellsVisible / (Global::levelHeight * Global::levelWidth);
		}

		void updateValueOfEntities() {
			aiUnitValue = 0;
			playerUnitValue = 0;
			playerBuildingValue = 0;
			for (auto& unit : Global::playerUnits) {
				playerUnitValue += unit->aiComp.value;
			}

			for (auto& unit : Global::aiUnits) {
				aiUnitValue += unit->aiComp.value;
			}

			for (auto& building : Global::buildingList) {

				playerBuildingValue += building->aiComp.value;

			}
		}

		bool isWithinBounds(int x, int z) {
			return x >= 0 && x < (int) Global::levelWidth &&
				   z >= 0 && z < (int) Global::levelHeight;
		}

		bool withinRangeOfOtherScoutTargets(int x, int z) {
			for (const auto& existingTarget : Global::scoutingTargetsInProgress) {
				int deltaX = existingTarget.colCoord - x;
				int deltaZ = existingTarget.rowCoord - z;
				float dist = sqrtf(deltaX * deltaX + deltaZ * deltaZ);
				if (dist < 4 * UNSEEN_RADIUS_THRESHOLD) {
					return true;
				}
			}
			return false;
		}

		Coord& getRandomSpawnLocation() {
			std::vector<Coord>::iterator randIt = aiSpawn.begin();
			std::advance(randIt, std::rand() % aiSpawn.size());
			return *randIt;
		}

		Coord findBestScoutLocation() {
			std::vector<std::vector<bool>> visited(Global::levelHeight, std::vector<bool>(Global::levelWidth));
			std::vector<std::vector<std::pair<int, int>>> parent(
					Global::levelHeight,
					std::vector<std::pair<int, int>>(Global::levelWidth,
													 {Coord::getInvalidCoord().colCoord,
													  Coord::getInvalidCoord().rowCoord}));
			//traverse tree with bfs
			int currentUnixTime = static_cast<int>(getUnixTime());
			Coord spawn = getRandomSpawnLocation();
			std::pair<int, int> root(spawn.colCoord, spawn.rowCoord);
			parent[spawn.rowCoord][spawn.colCoord] = root;
			visited[spawn.rowCoord][spawn.colCoord] = true;
			std::priority_queue<bfsState> queue;
			queue.push({spawn.colCoord, spawn.rowCoord, 0, 0});
			while (!queue.empty()) {
				bfsState u = queue.top();
				queue.pop();

				//use 2x because we want to capture diameter unseen dist
				if (u.unseenDistance >= 2 * UNSEEN_RADIUS_THRESHOLD && !withinRangeOfOtherScoutTargets(u.x, u.z)) {
					std::pair<int, int> traverser(u.x, u.z);
					//get to center of circle
					for (int i = UNSEEN_RADIUS_THRESHOLD; i > UNSEEN_RADIUS_THRESHOLD / 2 && traverser != root; i--) {
						traverser = parent[traverser.second][traverser.first]; //parent[z][x]
					}
					return Coord(traverser.first, traverser.second);
				}

				if (aiManagerRunIterations % 2 == 0) {
					auto start = std::begin(adj); //forward
					auto end = std::end(adj);
					for (auto dirIter = start; dirIter != end; dirIter++) {
						int nextX = u.x + dirIter->first;
						int nextZ = u.z + dirIter->second;
						int nextDist = u.unseenDistance;
						if (isWithinBounds(nextX, nextZ) && !visited[nextZ][nextX] &&
							AI::aStar::isTraversable(nextX, nextZ)) {
							visited[nextZ][nextX] = true;
							parent[nextZ][nextX] = {u.x, u.z};
							const int timeDelta = currentUnixTime - Global::aiVisibilityMap[nextZ][nextX];
							if (timeDelta > FOG_OF_WAR_TIME_THRESHOLD) {
								nextDist++;
							}
							queue.push({nextX, nextZ, nextDist, u.unseenTimeDeltaSum + timeDelta});
						}
					}
				} else {
					auto start = std::rbegin(adj); //backwards
					auto end = std::rend(adj);
					for (auto dirIter = start; dirIter != end; dirIter++) {
						int nextX = u.x + dirIter->first;
						int nextZ = u.z + dirIter->second;
						int nextDist = u.unseenDistance;
						if (isWithinBounds(nextX, nextZ) && !visited[nextZ][nextX] &&
							AI::aStar::isTraversable(nextX, nextZ)) {
							visited[nextZ][nextX] = true;
							parent[nextZ][nextX] = {u.x, u.z};
							const int timeDelta = currentUnixTime - Global::aiVisibilityMap[nextZ][nextX];
							if (timeDelta > FOG_OF_WAR_TIME_THRESHOLD) {
								nextDist++;
							}
							queue.push({nextX, nextZ, nextDist, u.unseenTimeDeltaSum + timeDelta});
						}
					}
				}
			}

			//assumes this is the player spawn and just goes to it
			Coord randomCoord = Coord::getRandomCoord((int) Global::levelWidth, (int) Global::levelHeight);
			while (!AI::aStar::isTraversable(randomCoord.colCoord, randomCoord.rowCoord)) {
				//make sure its traversable
				randomCoord = Coord::getRandomCoord((int) Global::levelWidth, (int) Global::levelHeight);
			}

			return randomCoord;
		}

		void updateUnitsSeen() {
			Global::playerUnitsSeenByAI.clear();//this is a hack because its slow, we need some removal proceedure
			Global::aiUnitsSeenByPlayer.clear();

			for (const auto& playerUnit : Global::playerUnits) {

				for (const auto& aiUnit : Global::aiUnits) {
					if (playerUnit->canSee(aiUnit)) {
						Global::playerUnitsSeenByAI.insert(aiUnit);
					}

					if (aiUnit->canSee(playerUnit)) {
						Global::aiUnitsSeenByPlayer.insert(playerUnit);
					}
				}
			}
		}

		std::shared_ptr<Entity> getBestScoutUnit(const Coord& targetLocation) {
			glm::vec3 targetLocation3(targetLocation.colCoord, 0, targetLocation.rowCoord);

			std::shared_ptr<Entity> bestUnit;
			float bestDist = std::numeric_limits<float>::max();
			for (const auto& unit : Global::aiUnits) {
				float dist = glm::l2Norm(targetLocation3, unit->getPosition());
				if (dist < bestDist && unit->unitComp.state == UnitState::IDLE) {
					bestUnit = unit;
					bestDist = dist;
				}
			}
			return bestUnit;
		}

		void cleanupCompletedScoutTargets() {
			for (const auto& aiUnit: Global::aiUnits) {
				if (!aiUnit->hasMoveTarget()) {
					Global::scoutingTargetsInProgress.erase(Coord(aiUnit->currentDestination));
				}
			}
		}


		void update(double elapsed_ms) {
			lastRunTimestamp += elapsed_ms;
			timedSpawner += elapsed_ms;

			if (lastRunTimestamp > AI_RUN_THRESHOLD) {
				lastRunTimestamp = 0; //reset as we past the threshold
			} else {
				return; //run only after we exceed the threshold
			}

			++aiManagerRunIterations;
			cleanupCompletedScoutTargets();
			updateValueOfEntities();
			updateUnitsSeen();

			//print grid
//			for (const auto& row: Global::aiVisibilityMap) {
//				for (const auto& cellLastSceneTime : row) {
//					printf("%d ", (uint8_t)cellLastSceneTime);
//				}
//				printf("\n");
//			}
			//send unit to scout
			updateVisibilityMap();

			if (percentVisible < AI_VISIBLE_THRESHOLD) {
				Coord loc = findBestScoutLocation();
				std::shared_ptr<Entity> bestUnit = getBestScoutUnit(loc);
				if (bestUnit) { //if not null
					bestUnit->moveTo(UnitState::SCOUT, {loc.colCoord, 0, loc.rowCoord});
					Global::scoutingTargetsInProgress.insert(loc);
				}
			}

			//simple ai spawning strategy
			//if ai has less units than players, then spawn an ai unit
			int playerValue = playerUnitValue + playerBuildingValue;
			logger(LogLevel::DEBUG) << "playerValue: " << playerValue << ", aiValue: " << aiUnitValue << '\n';
			if (playerValue > aiUnitValue || timedSpawner > TIMED_SPAWN_THRESHOLD) {
				//spawns a unit once per 1600 ms if less value, or once ever THRESHOLD seconds
				timedSpawner = 0;
				Coord spawnLocation = getRandomSpawnLocation();
				Unit::spawn(Model::MeshType::ENEMY_RANGED_LINE_UNIT,
							{spawnLocation.colCoord, 0, spawnLocation.rowCoord},
							GamePieceOwner::AI);
				logger(LogLevel::DEBUG) << "spawned at: " << spawnLocation << '\n';

			}
		}
	}
}