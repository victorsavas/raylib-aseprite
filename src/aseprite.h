#pragma once

#include <raylib.h>
#include <stdlib.h>

typedef enum AnimDirection
{
	FORWARDS,
	REVERSE,
	PINGPONG,
	REVERSEPINGPONG
} AnimDirection;

typedef struct Tag
{
	AnimDirection anim_direction;

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
	Frame* frames;
	int frame_count;

	Tag* tags;
	int tag_count;
} Aseprite;

typedef struct AnimTag
{
	int id;

	AnimDirection anim_direction;
	AnimDirection current_direction;

	int current_frame;

	int from_frame;
	int to_frame;

	int frame_count;

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

// Animation Tag functions

AnimTag CreateAnimTag(Aseprite ase, const char* tag_name);

void DrawAnim(Aseprite ase, AnimTag *anim_tag, float delta_time, float x, float y, Color tint);
void DrawAnimV(Aseprite ase, AnimTag* anim_tag, float delta_time, Vector2 pos, Color tint);
void DrawAnimScale(Aseprite ase, AnimTag *anim_tag, float delta_time, Vector2 pos, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint);