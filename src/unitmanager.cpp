//
// Created by eric on 3/16/18.
//

#include <complex>
#include "unitmanager.hpp"

namespace UnitManager {

	std::vector<std::shared_ptr<Entity>> selectedUnits;

	void init(size_t levelHeight, size_t levelWidth) {
		Global::aiVisibilityMap = std::vector<std::vector<int>>(levelHeight, std::vector<int>(levelWidth));
		Global::playerVisibilityMap = std::vector<std::vector<int>>(levelHeight, std::vector<int>(levelWidth));
	}

	bool isDead(std::shared_ptr<Entity>& unit) {
		if (unit->aiComp.currentHealth <= 0) {
			unit->softDelete();
		}
		return unit->aiComp.currentHealth <= 0;
	}

	void removeDead() {
		//std::cout << "before: " << playerUnits.size() << '\n';
		Global::playerUnits.erase(std::remove_if(Global::playerUnits.begin(), Global::playerUnits.end(), isDead),
								  Global::playerUnits.end());
		//std::cout << "after: " << playerUnits.size() << '\n';
		Global::aiUnits.erase(std::remove_if(Global::aiUnits.begin(), Global::aiUnits.end(), isDead),
							  Global::aiUnits.end());
	}

	void updateAreaSeenByUnit(const std::shared_ptr<Entity>& unit, int currentUnixTime,
							  std::vector<std::vector<int>>& visibilityMap) {
		int radius = unit->aiComp.visionRange;
		const int xCenter = unit->getPositionInt().colCoord;
		const int yCenter = unit->getPositionInt().rowCoord;
		int xMin = std::max(0, xCenter - radius);
		int xMax = std::min((int) Global::levelWidth, xCenter + radius);
		int yMin = std::max(0, yCenter - radius);
		int yMax = std::min((int) Global::levelHeight, yCenter + radius);

		for (int i = yMin; i < yMax; ++i) {
			for (int j = xMin; j < xMax; ++j) {
				int yp = i - yCenter;
				int xp = j - xCenter;
				if (xp * xp + yp * yp <= radius * radius) {
					visibilityMap[i][j] = currentUnixTime;
				}
			}
		}
	}

	void update(double elapsed_ms) {
		removeDead();
		int currentUnixTime = (int) getUnixTime();
		for (auto& playerUnit : Global::playerUnits) {
			playerUnit->unitComp.update();
			playerUnit->computeNextMoveLocation(elapsed_ms);
			Global::levelWithUnitsTraversalCostMap[(int) (playerUnit->getPosition().z + 0.5)][(int) (
					playerUnit->getPosition().x + 0.5)] = Config::OBSTACLE_COST;
		}

		for (auto& aiUnit : Global::aiUnits) {
			aiUnit->unitComp.update();
			aiUnit->computeNextMoveLocation(elapsed_ms);
			Global::levelWithUnitsTraversalCostMap[(int) (aiUnit->getPosition().z + 0.5)][(int) (
					aiUnit->getPosition().x + 0.5)] = Config::OBSTACLE_COST;
		}

		Model::collisionDetector.findCollisions(elapsed_ms);

		for (auto& playerUnit : Global::playerUnits) {
			playerUnit->move(elapsed_ms);
			updateAreaSeenByUnit(playerUnit, currentUnixTime, Global::playerVisibilityMap);
		}

		for (auto& aiUnit : Global::aiUnits) {
			aiUnit->move(elapsed_ms);
			updateAreaSeenByUnit(aiUnit, currentUnixTime, Global::aiVisibilityMap);
		}
	}

	//geometry stuff from UBC CPSC 490 code archive. this is used by selectUnitsInTrapezoid() since
	//we rectangle select creates trapezoids in the game world
	//this helps check if a unit is in the trapezoid
	typedef std::complex<double> pt;
	typedef std::vector<pt> pol;

	double cp(const pt& a, const pt& b) { return imag(conj(a) * b); }

	double dp(const pt& a, const pt& b) { return real(conj(a) * b); }

	bool pt_in_polygon(const pt& p, const pol& v) {
		double res = 0;
		for (size_t i = v.size() - 1, j = 0; j < v.size(); i = j++)
			res += atan2(cp(v[i] - p, v[j] - p), dp(v[i] - p, v[j] - p));
		return abs(res) > 1;
	} // will be either 2*PI or 0


//highlight gets only friendly units, point click gets both friendly and enemy units
//since player might want to inspect enemy unit
	std::vector<std::shared_ptr<Entity>>
	getUnitsInPolygon(glm::vec3 topLeft, glm::vec3 topRight, glm::vec3 bottomLeft, glm::vec3 bottomRight) {
		std::vector<std::shared_ptr<Entity>> returnUnits;

		for (const auto& playerUnit : Global::playerUnits) {
			pt playerUnitPos{playerUnit->getPosition().x, playerUnit->getPosition().z};
			pt topLeftP{topLeft.x, topLeft.z};
			pt topRightP{topRight.x, topRight.z};
			pt bottomLeftP{bottomLeft.x, bottomLeft.z};
			pt bottomRightP{bottomRight.x, bottomRight.z};
			if (pt_in_polygon(playerUnitPos, {topLeftP, topRightP, bottomRightP, bottomLeftP})) { //the
				returnUnits.push_back(playerUnit);
			}
		}

		return returnUnits;
	}

	std::pair<float, std::shared_ptr<Entity>> getClosestUnitToTarget(const glm::vec3& targetLocation) {
		float bestDist = FLT_MAX;
		std::shared_ptr<Entity> closestUnitToTarget;
		for (const auto& aiUnit : Global::aiUnits) {
			float unitDist = glm::distance(aiUnit->getPosition(), targetLocation);
			if (unitDist <= bestDist) {
				bestDist = unitDist;
				closestUnitToTarget = aiUnit;
			}
		}

		for (const auto& playerUnit : Global::playerUnits) {
			float unitDist = glm::distance(playerUnit->getPosition(), targetLocation);
			if (unitDist <= bestDist) {
				bestDist = unitDist;
				closestUnitToTarget = playerUnit;
			}
		}

		return {bestDist, closestUnitToTarget};
	}

	std::pair<float, std::shared_ptr<Entity>> getClosestBuildingToTarget(const glm::vec3& targetLocation) {
		float bestDist = FLT_MAX;
		std::shared_ptr<Entity> closestUnitToTarget;
		for (const auto& building : Global::buildingTileList) {
			float unitDist = glm::distance(building->getPosition(), targetLocation);
			if (unitDist <= bestDist) {
				bestDist = unitDist;
				closestUnitToTarget = building;
			}
		}

		return {bestDist, closestUnitToTarget};
	}


	void selectUnit(const glm::vec3& targetLocation) {
		selectedUnits.clear();
		std::pair<float, std::shared_ptr<Entity>> bestUnit = getClosestUnitToTarget(targetLocation);

		//add enemy units if just a point click
		if (bestUnit.first < Config::POINT_CLICK_DISTANCE_THRESHOLD) {
			selectedUnits.push_back(bestUnit.second);
			logger(LogLevel::DEBUG) << "selected: " << UnitManager::selectedUnits.size() << "\t target: " <<
					  bestUnit.second->getPosition().x << ' ' << bestUnit.second->getPosition().z << "\tactual loc: " <<
					  targetLocation.x << ' ' << targetLocation.z << ' ' << '\n';
		}
	}

	void selectUnitsInTrapezoid(const glm::vec3& topLeft, const glm::vec3& topRight,
								const glm::vec3& bottomLeft, const glm::vec3& bottomRight) {
		selectedUnits.clear();
		selectedUnits = getUnitsInPolygon(topLeft, topRight, bottomLeft, bottomRight);
	}


	void attackTargetLocationWithSelectedUnits(const glm::vec3& targetLocation, bool queueCommand) {
		//find unit closest to targetLocation
		std::pair<float, std::shared_ptr<Entity>> closestUnit = getClosestUnitToTarget(targetLocation);
		glm::vec3 position;
		if (closestUnit.first < Config::RIGHT_CLICK_ATTACK_WITHIN_RANGE_THRESHOLD) {
			position = closestUnit.second->getPosition();
		} else { // no unit found, so attackMove to that location
			position = targetLocation;
		}
		int n = 0;
		int nCutoff = 10; // We give up at that point
		for (auto& unit : selectedUnits) {
			glm::vec3 specificPosition = position + spiralOffset(n);
			while (!AI::aStar::isTraversable(specificPosition.x, specificPosition.z) && n < nCutoff) {
				n++;
				specificPosition = position + spiralOffset(n);
			}
			unit->moveTo(UnitState::ATTACK_MOVE, specificPosition, queueCommand);
			n++;
		}
	}

	void sortSelectedUnits() {
		auto comparator = [](const std::shared_ptr<Entity>& l, const std::shared_ptr<Entity>& r) {
			return l->meshType < r->meshType;
		};
		sort(selectedUnits.begin(), selectedUnits.end(), comparator);
	}

	glm::vec3 spiralOffset(int n) {
		float k = ceil((sqrt(n) - 1) / 2);
		float t = 2 * k + 1;
		float m = t * t;
		if (n >= (m - t))
			return {k - (m - n), 0, -k};
		else
			m = m - t;
		if (n >= (m - t))
			return {-k, 0, -k + (m - n)};
		else
			m = m - t;
		if (n >= (m - t))
			return {-k + (m - n), 0, k};
		else
			return {k, 0, k - (m - n - t)};
	}
}
