#pragma once

#include <raylib.h>
#include <stdlib.h>

typedef struct Tag
{
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
	int current_frame;

	int from_frame;
	int to_frame;

	int repeat;
	float timer;
} AnimTag;

Aseprite LoadAsepriteFromFile(const char *filename);
void UnloadAseprite(Aseprite ase);

AnimTag CreateAnimTag(Aseprite ase, const char* tag_name);

void DrawAseprite(Aseprite ase, int frame, float x, float y, Color tint);
void DrawAsepriteScale(Aseprite ase, int frame, Vector2 position, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint);

void DrawAnim(Aseprite ase, AnimTag *anim_tag, float delta_time, float x, float y, Color tint);
void DrawAnimScale(Aseprite ase, AnimTag *anim_tag, float delta_time, Vector2 position, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint);