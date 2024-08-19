#pragma once

#include <raylib.h>
#include <stdlib.h>

typedef enum AnimDirection
{
	FORWARDS,
	REVERSE
} AnimDirection;

// An Aseprite file tag data structure.
typedef struct Tag
{
	AnimDirection anim_direction;
	int ping_pong;

	int from_frame;
	int to_frame;

	int repeat;

	const char* name;
} Tag;

// A single Aseprite file frame data structure.
typedef struct Frame
{
	Texture texture;
	int duration_milliseconds;
} Frame;

// Centralized data structure that contains relevant Aseprite file data.
typedef struct Aseprite
{
	int ready;

	Frame* frames;
	int frame_count;

	Tag* tags;
	int tag_count;
} Aseprite;

// Data structure for playing animations.
typedef struct Animation
{
	Aseprite ase;
	int ready;

	int current_frame;

	int running;
	float speed;
	float timer;

	// Tag data

	int tag_mode;
	int tag_id;

	AnimDirection anim_direction;
	int ping_pong;

	int from_frame;
	int to_frame;

	int repeat;
	int loop;
} Animation;

// Load and unload functions.

Aseprite LoadAsepriteFromFile(const char *filename);
Aseprite LoadAsepriteFromMemory(const void *data, int size);
void UnloadAseprite(Aseprite ase);

// Animationless draw functions.

void DrawAseprite(Aseprite ase, int frame, float x, float y, Color tint);
void DrawAsepriteV(Aseprite ase, int frame, Vector2 pos, Color tint);
void DrawAsepriteScale(Aseprite ase, int frame, Vector2 pos, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint);

// Animation functions.	

Animation CreateSimpleAnimation(Aseprite ase);
Animation CreateAnimationTag(Aseprite ase, const char *tag_name);
Animation CreateAnimationTagId(Aseprite ase, int tag_id);

void SetAnimationSpeed(Animation *anim, float speed);

void PlayAnimation(Animation *anim);
void StopAnimation(Animation *anim);
void PauseAnimation(Animation *anim);

void AdvanceAnimation(Animation *anim);

void DrawAnimation(Animation anim, float x, float y, Color tint);
void DrawAnimationV(Animation anim, Vector2 pos, Color tint);
void DrawAnimationScale(Animation anim, Vector2 pos, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint);