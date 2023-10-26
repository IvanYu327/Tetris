#include "block.h"
#include <vector>
#include "constants.h"

using namespace std;
using namespace CONSTANTS;

Block::Block()
{
    cellSize = CELL_SIZE;
    drawXOffset = DRAW_GRID_X_OFFSET;
    drawYOffset = DRAW_GRID_Y_OFFSET;

    rotationState = 0;
    colors = GetCellColors();
    rowOffset = 0;
    columnOffset = 0;
}

void Block::Draw(int offsetX, int offsetY)
{
    vector<Position> tiles = GetCellPositions();
    for (Position item : tiles)
    {
        DrawRectangle(item.column * cellSize + offsetX + 1, item.row * cellSize + offsetY + 1, cellSize - 1, cellSize - 1, colors[id]);
    }
}

void Block::Move(int rows, int columns)
{
    rowOffset += rows;
    columnOffset += columns;
}

vector<Position> Block::GetCellPositions()
{
    vector<Position> tiles = cells[rotationState];
    vector<Position> movedTiles;

    for (Position item : tiles)
    {
        Position newPos = Position(item.row + rowOffset, item.column + columnOffset);
        movedTiles.push_back(newPos);
    }
    return movedTiles;
}

void Block::RotateCW()
{
    rotationState = (rotationState + 1) % cells.size();
}

void Block::RotateCCW()
{
    rotationState = (rotationState + 3) % cells.size();
}

void Block::Rotate180()
{
    rotationState = (rotationState + 2) % cells.size();
}
