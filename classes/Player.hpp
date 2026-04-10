/*
|------------------------------------------------------|
|                    PLAYER HEADER                     |              
|------------------------------------------------------|
|   This is the header file (.hpp) file that contains  |
|   declarations for the Player class along with the   | 
|   PlayerState class and its subclasses.              |
|                                                      |
|   Note that ONLY DECLARATIONS are made here. The     |
|   DEFINITION of all member variables and functions   |
|   are done in the PlayerStateMachine.cpp file        |
|                                                      |
|   When adding new classes to this file               |
|   (i.e. PlayerBlocking, PlayerAttacking, etc.),      |
|   Only have the class declarations and define them   |
|   later in the PlayerStateMachine.cpp file           |
|                                                      |
|------------------------------------------------------|
*/

#ifndef PLAYER
#define PLAYER

#include <raylib.h>
#include <raymath.h>
#include "Grid.hpp"

#define DASH_DURATION 0.2f

class Player;

class PlayerState {
public:
    Player* player;

    virtual ~PlayerState() {}
    virtual void Enter() = 0;
    virtual void Update(float delta_time) = 0;
    virtual void Exit() = 0;
};

class PlayerGrounded : public PlayerState {
public:
    void Enter();
    void Update(float delta_time);
    void Exit();
};

class PlayerAirborne : public PlayerState {
public:
    void Enter();
    void Update(float delta_time);
    void Exit();
};

class PlayerDashing : public PlayerState {
public:
    void Enter();
    void Update(float delta_time);
    void Exit();
};


class Player {
    PlayerState* current_state = nullptr;
    Grid* grid = nullptr;

public:
    // Player movement properties
    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;

    // Player physics properties
    float width, height;
    float speed;
    float mass;
    float inverse_mass;
    
    // Player other properties
    Color color;
    
    // Dash variables
    bool is_facing_right = true;
    float dash_time = 0.0f;
    float dash_duration = DASH_DURATION;
    bool has_dashed = false;
    Vector2 dash_direction = {0.0f, 0.0f};

    // Player states
    PlayerGrounded grounded;
    PlayerAirborne airborne;
    PlayerDashing dashing;

    // Keybinds
    KeyboardKey JUMP_KEY = KEY_SPACE;
    KeyboardKey DASH_KEY = KEY_ENTER;
    KeyboardKey UP_KEY = KEY_UP;
    KeyboardKey DOWN_KEY = KEY_DOWN;
    KeyboardKey LEFT_KEY = KEY_LEFT;
    KeyboardKey RIGHT_KEY = KEY_RIGHT;

    Player(Vector2 pos, float w, float h, float spd, float m);

    void Update(float delta_time);

    void Draw();

    void SetState(PlayerState* state);

    PlayerState* GetCurrentState();

    void LoadKeybinds(KeyboardKey jump, KeyboardKey dash, KeyboardKey up, 
                      KeyboardKey down, KeyboardKey left, KeyboardKey right);

    void SetGrid(Grid* grid) { this->grid = grid; }
    Grid* GetGrid() { return grid; }

    bool IsHittingFloor();
    bool IsHittingCeiling();
    bool IsHittingWall();
};

#endif