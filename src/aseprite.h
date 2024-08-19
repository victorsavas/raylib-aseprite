#pragma once

#include <raylib.h>
#include <stdlib.h>

typedef enum AnimDirection
{
	FORWARDS,
	REVERSE
} AnimDirection;

typedef struct Tag
{
	AnimDirection anim_direction;
	int ping_pong;

	int from_frame;
	int to_frame;

	int repeat;

	const char* name;
} Tag;

typedef struct Frame
{
	Texture texture;
	int duration_milliseconds;
} Frame;

typedef struct Aseprite
{
	int loaded;

	Frame* frames;
	int frame_count;

	Tag* tags;
	int tag_count;
} Aseprite;

typedef struct AnimTag
{
	Aseprite ase;

	int loaded;
	int id;

	AnimDirection anim_direction;
	int ping_pong;

	int current_frame;

	int from_frame;
	int to_frame;

	int repeat;
	float speed;
	float timer;

} AnimTag;

// Memory management functions

Aseprite LoadAsepriteFromFile(const char *filename);
void UnloadAseprite(Aseprite ase);

// Static draw functions

void DrawAseprite(Aseprite ase, int frame, float x, float y, Color tint);
void DrawAsepriteV(Aseprite ase, int frame, Vector2 pos, Color tint);
void DrawAsepriteScale(Aseprite ase, int frame, Vector2 pos, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint);

// Animation Tag functions. delta_time parameter must be provided in seconds.

AnimTag CreateAnimTag(Aseprite ase, const char* tag_name);

int AdvanceAnimTag(AnimTag *anim_tag, float delta_time);

void DrawAnim(AnimTag *anim_tag, float delta_time, float x, float y, Color tint);
void DrawAnimV(AnimTag* anim_tag, float delta_time, Vector2 pos, Color tint);
void DrawAnimScale(AnimTag *anim_tag, float delta_time, Vector2 pos, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint);