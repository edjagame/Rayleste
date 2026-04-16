/*
|------------------------------------------------------|
|                 PLAYER STATE MACHINE                 |              
|------------------------------------------------------|
|   This is the file that contains definitions for     |
|   all the functions declared in the Player.hpp       |
|   file.                                              |
|                                                      |
|   All functions from the Player.hpp SHOULD be        |
|   defined here (For Now)                             |
|                                                      |
|------------------------------------------------------|
*/


#include <raylib.h>
#include <raymath.h>
#include <iostream>
#include "Player.hpp"

#define GRAVITY 2000.0f
#define JUMP_MULTIPLIER 2.5f

const Color PLAYER_COLOR = {200, 0, 0, 255};
const Color DASHLESS_COLOR = {0, 200, 200, 255};
const Color DEAD_COLOR = {0, 0, 0, 255};

/**************************************************
 *            GLOBAL PLAYER FUNCTIONS             *
 **************************************************/
void Player::Update(float delta_time) {
    if (dash_cooldown_timer > 0.0f) {
        dash_cooldown_timer -= delta_time;
    }
    
    Vector2 old_position = position;
    
    position = Vector2Add(position, Vector2Scale(velocity, delta_time));
    
    if (LeftWallType() == TileType::SOLID || RightWallType() == TileType::SOLID) {
        position.x = old_position.x;
        velocity.x = 0.0f;
    }

    if (current_state != &dead && (FloorType() == TileType::SPIKE ||
        CeilingType() == TileType::SPIKE || 
        LeftWallType() == TileType::SPIKE || 
        RightWallType() == TileType::SPIKE)) {
        SetState(&dead);
    }
    
    current_state->Update(delta_time);
}

void Player::Draw() {
    DrawRectangle(position.x, position.y, width, height, color);
}

Player::Player(Vector2 pos, float w, float h, float spd, float m) {
    position = pos;
    width = w;
    height = h;
    speed = spd;
    mass = (m <= 0) ? 1.0f : m; // safety vs zero/negative mass
    inverse_mass = 1.0f / mass;
    color = PLAYER_COLOR;

    current_respawn_point = pos;

    grounded.player = &*this;
    airborne.player = &*this;
    dashing.player = &*this;
    wall_climbing.player = &*this;
    dead.player = &*this;

    SetState(&grounded);
}

void Player::SetState(PlayerState* state) {
    if (current_state != nullptr) {
        current_state->Exit();
    }

    current_state = state;
    current_state->Enter();
}

PlayerState* Player::GetCurrentState() {
    return current_state;
}

void Player::LoadKeybinds(KeyboardKey jump, KeyboardKey dash, KeyboardKey grab, KeyboardKey up, 
                          KeyboardKey down, KeyboardKey left, KeyboardKey right) {
    JUMP_KEY = jump;
    DASH_KEY = dash;
    GRAB_KEY = grab;
    UP_KEY = up;
    DOWN_KEY = down;
    LEFT_KEY = left;
    RIGHT_KEY = right;
}

/**************************************************
 *              ENTER STATE FUNCTIONS             *
 **************************************************/
void PlayerGrounded::Enter() {
    player->color = PLAYER_COLOR;
}

void PlayerAirborne::Enter() {
}

void PlayerDashing::Enter() {
    player->dash_time = 0.0f;
    player->velocity = {0.0f, 0.0f};
    
    // Determine dash direction once at the start
    bool up_key_pressed = IsKeyDown(player->UP_KEY);
    bool down_key_pressed = IsKeyDown(player->DOWN_KEY);
    bool left_key_pressed = IsKeyDown(player->LEFT_KEY);
    bool right_key_pressed = IsKeyDown(player->RIGHT_KEY);

    Vector2 dash_direction = {0.0f, 0.0f};
    
    if (right_key_pressed) dash_direction.x += 1.0f;
    if (left_key_pressed) dash_direction.x -= 1.0f;
    if (up_key_pressed) dash_direction.y -= 1.0f;
    if (down_key_pressed) dash_direction.y += 1.0f;
    
    // prevent dashing into the ground
    if (player->FloorType() == TileType::SOLID && dash_direction.y > 0.0f) {
        dash_direction.y = 0.0f;
    }
    
    // Use facing direction as default if no direction input
    if (dash_direction.x == 0.0f && dash_direction.y == 0.0f) {
        dash_direction.x = player->is_facing_right ? 1.0f : -1.0f;
    }
    
    // Normalize and store
    player->dash_direction = Vector2Normalize(dash_direction);
    player->color = DASHLESS_COLOR;
}

void PlayerWallClimbing::Enter() {
    if (player->velocity.y > WALL_SLIDE_SPEED) {
        player->velocity.y = WALL_SLIDE_SPEED;
    }
    player->velocity.x = 0.0f;
}

void PlayerDead::Enter() {
    player->respawn_timer = 0.0f;
    player->color = DEAD_COLOR;
}

/**************************************************
 *              EXIT STATE FUNCTIONS              *
 **************************************************/
void PlayerGrounded::Exit() {}

void PlayerAirborne::Exit() {}

void PlayerDashing::Exit() {}

void PlayerWallClimbing::Exit() {}

void PlayerDead::Exit() {}

/**************************************************
 *             UPDATE STATE FUNCTIONS             *
 **************************************************/
void PlayerGrounded::Update(float delta_time) {

    if (IsKeyDown(player->RIGHT_KEY)) {
        player->velocity.x = player->speed;
        player->is_facing_right = true;
    } else if (IsKeyDown(player->LEFT_KEY)) {
        player->velocity.x = -player->speed;
        player->is_facing_right = false;
    } else {
        player->velocity.x = 0.0f;
    }

    if (IsKeyDown(player->JUMP_KEY)) {
        player->velocity.y = -player->speed * JUMP_MULTIPLIER;
        player->SetState(&player->airborne);
    }

    bool on_wall = player->IsAdjacentToLeftWall() || player->IsAdjacentToRightWall();
    if (IsKeyDown(player->GRAB_KEY) && on_wall) {
        player->SetState(&player->wall_climbing);
        return;
    }

    if (IsKeyDown(player->DASH_KEY) && !player->has_dashed && player->dash_cooldown_timer <= 0.0f) {
        player->SetState(&player->dashing);
    }
    
    if(player->FloorType() != TileType::SOLID) {
        player->SetState(&player->airborne);
    }

    
}

void PlayerAirborne::Update(float delta_time) {
    if (IsKeyDown(player->RIGHT_KEY)) {
        player->velocity.x = player->speed;
        player->is_facing_right = true;
    } else if (IsKeyDown(player->LEFT_KEY)) {
        player->velocity.x = -player->speed;
        player->is_facing_right = false;
    } else {
        player->velocity.x = 0.0f;
    }
    
    if (IsKeyDown(player->DASH_KEY) && !player->has_dashed && player->dash_cooldown_timer <= 0.0f) {
        player->SetState(&player->dashing);
        player->has_dashed = true;
    }

    bool on_left_wall = player->IsAdjacentToLeftWall();
    bool on_right_wall = player->IsAdjacentToRightWall();
    bool on_wall = on_left_wall || on_right_wall;

    if (player->FloorType() != TileType::SOLID && on_wall && IsKeyDown(player->GRAB_KEY)) {
        player->SetState(&player->wall_climbing);
        return;
    }
    
    player->acceleration.y = GRAVITY;
    player->velocity.y += player->acceleration.y * delta_time;

    // snap to tile and switch back to grounded on touching a solid tile
    if (player->FloorType() == TileType::SOLID) {
        player->velocity.y = 0.0f;
        float tile_h = player->GetGrid()->tile_size_grid.y;
        int row = (int)((player->position.y + player->height) / tile_h);
        player->position.y = row * tile_h - player->height;
        player->SetState(&player->grounded);
        player->has_dashed = false; 
    }
}


void PlayerDashing::Update(float delta_time) {
    const float DASH_SPEED = player->speed * 4.0f; 
    player->dash_time += delta_time;
    
    Vector2 old_position = player->position;
    Vector2 dash_movement = Vector2Scale(player->dash_direction, DASH_SPEED * delta_time);
    player->position = Vector2Add(player->position, dash_movement);
    player->velocity = {0.0f, 0.0f};
    
    // Collision check during dashing
    bool hit_ceiling = player->CeilingType() == TileType::SOLID;
    bool hit_wall = player->LeftWallType() == TileType::SOLID || player->RightWallType() == TileType::SOLID;
    bool hit_floor = player->dash_direction.y > 0.0f && player->FloorType() == TileType::SOLID;
    
    if (hit_ceiling || hit_wall || hit_floor) {
        player->position = old_position;
        player->dash_cooldown_timer = DASH_COOLDOWN;
        player->SetState(&player->airborne);
        return;
    }
    
    if (player->dash_time >= player->dash_duration) {
        player->dash_cooldown_timer = DASH_COOLDOWN;
        if (player->FloorType() == TileType::SOLID) {
            player->has_dashed = false;
            player->SetState(&player->grounded);
        } else {
            player->SetState(&player->airborne);
        }
    }
}

void PlayerWallClimbing::Update(float delta_time) {
    bool on_left_wall = player->IsAdjacentToLeftWall();
    bool on_right_wall = player->IsAdjacentToRightWall();
    bool on_wall = on_left_wall || on_right_wall;

    if (player->FloorType() == TileType::SOLID && !IsKeyDown(player->UP_KEY)) {
        player->velocity.y = 0.0f;
        player->SetState(&player->grounded);
        player->has_dashed = false;
        return;
    }

    if (!on_wall || !IsKeyDown(player->GRAB_KEY)) {
        player->SetState(&player->airborne);
        return;
    }

    if (IsKeyDown(player->DASH_KEY) && !player->has_dashed && player->dash_cooldown_timer <= 0.0f) {
        player->SetState(&player->dashing);
        player->has_dashed = true;
        return;
    }

    if (IsKeyDown(player->JUMP_KEY)) {
        float wall_jump_x = player->speed * WALL_JUMP_HORIZONTAL_MULTIPLIER;
        if (on_left_wall) {
            player->velocity.x = wall_jump_x;
            player->is_facing_right = true;
        } else {
            player->velocity.x = -wall_jump_x;
            player->is_facing_right = false;
        }

        player->velocity.y = -player->speed * JUMP_MULTIPLIER;
        player->SetState(&player->airborne);
        return;
    }

    player->velocity.x = 0.0f;

    if (IsKeyDown(player->UP_KEY)) {
        player->velocity.y = -WALL_CLIMB_SPEED;
    } else if (IsKeyDown(player->DOWN_KEY)) {
        player->velocity.y = WALL_CLIMB_SPEED;
    } else {
        player->velocity.y = WALL_SLIDE_SPEED;
    }

    if (player->CeilingType() == TileType::SOLID && player->velocity.y < 0.0f) {
        player->velocity.y = 0.0f;
    }
}

void PlayerDead::Update(float delta_time) {
    player->respawn_timer += delta_time;
    if (player->respawn_timer >= player->respawn_time) { 
        player->position = player->current_respawn_point;
        player->velocity = {0.0f, 0.0f};
        player->SetState(&player->airborne);
    }
}


/**************************************************
 *             OTHER PLAYER FUNCTIONS             *
 **************************************************/

TileType Player::FloorType() {
    Vector2 feet_position = { position.x + width / 2.0f, position.y + height };
    return GetGrid()->GetTileType(feet_position);
}

TileType Player::CeilingType() {
    Vector2 head_position = { position.x + width / 2.0f, position.y };
    return GetGrid()->GetTileType(head_position);
}

TileType Player::LeftWallType() {
    Vector2 left_position = { position.x + 1, position.y + height / 2.0f };
    return GetGrid()->GetTileType(left_position);
}

TileType Player::RightWallType() {
    Vector2 right_position = { position.x + width - 1, position.y + height / 2.0f };
    return GetGrid()->GetTileType(right_position);
}

bool Player::IsAdjacentToLeftWall() {
    Vector2 left_position = { position.x - 1, position.y + height / 2.0f };
    return GetGrid()->GetTileType(left_position) == TileType::SOLID;
}

bool Player::IsAdjacentToRightWall() {
    Vector2 right_position = { position.x + width + 1, position.y + height / 2.0f };
    return GetGrid()->GetTileType(right_position) == TileType::SOLID;
}