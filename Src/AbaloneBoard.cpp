#include "AbaloneBoard.h"
#include <iostream>
#include <fstream>
#include <math.h>

AbaloneBoard::AbaloneBoard()
{
	currentPlayer = Player1;
	this->BoardCenterX = 0;
	this->BoardCenterZ = 0;
	this->FieldDistance = 2.5;
	this->BallCenterY = 1;

	initBoardFields();
	calcBoardFieldPos();
}

AbaloneBoard::AbaloneBoard(float BoardCenterX, float BoardCenterZ, float FieldDistance, float BallCenterY)
{
	currentPlayer = Player1;
	this->BoardCenterX = BoardCenterX;
	this->BoardCenterZ = BoardCenterZ;
	this->FieldDistance = FieldDistance;
	this->BallCenterY = BallCenterY;

	initBoardFields();
	calcBoardFieldPos();
}

AbaloneBoard::~AbaloneBoard(void)
{
}

Player AbaloneBoard::getCurrentPlayer()
{
	return currentPlayer;
}
Player AbaloneBoard::getOppositeCurrentPlayer()
{
	if (currentPlayer == Player1)
		return  Player2;
	else if(currentPlayer == Player2)
		return  Player1;
}

Player AbaloneBoard::nextPlayer()
{
	if (currentPlayer == Player1)
		currentPlayer = Player2;
	else
		currentPlayer = Player1;
	return currentPlayer;
}

Player AbaloneBoard::getWinner()
{
	if (getNumberLostBalls(Player1) >= BallsToThrowOut)
		return Player2;
	else if (getNumberLostBalls(Player2) >= BallsToThrowOut)
		return Player1;
	else
		return None;
}

void AbaloneBoard::initBoardFields()
{
	for (int x = 0; x < BoardSize; x++) {
		for (int y = 0; y < BoardSize; y++) {
			boardPos[x][y].owner = None;
			boardPos[x][y].id = 0;
			boardPos[x][y].selected = false;
		}
	}

	for (int b = 0; b < MaxBallsOut; b++) {
		lostBalls[b].owner = None;
		lostBalls[b].id = 0;
		lostBalls[b].selected = false;
	}
}

void AbaloneBoard::calcBoardFieldPos()
{
	int tmp, i;
	float RowDistance, z, x;

	RowDistance = sqrt(pow((float) FieldDistance, (int) 2) - pow((float)  FieldDistance / 2, (int) 2));

	for (int v = 0; v < BoardSize; v++) {

		z = BoardCenterZ - (4 - v) * RowDistance;
		tmp = 4 - abs(4 - v);
		x = BoardCenterX - (tmp + 2 - (float) tmp / 2) * FieldDistance;
		i = 0;

		for (int h = 0; h < BoardSize; h++) {

			if (isPosInsideBoard(v, h)) 
			{
				boardPos[v][h].x = x + i * FieldDistance;
				boardPos[v][h].y = BallCenterY;
				boardPos[v][h].z = z;
				i++;
			}
		}

	}
}

bool AbaloneBoard::isPosInsideBoard(int x, int y)
{
	if (x < 0 || x >= BoardSize || y < 0 || y >= BoardSize || (x - y <= -EdgeLength) || (y - x <= -EdgeLength)) 
		return false;
	else 
		return true;
}

void AbaloneBoard::nextPos(int *x, int *y, MoveDirection d)
{
	if ((d == dNorthWest) || (d == dNorthEast)) 
		(*x)--;
	else if ((d == dSouthEast) || (d == dSouthWest)) 
		(*x)++;

	if ((d == dNorthWest) || (d == dWest)) 
		(*y)--;
	else if ((d == dSouthEast) || (d == dEast)) 
		(*y)++;
}

void AbaloneBoard::printBoard()
{
	for (int x = 0; x < BoardSize; x++) {

		for (int t = 0; t < abs(x - EdgeLength + 1); t++) {
				std::cout << ' ';
		}

		for (int y = 0; y < BoardSize; y++) {

			if (isPosInsideBoard(x, y)) 
			{
				std::cout << boardPos[x][y].owner << ' ';
			}

		}

		std::cout << std::endl;
	}
}

bool AbaloneBoard::isMoveAllowed(int x, int y, MoveDirection d)
{
	if (!isPosInsideBoard(x, y)) return false;
	if (boardPos[x][y].owner != currentPlayer) return false;

	nextPos(&x, &y, d);

	if (!isPosInsideBoard(x, y)) return false;
	if (boardPos[x][y].owner == None) return true;
	if (boardPos[x][y].owner != currentPlayer) return false;		// 1 to 1 stalemate

	nextPos(&x, &y, d);
	if (!isPosInsideBoard(x, y)) return false;
	if (boardPos[x][y].owner == None) return true;
	if (boardPos[x][y].owner == currentPlayer) 
	{
		// here we know that the currentPlayer wants to move at least 3 own balls in this direction
		nextPos(&x, &y, d);
		if (!isPosInsideBoard(x, y)) return false;
		if (boardPos[x][y].owner == None) return true;
		if (boardPos[x][y].owner == currentPlayer) return false;

		nextPos(&x, &y, d);
		if (!isPosInsideBoard(x, y)) return true;					// 3 to 1 Sumito with kick out
		if (boardPos[x][y].owner == None) return true;
		if (boardPos[x][y].owner == currentPlayer) return false;

		nextPos(&x, &y, d);
		if (!isPosInsideBoard(x, y)) return true;					// 3 to 2 Sumito with kick out
		if (boardPos[x][y].owner != None) return false;				// 3 to 3 stalemate
	}
	else
	{
		// here we know that the currentPlayer wants to move 2 own balls and is facing at least 1 opposing ball
		nextPos(&x, &y, d);
		if (!isPosInsideBoard(x, y)) return true;					// 2 to 1 Sumito with kick out
		if (boardPos[x][y].owner != None) return false;				// 2 to 2 stalemate
	}
	
	return true;
}

MoveDirection AbaloneBoard::getOppositeDirection(MoveDirection d)
{
	switch(d)
	{
		case dNorthWest:
			return dSouthEast;
		case dNorthEast:
			return dSouthWest;
		case dEast:
			return dWest;
		case dSouthEast:
			return dNorthWest;
		case dSouthWest:
			return dNorthEast;
		default:
			return dEast;
	}
}

void AbaloneBoard::move(int x, int y, MoveDirection d, bool savemove)
{
	int i = 0, preX, preY;
	MoveDirection oppositeDirection;

	if (!isMoveAllowed(x, y, d)) return;

	while (isPosInsideBoard(x, y) && (boardPos[x][y].owner != None)) {
		nextPos(&x, &y, d);
		i++;
	}

	oppositeDirection = getOppositeDirection(d);	

	while (i > 0) {
		i--;
		preX = x;
		preY = y;
		nextPos(&x, &y, oppositeDirection);
		if (!isPosInsideBoard(preX, preY))
			throwBallOut(boardPos[x][y].owner, x, y, d);
		else
			boardPos[preX][preY].owner = boardPos[x][y].owner;	
	}


	// the ball that was moved
	boardPos[x][y].owner = None;

	generateBallIDs();

	if (savemove)
	{
		Move myMove;

		myMove.fromX = x;
		myMove.fromY = y;
		myMove.direction = d;
		//nextPos(&x, &y, d);
		//myMove.toX = x;
		//myMove.toY = y;
		moves.push_back(myMove);
	}

	nextPlayer();
}


void AbaloneBoard::moveSelected(MoveDirection d)
{
	for (int x = 0; x < BoardSize; x++) {

		for (int y = 0; y < BoardSize; y++) {

			if (isPosInsideBoard(x, y) && boardPos[x][y].selected) 
			{
				move(x, y, d);
				return;
			}

		}
	}
}


void AbaloneBoard::throwBallOut(Player victim,int x ,int y, MoveDirection d)
{ 
	for (int b = 0; b < MaxBallsOut; b++) 
	{	
		if (lostBalls[b].owner == None) 
		{
			lostBalls[b].owner = victim;
			lostBalls[b].y = BallCenterY - 0.05;
			if (d == dWest) 
			{
				lostBalls[b].x= boardPos[x][y].x - FieldDistance * 1.3;
				lostBalls[b].z=boardPos[x][y].z;
			}
			else if (d == dEast)
			{
				lostBalls[b].x= boardPos[x][y].x + FieldDistance * 1.3;
				lostBalls[b].z=boardPos[x][y].z;
			}
			else if ((d == dNorthWest) || (d == dNorthEast))
			{
				lostBalls[b].z= boardPos[x][y].z - FieldDistance *0.87;				
				if (d == dNorthWest)
					lostBalls[b].x= boardPos[x][y].x - FieldDistance *0.87;
				else
					lostBalls[b].x= boardPos[x][y].x + FieldDistance *0.87 ;
			}
			else if ((d == dSouthEast) || (d == dSouthWest)) 
			{	
				lostBalls[b].z= boardPos[x][y].z + FieldDistance*0.87;
				if (d == dSouthEast)
					lostBalls[b].x= boardPos[x][y].x + FieldDistance*0.87;
				else
					lostBalls[b].x= boardPos[x][y].x - FieldDistance*0.87;
			}
			if (lostBalls[b].z > 1.2)
			{
				lostBalls[b].z = 1.5;
				if (d == dSouthWest)
					lostBalls[b].x = lostBalls[b].x - 0.03;
				else
					lostBalls[b].x = lostBalls[b].x + 0.03;
			}
			if (lostBalls[b].z <-1.2)
			{
				lostBalls[b].z =-1.5;
				if (d == dNorthWest)
					lostBalls[b].x = lostBalls[b].x - 0.03;
				else
					lostBalls[b].x = lostBalls[b].x + 0.03;
			}

			for (int a=0; a < b; a++) 
			{	
				//std::cout <<  " bx  " << lostBalls[b].x << "  ax  " <<lostBalls[a].x << "  bz  " << lostBalls[b].z  << "  az  " << lostBalls[a].z << std::endl;
				if (((lostBalls[b].x >= lostBalls[a].x - 0.1) && (lostBalls[b].x <= lostBalls[a].x + 0.1)) && ((lostBalls[b].z >= lostBalls[a].z - 0.1) && (lostBalls[b].z <= lostBalls[a].z + 0.1)))
				{
					if (d == dWest && lostBalls[b].z <= 0)
					{
						lostBalls[b].z = lostBalls[b].z - FieldDistance*0.87;
						lostBalls[b].x = lostBalls[b].x + FieldDistance/2;
					}
					else if ((d == dWest) && (lostBalls[b].z > 0))
					{
						lostBalls[b].z = lostBalls[b].z + FieldDistance*0.87;
						lostBalls[b].x = lostBalls[b].x + FieldDistance/2;
					}
					else if ((d == dEast) && (lostBalls[b].z <= 0))
					{
						lostBalls[b].z = lostBalls[b].z - FieldDistance*0.87;
						lostBalls[b].x = lostBalls[b].x - FieldDistance/2;
					}
					else if ((d == dWest) && (lostBalls[b].z > 0))
					{
						lostBalls[b].z = lostBalls[b].z + FieldDistance*0.87;
						lostBalls[b].x = lostBalls[b].x - FieldDistance/2;
					}
					else if ((d == dNorthWest) && (lostBalls[b].z > -1.2))
					{
						lostBalls[b].z = lostBalls[b].z - FieldDistance*0.87;
						lostBalls[b].x = lostBalls[b].x + FieldDistance/2;
					}
					else if ((d == dNorthWest) && (lostBalls[b].z <= -1.2))
					{
						lostBalls[b].x = lostBalls[b].x + FieldDistance* 0.95;
					}
					else if ((d == dNorthEast) && (lostBalls[b].z > -1.2))
					{
						lostBalls[b].z = lostBalls[b].z - FieldDistance*0.87;
						lostBalls[b].x = lostBalls[b].x - FieldDistance/2;
					}
					else if ((d == dNorthEast) && (lostBalls[b].z <= -1.2))
					{
						lostBalls[b].x = lostBalls[b].x - FieldDistance* 0.95;
					}
					else if ((d == dSouthWest) && (lostBalls[b].z < 1.2))
					{
						lostBalls[b].z = lostBalls[b].z - FieldDistance*0.87;
						lostBalls[b].x = lostBalls[b].x - FieldDistance/2;
					}
					else if ((d == dSouthWest) && (lostBalls[b].z >= 1.2))
					{
						lostBalls[b].x = lostBalls[b].x + FieldDistance* 0.95;
					}
					else if ((d == dSouthEast) && (lostBalls[b].z < 1.2))
					{
						lostBalls[b].z = lostBalls[b].z - FieldDistance*0.87;
						lostBalls[b].x = lostBalls[b].x + FieldDistance/2;
					}
					else if ((d == dSouthEast) && (lostBalls[b].z >= 1.2))
					{
						lostBalls[b].x = lostBalls[b].x - FieldDistance * 0.95;
					}
				}
			}
			break;
		}
	
	}
}


int AbaloneBoard::getNumberLostBalls(Player p)
{
	int count = 0;
	for (int b = 0; b < MaxBallsOut; b++) {
		
		if (lostBalls[b].owner == p) {
			count++;
		}
	}

	return count;
}


void AbaloneBoard::generateBallIDs()
{
	int aID = 0;
	for (int x = 0; x < BoardSize; x++) {

		for (int y = 0; y < BoardSize; y++) {

			if (isPosInsideBoard(x, y) && boardPos[x][y].owner != None) 
			{
				aID++;
				boardPos[x][y].id = aID;
			}
			else
			{
				boardPos[x][y].id = 0;
			}

		}
	}
}


bool AbaloneBoard::loadSetup(std::string filename)
{
	std::string line;
	std::ifstream myfile(filename.c_str());

	// reset board
	moves.clear();
	initBoardFields();

	if (myfile.is_open())
	{
		int x = -1, y;
		while (!myfile.eof())
		{
			y = 0;
			std::getline(myfile,line);

			// todo isPosInsideBoard()
			for (unsigned int i = 0; i < line.size(); i++)
			{
				
				if ((i == 0) && (line.at(i) == '#')) {
					x--;
					break;
				}
				else if ((x == -1) && (i == 0))  {
					if (line.at(i) == '2')
						currentPlayer = Player2;
					else
						currentPlayer = Player1;
					break;
				}
				else if (line.at(i) == '0') {
					y++;
				}
				else if (line.at(i) == '1') {
					if (x > 4)
					{
						boardPos[x][y+x-4].owner = Player1;
					}
					else
					{
						boardPos[x][y].owner = Player1;
					}
					y++;
				}
				else if (line.at(i) == '2') {
					
					if (x > 4)
					{
						boardPos[x][y+x-4].owner = Player2;
					}
					else
					{
						boardPos[x][y].owner = Player2;
					}
					y++;
				}
			}
			x++;
		}

	/*
		while (inFile >> p) {
			sum = sum + p;
		}

	*/

		myfile.close();
		generateBallIDs();
		return true;
	}
	else {
		return false;
	}
}


bool AbaloneBoard::saveSetup(std::string filename)
{
	std::string line;
	std::ofstream myfile(filename.c_str());
	if (myfile.is_open())
	{
		myfile << currentPlayer << std::endl;

		for (int x = 0; x < BoardSize; x++) {

			for (int t = 0; t < abs(x - EdgeLength + 1); t++) {
				myfile << ' ';
			}

			for (int y = 0; y < BoardSize; y++) {

				if (isPosInsideBoard(x, y)) 
				{
					myfile << boardPos[x][y].owner << ' ';
				}

			}

			myfile << std::endl;
		}

		myfile.close();
		return true;
	}
	else {
		return false;
	}
}



bool AbaloneBoard::saveGame(std::string filename, std::string gamevariant)
{
	std::string line;
	std::ofstream myfile(filename.c_str());
	if (myfile.is_open())
	{
		myfile << gamevariant;
		for (int i = 0; i < moves.size(); i++) {
			//myfile << std::endl << moves[i].fromX << moves[i].fromY << ',' << moves[i].toX << moves[i].toY;
			myfile << std::endl << moves[i].fromX << moves[i].fromY << moves[i].direction;
		}

		myfile.close();
		return true;
	}
	else {
		return false;
	}
}


MoveDirection AbaloneBoard::getDirection(int fromX, int fromY, int toX, int toY)
{
	// not working properly
	if (fromY - toY > 0) 
	{
		if (fromX - toX > 0)
			return dNorthWest;
		else if (fromX - toX == 0)
			return dWest;
		else
			return dNorthEast;
	}	
	else
	{
		if (fromX - toX > 0)
			return dSouthWest;
		else if (fromX - toX == 0)
			return dEast;
		else
			return dSouthEast;
	}
}