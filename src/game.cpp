#include "game.h"
#include <random>
#include <cmath>
#include <iostream>

using namespace std;

Game::Game()
{
    grid = Grid();

    blocks = GetAllBlocks();

    gameOver = false;
    score = 0;

    currentBlock = GetRandomBlock();
    for (int i = 0; i < 5; i++)
        nextBlocks.emplace_back(GetRandomBlock());

    // Load music and sounds
    InitAudioDevice();
    music = LoadMusicStream("src/sounds/music.mp3");
    PlayMusicStream(music);
    rotateSound = LoadSound("src/sounds/rotate.mp3");
    clearSound = LoadSound("src/sounds/clear.mp3");
}

Game::~Game()
{
    UnloadSound(rotateSound);
    UnloadSound(clearSound);
    UnloadMusicStream(music);
    CloseAudioDevice();
}

Block Game::GetRandomBlock()
{
    if (blocks.empty())
        blocks = GetAllBlocks();

    int index = rand() % blocks.size();
    Block block = blocks[index];
    blocks.erase(blocks.begin() + index);
    return block;
}

vector<Block> Game::GetAllBlocks()
{
    return {LBlock(), JBlock(), IBlock(), OBlock(), SBlock(), ZBlock(), TBlock()};
}

void Game::Draw()
{
    grid.Draw();
    currentBlock.Draw();

    Block nextBlock = nextBlocks[0];

    switch (nextBlock.id)
    {
    case 3:
        nextBlock.Draw(255, 290);
        break;
    case 4:
        nextBlock.Draw(255, 280);
        break;
    default:
        nextBlock.Draw(270, 270);
        break;
    }
}

void Game::HandleInput()
{
    int keyPressed = GetKeyPressed();

    if (keyPressed == KEY_SPACE)
        HardDrop();
    if (keyPressed == KEY_LEFT)
    {
        MoveBlockLeft();
        moveLeft = true;
        moveRight = false;
        dasTimer = 0.0f;
        arrTimer = 0.0f;
    }
    if (keyPressed == KEY_RIGHT)
    {
        MoveBlockRight();
        moveRight = true;
        moveLeft = false;
        dasTimer = 0.0f;
        arrTimer = 0.0f;
    }
    if (keyPressed == KEY_UP)
        RotateBlockCW();
    if (keyPressed == KEY_X)
        RotateBlockCCW();
    if (keyPressed == KEY_Z)
        RotateBlock180();
    if (keyPressed == KEY_R)
        Reset();
    if (keyPressed == KEY_DOWN)
        SoftDrop();

    // Check if a movement key is currently pressed
    bool movingLeft = IsKeyDown(KEY_LEFT) && moveLeft;
    bool movingRight = IsKeyDown(KEY_RIGHT) && moveRight;

    // Handle left movement using DAS
    if (movingLeft)
    {
        // enter ARR
        if (dasTimer >= DAS_DELAY)
        {
            arrTimer += GetFrameTime();
            int timesToMove = floor(arrTimer / ARR_DELAY);

            if (ARR_DELAY == 0)
                MoveBlockLeft(50);
            else
                MoveBlockLeft(timesToMove);

            arrTimer -= timesToMove * ARR_DELAY;
        }
        // Increment DAS Delay
        else
        {
            dasTimer += GetFrameTime();
        }
    }

    // Handle right movement using DAS
    if (movingRight)
    {
        if (dasTimer >= DAS_DELAY)
        {
            arrTimer += GetFrameTime();

            int timesToMove = floor(arrTimer / ARR_DELAY);

            if (ARR_DELAY == 0)
                MoveBlockRight(50);
            else
                MoveBlockRight(timesToMove);

            arrTimer -= timesToMove * ARR_DELAY;
        }
        else
        {
            dasTimer += GetFrameTime();
        }
    }
}

void Game::MoveBlockLeft(int count)
{
    if (gameOver)
        return;

    for (int i = 0; i <= count; i++)
    {
        currentBlock.Move(0, -1);
        if (IsBlockOutside() || !BlockFits())
        {
            currentBlock.Move(0, 1);
            break;
        }
    }
}

void Game::MoveBlockRight(int count)
{
    if (gameOver)
        return;

    for (int i = 0; i <= count; i++)
    {
        currentBlock.Move(0, 1);
        if (IsBlockOutside() || !BlockFits())
        {
            currentBlock.Move(0, -1);
            break;
        }
    }
}

void Game::MoveBlockDown()
{
    if (gameOver)
        return;

    currentBlock.Move(1, 0);
    if (IsBlockOutside() || !BlockFits())
    {
        currentBlock.Move(-1, 0);
        LockBlock();
    }
}

void Game::SoftDrop()
{
    if (gameOver)
        return;

    currentBlock.Move(1, 0);
    if (IsBlockOutside() || !BlockFits())
    {
        currentBlock.Move(-1, 0);
        LockBlock();
    }
    else
        UpdateScore(0, 10);
}

void Game::HardDrop()
{
    if (gameOver)
        return;

    int blocksMoved = 0;
    while (!IsBlockOutside() && BlockFits())
    {
        currentBlock.Move(1, 0);
        blocksMoved++;
    }

    blocksMoved--;
    currentBlock.Move(-1, 0);
    LockBlock();
    UpdateScore(0, blocksMoved * 10);
}

void Game::RotateBlockCW()
{
    if (gameOver)
        return;

    currentBlock.RotateCW();
    if (IsBlockOutside() || !BlockFits())
        currentBlock.RotateCCW();
    else
        PlaySound(rotateSound);
}

void Game::RotateBlockCCW()
{
    if (gameOver)
        return;

    currentBlock.RotateCCW();
    if (IsBlockOutside() || !BlockFits())
        currentBlock.RotateCW();
    else
        PlaySound(rotateSound);
}

void Game::RotateBlock180()
{
    if (gameOver)
        return;

    currentBlock.Rotate180();
    if (IsBlockOutside() || !BlockFits())
        currentBlock.Rotate180();
    else
        PlaySound(rotateSound);
}

void Game::LockBlock()
{
    vector<Position> tiles = currentBlock.GetCellPositions();

    for (Position item : tiles)
        grid.grid[item.row][item.column] = currentBlock.id;

    currentBlock = nextBlocks[0];
    if (!BlockFits())
        gameOver = true;

    nextBlocks.erase(nextBlocks.begin());
    nextBlocks.emplace_back(GetRandomBlock());

    int rowsCleared = grid.ClearFullRows();
    if (rowsCleared > 0)
    {
        PlaySound(clearSound);
        UpdateScore(rowsCleared, 0);
    }
}

bool Game::IsBlockOutside()
{
    vector<Position> tiles = currentBlock.GetCellPositions();
    for (Position item : tiles)
        if (grid.IsCellOutside(item.row, item.column))
            return true;

    return false;
}

bool Game::BlockFits()
{
    vector<Position> tiles = currentBlock.GetCellPositions();

    for (Position item : tiles)
        if (!grid.IsCellEmpty(item.row, item.column))
            return false;

    return true;
}

void Game::Reset()
{
    grid.Initialize();
    blocks = GetAllBlocks();
    currentBlock = GetRandomBlock();
    gameOver = false;

    for (int i = 0; i < 5; i++)
        nextBlocks.emplace_back(GetRandomBlock());

    score = 0;
}

void Game::UpdateScore(int linesCleared, int moveDownPoints)
{
    if (gameOver)
        return;

    switch (linesCleared)
    {
    case 1:
        score += 100;
        break;
    case 2:
        score += 300;
        break;
    case 3:
        score += 500;
        break;
    case 4:
        score += 1000;
        break;
    default:
        break;
    }

    score += moveDownPoints;
}
