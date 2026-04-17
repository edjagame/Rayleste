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

#define GRAVITY 4000.0f
#define JUMP_MULTIPLIER 2.0f

#define DASH_MULTIPLIER 3.0f
#define DASH_DURATION 0.2f
#define DASH_COOLDOWN 0.3f
#define RESPAWN_TIME 1.0f

#define WALL_CLIMB_SPEED 180.0f
#define WALL_GRAB_DURATION 2.0f
#define WALL_JUMP_HORIZONTAL_MULTIPLIER 3.0f
#define WALL_JUMP_COOLDOWN 0.1f

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

class PlayerWallClimbing : public PlayerState {
public:
    void Enter();
    void Update(float delta_time);
    void Exit();
};

class PlayerDead : public PlayerState {
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
    float dash_cooldown_timer = 0.0f;
    bool has_dashed = false;
    Vector2 dash_direction = {0.0f, 0.0f};

    // Wall climbing variables
    bool can_wall_grab = true;
    float wall_climb_speed = WALL_CLIMB_SPEED;
    float wall_grab_timer = 0.0f;
    float wall_jump_horizontal_multiplier = WALL_JUMP_HORIZONTAL_MULTIPLIER;
    float wall_jump_cooldown_timer = 0.0f;

    // Respawn variables
    Vector2 current_respawn_point = {0.0f, 0.0f};
    float respawn_timer = 0.0f;
    float respawn_time = RESPAWN_TIME;

    // Player states
    PlayerGrounded grounded;
    PlayerAirborne airborne;
    PlayerDashing dashing;
    PlayerWallClimbing wall_climbing;
    PlayerDead dead;
    // Keybinds (defaults, changed in settings.ini)
    KeyboardKey JUMP_KEY = KEY_SPACE;
    KeyboardKey DASH_KEY = KEY_ENTER;
    KeyboardKey GRAB_KEY = KEY_LEFT_SHIFT;
    KeyboardKey UP_KEY = KEY_UP;
    KeyboardKey DOWN_KEY = KEY_DOWN;
    KeyboardKey LEFT_KEY = KEY_LEFT;
    KeyboardKey RIGHT_KEY = KEY_RIGHT;

    Player(Vector2 pos, float w, float h, float spd, float m);

    void Update(float delta_time);

    void Draw();

    void SetState(PlayerState* state);

    PlayerState* GetCurrentState();

    void LoadKeybinds(KeyboardKey jump, KeyboardKey dash, KeyboardKey grab, KeyboardKey up, 
                      KeyboardKey down, KeyboardKey left, KeyboardKey right);

    void SetGrid(Grid* grid) { this->grid = grid; }
    Grid* GetGrid() { return grid; }
    

    // Movement and collision
    Vector2 StepwiseMove(Vector2 movement);
    
    // Collision checks
    TileType FloorType();
    TileType CeilingType();
    TileType LeftWallType();
    TileType RightWallType();
    bool IsAdjacentToLeftWall();
    bool IsAdjacentToRightWall();
};

#endif