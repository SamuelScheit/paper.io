#pragma once

extern int IDPlayer;
extern unsigned char Board[100][100];

int GetBoardSizeY();
int GetBoardSizeX();
int GetScore(int PlayerID);
int GetGameOver(int PlayerID);
int GetBoard(int y, int x);
int GetPlayerPositionX(int PlayerID);
int GetPlayerPositionY(int PlayerID);

void SetInput(char Value);
void SetBoardSize(int BoardY, int BoardX);
void SetPlayerPostion(int y, int x, int PlayerID);
void SetInput(char Value, int PlayerID);
void SetBoard(int y, int x, char value);
void Reset();
void Initialize(int PlayerID);
int BoardEditing(int PlayerID);
