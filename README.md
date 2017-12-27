# Algorithm-for-tank-game
Using A-star shortest path algorithm

Source: https://github.com/ThanhDinhDat/tankgame-tn14

Problem: provide an algorithm for nextmove() in IPlayer class for each tank to automatically move and destroy the enemy's HQ. It is possible to use all the functions in IGameInfo, IPlayerInfo, IMap, IBlock, IBridge, ITank, IHeadquarter, ISpring classes.

Rules: 
       - Tanks of 2 teams cannot go to the same block at the same time.
       
       - If a tank of team A goes to the block shot by tank of team B, it will loose one HP.
       
       - The game finishes after T moves or a HQ is destroyed.
       
How to win: 
            - If HQ of 2 teams is destroyed simultaneously, the result is draw.
            
            - The team with the HQ left is the winner.
            
            - If both HQs survive after T moves then the winner is decided by following rules:
            
            1) The team with total HP of all tanks greater is the winner.
            
            2) Total Manhattan distance of all tanks less is the winner.

Solution: using A-star shortest path algorithm to find the path for all tanks to go back and protect HQ. If in case the enemy's HQ in our range on the way back to our HQ and number of bullets of the tank is greater than HP of the enemy's HQ, our tank will fire the enemy's HQ.

Team: 4 members.

Position: team leader.
