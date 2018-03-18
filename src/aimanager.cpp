//
// Created by eric on 2/25/18.
//

#include "aimanager.hpp"

Building* AiManager::bestBuildingToAttack(std::vector<Building>& buildings, Entity& entity) {
    float bestAttackValue = -1;
    Building *building;

    if (buildings.size() <= 0) {
        // TODO: take care of case where length of building list passed in is 0
    }

    for (auto& currentBuilding : buildings) {
        int buildingValue = currentBuilding.buildingValue;

        float distanceToBuilding = 0;//getDistanceBetweenEntities(currentBuilding, entity); //fixme to revert

        float attackValue = buildingValue - (distanceToBuilding * PRIORITIZE_CLOSER_ATTACKS);

        if (attackValue > bestAttackValue) {
            bestAttackValue = attackValue;
            building = &currentBuilding;
        }
    }

    return building;
}

Building* AiManager::getHighestValuedBuilding(std::vector<Building>& buildings) {
    int highestValueSoFar = -1;
    Building *building;

    if (buildings.size() <= 0) {
        // TODO: take care of case where length of building list passed in is 0
    }

    for (auto& currentBuilding : buildings) {
        if(currentBuilding.buildingValue > highestValueSoFar) {
            highestValueSoFar = currentBuilding.buildingValue;
            building = &currentBuilding;
        }
    }

    return building;
}