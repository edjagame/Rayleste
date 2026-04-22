#include <raylib.h>
#include <raymath.h>
#include <iostream>
#include <string>
#include "Player.hpp"
#include "SceneManager.hpp"

const Color PLAYER_COLOR = {255, 255, 255, 255};
const Color DASH_COLOR = {0, 200, 0, 255};
const Color DASHLESS_COLOR = {200, 0, 0, 255};
const Color DEAD_COLOR = {0, 0, 0, 255};

static void DrawPlayerStatusBar(const Player& player);

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
    TryConsumeDashRefill();

    if (current_state != &dead && (FloorType() == TileType::SPIKE ||
        CeilingType() == TileType::SPIKE || 
        LeftWallType() == TileType::SPIKE || 
        RightWallType() == TileType::SPIKE)) {
        SetState(&dead);
    }

    UpdateAnimation(delta_time);
}

void Player::Draw() {
    
    // Loads the appropriate animation based on the current state.
    // THis defaults to a rectangle if ever something goes wrong with loading the animations.
    const PlayerAnimationClip* clip = GetClipByType(current_animation);
    if (clip == nullptr || clip->texture.id == 0) {
        DrawRectangle(position.x, position.y, width, height, color);
        return;
    }
    
    const float frame_size = float(clip->texture.height);
    
    ///////////////////////////////////////////////////////////////
    // This block of code resizes the player sprite and draws it //
    ///////////////////////////////////////////////////////////////
    Rectangle source = {
        frame_size * current_frame,
        0.0f,
        frame_size,
        frame_size
    };

    if (!is_facing_right) {
        source.x += frame_size;
        source.width = -frame_size;
    }

    const float draw_height = height;
    const float draw_width = draw_height;
    Rectangle destination = {
        position.x + (width - draw_width) * 0.5f,
        position.y + (height - draw_height),
        draw_width,
        draw_height
    };

    DrawTexturePro(clip->texture, source, destination, {0.0f, 0.0f}, 0.0f, color);
    DrawPlayerStatusBar(*this);
}

// Called within the state machine to change the current animation based on Player State
void Player::SetAnimation(PlayerAnimationType type) {
    if (current_animation == type) {
        return;
    }

    current_animation = type;
    current_frame = 0;
    animation_timer = 0.0f;
}

// animation clip getter
const Player::PlayerAnimationClip* Player::GetClipByType(PlayerAnimationType type) const {
    switch (type) {
        case PlayerAnimationType::IDLE:
            return &idle_animation;
        case PlayerAnimationType::RUN:
            return &run_animation;
        case PlayerAnimationType::JUMP:
            return &jump_animation;
        case PlayerAnimationType::FALL:
            return &fall_animation;
        case PlayerAnimationType::FALL_POSE:
            return &fall_pose_animation;
        case PlayerAnimationType::DASH:
            return &dash_animation;
        case PlayerAnimationType::DEATH:
            return &death_animation;
        default:
            return nullptr;
    }
}


void Player::UpdateAnimation(float delta_time) {
    const PlayerAnimationClip* clip = GetClipByType(current_animation);

    // switch frames every frame_duration seconds
    animation_timer += delta_time;
    while (animation_timer >= clip->frame_duration) {
        animation_timer -= clip->frame_duration;
        current_frame++;

        if (current_frame >= clip->frame_count) {
            // reset to first frame if looping, otherwise stay on last frame
            if (clip->loop) {
                current_frame = 0;
            } else {
                current_frame = clip->frame_count - 1;
                break;
            }
        }
    }
}

bool Player::LoadAnimationClip(PlayerAnimationClip& clip, const std::string& texture_path, float frame_duration, bool loop) {
    clip.texture = ResourceManager::GetInstance()->GetTexture(texture_path);
    if (clip.texture.id == 0) {
        std::cerr << "Failed to load player animation: " << texture_path << std::endl;
        return false;
    }

    // since sprite sheet is laid out horizontally, frame count is total width divided by height
    clip.frame_count = clip.texture.width / clip.texture.height;
    if (clip.frame_count < 1) {
        clip.frame_count = 1;
    }

    clip.frame_duration = frame_duration;
    clip.loop = loop;
    return true;
}

void Player::UnloadAnimationClip(PlayerAnimationClip& clip) {
    clip.texture = {0};
    clip.frame_count = 1;
    clip.frame_duration = 0.1f;
    clip.loop = true;
}

bool Player::LoadAnimations(
    const std::string& idle_path,
    const std::string& run_path,
    const std::string& jump_path,
    const std::string& fall_path,
    const std::string& fall_pose_path,
    const std::string& dash_path,
    const std::string& death_path
) {
    UnloadAnimations();

    // Makes sure all animations load successfully
    bool success = true;
    success = success && LoadAnimationClip(idle_animation, idle_path, 0.5f, true);
    success = success && LoadAnimationClip(run_animation, run_path, 0.1f, true);
    success = success && LoadAnimationClip(jump_animation, jump_path, 0.1f, false);
    success = success && LoadAnimationClip(fall_animation, fall_path, 0.1f, true);
    success = success && LoadAnimationClip(fall_pose_animation, fall_pose_path, 0.1f, true);
    success = success && LoadAnimationClip(dash_animation, dash_path, 0.05f, false);
    success = success && LoadAnimationClip(death_animation, death_path, 0.2f, false);

    if (!success) {
        UnloadAnimations();
        std::cout << "Failed to load one or more player animations" << std::endl;
        return false;
    }

    current_animation = PlayerAnimationType::IDLE;
    current_frame = 0;
    animation_timer = 0.0f;
    return true;
}

void Player::UnloadAnimations() {
    UnloadAnimationClip(idle_animation);
    UnloadAnimationClip(run_animation);
    UnloadAnimationClip(jump_animation);
    UnloadAnimationClip(fall_animation);
    UnloadAnimationClip(fall_pose_animation);
    UnloadAnimationClip(dash_animation);
    UnloadAnimationClip(death_animation);
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
    player->can_wall_grab = true;
    player->wall_grab_timer = 0.0f;
    player->SetAnimation(Player::PlayerAnimationType::FALL_POSE);
}

void PlayerAirborne::Enter() {
    player->SetAnimation(Player::PlayerAnimationType::JUMP);
}

void PlayerDashing::Enter() {
    player->dash_time = 0.0f;
    player->velocity = {0.0f, 0.0f};
    player->PlayDashSound();
    
    // Determine dash direction at the start
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
    player->SetAnimation(Player::PlayerAnimationType::DASH);
}

void PlayerWallClimbing::Enter() {
    player->velocity = {0.0f, 0.0f};
    player->SetAnimation(Player::PlayerAnimationType::FALL);
}

void PlayerDead::Enter() {
    player->respawn_timer = 0.0f;
    player->velocity = {0.0f, 0.0f};
    (*player->death_counter)++; //spent 30 minutes on this bug
    player->PlayDeathSound();
    player->SetAnimation(Player::PlayerAnimationType::DEATH);
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

    // Set animation to running or idle based on velocity vecotr
    if (fabsf(player->velocity.x) > 1.0f) {
        player->SetAnimation(Player::PlayerAnimationType::RUN);
    } else {
        player->SetAnimation(Player::PlayerAnimationType::IDLE);
    }

    // On jump, set to airborne
    if (IsKeyPressed(player->JUMP_KEY)) {
        player->velocity.y = -player->speed * JUMP_MULTIPLIER;
        player->PlayJumpSound();
        player->SetState(&player->airborne);
        return;
    }

    // Check for wall grab input and if player is adjacent to a wall
    bool on_wall = player->IsAdjacentToLeftWall() || player->IsAdjacentToRightWall();
    if (IsKeyDown(player->GRAB_KEY) && on_wall && player->can_wall_grab) {
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
        } else {
            player->velocity.x = 0.0f;
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
    if (on_wall && IsKeyDown(player->GRAB_KEY) && player->can_wall_grab && player->wall_jump_cooldown_timer <= 0.0f) {
        player->SetState(&player->wall_climbing);
        return;
    }

    // Set animation to jumping or falling based on vertical velocity
    if (player->velocity.y < -80.0f) {
        player->SetAnimation(Player::PlayerAnimationType::JUMP);
    } else {
        player->SetAnimation(Player::PlayerAnimationType::FALL);
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

        // decreases wall grab timer whenever wall jumping
        player->wall_grab_timer = Clamp(player->wall_grab_timer + WALL_JUMP_PENALTY, 0.0f, WALL_GRAB_DURATION);
        player->can_wall_grab = player->wall_grab_timer < WALL_GRAB_DURATION;   

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
        player->GetGrid()->ResetTiles();
        player->position = player->current_respawn_point;
        player->velocity = {0.0f, 0.0f};
        player->SetState(&player->airborne);
    }
}


/**************************************************
 *             OTHER PLAYER FUNCTIONS             *
 **************************************************/

float padding = 3.0f; 

Vector2 Player::GetCenterPosition() const {
    return {
        position.x + width / 2.0f,
        position.y + height / 2.0f
    };
}

TileType Player::CenterTileType() {
    return GetGrid()->GetTileType(GetCenterPosition());
}

void Player::TryConsumeDashRefill() {
    if (!has_dashed) {
        return;
    }

    const Vector2 center = GetCenterPosition();
    
    if (GetGrid()->GetTileType(center) != TileType::DASH_REFILL) {
        return;
    }

    has_dashed = false;
    GetGrid()->SetTileId(center, 0);
}

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

const float wall_check_offset = 2.0f;

TileType Player::LeftWallType() {
    Vector2 left_top = { position.x + wall_check_offset, position.y + padding };
    Vector2 left_middle = { position.x + wall_check_offset, position.y + height / 2.0f };
    Vector2 left_bottom = { position.x + wall_check_offset, position.y + height - padding };
    
    if (GetGrid()->GetTileType(left_top) == TileType::SOLID ||
        GetGrid()->GetTileType(left_middle) == TileType::SOLID ||
        GetGrid()->GetTileType(left_bottom) == TileType::SOLID) {
        return TileType::SOLID;
    }
    
    return GetGrid()->GetTileType(left_middle);
}

TileType Player::RightWallType() {
    Vector2 right_top = { position.x + width - wall_check_offset, position.y + padding };
    Vector2 right_middle = { position.x + width - wall_check_offset, position.y + height / 2.0f };
    Vector2 right_bottom = { position.x + width - wall_check_offset, position.y + height - padding };
    
    if (GetGrid()->GetTileType(right_top) == TileType::SOLID ||
        GetGrid()->GetTileType(right_middle) == TileType::SOLID ||
        GetGrid()->GetTileType(right_bottom) == TileType::SOLID) {
        return TileType::SOLID;
    }
    
    return GetGrid()->GetTileType(right_middle);
}

bool Player::IsAdjacentToLeftWall() {
    Vector2 left_top = { position.x - wall_check_offset, position.y + padding };
    Vector2 left_middle = { position.x - wall_check_offset, position.y + height / 2.0f };
    Vector2 left_bottom = { position.x - wall_check_offset, position.y + height - padding };
    
    return GetGrid()->GetTileType(left_top) == TileType::SOLID ||
           GetGrid()->GetTileType(left_middle) == TileType::SOLID ||
           GetGrid()->GetTileType(left_bottom) == TileType::SOLID;
}

bool Player::IsAdjacentToRightWall() {
    Vector2 right_top = { position.x + width + wall_check_offset, position.y + padding };
    Vector2 right_middle = { position.x + width + wall_check_offset, position.y + height / 2.0f };
    Vector2 right_bottom = { position.x + width + wall_check_offset, position.y + height - padding };
    
    return GetGrid()->GetTileType(right_top) == TileType::SOLID ||
           GetGrid()->GetTileType(right_middle) == TileType::SOLID ||
           GetGrid()->GetTileType(right_bottom) == TileType::SOLID;
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

static void DrawPlayerStatusBar(const Player& player) {
    float bar_width = player.width;
    float bar_height = 5.0f;
    float bar_x = player.position.x;
    float bar_y = player.position.y - 10.0f;
    float wall_grab_ratio = Clamp((WALL_GRAB_DURATION - player.wall_grab_timer) / WALL_GRAB_DURATION, 0.0f, 1.0f);
    Color bar_color = player.has_dashed ? DASHLESS_COLOR : DASH_COLOR;
    Color background_color = {
        static_cast<unsigned char>(bar_color.r * 0.35f),
        static_cast<unsigned char>(bar_color.g * 0.35f),
        static_cast<unsigned char>(bar_color.b * 0.35f),
        bar_color.a
    };

    //draws the background bar first then the foreground bar after
    DrawRectangleRec({bar_x, bar_y, bar_width, bar_height}, background_color);
    DrawRectangleRec({bar_x, bar_y, bar_width * wall_grab_ratio, bar_height}, bar_color);
}