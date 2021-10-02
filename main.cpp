#include "lux/kit.hpp"
#include "lux/define.cpp"
#include <string.h>
#include <vector>
#include <set>
#include <stdio.h>

using namespace std;
using namespace lux;

// TODO: move this as a method of cell
vector<Cell *> getAdjacentCells(Cell const *cell, GameMap &gameMap, vector<Cell *> const &resourceMask, vector<Cell *> const &cityMask)
{
	vector<Cell *> adjacentCells{};

	for (int y = -1; y <= 1; y++)
	{
		for (int x = -1; x <= 1; x++)
		{
			if (!(x == 0 && y == 0))
			{
				const Position position = cell->pos + Position(x, y);
				if (gameMap.isValidPosition(position))
				{
					Cell *otherCell = gameMap.getCellByPos(position);

					if (!(count(resourceMask.begin(), resourceMask.end(), otherCell)))
					{
						if (!(count(cityMask.begin(), cityMask.end(), otherCell)))
						{
							adjacentCells.push_back(otherCell);
						}
					}
				}
			}
		}
	}
	return adjacentCells;
}

Cell *getClosestCell(Cell const *cell, vector<Cell *> const &cellVector)
{
	Cell *closestCell;
	float closestDist = 9999999;
	for (auto it = cellVector.begin(); it != cellVector.end(); it++)
	{
		auto otherCell = *it;
		float dist = otherCell->pos.distanceTo(cell->pos);
		if (dist < closestDist)
		{
			closestDist = dist;
			closestCell = otherCell;
		}
	}
	return closestCell;
}

Cell *getCloserPoorestCityTile(Cell const *cell, GameMap &gameMap, Player &player)
{
	City *poorestCity = &player.cities.begin()->second;
	for (auto &element : player.cities)
	{
		lux::City *city = &element.second;
		if (city->fuel < poorestCity->fuel)
		{
			poorestCity = city;
		}
	}

	vector<Cell *> cityCells{};
	for (CityTile &cityTile : poorestCity->citytiles)
	{
		cityCells.push_back(gameMap.getCellByPos(cityTile.pos));
	}

	return getClosestCell(cell, cityCells);
}

int main()
{
	kit::Agent gameState = kit::Agent();
	// initialize
	gameState.initialize();

	const int max_cities = 2;
	map<string, Position> building_objectives{};

	while (true)
	{
		/** Do not edit! **/
		// wait for updates
		gameState.update();

		vector<string> actions = vector<string>();

		/** AI Code Goes Below! **/

		Player &player = gameState.players[gameState.id];
		Player &opponent = gameState.players[(gameState.id + 1) % 2];
		GameMap &gameMap = gameState.map;

		// initialize resourceTiles and cityTiles
		vector<Cell *> resourceTiles = vector<Cell *>();
		vector<Cell *> cityTiles = vector<Cell *>();
		for (int y = 0; y < gameMap.height; y++)
		{
			for (int x = 0; x < gameMap.width; x++)
			{
				Cell *cell = gameMap.getCell(x, y);
				if (cell->hasResource())
				{
					resourceTiles.push_back(cell);
				}
				if (cell->hasCitytile(player.team))
				{
					cityTiles.push_back(cell);
				}
			}
		}

		// we iterate over all our units and do something with them
		for (int i = 0; i < player.units.size(); i++)
		{
			Unit unit = player.units[i];
			Cell *unitCell = gameMap.getCellByPos(unit.pos);

			// get current objective
			Position *objective = 0;
			if (building_objectives.count(unit.id))
			{
				objective = &building_objectives.at(unit.id);
			}

			if (unit.isWorker() && unit.canAct())
			{

				// 1. if no objective, and has enough resource: set objective
				if (objective == 0 && unit.has_enough_resources() && cityTiles.size() < max_cities)
				{
					vector<Cell *> possibleCells = getAdjacentCells(
						unitCell,
						gameMap,
						resourceTiles,
						cityTiles);

					Cell *closestCell = getClosestCell(gameMap.getCellByPos(unit.pos), possibleCells);
					Position &pos = closestCell->pos;
					building_objectives.insert({unit.id, pos});
					objective = &pos;
					actions.push_back(Annotate::circle(pos.x, pos.y));
				}

				// if objective
				//  if at position: build
				//  else move toward pos

				if (objective != 0)
				{
					Position targetPosition = *objective;
					if (unit.pos == targetPosition)
					{
						actions.push_back(unit.buildCity());
						building_objectives.erase(unit.id);
					}
					else
					{
						auto dir = unit.pos.directionTo(targetPosition);
						actions.push_back(unit.move(dir));
					}
				}

				else if (unit.getCargoSpaceLeft() > 0)
				{
					// if the unit is a worker and we have space in cargo, lets find the nearest resource tile and try to mine it
					Cell *closestResourceTile;
					float closestDist = 9999999;
					for (auto it = resourceTiles.begin(); it != resourceTiles.end(); it++)
					{
						auto cell = *it;
						if (cell->resource.type == ResourceType::coal && !player.researchedCoal())
							continue;
						if (cell->resource.type == ResourceType::uranium && !player.researchedUranium())
							continue;
						float dist = cell->pos.distanceTo(unit.pos);
						if (dist < closestDist)
						{
							closestDist = dist;
							closestResourceTile = cell;
						}
					}
					if (closestResourceTile != nullptr)
					{
						auto dir = unit.pos.directionTo(closestResourceTile->pos);
						actions.push_back(unit.move(dir));
					}
				}
				else
				{
					// if unit is a worker and there is no cargo space left, and we have cities, lets return to them
					if (player.cities.size() > 0)
					{
						Cell *closestCityTile = getCloserPoorestCityTile(unitCell, gameMap, player);

						// not sure if useful
						if (closestCityTile != nullptr)
						{
							auto dir = unit.pos.directionTo(closestCityTile->pos);
							actions.push_back(unit.move(dir));
						}
					}
				}
			}
		}

		// you can add debug annotations using the methods of the Annotate class.
		// actions.push_back(Annotate::circle(0, 0));

		/** AI Code Goes Above! **/

		/** Do not edit! **/
		for (int i = 0; i < actions.size(); i++)
		{
			if (i != 0)
				cout << ",";
			cout << actions[i];
		}
		cout << endl;
		// end turn
		gameState.end_turn();
	}

	return 0;
}
