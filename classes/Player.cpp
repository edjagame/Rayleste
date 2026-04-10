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

#define GRAVITY 1500.0f
#define JUMP_MULTIPLIER 2.0f

/**************************************************
 *            GLOBAL PLAYER FUNCTIONS             *
 **************************************************/
void Player::Update(float delta_time) {
    position = Vector2Add(position, Vector2Scale(velocity, delta_time));
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
    color = WHITE;

    grounded.player = &*this;
    airborne.player = &*this;
    dashing.player = &*this;

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

void Player::LoadKeybinds(KeyboardKey jump, KeyboardKey dash, KeyboardKey up, 
                          KeyboardKey down, KeyboardKey left, KeyboardKey right) {
    JUMP_KEY = jump;
    DASH_KEY = dash;
    UP_KEY = up;
    DOWN_KEY = down;
    LEFT_KEY = left;
    RIGHT_KEY = right;
}

/**************************************************
 *              ENTER STATE FUNCTIONS             *
 **************************************************/
void PlayerGrounded::Enter() {
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
    
    // Use facing direction as default if no direction input
    if (dash_direction.x == 0.0f && dash_direction.y == 0.0f) {
        dash_direction.x = player->is_facing_right ? 1.0f : -1.0f;
    }
    
    // Normalize and store
    player->dash_direction = Vector2Normalize(dash_direction);
}

/**************************************************
 *              EXIT STATE FUNCTIONS              *
 **************************************************/
void PlayerGrounded::Exit() {}

void PlayerAirborne::Exit() {}

void PlayerDashing::Exit() {}

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

    if (IsKeyDown(player->DASH_KEY) && !player->has_dashed) {
        player->SetState(&player->dashing);
    }
    
    if(!player->IsHittingFloor()) {
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
    
     if (IsKeyDown(player->DASH_KEY) && !player->has_dashed) {
        player->SetState(&player->dashing);
        player->has_dashed = true;
    }
    
    player->acceleration.y = GRAVITY;
    player->velocity.y += player->acceleration.y * delta_time;

    // snap to tile and switch back to grounded on touching a solid tile
    if (player->IsHittingFloor()) {
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
    
    Vector2 dash_movement = Vector2Scale(player->dash_direction, DASH_SPEED * delta_time);
    player->position = Vector2Add(player->position, dash_movement);
    player->velocity = {0.0f, 0.0f}; 
    
    if (player->dash_time >= player->dash_duration) {
        if (player->IsHittingFloor()) {
            player->SetState(&player->grounded);
        } else {
            player->SetState(&player->airborne);
        }
    }
}


/**************************************************
 *             OTHER PLAYER FUNCTIONS             *
 **************************************************/


bool Player::IsHittingFloor() {
    Vector2 feet_position = { position.x + width / 2.0f, position.y + height };
    return GetGrid()->IsSolidTile(feet_position);
}

bool Player::IsHittingCeiling() {
    Vector2 head_position = { position.x + width / 2.0f, position.y };
    return GetGrid()->IsSolidTile(head_position);
}

bool Player::IsHittingWall() {
    Vector2 right_position = { position.x + width, position.y + height / 2.0f };
    Vector2 left_position = { position.x, position.y + height / 2.0f };
    return GetGrid()->IsSolidTile(right_position) || GetGrid()->IsSolidTile(left_position);
}