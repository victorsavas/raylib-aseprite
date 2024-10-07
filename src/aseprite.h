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
	ASEPRITE_LOAD_LAYERS = 2,
	ASEPRITE_LOAD_TAGS = 4,
	ASEPRITE_LOAD_PALETTE = 8,
	ASEPRITE_LOAD_ALL = 15
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

	float x_offset;
	float y_offset;

	Rectangle visible_area;

	float opacity;
} AseCel;

typedef struct AseLayer
{
	int id;
	const char *name;

	float opacity;

	AseCel *cels;

} AseLayer;

// A single Aseprite file frame data structure.
typedef struct AseFrame
{
	int id;

	Rectangle source;
	int duration_milliseconds;

} AseFrame;

// Centralized data structure that contains relevant Aseprite file data.
typedef struct Aseprite
{
	AseLoadFlags flags;

	int width;
	int height;

	Texture2D frames_texture;
	AseFrame *frames;
	int frame_count;

	AseLayer *layers;
	int layer_count;
	int layer_cel_count;

	AseTag *tags;
	int tag_count;

	Color *palette;
	int color_count;
} Aseprite;

// Data structure for playing animations.
typedef struct AseAnimation
{
	Aseprite *ase;
	int ready;

	int current_frame;

	int running;
	float speed;
	float timer;

	// Tag data

	int tag_mode;
	AseTag current_tag;
} AseAnimation;

// Load functions.

Aseprite LoadAsepriteFromFile(const char *filename, AseLoadFlags flags);
Aseprite LoadAsepriteFromMemory(const void *data, int size, AseLoadFlags flags);
void UnloadAseprite(Aseprite *ase);

int IsAsepriteReady(Aseprite ase);

// Motionless draw functions.

void DrawFrame(Aseprite ase, int frame, float x, float y, Color tint);
void DrawFrameV(Aseprite ase, int frame, Vector2 position, Color tint);
void DrawFrameEx(Aseprite ase, int frame, Vector2 position, float scale, float rotation, Color tint);
void DrawFrameScale(Aseprite ase, int frame, Vector2 position, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint);

void DrawCel(Aseprite ase, int layer, int frame, float x, float y, Color tint);
void DrawCelV(Aseprite ase, int layer, int frame, Vector2 position, Color tint);
void DrawCelEx(Aseprite ase, int layer, int frame, Vector2 position, float scale, float rotation, Color tint);
void DrawCelScale(Aseprite ase, int layer, int frame, Vector2 position, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint);

// Animation functions.	

AseAnimation CreateSimpleAnimation(Aseprite *ase);
AseAnimation CreateAnimationTag(Aseprite *ase, const char *tag_name);
AseAnimation CreateAnimationTagId(Aseprite *ase, int tag_id);

int IsAnimationReady(AseAnimation anim);

void SetAnimationSpeed(AseAnimation *anim, float speed);

void PlayAnimation(AseAnimation *anim);
void StopAnimation(AseAnimation *anim);
void PauseAnimation(AseAnimation *anim);

void AdvanceAnimation(AseAnimation *anim);

void DrawAnimation(AseAnimation anim, float x, float y, Color tint);
void DrawAnimationV(AseAnimation anim, Vector2 position, Color tint);
void DrawAnimationEx(AseAnimation anim, Vector2 position, float scale, float rotation, Color tint);
void DrawAnimationScale(AseAnimation anim, Vector2 position, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint);

void DrawAnimLayer(AseAnimation anim, int layer, float x, float y, Color tint);
void DrawAnimLayerV(AseAnimation anim, int layer, Vector2 position, Color tint);
void DrawAnimLayerEx(AseAnimation anim, int layer, Vector2 position, float scale, float rotation, Color tint);
void DrawAnimLayerScale(AseAnimation anim, int layer, Vector2 position, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint);