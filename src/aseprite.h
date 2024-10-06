#pragma once

#include <raylib.h>

typedef enum AseAnimDirection
{
	ASEPRITE_ANIM_FORWARDS,
	ASEPRITE_ANIM_REVERSE
} AseAnimDirection;

typedef enum AseLoadFlags
{
	ASEPRITE_LOAD_FRAMES = 1,
	ASEPRITE_LOAD_CELS = 2,
	ASEPRITE_LOAD_LAYERS = 4,
	ASEPRITE_LOAD_TAGS = 8,
	ASEPRITE_LOAD_PALETTE = 16
} AseLoadFlags;

// An Aseprite file tag data structure.
typedef struct AseTag
{
	int id;
	const char *name;

	Color color;

	AseAnimDirection anim_direction;
	int ping_pong;

	int from_frame;
	int to_frame;

	int repeat;
	int loop;
} AseTag;

typedef struct AseCel
{
	int active;

	Texture2D texture;

	float x;
	float y;

	float opacity;
} AseCel;

typedef struct AseLayer
{
	int id;
	const char *name;

	float opacity;
} AseLayer;

// A single Aseprite file frame data structure.
typedef struct AseFrame
{
	int id;

	Rectangle source;
	int duration_milliseconds;

	AseCel* cels;
	int cel_count;
} AseFrame;

// Centralized data structure that contains relevant Aseprite file data.
typedef struct Aseprite
{
	AseLoadFlags flags;

	float width;
	float height;

	Texture2D frames_texture;
	AseFrame *frames;
	int frame_count;

	AseLayer *layers;
	int layer_count;

	AseTag *tags;
	int tag_count;

	Color *palette;
	int color_count;
} Aseprite;

// Data structure for playing animations.
typedef struct AseAnimTag
{
	Aseprite ase;
	int ready;

	int current_frame;

	int running;
	float speed;
	float timer;

	// Tag data

	int tag_mode;
	AseTag current_tag;
} AseAnimTag;

// Load and unload functions.

Aseprite LoadAsepriteFromFile(const char *filename, AseLoadFlags flags);
Aseprite LoadAsepriteFromMemory(const void *data, int size, AseLoadFlags flags);
void UnloadAseprite(Aseprite ase);

// Animationless draw functions.

void DrawAseprite(Aseprite ase, int frame, float x, float y, Color tint);
void DrawAsepriteV(Aseprite ase, int frame, Vector2 pos, Color tint);
void DrawAsepriteEx(Aseprite ase, int frame, Vector2 pos, float rotation, float scale, Color tint);
void DrawAsepriteScale(Aseprite ase, int frame, Vector2 pos, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint);

// Animation functions.	

AseAnimTag CreateAnimTag(Aseprite ase, const char *tag_name);
AseAnimTag CreateAnimTagId(Aseprite ase, int tag_id);

void SetAnimTagSpeed(AseAnimTag *anim, float speed);

void PlayAnimTag(AseAnimTag *anim);
void StopAnimTag(AseAnimTag *anim);
void PauseAnimTag(AseAnimTag *anim);

void AdvanceAnimTag(AseAnimTag *anim);

void DrawAnimTag(AseAnimTag anim, float x, float y, Color tint);
void DrawAnimTagV(AseAnimTag anim, Vector2 pos, Color tint);
void DrawAnimTagEx(AseAnimTag anim, Vector2 pos, float rotation, float scale, Color tint);
void DrawAnimTagScale(AseAnimTag anim, Vector2 pos, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint);