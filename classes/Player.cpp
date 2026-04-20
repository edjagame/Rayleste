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
    
    if (wall_jump_cooldown_timer > 0.0f) {
        wall_jump_cooldown_timer -= delta_time;
    }
    
    current_state->Update(delta_time);

    if (current_state != &dead && (FloorType() == TileType::SPIKE ||
        CeilingType() == TileType::SPIKE || 
        LeftWallType() == TileType::SPIKE || 
        RightWallType() == TileType::SPIKE)) {
        SetState(&dead);
    }
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

Vector2 Player::StepwiseMove(Vector2 movement) {
    // Movement accumulator
    Vector2 moved = {0.0f, 0.0f};

    // Split horizontal movement into discrete steps and check for 
    // collisions at each step
    if (movement.x != 0.0f) {
        int x_steps = static_cast<int>(ceil(fabs(movement.x)));
        if (x_steps < 1) x_steps = 1;
        float x_step = movement.x / x_steps;

        for (int i = 0; i < x_steps; i++) {
            float previous_x = position.x;
            position.x += x_step;
            
            // checks if moving to the left and hits a solid tile on the left and vice versa
            bool hit_wall = (x_step > 0.0f && RightWallType() == TileType::SOLID) ||
                            (x_step < 0.0f && LeftWallType() == TileType::SOLID);

            if (hit_wall) {
                position.x = round(previous_x);
                velocity.x = 0.0f;
                break;
            }

            moved.x += x_step;
        }
    }

    // Split vertical movement into discrete steps and check for collisions at each step
    if (movement.y != 0.0f) {
        int y_steps = static_cast<int>(ceil(fabs(movement.y)));
        if (y_steps < 1) y_steps = 1;

        float y_step = movement.y / y_steps;
        for (int i = 0; i < y_steps; i++) {
            float previous_y = position.y;
            position.y += y_step;

            bool hit_surface = (y_step > 0.0f && FloorType() == TileType::SOLID) ||
                               (y_step < 0.0f && CeilingType() == TileType::SOLID);
            if (hit_surface) {
                position.y = round(previous_y);
                velocity.y = 0.0f;
                break;
            }

            moved.y += y_step;
        }
    }

    return moved;
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
    player->can_wall_grab = true;
    player->wall_grab_timer = 0.0f;
}

void PlayerAirborne::Enter() {
}

void PlayerDashing::Enter() {
    player->dash_time = 0.0f;
    player->velocity = {0.0f, 0.0f};
    player->PlayDashSound();
    
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
    player->velocity = {0.0f, 0.0f};
}

void PlayerDead::Enter() {
    player->respawn_timer = 0.0f;
    player->color = DEAD_COLOR;
    player->velocity = {0.0f, 0.0f};
    player->PlayDeathSound();
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

    // Moving on ground
    Vector2 ground_movement = {0.0f, 0.0f};
    
    if (IsKeyDown(player->RIGHT_KEY)) {
        player->velocity.x = player->speed;
        player->is_facing_right = true;
    } else if (IsKeyDown(player->LEFT_KEY)) {
        player->velocity.x = -player->speed;
        player->is_facing_right = false;
    } else {
        player->velocity.x = 0.0f;
    }
    
    ground_movement.x = player->velocity.x * delta_time;
    player->StepwiseMove(ground_movement);

    // On jump, set to airborne
    if (IsKeyDown(player->JUMP_KEY)) {
        player->velocity.y = -player->speed * JUMP_MULTIPLIER;
        player->PlayJumpSound();
        player->SetState(&player->airborne);
        return;
    }

    // Check for wall grab input and if player is adjacent to a wall
    bool on_wall = player->IsAdjacentToLeftWall() || player->IsAdjacentToRightWall();
    if (IsKeyDown(player->GRAB_KEY) && on_wall) {
        player->SetState(&player->wall_climbing);
        return;
    }

    // Get to dashing
    if (IsKeyDown(player->DASH_KEY) && !player->has_dashed && player->dash_cooldown_timer <= 0.0f) {
        player->SetState(&player->dashing);
        return;
    }
    
    // Falling off edges
    if(player->FloorType() != TileType::SOLID) {
        player->SetState(&player->airborne);
    }
}

void PlayerAirborne::Update(float delta_time) {
    // Horizontal movement in air
    if (player->wall_jump_cooldown_timer <= 0.0f) {
        if (IsKeyDown(player->RIGHT_KEY)) {
            player->velocity.x = player->speed;
            player->is_facing_right = true;
        } else if (IsKeyDown(player->LEFT_KEY)) {
            player->velocity.x = -player->speed;
            player->is_facing_right = false;
        }
    }
    
    // Check for dash input
    if (IsKeyDown(player->DASH_KEY) && !player->has_dashed && player->dash_cooldown_timer <= 0.0f) {
        player->SetState(&player->dashing);
        player->has_dashed = true;
        return;
    }

    // Apply gravity
    player->acceleration.y = GRAVITY;
    player->velocity.y += player->acceleration.y * delta_time;

    Vector2 air_movement = { player->velocity.x * delta_time, player->velocity.y * delta_time };
    player->StepwiseMove(air_movement);

    // snap to tile and switch back to grounded on touching a solid tile
    if (player->FloorType() == TileType::SOLID) {
        player->velocity.y = 0.0f;
        float tile_h = player->GetGrid()->tile_size_grid.y;
        int row = (int)((player->position.y + player->height) / tile_h);
        player->position.y = row * tile_h - player->height;
        player->wall_jump_cooldown_timer = 0.0f;
        player->SetState(&player->grounded);
        player->has_dashed = false; 
        return;
    }

    // Check for wall grab after movement so wall-jump impulse can separate from the wall
    bool on_left_wall = player->IsAdjacentToLeftWall();
    bool on_right_wall = player->IsAdjacentToRightWall();
    bool on_wall = on_left_wall || on_right_wall;
    if (on_wall && IsKeyDown(player->GRAB_KEY)) {
        player->SetState(&player->wall_climbing);
        return;
    }
}


void PlayerDashing::Update(float delta_time) {
    const float DASH_SPEED = player->speed * DASH_MULTIPLIER; 
    player->dash_time += delta_time;

    // Calculate how far the player should move in a frame
    Vector2 delta_dash = Vector2Scale(player->dash_direction, DASH_SPEED * delta_time);
    player->velocity = {0.0f, 0.0f};
    Vector2 previous_position = player->position;

    player->StepwiseMove(delta_dash);
    
    bool has_collided = Vector2Distance(player->position, Vector2Add(previous_position, delta_dash)) > 0.01f;
    
    // Immediately end dash on collision
    if (has_collided) {
        player->dash_cooldown_timer = DASH_COOLDOWN;
        player->SetState(&player->airborne);
        return;
    }
    
    // End dash after duration
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

    player->wall_grab_timer += delta_time;
    player->can_wall_grab = player->wall_grab_timer < WALL_GRAB_DURATION;

    if (!player->can_wall_grab) {
        player->SetState(&player->airborne);
        return;
    }

    // switch to grounded if on the floor
    if (player->FloorType() == TileType::SOLID && !IsKeyDown(player->UP_KEY)) {
        player->velocity.y = 0.0f;
        player->SetState(&player->grounded);
        player->has_dashed = false;
        return;
    }

    // wall jump input
    if (IsKeyPressed(player->JUMP_KEY)) {
        float wall_jump_x = player->speed * WALL_JUMP_HORIZONTAL_MULTIPLIER;
        if (on_left_wall) {
            player->velocity.x = wall_jump_x;
            player->is_facing_right = true;
        } else {
            player->velocity.x = -wall_jump_x;
            player->is_facing_right = false;
        }

        player->velocity.y = -player->speed * JUMP_MULTIPLIER;
        player->PlayJumpSound();
        player->wall_jump_cooldown_timer = WALL_JUMP_COOLDOWN;
        player->SetState(&player->airborne);
        return;
    }

    // Player lets go of the wall grab or is not on a wall
    if (!on_wall || !IsKeyDown(player->GRAB_KEY)) {
        player->SetState(&player->airborne);
        return;
    }
    
    if (IsKeyDown(player->DASH_KEY) && !player->has_dashed && player->dash_cooldown_timer <= 0.0f) {
        player->SetState(&player->dashing);
        player->has_dashed = true;
        return;
    }

    player->velocity.x = 0.0f;

    if (IsKeyDown(player->UP_KEY)) {
        player->velocity.y = -WALL_CLIMB_SPEED;
    } else if (IsKeyDown(player->DOWN_KEY)) {
        player->velocity.y = WALL_CLIMB_SPEED;
    } else {
        player->velocity.y = 0.0f;
    }

    if (player->CeilingType() == TileType::SOLID && player->velocity.y < 0.0f) {
        player->velocity.y = 0.0f;
    }
    
    Vector2 climb_movement = { 0.0f, player->velocity.y * delta_time };
    player->StepwiseMove(climb_movement);
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

float padding = 3.0f; 

TileType Player::FloorType() {
    Vector2 feet_left = { position.x + padding, position.y + height };
    Vector2 feet_center = { position.x + width / 2.0f, position.y + height };
    Vector2 feet_right = { position.x + width - padding, position.y + height };
    
    if (GetGrid()->GetTileType(feet_left) == TileType::SOLID ||
        GetGrid()->GetTileType(feet_center) == TileType::SOLID ||
        GetGrid()->GetTileType(feet_right) == TileType::SOLID) {
        return TileType::SOLID;
    }
    
    return GetGrid()->GetTileType(feet_center);
}

TileType Player::CeilingType() {
    Vector2 head_left = { position.x + padding, position.y };
    Vector2 head_center = { position.x + width / 2.0f, position.y };
    Vector2 head_right = { position.x + width - padding, position.y };
    
    if (GetGrid()->GetTileType(head_left) == TileType::SOLID ||
        GetGrid()->GetTileType(head_center) == TileType::SOLID ||
        GetGrid()->GetTileType(head_right) == TileType::SOLID) {
        return TileType::SOLID;
    }
    
    return GetGrid()->GetTileType(head_center);
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

/**************************************************
 *             SOUND EFFECT FUNCTIONS             *
 **************************************************/

void Player::PlayJumpSound() {
    if (sound_jump != nullptr && !IsSoundPlaying(*sound_jump)) {
        PlaySound(*sound_jump);
    }
}

void Player::PlayDashSound() {
    if (sound_dash != nullptr && !IsSoundPlaying(*sound_dash)) {
        PlaySound(*sound_dash);
    }
}

void Player::PlayDeathSound() {
    if (sound_death != nullptr && !IsSoundPlaying(*sound_death)) {
        PlaySound(*sound_death);
    }
}