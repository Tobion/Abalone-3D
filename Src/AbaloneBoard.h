#ifndef ABALONEBOARDH
#define ABALONEBOARDH

#include <string>
#include <vector>


#define Players 2
#define BallsToThrowOut 6
#define MaxBallsOut Players*BallsToThrowOut
#define EdgeLength 5
#define BoardSize EdgeLength*2-1


enum MoveDirection { dNorthWest, dNorthEast, dEast, dSouthEast, dSouthWest, dWest };

enum Color { cWhite, cBlack };

enum AlphaIndex { A, B, C, D, E, F, G, H, I };

enum Player { None, Player1, Player2 };

struct Ball {
	float x;          
	float y;
	float z;
	int id;
	bool selected;
	Player owner;      
};

struct Move {
	int fromX;          
	int fromY;
	MoveDirection direction;
	//int toX;
	//int toY;     
};

class AbaloneBoard
{
private:
	float BoardCenterX;
	float BoardCenterZ;
	float FieldDistance;
	float BallCenterY;

	Player currentPlayer;
	
	void calcBoardFieldPos();
	void nextPos(int *x, int *y, MoveDirection d);
	MoveDirection getOppositeDirection(MoveDirection d);

	void throwBallOut(Player victim, int x , int y, MoveDirection d);
	void generateBallIDs();
public:
	AbaloneBoard();
	AbaloneBoard(float BoardCenterX, float BoardCenterZ, float FieldDistance, float BallCenterY);
	~AbaloneBoard(void);

	Ball boardPos[BoardSize][BoardSize];
	Ball lostBalls[MaxBallsOut];

	std::vector<Move> moves;

	Player getCurrentPlayer();
	Player getOppositeCurrentPlayer();
	Player nextPlayer();
	Player getWinner();

	MoveDirection getDirection(int fromX, int fromY, int toX, int toY);

	void initBoardFields();
	void printBoard();
	bool isPosInsideBoard(int x, int y);
	bool isMoveAllowed(int x, int y, MoveDirection d);
	void move(int x, int y, MoveDirection d, bool savemove = true);
	void moveSelected(MoveDirection d);
	int getNumberLostBalls(Player p);
	bool loadSetup(std::string filename);
	bool saveSetup(std::string filename);
	bool saveGame(std::string filename, std::string gamevariant);
};


#endif