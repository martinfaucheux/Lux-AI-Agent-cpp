#ifndef pathfinder_h
#define pathfinder_h

#include <vector>
#include <deque>
#include <set>
#include <stdio.h>
#include "map.hpp"
#include "position.hpp"

namespace lux
{
    using namespace std;

    class PathFinder
    {
    public:
        static DIRECTIONS getDirection(GameMap const &gameMap, Position const startPos, Position const &endPos)
        {
            vector<Position> path = breadthFirstSearch(gameMap, startPos, endPos);
            DIRECTIONS direction = DIRECTIONS::CENTER;

            if (path.size() > 1)
            {
                Position nextPos = path[1];
                Position posDiff = nextPos - startPos;
                direction = posDiff.toDirection();
            }
            return direction;
        }

    private:
        static vector<Position> breadthFirstSearch(GameMap const &gameMap, Position const &startPos, Position const &endPos)
        {
            vector<Position> defaultPath{startPos};
            deque<vector<Position>> queue{defaultPath};
            set<Position> seen{startPos};

            while (!queue.empty())
            {
                vector<Position> const path = queue.front();
                queue.pop_front();
                Position currentPos = path.back();

                if (currentPos == endPos)
                {
                    return path;
                }
                for (Position &otherPos : gameMap.getPlusNeighbors(currentPos))
                {

                    if (!seen.count(otherPos))
                    {
                        const Cell *cell = gameMap.getCellByPos(otherPos);
                        if (cell->citytile == 0)
                        {
                            vector<Position> cpPath = path;
                            cpPath.push_back(otherPos);

                            queue.push_back(cpPath);
                            seen.insert(otherPos);
                        }
                    }
                }
            }
            return defaultPath;
        };
    };

};
#endif
