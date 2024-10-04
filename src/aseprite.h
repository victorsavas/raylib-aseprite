#pragma once

#include <raylib.h>

typedef enum AnimDirection
{
	ASEPRITE_ANIM_FORWARDS,
	ASEPRITE_ANIM_REVERSE
} AnimDirection;

typedef enum LoadFlags
{
	ASEPRITE_LOAD_FRAMES = 1,
	ASEPRITE_LOAD_CELS = 2,
	ASEPRITE_LOAD_LAYERS = 4,
	ASEPRITE_LOAD_TAGS = 8,
} LoadFlags;

// An Aseprite file tag data structure.
typedef struct Tag
{
	int id;
	const char *name;

	Color color;

	AnimDirection anim_direction;
	int ping_pong;

	int from_frame;
	int to_frame;

	int repeat;
	int loop;
} Tag;

typedef struct Cel
{
	int active;

	Texture2D texture;

	float x;
	float y;

	float opacity;
} Cel;

typedef struct Layer
{
	int id;
	const char *name;

	float opacity;
} Layer;

// A single Aseprite file frame data structure.
typedef struct Frame
{
	int id;

	Rectangle source;
	int duration_milliseconds;

	Cel* cels;
	int cel_count;
} Frame;

// Centralized data structure that contains relevant Aseprite file data.
typedef struct Aseprite
{
	LoadFlags flags;

	float width;
	float height;

	Texture2D frames_texture;
	Frame *frames;
	int frame_count;

	Layer *layers;
	int layer_count;

	Tag *tags;
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
	Tag current_tag;
} Animation;

// Load and unload functions.

Aseprite LoadAsepriteFromFile(const char *filename, LoadFlags flags);
Aseprite LoadAsepriteFromMemory(const void *data, int size, LoadFlags flags);
void UnloadAseprite(Aseprite ase);

// Animationless draw functions.

void DrawAseprite(Aseprite ase, int frame, float x, float y, Color tint);
void DrawAsepriteV(Aseprite ase, int frame, Vector2 pos, Color tint);
void DrawAsepriteEx(Aseprite ase, int frame, Vector2 pos, float rotation, float scale, Color tint);
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
void DrawAnimationEx(Animation anim, Vector2 pos, float rotation, float scale, Color tint);
void DrawAnimationScale(Animation anim, Vector2 pos, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint);