//GROUP03
//Dinh Dat Thanh
//Tran Van Duy Tue
//Nguyen Tang Bao An
//Mai Thieu Nhan

//changelog:
//added:
//+ stuckTanksList: an unordered map to save the which obstacle stops each tank (1)
//+ tanksStuckBy: a list of tanks stuck by an obstacle	(2)
//+ change to Block: instead of push into PosObject, now assign the obstacle to each tank. see in Block()
//+ choose nearest tank to run: change from manhattan to counting steps from astar
//+ change the method of finding nearest tank to run: by just using the alives list and (1), (2). headline: "//find the closest tank to the HQ to run"
//+ change the check for blocking method: using (1) and (2). headline: "//IF THERE IS A BLOCK"
//+ added recursion when tanks are block <-- add a variable to check when recursion is called
//+ if the tank is block by a friendly tank which is not block --> recalculate the astar path
//+ if all the tank is block -> recalculate the astar path
//+ if the tank is blocked by a friendly tank which is in FinishHQ -> add it to FinishHQ too
//+ if HQ is in enemy range -> fire immediately
//TODO:
//CHANGE NEAREST ALGO -> CHECK
//HQ IN RANGE TANK -> CHECK
//CHOOSE TANK TO MOVE -> CHECK
//ALL BLOCK AND AT HQ -> CHECK
//BLOCK BY AN ENEMY TANK BUT NO AMMO -> CHECK
//TODO:
//IN RANGE RETURN TANK:check
//CHECK BLOCK:check
//CHON TANK DE DI:check
//XE TANK SAU BAN BLOCK DE XE TANK TRUOC CHAY TIEP: DA BAN DUOC, XE TANK TRC VAN KHONG CHAY: check
//KIEM SOAT NEU TRONG LUOT XE CHET: check (map9 bug)
//TODO: CHECK THE SKIP MOVE -> CHECK
//TODO
//IF A TANK IS DEAD: FinishHQ
//TODO:
//CHECK IF ANY TANK IS UNDER ATTACK BUT NOT AT HQ -> RUN -> CHECK
//CHECK IF HQ IS UNDER ATTACK BUT NO TANK IN HQ CAN DEFEND -> SACRIFICE ONE TANK

#include "Group03.h"
#include <list>
#include <cstdlib>

using namespace std;

Group03::~Group03() {}

void Group03::onBindPlayerInfo(IPlayerInfo* playerInfo) {
	_playerInfo = playerInfo;
}
void Group03::onBindGameInfo(IGameInfo* gameInfo) {
	_gameInfo = gameInfo;
}

Group03::Group03()
	: _playerInfo(NULL), _gameInfo(NULL) {
		chosenOne=NULL;
		haveShortestPathList=false;
		myNemesis=NULL;
		Attacked=false;
		CurrentHQ=0;
		IniHQ=0;
		enterRecursion = false;
}
IPlayerInfo* Group03::getPlayerInfo() const {
	return _playerInfo;
}

bool Group03::onStart() {
	return true;
}

bool Group03::onFinish(){
	return true;
}

//FINAL PROJECT ---- GROUP 03
//FIX CODE BELOW THIS LINE
//STILL UNDER DEVELOPMENT
Command Group03::nextMove() {
	//GET THE NEMESIS
	if(!myNemesis)
	{
		std::vector<IPlayerInfo*> Info=_gameInfo->getPlayersInfo();
		if(Info[1]!=_playerInfo)
			myNemesis=Info[1];
		else myNemesis=Info[0];
	}

	//TO CHECK IF ANY TANK IS UNDER ATTACK
	ITank* attackedTank = NULL;

	//TO CHECK IF HQ IS UNDER ATTACK
	Attacked = false;

	list<ITank*> dead = _playerInfo->getDeadTanks();
	list<ITank*> alives = _playerInfo->getAliveTanks();
	int numOfTank = alives.size();
	pair<int,int> HQenemy=myNemesis->getHeadquarterPosition();
	IMap* gameMap = _gameInfo->getMap();


	if (!enterRecursion) {
		//CHECK IF ANY TANK IN THE FINISH HQ IS DEAD
		if (!FinishHQ.empty()) {
			list<ITank*>::iterator deadTank;
			for (deadTank = FinishHQ.begin(); deadTank != FinishHQ.end(); deadTank++)
				if (!(*deadTank)->isAlive()) {
					deadTank = FinishHQ.erase(deadTank);
					if (deadTank == FinishHQ.end())
						break;
				}
		}
		CommandInfo myLastMove = _playerInfo->getLastMove();
		CommandInfo enemyLastMove = myNemesis->getLastMove();

		//CHECK IF THE LAST MOVE IS SKIP BY GAME
		//if the last move is MOVE
		if (myLastMove.originalCommand.getActionType() == Command::MOVE) {
			//get the tank of last move
			ITank* lastTank = myLastMove.originalCommand.getReceivingObject();
			//if the tank is alive
			if (lastTank->isAlive()) {
				//and MOVE is not changed to SKIP
				if (myLastMove.executedCommand.getActionType() != Command::SKIP) {
					//pop out the last move
					shortestPathList[toStringTank(lastTank)].pop();
				}
				else
				{
					//get the position of the skipped block
					pair<int, int> firePos = myLastMove.originalCommand.getTargetPosition();
					//and fire at it
					return Command(lastTank, Command::FIRE, firePos.first, firePos.second);
				}
			}
		}

		//CHECK IF ANY TANK IS UNDER ATTACK BUT NOT AT HQ 
		if (enemyLastMove.executedCommand.getActionType() == Command::FIRE) {
			pair<int, int> attackPos = enemyLastMove.executedCommand.getTargetPosition();
			if (gameMap->isTank(attackPos.first, attackPos.second, _playerInfo->getPlayerMapID())) {
				attackedTank = _gameInfo->getTank(attackPos.first, attackPos.second);
				if (attackedTank->isAlive()) {
					//CHECK IF THAT TANK IS AT HQ
					list<ITank*>::iterator attackedTankHQ;
					for (attackedTankHQ = FinishHQ.begin(); attackedTankHQ != FinishHQ.end(); attackedTank++) {
						if ((*attackedTankHQ) == attackedTank)
							break;
					}
					//IF IT IS, THEN DO NOTHING
					if (attackedTankHQ != FinishHQ.end())
						attackedTank = NULL;
				}
				else
					attackedTank = NULL;
			}
		}

		//take the HP of our HQ
		if(IniHQ==0)
		{
			CurrentHQ= _playerInfo->getHeadquarter()->getHP();
			IniHQ=1;
		}
		if(CurrentHQ > _playerInfo->getHeadquarter()->getHP())
		{
			CurrentHQ = _playerInfo->getHeadquarter()->getHP();
			Attacked=true;
		}
		else
			Attacked=false;

		//8/17/16
		//A LIST OF OBSTACLE TO CHECK IF NEXT TURN THE OBSTACLE IS STILL THERE
		list<pair<int, int>>::iterator obstacleIt;
		list<ITank*>::iterator stuckListIt;

		//SURF THROUGH THE OBSTACLE LIST
		for (obstacleIt = obstacleList.begin(); obstacleIt != obstacleList.end(); obstacleIt++) {
			//CHECK IF THE POSITION IS AN EMPTY SPACE OR A BRIDGE
			if (gameMap->isEmptySpace((*obstacleIt).first, (*obstacleIt).second) || gameMap->isBridge((*obstacleIt).first, (*obstacleIt).second)) {
				//GET THE LIST OF TANKS STUCK BY THAT POS
				list<ITank*> stuckList = tanksStuckBy[toString((*obstacleIt))];
				//REMOVE THEIR BLOCKING STATE
				for (stuckListIt = stuckList.begin(); stuckListIt != stuckList.end(); stuckListIt++)
					stuckTanksList.erase(toStringTank(*stuckListIt));
				//REMOVE THE LIST
				tanksStuckBy.erase(toString((*obstacleIt)));
				//REMOVE THE OBSTACLE FROM THE MONITORING LIST
				obstacleIt = obstacleList.erase(obstacleIt);
				if (obstacleIt == obstacleList.end())
					break;
			}
		}
	//
	}
	enterRecursion = false;
	
	if (numOfTank > 0) {
		list<ITank*>::iterator it = alives.begin();
		pair<int, int> HQPos = _playerInfo->getHeadquarterPosition();

		if (!haveShortestPathList) //if the list of shortestPath is not made yet
		{
			for (it ; it != alives.end(); it++) {
				shortestPathList[toStringTank((*it))] = aStarAlgo(_playerInfo, _gameInfo, (*it)->getPosition(), HQPos); //get the shortestPath queue for a tank
			}
			haveShortestPathList=true;
		}

		//check any tank at the HQ can fire any local enemy
		if(!FinishHQ.empty())
		{
			list<ITank*>::iterator Soldier = FinishHQ.begin();
			list<ITank*>::iterator Nemesis;
			for (Soldier;Soldier != FinishHQ.end();Soldier++)
			{
				if(!findTankInRange(*Soldier).empty()) {
					//FIRE IMMEDIATELY IF HQ IS IN ENEMY RANGE OR THE TANK IS IN ENEMY RANGE
					list<ITank*> nemesisList = findTankInRange(*Soldier);
					for (Nemesis = nemesisList.begin(); Nemesis != nemesisList.end(); Nemesis++)
						if (manhattanDist((*Nemesis)->getPosition(), HQPos) <= (*Nemesis)->getRange() || manhattanDist((*Nemesis)->getPosition(), (*Soldier)->getPosition()) <= (*Nemesis)->getRange())
							break;
					if (Nemesis != nemesisList.end() && Soldier!=FinishHQ.end() && (*Soldier)->isAlive() && (*Soldier)->hasAmmo())
					{
						Command::Action actChoice=Command::FIRE;
						ITank* opposite = (*findTankInRange(*Soldier).begin());
						if(!isNearSpring(opposite))
						{
							pair<int,int> Bombpos=opposite->getPosition();
							return Command(*Soldier, actChoice,Bombpos.first, Bombpos.second);
						}
					}
				}
			}
			//NO TANK CAN FIRE AT ENEMY BUT THE HQ IS STILL UNDER ATTACKED
			/*if (Soldier == FinishHQ.end() && Attacked) {
				ITank* attackEnemy = myNemesis->getLastMove().executedCommand.getReceivingObject();
			}*/
		}
		ITank* nextTank;
		int tempPathLength;
		int smallestPath = INT_MAX;
		list<ITank*>::iterator atHQ;
		ITank* tankToRun = NULL;

		//if no tank is being attacked and need to run
		if (attackedTank == NULL) {
			//find the closest tank to the HQ to run
			for (it = alives.begin(); it != alives.end(); it++) {
				//check if the tank is block
				if (stuckTanksList.find(toStringTank((*it))) != stuckTanksList.end())
					continue;
				//check if it is at HQ
				if (!FinishHQ.empty()) {
					for (atHQ = FinishHQ.begin(); atHQ != FinishHQ.end(); atHQ++) {
						if ((*atHQ) == (*it)) {
							break;
						}
					}
					if (atHQ != FinishHQ.end() && (*atHQ) == (*it))
						continue;
				}
				tempPathLength = shortestPathList[toStringTank((*it))].size();
				//if the path length is smaller than the previous one
				if (tempPathLength < smallestPath) {
					tankToRun = (*it);
					smallestPath = tempPathLength;
				}
				//if they are equal
				else if (tempPathLength == smallestPath) {
					//check if the newer has more HP
					if (tankToRun->getHP() < (*it)->getHP()) {
						tankToRun = (*it);
					}
					//if the same HP
					else if (tankToRun->getHP() == (*it)->getHP()) {
						//check if the newer has more Ammo
						if (tankToRun->getAmmoNumber() < (*it)->getAmmoNumber())
							tankToRun = (*it);
					}
				}
			}
			//IF ALL TANKS ARE STUCK AND AT HQ
			if (tankToRun == NULL) {
				return Command(NULL, Command::SKIP, 0, 0);
			}

			nextTank = tankToRun;
			chosenOne = tankToRun;
		}
		//IF THERE IS A TANK BEING ATTACKED
		else
		{
			nextTank = attackedTank;
			chosenOne = attackedTank;
		}

		//GET THE POSITION FOR THE EXECUTED TANK
		pair<int, int> tankPos = nextTank->getPosition();
		Command::Action actChoice = Command::MOVE;

		//if find the HQ of enemy nearby--> only fire incase the ammo left is greater than the HP of the HQ of enemy
		if(manhattanDist(tankPos,HQenemy) <= nextTank->getRange())
		{
			if(nextTank->getAmmoNumber() >= myNemesis->getHeadquarter()->getHP() &&
				(!findTankInRange(nextTank).empty() && nextTank->getHP() >= myNemesis->getHeadquarter()->getHP() ||
				findTankInRange(nextTank).empty()))
			{
				actChoice=Command::FIRE;
				return Command(nextTank, actChoice,HQenemy.first, HQenemy.second);
			}
		}

		//SEE A TANK --> FIRE
		if (Attacked == true)
		{
			if(!findTankInRange(nextTank).empty()&&nextTank->hasAmmo())
			{
				actChoice = Command::FIRE;
				//CUA TAO
				std::list<ITank*> enemyTankList = findTankInRange(nextTank);
				std::list<ITank*>::iterator suitableTank;
				for (suitableTank = enemyTankList.begin(); suitableTank != enemyTankList.end(); suitableTank++) {
					//if the tank is the one firing our HQ
					if((*suitableTank) == myNemesis->getLastMove().executedCommand.getReceivingObject())
						break;
				}
				if (suitableTank != enemyTankList.end()) {
					ITank * opposite= (*suitableTank);
					//CHECK IF THAT TANK IS NEAR A SPRING
					ISpring* nearSpring = isNearSpring(opposite);
					pair<int,int> Bombpos;

					//IF IT IS AND IN RANGE
					if (nearSpring && manhattanDist(nearSpring->getPosition(), nextTank->getPosition()) <= nextTank->getRange())
					{
						Bombpos = nearSpring->getPosition(); //THE BOMBPOS IS THAT SPRING
						return Command(nextTank, actChoice,Bombpos.first, Bombpos.second);
					}
					//
					//ITank * opposite=*(findTankInRange(nextTank).begin());
					//IF IT IS NOT
					else if (!nearSpring)
					{
						Bombpos = opposite->getPosition();
						return Command(nextTank, actChoice,Bombpos.first, Bombpos.second);
					}
				}
			}
		}

		//ACCESS THE NEXT STEP IN THE QUEUE
		actChoice = Command::MOVE;
		queue<pair<int, int>> shortestPath;
		shortestPath = shortestPathList[toStringTank(nextTank)];  //GET THE SHORTEST PATH QUEUE FOR THE EXECUTED TANK
		if (shortestPath.empty())
			return Command(nextTank, Command::SKIP, 0, 0);
		pair<int, int> moveNext = shortestPath.front();
		int moveLocX = tankPos.first + moveNext.first;
		int moveLocY = tankPos.second + moveNext.second;
		//IF THERE IS A BLOCK
		if(!(gameMap->isEmptySpace(moveLocX,moveLocY) || gameMap->isBridge(moveLocX,moveLocY)))
		{
			//IF THE TANK IS STUCK
			if(Block(nextTank,moveNext))
			{
				//FIND IF THERE IS ANY "DANGEROUS" OBSTACLE BLOCKING THE TANK
				if(stuckTanksList.find(toStringTank(nextTank)) != stuckTanksList.end())
				{
					//GET THE POS OF THAT OBSTACLE
					pair<int,int> Kill = stuckTanksList[toStringTank(nextTank)];
					//IF THE TANK HAS THE ABILITY TO KILL THAT OBSTACLE
					if(manhattanDist(tankPos,Kill) <= nextTank->getRange() && nextTank->hasAmmo())
					{
						actChoice=Command::FIRE;
						//because that obstacle is going to be killed -> remove all the tank-obstacle assignment

						//get the list of tank stuck by that obstacle
						list<ITank*> listOfStuckTank = tanksStuckBy[toString(Kill)];

						//surf through that list and remove the obstacle->tank assignment
						list<ITank*>::iterator stuckTank;
						for (stuckTank = listOfStuckTank.begin(); stuckTank != listOfStuckTank.end(); stuckTank++)
							stuckTanksList.erase(toStringTank((*stuckTank)));

						//and then remove the list
						tanksStuckBy.erase(toString(Kill));

						//erase the obstacle from the list of obstacles need monitoring
						list<pair<int, int>>::iterator emptyBlock;
						for (emptyBlock = obstacleList.begin(); emptyBlock != obstacleList.end(); emptyBlock++) {
							if ((*emptyBlock) == Kill) {
								obstacleList.erase(emptyBlock);
								break;
							}
						}

						//THEN FIRE
						return Command(nextTank, actChoice,Kill.first, Kill.second);
					}
				}
				else {
					//if the tank is block but by no dangerous object => block by a friendly tank which is not block by another obstacle
					//RECALCULATE THE ASTAR ALGO --> NEW
					shortestPathList[toStringTank(nextTank)] = aStarAlgo(_playerInfo, _gameInfo, nextTank->getPosition(), HQPos);
					//IF THE PATH STILL LEAD TO THE FRIENDLY TANK -> CHECK IF FRIENDLY TANK IS AT HQ
					if (!shortestPathList[toStringTank(nextTank)].empty()) {
						pair<int, int> tempNextMove = shortestPathList[toStringTank(nextTank)].front();
						if (tempNextMove == moveNext)
							if (gameMap->isTank(moveLocX, moveLocY, _playerInfo->getPlayerMapID())) {
								ITank* friendlyTank = _gameInfo->getTank(moveLocX, moveLocY);
								//check if it is at HQ
								if (!FinishHQ.empty())
									for (atHQ = FinishHQ.begin(); atHQ != FinishHQ.end(); atHQ++)
										if ((*atHQ) == friendlyTank) {
											FinishHQ.push_back(nextTank);
											shortestPathList.erase(toStringTank(nextTank));
										}
							}
					}
				}
				enterRecursion = true;
				return this->nextMove();
			}
			else if(moveLocX == HQPos.first && moveLocY == HQPos.second)
			//THE TANK ENTERS PROTECTING MODE
			{
				FinishHQ.push_front(nextTank);
				return this->nextMove();
			}
			else
			{
				actChoice=Command::FIRE;
				return Command(nextTank, actChoice, moveLocX, moveLocY);
			}
		}
		else
			return Command(nextTank, actChoice, moveLocX, moveLocY);
	}
	return Command();
}

std::queue<std::pair<int, int>> Group03::aStarAlgo(IPlayerInfo* playerInfo, IGameInfo* gameInfo, pair<int, int> start, pair<int, int> dest) {
	std::queue<std::pair<int, int>> shortestPath;
	IMap* gameMap = gameInfo->getMap();	//get the IMap for later reference
	unordered_map<std::string, pair<int, int>> cameFrom;	//save the parent node of the Current node
	unordered_map<std::string, int> costOf;	//the cost of the current Node
	unordered_map<std::string, int>::iterator it;	//the iterator to search for key in the costOf map
	std::priority_queue<PriorityQueueEle, std::vector<PriorityQueueEle>, Compare> openSet;	//the set of node that still need to be evaluated

	openSet.push(PriorityQueueEle(start, 0));	//only Start node is known
	cameFrom[toString(start)] = std::make_pair(-1, -1);	//the start node come from nothing
	costOf[toString(start)] = 0;	//because start node

	pair<int, int> current;
	pair<int, int> next;
	int locNeighborX;
	int locNeighborY;
	int tempCost;
	int newCost;
	while (!openSet.empty())	//run the algo until the open set is empty or the dest is met
	{
		current = openSet.top().loc;	//access the highest priority
		openSet.pop();	//and delete it

		if (current == dest)	//if the dest is met
		{
			reconstructPath(shortestPath, cameFrom, start, dest);	//reconstuct the path
			return shortestPath;	//and end the algo
		}

		//check with 4 directions: UP, DOWN, RIGHT, LEFT
		pair<int,int> delta[4] = {
			pair<int,int>(0,-1), // north
			pair<int,int>(0,1), // south
			pair<int,int>(1,0), // east
			pair<int,int>(-1,0) // west
		};

		for (int deltaChoice = 0; deltaChoice < 4; deltaChoice++) {
			locNeighborX = current.first + delta[deltaChoice].first;
			locNeighborY = current.second + delta[deltaChoice].second;

			//if the neighbor location is out of bound
			if (locNeighborX < 0 || locNeighborX > gameMap->getWidth() - 1 || locNeighborY < 0 || locNeighborY > gameMap->getHeight() - 1)
				continue;

			//if the neighbor is an empty space or a bridge or our HQ -> cost = 1
			if (gameMap->isEmptySpace(locNeighborX, locNeighborY) ||
				gameMap->isBridge(locNeighborX, locNeighborY) ||
				gameMap->isHeadquarter(locNeighborX, locNeighborY,_playerInfo->getPlayerMapID()))
				tempCost = 1;
			//if it is HQ of the enemy
			else if(gameMap->isHeadquarter(locNeighborX, locNeighborY, myNemesis->getPlayerMapID()))
			{
				tempCost = 1 + myNemesis->getHeadquarter()->getHP();
			}
			else if (gameMap->isBlock(locNeighborX, locNeighborY))
				tempCost = 1 + gameInfo->getBlock(locNeighborX, locNeighborY)->getHP();
			else if (gameMap->isSpring(locNeighborX, locNeighborY))
				tempCost = 1 + gameInfo->getSpring(locNeighborX, locNeighborY)->getHP();
			else if (gameMap->isTank(locNeighborX, locNeighborY))
				tempCost = 1 + gameInfo->getTank(locNeighborX, locNeighborY)->getHP();
			//if it is water or else who knows
			else
				tempCost = INT_MAX;

			if (tempCost == INT_MAX)	//if WATER then skip the move
				continue;
			else
				newCost = costOf[toString(current)] + tempCost;	//calculate the temporary cost the the neighbor node

			//this tries to find if the neighbor node has been discovered or not
			next = std::make_pair(locNeighborX, locNeighborY);
			it = costOf.find(toString(next));

			if (it == costOf.end() || newCost < it->second) //if it has not been discovered or the new cost is lower than its current cost
			{
				costOf[toString(next)] = newCost;
				cameFrom[toString(next)] = current;
				openSet.push(PriorityQueueEle(next, newCost + manhattanDist(next, dest)));
			}
		}
	}
	return shortestPath;
}

int Group03::manhattanDist(pair<int, int> node, pair<int, int> dest) {
	return abs(node.first - dest.first) + abs(node.second - dest.second);
}

void Group03::reconstructPath(std::queue<std::pair<int, int>> &path, std::unordered_map<std::string, std::pair<int, int>> cameFrom, std::pair<int, int> start, std::pair<int, int> dest) {
	pair<int, int> previous = cameFrom[toString(dest)];
	if (previous == std::make_pair(-1, -1)) {
		path.push(std::make_pair(0 , 0));
		return;
	}
	if (previous != start)
		reconstructPath(path, cameFrom, start, previous);
	path.push(std::make_pair(dest.first - previous.first, dest.second - previous.second));
}

std::string Group03::toString(pair<int, int> p) {
	return std::to_string(static_cast<long long>(p.first)) + "|" + std::to_string(static_cast<long long>(p.second));
}

//8/13/16
std::list<ITank*> Group03::findTankInRange(ITank* myTank) {
	std::list<ITank*> tankList;
	if (!myNemesis)
		return tankList;
	list<ITank*> enemyTank = myNemesis->getAliveTanks();
	list<ITank*>::iterator it;
	for (it = enemyTank.begin(); it != enemyTank.end(); it++)
		if (manhattanDist(myTank->getPosition(), (*it)->getPosition()) <= myTank->getRange())
			tankList.push_front((*it));
	return tankList;
}

//DEFINE WHEN A TANK IS STUCK
bool Group03::Block(ITank* nextTank,std::pair<int,int> NextMove)
{
	pair<int,int> TankPos=nextTank->getPosition();
	IMap* gameMap = _gameInfo->getMap();
	int movelocX=TankPos.first + NextMove.first;
	int movelocY=TankPos.second + NextMove.second;
	pair<int,int> movePos = std::make_pair(movelocX, movelocY);

	//IF THE NEXT MOVE IS A
	if(gameMap->isBlock(movelocX,movelocY) ||	//BLOCK
		gameMap->isSpring(movelocX,movelocY) ||	//SPRING
		gameMap->isTank(movelocX,movelocY,myNemesis->getPlayerMapID()) || //ENEMY TANK
		gameMap->isHeadquarter(movelocX, movelocY, myNemesis->getPlayerMapID()))	//ENEMY HEADQUARTER
	{
		if(!nextTank->hasAmmo())	//IF CURRENT TANK HAS NO AMMO
		{
			//assign the position of that obstacle to the tank
			stuckTanksList[toStringTank(nextTank)] = movePos;
			//push that tank to the stuck lits of that obstacle
			tanksStuckBy[toString(movePos)].push_front(nextTank);
			//push the obstacle to the list of obstacle need to be monitored
			obstacleList.push_back(std::make_pair(movelocX, movelocY));
			return true;
		}
	}
	//OR
	else if(gameMap->isTank(movelocX,movelocY,_playerInfo->getPlayerMapID())) //OUR TANKKKK
	{
		//get the friendly tank
		ITank* friendlyTank = _gameInfo->getTank(movelocX, movelocY);
		//find if the friendly tank is also stuck by an obstacle
		if (stuckTanksList.find(toStringTank(friendlyTank)) != stuckTanksList.end()) {
			//get that obstacle
			pair<int, int> obstacle = stuckTanksList[toStringTank(friendlyTank)];
			//assign that obstacle to the current tank
			stuckTanksList[toStringTank(nextTank)] = obstacle;
			//push the current tank to the list of tank stuck by that obstacle
			tanksStuckBy[toString(obstacle)].push_front(nextTank);
		}
		return true;
	}
	else if(gameMap->isWater(movelocX, movelocY))
		return true;

	return false;
}

std::string Group03::toStringTank(ITank* tank) {
	const void * address = static_cast<const void*>(tank);
	std::stringstream ss;
	ss << address;
	std::string stringRep = ss.str();
	return stringRep;
}

ISpring* Group03::isNearSpring(ITank* tank) {
	IMap* gameMap = _gameInfo->getMap();
	std::pair<int, int> tankPos = tank->getPosition();
	int locX;
	int locY;
	std::pair<int,int> delta[4] = {
		std::pair<int,int>(0,-1), // north
		std::pair<int,int>(0,1), // south
		std::pair<int,int>(1,0), // east
		std::pair<int,int>(-1,0) // west
	};

	for (int i = 0; i < 4; i++) {
		locX = tankPos.first + delta[i].first;
		locY = tankPos.second + delta[i].second;
		if (gameMap->isSpring(locX, locY))
			return _gameInfo->getSpring(locX, locY);
	}
	return NULL;
}

//FIX CODE ABOVE THIS LINE
