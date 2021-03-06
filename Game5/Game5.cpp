#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>
#include <conio.h>
#include <ctype.h>

#include "consoleColor.h"
#include "level.h"
#include "weaponType.h"
#include "unitType.h"
#include "unitData.h"




// CONSTANTS


const int maxUnitsCount = 35;



// LOGICS 

HANDLE consoleHandle = 0;
bool isGameActive = true;



unsigned char levelData[rowsCount][columnsCount];

UnitData unitsData[maxUnitsCount];
int unitsCount = 0;
int heroIndex = 0;

// Functions
void SetupSystem()
{
	

	srand(time(0));

	consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	// Hide console cursor
	CONSOLE_CURSOR_INFO cursorInfo;
	cursorInfo.dwSize = 1;
	cursorInfo.bVisible = 0;
	SetConsoleCursorInfo(consoleHandle, &cursorInfo);
}

void Initialize()
{
	unitsCount = 0;
	//Load Level
	for (int r = 0; r < columnsCount; r++)
	{
		for (int c = 0; c < columnsCount; c++)
		{
			unsigned char cellSymbol = levelData0[r][c];

			levelData[r][c] = cellSymbol;

			switch (cellSymbol)
			{
			case CellSymbol_Hero:
				heroIndex = unitsCount;

			case CellSymbol_Orc:
			case CellSymbol_Skeleton:
			  {
				UnitType unitType = GetUnitTypeFromCell(cellSymbol);
				unitsData[unitsCount].type = unitType;
				unitsData[unitsCount].row = r;
				unitsData[unitsCount].column = c;
				unitsData[unitsCount].weapon = GetUnitDefaultWeapon(unitType);
				unitsData[unitsCount].health = GetUnitDefaultHealth(unitType);
				unitsCount++;

				break;
			  }
			}
		}
	}
}

void Render()
{
	srand(time(0));
	consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	// Move console cursor to (0,0)
	COORD cursorCoord;
	cursorCoord.X = 0;
	cursorCoord.Y = 0;
	SetConsoleCursorPosition(consoleHandle, cursorCoord);

	// Draw game title
	SetConsoleTextAttribute(consoleHandle, ConsoleColor_Green);
	printf("\n\tDUNGEONS AND ORCS");



	//Draw level

	printf("\n\n\t");
	for (int r = 0; r < rowsCount; r++)
	{
		for (int c = 0; c < columnsCount; c++)
		{
			unsigned char cellSymbol = levelData[r][c];

			unsigned char renderCellSymbol = GetRenderCellSymbol(cellSymbol);
			ConsoleColor cellColor = GetRenderCellSymbolColor(cellSymbol);

			SetConsoleTextAttribute(consoleHandle, cellColor);
			printf("%c", renderCellSymbol);



		}
		printf("\n\t");
	}
}

void MoveUnitTo(UnitData *pointerToUnitData, int row, int column)
{
	// Ignore dead units
	if (pointerToUnitData->health <= 0)
	{
		return;
	}
	unsigned char unitSymbol = levelData[pointerToUnitData->row][pointerToUnitData->column];
	unsigned char destinationCellSymbol = levelData[row][column];
	bool canMoveToCell = false;

	switch (destinationCellSymbol)
	{
	case CellSymbol_Empty:
	{
		canMoveToCell = true;
		break;
	}
	// Units cells
	case CellSymbol_Hero:
	case CellSymbol_Orc:
	case CellSymbol_Skeleton:
	{
		UnitType destinationUnitType = GetUnitTypeFromCell(destinationCellSymbol);
		// if destination unit have other type
		if (pointerToUnitData->type != destinationUnitType)
		{
			// Find enemy unit struct
			for (int u = 0; u < unitsCount; u++)
			{
				//Ignore dead units
				if (unitsData[u].health <= 0)
					continue;

				if (unitsData[u].row == row && unitsData[u].column == column)
				{

					// Calculate weapon damage
					int damage = GetWeaponDamage(pointerToUnitData->weapon);

					// Deal damage
					unitsData[u].health = unitsData[u].health - damage;

					// if enemy unit die
					if (unitsData[u].health <= 0.0f)
					{
						levelData[row][column] = CellSymbol_Empty;
					}

					break;


				}

			}

		}

		break;
	}
	}



	if (pointerToUnitData->type == UnitType_Hero)
	{
		switch (destinationCellSymbol)
		{
		case CellSymbol_Stick:
		case CellSymbol_Club:
		case CellSymbol_Spear:
		case CellSymbol_Saber:
		{
			canMoveToCell = true;

			WeaponType weaponType = GetWeaponTypeFromCell(destinationCellSymbol);
			if (unitsData[heroIndex].weapon < weaponType)
			{
				unitsData[heroIndex].weapon = weaponType;
			}
			break;
		}

		case CellSymbol_Exit:
		{
			isGameActive = false;
			break;
		}
		}
	}

	if (canMoveToCell)
	{
		// Remove unit symbol from previous position
		levelData[pointerToUnitData->row][pointerToUnitData->column] = CellSymbol_Empty;

		// Set new hero position
		pointerToUnitData->row = row;
		pointerToUnitData->column = column;

		// Set hero symbol to new position
		levelData[pointerToUnitData->row][pointerToUnitData->column] = unitSymbol;
	}
}


void UpdateAI()
{
	// Pass all units
	for (int u = 0; u < unitsCount; u++)
	{
		// Ignore hero
		if (u == heroIndex)
			continue;

		// Ignore dead units
		if (unitsData[u].health <= 0)
			continue;

		// Distance to hero
		int distanceToHeroR = abs(unitsData[heroIndex].row - unitsData[u].row);
		int distanceToHeroC = abs(unitsData[heroIndex].column - unitsData[u].column);

		// If hero nearby
		if ((distanceToHeroR + distanceToHeroC) == 1)
		{
			// Attack hero
			MoveUnitTo(&unitsData[u], unitsData[heroIndex].row, unitsData[heroIndex].column);
		}
		else
		{
			// Random move
			switch (rand() % 4)
			{
			case 0:
				MoveUnitTo(&unitsData[u], unitsData[u].row - 1, unitsData[u].column);
				break;

			case 1:
				MoveUnitTo(&unitsData[u], unitsData[u].row + 1, unitsData[u].column);
				break;

			case 2:
				MoveUnitTo(&unitsData[u], unitsData[u].row, unitsData[u].column - 1);
				break;

			case 3:
				MoveUnitTo(&unitsData[u], unitsData[u].row, unitsData[u].column + 1);
				break;
			}

		}
	}
}

void Update()
{
	unsigned char inputChar = _getch();
	inputChar = tolower(inputChar);

	switch (inputChar)
	{
		// Up
	case 'w':
		MoveUnitTo(&unitsData[heroIndex], unitsData[heroIndex].row - 1, unitsData[heroIndex].column);
		break;

		// Down
	case 's':
		MoveUnitTo(&unitsData[heroIndex], unitsData[heroIndex].row + 1, unitsData[heroIndex].column);
		break;

		// Left
	case 'a':
		MoveUnitTo(&unitsData[heroIndex], unitsData[heroIndex].row, unitsData[heroIndex].column - 1);
		break;

		// Right
	case 'd':
		MoveUnitTo(&unitsData[heroIndex], unitsData[heroIndex].row, unitsData[heroIndex].column + 1);
		break;

		// Restart level
	case 'r':
		Initialize();
		break;
	}


	// AI turn
	UpdateAI();


	// Hero death
	if (unitsData[heroIndex].health <= 0)
	{
		isGameActive = false;
	}
	else
	{
		// Health regeneration
		if (unitsData[heroIndex].health < GetUnitDefaultHealth(UnitType_Hero))
		{
			unitsData[heroIndex].health++;
		}
	}

}


void Shutdown()
{
	system("cls");
	printf("\n\tGame over...");
	_getch();
}


int main()
{
	SetupSystem();
	Initialize();
	do
	{
		Render();
		Update();
	} while (isGameActive);

	Shutdown();

    return 0;
}

