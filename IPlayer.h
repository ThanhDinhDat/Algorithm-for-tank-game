//GROUP03

#pragma once

#include "IPlayer.h"
#include "IPlayerInfo.h"
#include "IGameInfo.h"
#include "Command.h"
#include <queue>
#include <unordered_map>
#include <iostream>
#include <list>
#include <cstdlib>
#include <sstream>

class Group03 : public IPlayer {
private:
	bool haveShortestPathList; //check if there is a shortestPath list for each tank
	bool Attacked;	//check if the HQ is being attacked
	int CurrentHQ, IniHQ;	//to check if the HQ of the HQ is changed

	//FIND TANK WHICH IS ON PROTECTING MOD
	std::list<ITank*> FinishHQ;

	//an object to use with PriorityQueue
	struct PriorityQueueEle {
		std::pair<int, int> loc;
		int priorityVal;

		PriorityQueueEle(std::pair<int, int> location, int priority) {
			loc = location;
			priorityVal = priority;
		}
	};

	//a compare class to use with the C++ std::priority_queue
	class Compare {
	public:
		bool operator() (PriorityQueueEle ele1, PriorityQueueEle ele2) {
			return ele1.priorityVal > ele2.priorityVal;	//the samller the higher the priority
		}
	};

	int manhattanDist(std::pair<int, int> node, std::pair<int, int> dest); //calculate the manhattan dist between two nodes
	void reconstructPath(std::queue<std::pair<int, int>> &path, std::unordered_map<std::string, std::pair<int, int>> cameFrom, std::pair<int, int> start, std::pair<int, int> dest); //reconstruct the path after is is found with the algo
	std::string toString(std::pair<int, int> p); //convert a pair to a string to use the key hashing in unordered map

	//8/10/16
	//ENEMY PLAYER INFO SUPER FUN
	IPlayerInfo* myNemesis;
	ITank* chosenOne;

	//get the list of tank in range
	std::list<ITank*> findTankInRange(ITank* myTank);	

	//FIND IF THE TANK IS STUCK
	bool Block(ITank* nextTank,std::pair<int,int> NextMove);

	//8/11/16
	//a list of shortest path bind with tanks
	std::unordered_map<std::string, std::queue<std::pair<int, int>>> shortestPathList;

	//convert the address of ITank* to string for hashing
	std::string toStringTank(ITank* tank);

	//the astar algorithm (NOW RETURN A QUEUE)
	std::queue<std::pair<int, int>> aStarAlgo(IPlayerInfo* playerInfo, IGameInfo* gameInfo, std::pair<int, int> start, std::pair<int, int> dest);

	//the list of obstacle of the previous tanks
	std::queue<std::pair<int,int>> PosOject;

	//check if a tank is near a spring, return NULL if is not 
	ISpring* isNearSpring(ITank* tank);

	//a list of stuck tank
	std::unordered_map<std::string, std::pair<int, int>> stuckTanksList;

	//a list of tank stuck by one obj
	std::unordered_map<std::string, std::list<ITank*>> tanksStuckBy;

	//to check when a recursion is called
	bool enterRecursion;

	//a list of obstacle
	std::list<std::pair<int, int>> obstacleList;
public:
	Group03();
	~Group03();

	void onBindPlayerInfo(IPlayerInfo* playerInfo);
	void onBindGameInfo(IGameInfo* gameInfo);

	IPlayerInfo* getPlayerInfo() const;

	bool onStart();
	bool onFinish();
	Command nextMove();

protected:
	IPlayerInfo* _playerInfo;
	IGameInfo* _gameInfo;

	int _currentTank;
};

