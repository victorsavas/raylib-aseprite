#include <stdlib.h>
#include <raylib.h>

#define CUTE_ASEPRITE_IMPLEMENTATION
#include <cute_aseprite.h>

#include <aseprite.h>

static Aseprite _load_aseprite(ase_t *cute_ase, LoadFlags flags);

static void _load_aseprite_basic(ase_t *cute_ase, Aseprite *ase);
static void _load_aseprite_frames(ase_t *cute_ase, Aseprite *ase);
static void _load_aseprite_layers(ase_t *cute_ase, Aseprite *ase);
static void _load_aseprite_tags(ase_t *cute_ase, Aseprite *ase);

static int _aseprite_flags_check(LoadFlags flags, LoadFlags check);

static Animation _create_animation_from_tag(Aseprite ase, Tag tag);
static void _advance_animation_tag_mode(Animation *anim);

#define P_ANIMATION_CHECK(anim) if (anim == NULL) return; if (!anim->ready || !_aseprite_flags_check(anim->ase.flags, ASEPRITE_LOAD_TAGS)) return;

// Memory management functions

Aseprite _load_aseprite(ase_t *cute_ase, LoadFlags flags)
{
	if (cute_ase == NULL || flags == 0)
		return (Aseprite){0};
	
	Aseprite ase = {0};

	ase.flags = flags;

	_load_aseprite_basic(cute_ase, &ase);

	if (flags & ASEPRITE_LOAD_FRAMES)
		_load_aseprite_frames(cute_ase, &ase);

	if (flags & ASEPRITE_LOAD_LAYERS)
		_load_aseprite_layers(cute_ase, &ase);

	if (flags & ASEPRITE_LOAD_TAGS)
		_load_aseprite_tags(cute_ase, &ase);

	return ase;
}

void _load_aseprite_basic(ase_t *cute_ase, Aseprite *ase)
{
	ase->frame_count = cute_ase->frame_count;
	ase->frames = (Frame *)malloc(sizeof(Frame) * cute_ase->frame_count);

	for (int i = 0; i < cute_ase->frame_count; i++)
	{
		Rectangle frame_source =
		{
			.x = cute_ase->w * i,
			.y = 0,
			.width = cute_ase->w,
			.height = cute_ase->h
		};

		ase->frames[i].id = i;

		ase->frames[i].source = frame_source;
		ase->frames[i].duration_milliseconds = (cute_ase->frames)[i].duration_milliseconds;
	}
}
void _load_aseprite_frames(ase_t *cute_ase, Aseprite *ase)
{
	Image image = GenImageColor(cute_ase->w * cute_ase->frame_count, cute_ase->h, BLANK);

	for (int i = 0; i < cute_ase->frame_count; i++)
	{
		Image frame =
		{
			.width = cute_ase->w,
			.height = cute_ase->h,
			.mipmaps = 1,
			.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
			.data = (cute_ase->frames)[i].pixels
		};

		Rectangle source = 
		{
			.x = 0,
			.y = 0,
			.width = cute_ase->w,
			.height = cute_ase->h
		};

		ImageDraw(&image, frame, source, ase->frames[i].source, WHITE);
	}

	ase->frames_texture = LoadTextureFromImage(image);

	UnloadImage(image);
}
void _load_aseprite_layers(ase_t *cute_ase, Aseprite *ase)
{
	// Load layers

	ase->layers = (Layer *)malloc(cute_ase->layer_count * sizeof(Layer));

	for (int j = 0; j < cute_ase->layer_count; j++)
	{
		ase->layers[j] =
		(Layer){
			.id = j,

			.opacity = cute_ase->layers[j].opacity,

			.name = strdup(cute_ase->layers[j].name),
			.texture_height = j * cute_ase->h
		};
	}

	// Load cels

	Image image = GenImageColor(cute_ase->w * cute_ase->frame_count, cute_ase->h * cute_ase->layer_count, BLANK);

	for (int i = 0; i < cute_ase->frame_count; i++)
	{
		for (int j = 0; j < cute_ase->frames[i].cel_count; j++)
		{
			ase_cel_t cute_cel = cute_ase->frames[i].cels[j];

			Image cel =
			{
				.width = cute_cel.w,
				.height = cute_cel.h,
				.mipmaps = 1,
				.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
				.data = cute_cel.pixels
			};

			Rectangle source =
			{
				.x = 0,
				.y = 0,
				.width = cute_cel.w,
				.height = cute_cel.h
			};

			Rectangle dest =
			{
				.x = cute_ase->w * i + cute_cel.x,
				.y = cute_ase->h * j + cute_cel.y,
				.width = cute_cel.w,
				.height = cute_cel.h
			};

			ImageDraw(&image, cel, source, dest, WHITE);
		}
	}

	ase->layers_texture = LoadTextureFromImage(image);
}
void _load_aseprite_tags(ase_t *cute_ase, Aseprite *ase)
{
	ase->tag_count = cute_ase->tag_count;
	ase->tags = (Tag *)malloc(sizeof(Tag) * cute_ase->tag_count);

	for (int i = 0; i < cute_ase->tag_count; i++)
	{
		ase_tag_t tag = (cute_ase->tags)[i];

		ase->tags[i].id = i;
		ase->tags[i].name = strdup(tag.name);

		ase->tags[i].color =
		(Color){
			.r = tag.r,
			.g = tag.g,
			.b = tag.b,
			.a = 255
		};

		ase->tags[i].anim_direction = tag.loop_animation_direction & 1;
		ase->tags[i].ping_pong = (tag.loop_animation_direction & 2) >> 1;

		ase->tags[i].from_frame = tag.from_frame;
		ase->tags[i].to_frame = tag.to_frame;

		ase->tags[i].repeat = tag.repeat;
		ase->tags[i].loop = !tag.repeat;
	}
}

Aseprite LoadAsepriteFromFile(const char *filename, LoadFlags flags)
{
	if (!FileExists(filename))
		return (Aseprite){0};

	ase_t *cute_ase = cute_aseprite_load_from_file(filename, NULL);

	Aseprite ase = _load_aseprite(cute_ase, flags);

	cute_aseprite_free(cute_ase);

	return ase;
}
Aseprite LoadAsepriteFromMemory(const void *data, int size, LoadFlags flags)
{
	ase_t *cute_ase = cute_aseprite_load_from_memory(data, size, NULL);

	Aseprite ase = _load_aseprite(cute_ase, flags);

	cute_aseprite_free(cute_ase);

	return ase;
}
void UnloadAseprite(Aseprite ase)
{
	if (ase.flags & ASEPRITE_LOAD_FRAMES)
	{
		UnloadTexture(ase.frames_texture);

		free(ase.frames);
	}

	if (ase.flags & ASEPRITE_LOAD_LAYERS)
	{
		UnloadTexture(ase.layers_texture);

		for (int i = 0; i < ase.layer_count; i++)
		{
			Layer layer = ase.layers[i];

			free((void *)layer.name);
		}

		free((void *)ase.layers);
	}

	if (ase.flags & ASEPRITE_LOAD_TAGS)
	{
		for (int i = 0; i < ase.tag_count; i++)
		{
			Tag tag = ase.tags[i];

			free((void *)tag.name);
		}

		free(ase.tags);
	}
}

int _aseprite_flags_check(LoadFlags flags, LoadFlags check)
{
	return (flags & check) == check;
}

// Static draw functions

void DrawAseprite(Aseprite ase, int frame, float x, float y, Color tint)
{
	if (!_aseprite_flags_check(ase.flags, ASEPRITE_LOAD_FRAMES))
		return;

	if (frame >= ase.frame_count)
		return;

	Texture2D texture = ase.frames_texture;
	Rectangle source = ase.frames[frame].source;

	DrawTextureRec(texture, source, (Vector2){x, y}, WHITE);
}
void DrawAsepriteV(Aseprite ase, int frame, Vector2 pos, Color tint)
{
	if (!_aseprite_flags_check(ase.flags, ASEPRITE_LOAD_FRAMES))
		return;

	if (frame >= ase.frame_count)
		return;

	Texture2D texture = ase.frames_texture;
	Rectangle source = ase.frames[frame].source;

	DrawTextureRec(texture, source, pos, WHITE);
}
void DrawAsepriteEx(Aseprite ase, int frame, Vector2 pos, float rotation, float scale, Color tint)
{
	if (!_aseprite_flags_check(ase.flags, ASEPRITE_LOAD_FRAMES))
		return;

	if (frame >= ase.frame_count)
		return;

	Texture2D texture = ase.frames_texture;
	Rectangle source = ase.frames[frame].source;

	Rectangle dest = 
	{
		.x = pos.x,
		.y = pos.y,
		.width = source.width * scale,
		.height = source.height * scale
	};

	DrawTexturePro(texture, source, dest, (Vector2){0,0}, rotation, WHITE);
}
void DrawAsepriteScale(Aseprite ase, int frame, Vector2 position, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint)
{
	if (!_aseprite_flags_check(ase.flags, ASEPRITE_LOAD_FRAMES))
		return;

	if (frame >= ase.frame_count)
		return;

	Texture2D texture = ase.frames_texture;
	Rectangle source = ase.frames[frame].source;

	Rectangle dest = 
	{
		.x = position.x,
		.y = position.y,
		.width = source.width * x_scale,
		.height = source.height * y_scale
	};

	origin.x *= x_scale;
	origin.y *= y_scale;

	DrawTexturePro(texture, source, dest, origin, rotation, WHITE);
}

// Animation Tag functions

Animation _create_animation_from_tag(Aseprite ase, Tag tag)
{
	Animation anim =
	{
		.ase = ase,
		.ready = 1,

		.running = 1,
		.speed = 1,
		.timer = 0,

		.tag_mode = 1,
		.current_tag = tag
	};

	switch (tag.anim_direction)
	{
		case FORWARDS:
			anim.current_frame = tag.from_frame;
			break;
		case REVERSE:
			anim.current_frame = tag.to_frame;
			break;
	}

	return anim;
}
Animation CreateSimpleAnimation(Aseprite ase)
{
	return (Animation){
		.ase = ase,
		.ready = 1,

		.current_frame = 0,

		.running = 1,
		.speed = 1,
		.timer = 0,

		.tag_mode = 0,
		.current_tag = 
		{
			.id = -1,
			.name = NULL,

			.color =
			(Color){
				.r = 0,
				.g = 0,
				.b = 0,
				.a = 255
			},

			.anim_direction = FORWARDS,
			.ping_pong = 0,

			.from_frame = 0,
			.to_frame = 0,

			.repeat = 0,
			.loop = 1
		}
	};
}
Animation CreateAnimationTag(Aseprite ase, const char *tag_name)
{
	if (!_aseprite_flags_check(ase.flags, ASEPRITE_LOAD_TAGS))
		return (Animation){0};

	// Please don't name two tags with the same name.

	for (int i = 0; i < ase.tag_count; i++)
	{
		Tag current_tag = ase.tags[i];
		const char *current_tag_name = current_tag.name;

		if (strcmp(tag_name, current_tag_name) == 0)
		{	
			return _create_animation_from_tag(ase, current_tag);
		}
	}

	// If no tags are found, an empty tag is returned as default.

	return (Animation){0};
}
Animation CreateAnimationTagId(Aseprite ase, int tag_id)
{
	if (!_aseprite_flags_check(ase.flags, ASEPRITE_LOAD_TAGS))
		return (Animation){0};

	Tag tag = ase.tags[tag_id];

	return _create_animation_from_tag(ase, tag);
}

void SetAnimationSpeed(Animation *anim, float speed)
{
	P_ANIMATION_CHECK(anim)
	
	if (speed < 0)
		anim->current_tag.anim_direction = !anim->current_tag.anim_direction;

	anim->speed = speed;
}

void PlayAnimation(Animation *anim)
{
	P_ANIMATION_CHECK(anim)
	
	anim->running = 1;
}
void StopAnimation(Animation *anim)
{
	P_ANIMATION_CHECK(anim)
	
	anim->running = 0;
}
void PauseAnimation(Animation *anim)
{
	P_ANIMATION_CHECK(anim)

	anim->running = !anim->running;
}

void _advance_animation_tag_mode(Animation *anim)
{
	switch (anim->current_tag.anim_direction)
	{
		case FORWARDS:
			anim->current_frame++;

			if (anim->current_frame > anim->current_tag.to_frame)
			{
				if (!anim->current_tag.loop)
					anim->current_tag.repeat--;

				if (anim->current_tag.ping_pong)
				{
					anim->current_tag.anim_direction = REVERSE;

					anim->current_frame -= 2;
				}
				else
					anim->current_frame = anim->current_tag.from_frame;
			}
			break;
		case REVERSE:
			anim->current_frame--;

			if (anim->current_frame < anim->current_tag.from_frame)
			{
				if (!anim->current_tag.loop)
					anim->current_tag.repeat--;

				if (anim->current_tag.ping_pong)
				{
					anim->current_tag.anim_direction = FORWARDS;

					anim->current_frame += 2;
				}
				else
					anim->current_frame = anim->current_tag.to_frame;
			}
			break;
	}

	if (anim->current_tag.repeat == 0 && !anim->current_tag.loop)	// If the tag loops are exhausted
	{
		int next_tag_id = anim->current_tag.id + 1;
		
		if (next_tag_id >= anim->ase.tag_count)
			next_tag_id = 0;

		anim->current_tag.id = next_tag_id;

		anim->current_frame = anim->current_tag.to_frame + 1;

		if (anim->current_frame >= anim->ase.frame_count)
			anim->current_frame = 0;
		
		Tag next_tag = anim->ase.tags[next_tag_id];

		anim->current_tag.anim_direction = next_tag.anim_direction;
		anim->current_tag.ping_pong = next_tag.ping_pong;

		anim->current_tag.from_frame = next_tag.from_frame;
		anim->current_tag.to_frame = next_tag.to_frame;

		anim->current_tag.repeat = next_tag.repeat;

		if (next_tag.from_frame <= anim->current_frame && anim->current_frame <= next_tag.to_frame)
		{
			switch (next_tag.anim_direction)
			{
				case FORWARDS:
					anim->current_frame = next_tag.from_frame;
					break;
				case REVERSE:
					anim->current_frame = next_tag.to_frame;
					break;
			}
		}
		else
			anim->tag_mode = 0;
	}
}

void AdvanceAnimation(Animation *anim)
{
	P_ANIMATION_CHECK(anim)

	float delta_time = GetFrameTime();

	if (delta_time == 0 || anim->speed == 0 || !anim->running)
		return;

	Aseprite ase = anim->ase;

	int frame = anim->current_frame;

	float miliseconds = (int)(1000.f * delta_time * anim->speed);
	anim->timer += miliseconds;

	float frame_duration = (float)ase.frames[frame].duration_milliseconds;

	if (anim->timer < frame_duration)
		return;
	
	anim->timer -= frame_duration;

	if (anim->tag_mode)
	{
		_advance_animation_tag_mode(anim);
		return;
	}

	anim->current_frame++;

	if (anim->current_frame >= anim->ase.frame_count)
		anim->current_frame = 0;
	
	if (anim->current_tag.id != -1 && anim->current_frame == anim->current_tag.from_frame)
	{
		anim->tag_mode = 1;

		switch (anim->current_tag.anim_direction)
		{
			case FORWARDS:
				anim->current_frame = anim->current_tag.from_frame;
				break;
			case REVERSE:
				anim->current_frame = anim->current_tag.to_frame;
				break;
		}
	}
}

void DrawAnimation(Animation anim, float x, float y, Color tint)
{
	DrawAseprite(anim.ase, anim.current_frame, x, y, tint);
}
void DrawAnimationV(Animation anim, Vector2 pos, Color tint)
{
	DrawAsepriteV(anim.ase, anim.current_frame, pos, tint);
}
void DrawAnimationEx(Animation anim, Vector2 pos, float rotation, float scale, Color tint)
{
	DrawAsepriteEx(anim.ase, anim.current_frame, pos, rotation, scale, tint);
}
void DrawAnimationScale(Animation anim, Vector2 position, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint)
{
	DrawAsepriteScale(anim.ase, anim.current_frame, position, origin, x_scale, y_scale, rotation, tint);
}