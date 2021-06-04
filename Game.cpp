#include "stdafx.h"
#include "game.h"
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <string>
#include "socket.h"

#define MAXPLAYER 10

int DOWN = 0;
int UP = 1;
int LEFT = 2;
int RIGHT = 3;

using namespace std;

//  Board[ Y][ X];

class FloodFillPoint {
public:
	int x;
	int y;
	int Direction[4];
};

class PLAYER {
public:
	int x;
	int y;
	int GameOver;
	int Score;
	char input;
	char BeforeInput;
	BOOL OutOfBase = false; //If the Player is out of the base
	BOOL WaitForFinishDraw = false; //If the Player is OutOfBase and the Drawing Algorithm is waiting for the Player to entering the Base again
};
int IDPlayer = 0;
PLAYER Player[MAXPLAYER];
unsigned char Board[100][100]; //The Board
unsigned char BoardBase[100][100]; //The Board where the Base is saved
int Board_Y = 100; //The Board Size Y
int Board_X = 100; //The Board Size X
int i = 0; //Debug Purpose

int GetPlayerPositionX(int PlayerID) {
	return Player[PlayerID].x;
}

int GetPlayerPositionY(int PlayerID) {
	return Player[PlayerID].y;
}

int GetBoardSizeX() {
	return Board_X;
}

int GetBoardSizeY() {
	return Board_Y;
}

int GetScore(int PlayerID) {
	return Player[PlayerID].Score;
}
int GetGameOver(int PlayerID) {
	return Player[PlayerID].GameOver;
}

int GetBoard(int y, int x) {
	return Board[y][x];
}

void SetScore(int value, int PlayerID) {
	Player[PlayerID].Score = value;
}

void SetPlayerPostion(int y, int x, int PlayerID) {
	Player[PlayerID].y = y;
	Player[PlayerID].x = x;
}

void SetBoard(int y, int x, char value) {
	Board[y][x] = value;
}

void SetInput(char Value, int PlayerID) {
	if (role == SERVER) {
		Player[PlayerID].BeforeInput = Player[PlayerID].input;
		Player[PlayerID].input = Value;
	}
	else if (role == CLIENT) {
		SendInput(Value);
	}
}

void SetGameOver(int value, int PlayerID) {
	Player[PlayerID].GameOver = value;
}

void Border() {
#if 1
	for (int y = 0; y < 100; y++) {
		for (int x = 0; x < 100; x++) {
			if (Board[y][x] == 4) {
				Board[y][x] = 0;
			}
		}
	}

	for (int i = 0; i < Board_Y - 1; i++) { //Links
		Board[i][0] = 4;
	}
	for (int i = 0; i < Board_X - 1; i++) { //Unten
		Board[Board_Y - 1][i] = 4;
	}
	for (int i = 0; i < Board_Y - 1; i++) { //Rechts
		Board[i][Board_X - 1] = 4;
	}
	for (int i = 0; i < Board_X - 1; i++) { //Oben
		Board[0][i] = 4;
	}
#endif
}


void SetBoardSize(int BoardY, int BoardX) {
	Board_Y = BoardY;
	Board_X = BoardX;
	Border();
}

int Random(int to) {
	if (to == 0) {
		return ERROR;
	}
	int Value = rand() % to;

	return Value;
}

void CopyBoardBase(int PlayerID) {
	for (int y = 0; y < 100; y++) {
		for (int x = 0; x < 100; x++) {
			if (BoardBase[y][x] == (3 + PlayerID * 10)) {
				Board[y][x] = BoardBase[y][x];
			}
		}
	}
}

void Reset() {
	srand(time(NULL));
	memset(&Board[0][0], 0, sizeof(Board));
	memset(&BoardBase[0][0], 0, sizeof(BoardBase));
	Border();

	for (int i = 0; i < MAXPLAYER; i++) {
		do {
			Player[i].x = Random(Board_X);
			Player[i].y = Random(Board_Y);
		} while (
			Board[Player[i].x][Player[i].y] != 0 ||
			Player[i].x < 2 || Player[i].y < 2 ||
			Player[i].x > Board_X - 2 || Player[i].y > Board_Y - 2
			);

		Player[i].GameOver = false;
		Player[i].Score = 0;
		Player[i].OutOfBase = false;
		Player[i].WaitForFinishDraw = false;

		BoardBase[Player[i].y][Player[i].x] = (3 + i * 10);
	}
}

void Fill(
	int y,
	int x
) {

	FloodFillPoint FloodPoint[10000];
	int Fillcounter = 0;
	int i = 0;
	BOOL Ausnahme = false;

	FloodPoint[0].y = 0;
	FloodPoint[0].x = 0;

	while (1) {

		if (Ausnahme == true) {

			Fillcounter--;

			x = FloodPoint[Fillcounter].x;
			y = FloodPoint[Fillcounter].y;

		}
		else {
			i++;
			Fillcounter++;
			FloodPoint[Fillcounter].x = x;
			FloodPoint[Fillcounter].y = y;
		}

		if (y > 100 || y < 0 || x > 100 || x < 0 || Fillcounter <= 0) {
			break;
		}

		if (Board[y + 1][x] == 0) {
			FloodPoint[Fillcounter].Direction[DOWN] = true;
			y++;
			Board[y][x] = 9;
			Ausnahme = false;
			continue;
		} if (Board[y - 1][x] == 0) {
			FloodPoint[Fillcounter].Direction[UP] = true;
			y--;
			Board[y][x] = 9;
			Ausnahme = false;
			continue;
		} if (Board[y][x + 1] == 0) {
			FloodPoint[Fillcounter].Direction[RIGHT] = true;
			x++;
			Board[y][x] = 9;
			Ausnahme = false;
			continue;
		} if (Board[y][x - 1] == 0) {
			FloodPoint[Fillcounter].Direction[LEFT] = true;
			x--;
			Board[y][x] = 9;
			Ausnahme = false;
			continue;
		}

		Ausnahme = true;
	}
	i++;
}


void Initialize(int PlayerID) {

	strcat(&GameStatus[0], " - Respawned" + PlayerID);

	for (int y = 0; y < 100; y++) {
		for (int x = 0; x < 100; x++) {
			if (Board[y][x] == (1 + PlayerID * 10)) {
				Board[y][x] = 0;
			} if (Board[y][x] == (2 + PlayerID * 10)) {
				Board[y][x] = 0;
			} if (Board[y][x] == (3 + PlayerID * 10)) {
				Board[y][x] = 0;
				BoardBase[y][x] = 0;
			}
		}
	}
	do {
		Player[PlayerID].x = Random(Board_X);
		Player[PlayerID].y = Random(Board_Y);
	} while (
		Board[Player[PlayerID].x][Player[PlayerID].y] != 0 ||
		Player[PlayerID].x < 2 || Player[PlayerID].y < 2 ||
		Player[PlayerID].x > Board_X - 2 || Player[PlayerID].y > Board_Y - 2
		);

	Player[PlayerID].input = '\0';
	Player[PlayerID].GameOver = false;
	Player[PlayerID].Score = 0;
	Player[PlayerID].OutOfBase = false;
	Player[PlayerID].WaitForFinishDraw = false;
	BoardBase[Player[PlayerID].y][Player[PlayerID].x] = (3 + PlayerID * 10);
}

char OppositeInput(char input) {
	switch (input) {
	case 'w':
		return 's';
		break;
	case 's':
		return 'w';
		break;
	case 'a':
		return 'd';
		break;
	case 'd':
		return 'a';
		break;
	}
	return 0;
}

int BoardEditing(int PlayerID) {

	if (Player[PlayerID].GameOver == false) {

		Player[PlayerID].OutOfBase = true;
		CopyBoardBase(PlayerID);
		Border();

		Board[Player[PlayerID].y][Player[PlayerID].x] = (2 + PlayerID * 10);

		switch (Player[PlayerID].input) {
		case 'd':
			Player[PlayerID].x++;
			break;
		case 'a':
			Player[PlayerID].x--;
			break;
		case 's':
			Player[PlayerID].y++;
			break;
		case 'w':
			Player[PlayerID].y--;
			break;
		default:
			Board[Player[PlayerID].y][Player[PlayerID].x] = (1 + PlayerID * 10);
			break;
		}

		for (int i = 0; i < MAXPLAYER; i++) {
			if (i == PlayerID) {
				continue;
			}
			else {
				if (Board[Player[PlayerID].y][Player[PlayerID].x] == (1 + i * 10)) {
					Player[i].GameOver = true;
				}
				if (Board[Player[PlayerID].y][Player[PlayerID].x] == (2 + i * 10)) {
					Player[i].GameOver = true;
				}
				if (Board[Player[PlayerID].y][Player[PlayerID].x] == (3 + i * 10)) {
					BoardBase[Player[PlayerID].y][Player[PlayerID].x] = 0;
				}
			}
		}

		if (Board[Player[PlayerID].y][Player[PlayerID].x] == (2 + PlayerID * 10) || Board[Player[PlayerID].y][Player[PlayerID].x] == 4 || Player[PlayerID].y >= Board_Y - 1 || Player[PlayerID].x >= Board_X - 1) {
			if (Player[PlayerID].BeforeInput == OppositeInput(Player[PlayerID].input)) {
				Player[PlayerID].input = Player[PlayerID].BeforeInput;
				switch (Player[PlayerID].input) {
				case 'd':
					Player[PlayerID].x++;
					break;
				case 'a':
					Player[PlayerID].x--;
					break;
				case 's':
					Player[PlayerID].y++;
					break;
				case 'w':
					Player[PlayerID].y--;
					break;
				default:
					Board[Player[PlayerID].y][Player[PlayerID].x] = (1 + PlayerID * 10);
					break;
				}
				switch (Player[PlayerID].input) {
				case 'd':
					Player[PlayerID].x++;
					break;
				case 'a':
					Player[PlayerID].x--;
					break;
				case 's':
					Player[PlayerID].y++;
					break;
				case 'w':
					Player[PlayerID].y--;
					break;
				default:
					Board[Player[PlayerID].y][Player[PlayerID].x] = (1 + PlayerID * 10);
					break;
				}

				return 0;
			}
			else {
				Player[PlayerID].GameOver = true;
			}
		}

		Board[Player[PlayerID].y][Player[PlayerID].x] = (1 + PlayerID * 10);

		if (BoardBase[Player[PlayerID].y][Player[PlayerID].x] == (3 + PlayerID * 10)) {
			Player[PlayerID].OutOfBase = false;
		}

		if (Player[PlayerID].OutOfBase == true) {
			Player[PlayerID].WaitForFinishDraw = true;
		}

		if (Player[PlayerID].OutOfBase == false && Player[PlayerID].WaitForFinishDraw == true) {

#if 1
			int FillX = 0;
			int FillY = 0;
			BOOL skip = false;

			if (FillX == 0 && FillY == 0 && Board[1][1] == 0) {
				Fill(1, 1);
			}

			while (FillX < 100 - 1) {
				FillX++;
				if (Board[FillY + 1][FillX] == 0) {
					Fill(FillY + 1, FillX);
				}
			}

			if (Board[1][Board_X - 2] == 0) {
				Fill(1, Board_X - 2);
			}

			while (FillY < Board_Y - 1) {
				FillY++;
				if (Board[FillY][FillX - 1] == 0) {
					Fill(FillY, FillX - 1);
				}
			}

			if (Board[Board_Y - 2][Board_X - 2] == 0) {
				Fill(Board_Y - 1, Board_X - 1);
			}

			while (FillX > 0) {
				FillX--;
				if (Board[FillY - 1][FillX] == 0) {
					Fill(FillY - 1, FillX);
				}
			}

			if (Board[Board_Y - 1][1] == 0) {
				Fill(Board_Y - 1, 1);
			}

			while (FillY > 0) {
				FillY--;
				if (Board[FillY][FillX + 1] == 0) {
					Fill(FillY, FillX + 1);
				}
			}
#endif
			/*

			skip = false;

			if (FillX == 0 && Board[FillY][FillX + 1] != 0 && FillY == 0 && Board[FillY + 1][FillX] != 0 && skip == false) {
			FillX = 1;
			FillY = 1;
			goto skip;
			} if (FillX == Board_X - 1 && Board[FillY][FillX - 1] != 0 && FillY == Board_Y - 1 && Board[FillY - 1][FillX] != 0 && skip == false) {
			FillX = Board_X - 1;
			FillY = Board_Y - 1;
			goto skip;
			}


			if (Board[FillY][FillX + 1] != 0 && skip == false) {
			FillX++;
			} else {
			skip = true;
			} if (Board[FillY + 1][FillX] != 0 && skip == false) {
			FillY++;
			} else {
			skip = true;
			} if (Board[FillY][FillX - 1] != 0 && skip == false) {
			FillX--;
			} else {
			skip = true;
			} if (Board[FillY - 1][FillX] != 0 && skip == false) {
			FillY--;
			} else {
			skip = true;
			}

			skip:

			if (skip == false) {
			Fill(FillY, FillX);
			}*/


			for (int y = 0; y < Board_Y - 1; y++) {
				for (int x = 0; x < Board_X - 1; x++) {
					if (Board[y][x] == (2 + PlayerID * 10) || Board[y][x] == 0) {
						BoardBase[y][x] = (3 + PlayerID * 10);
						Player[PlayerID].Score++;
					}

					if (Board[y][x] == 9) {
						Board[y][x] = 0;
					}
				}
			}
			Player[PlayerID].Score--;

			Player[PlayerID].WaitForFinishDraw = false;
		}

	}
	else {
		Initialize(PlayerID);
	}
	return 0;
}