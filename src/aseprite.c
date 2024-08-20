#include <raylib.h>

#define CUTE_ASEPRITE_IMPLEMENTATION
#include <cute_aseprite.h>

#include <aseprite.h>

static Aseprite _load_aseprite(ase_t *cute_ase, LoadFlags flags);

static void _load_aseprite_frames(ase_t *cute_ase, Aseprite *ase);
static void _load_aseprite_layers(ase_t *cute_ase, Aseprite *Ase);
static void _load_aseprite_tags(ase_t *cute_ase, Aseprite *ase);

static int _aseprite_flags_check(LoadFlags flags, LoadFlags check);

static Animation _create_animation_from_tag(Aseprite ase, Tag tag, int id);
static void _advance_animation_tag_mode(Animation *anim);

#define P_ANIMATION_CHECK(anim) if (anim == NULL) return; if (!anim->ready || !_aseprite_flags_check(anim->ase.flags, ASEPRITE_LOAD_TAGS)) return;

// Memory management functions

Aseprite _load_aseprite(ase_t *cute_ase, LoadFlags flags)
{
	if (cute_ase == NULL || flags == 0)
		return (Aseprite){0};
	
	Aseprite ase = {0};

	ase.flags = flags;

	// Load frames

	_load_aseprite_frames(cute_ase, &ase);

	// Load layers

	if (flags & ASEPRITE_LOAD_LAYERS)
		_load_aseprite_layers(cute_ase, &ase);

	// Load tags

	if (flags & ASEPRITE_LOAD_TAGS)
		_load_aseprite_tags(cute_ase, &ase);

	return ase;
}

void _load_aseprite_frames(ase_t *cute_ase, Aseprite *ase)
{
	Image image;

	int load_texture = ase->flags & ASEPRITE_LOAD_FRAMES;

	if (load_texture)
		image = GenImageColor(cute_ase->w * cute_ase->frame_count, cute_ase->h, BLANK);

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

		ase->frames[i].source = frame_source;
		ase->frames[i].duration_milliseconds = (cute_ase->frames)[i].duration_milliseconds;

		if (!load_texture)
			continue;

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

		ImageDraw(&image, frame, source, frame_source, WHITE);
	}

	if (load_texture)
	{
		ase->frames_texture = LoadTextureFromImage(image);

		UnloadImage(image);
	}
}
void _load_aseprite_layers(ase_t *cute_ase, Aseprite *Ase)
{
	// Load layers
}
void _load_aseprite_tags(ase_t *cute_ase, Aseprite *ase)
{
	ase->tag_count = cute_ase->tag_count;
	ase->tags = (Tag *)malloc(sizeof(Tag) * cute_ase->tag_count);

	for (int i = 0; i < cute_ase->tag_count; i++)
	{
		ase_tag_t tag = (cute_ase->tags)[i];

		ase->tags[i].anim_direction = tag.loop_animation_direction & 1;
		ase->tags[i].ping_pong = (tag.loop_animation_direction & 2) >> 1;

		ase->tags[i].from_frame = tag.from_frame;
		ase->tags[i].to_frame = tag.to_frame;

		ase->tags[i].repeat = tag.repeat;

		ase->tags[i].name = strdup(tag.name);
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
	UnloadTexture(ase.frames_texture);

	free(ase.frames);

	for (int i = 0; i < ase.tag_count; i++)
	{
		Tag tag = ase.tags[i];

		free((void *)tag.name);
	}

	free(ase.tags);
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

	DrawTexturePro(texture, source, dest, pos, rotation, WHITE);
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

Animation _create_animation_from_tag(Aseprite ase, Tag tag, int id)
{
	Animation anim =
	{
		.ase = ase,

		.tag_mode = 1,
		.tag_id = id,

		.ready = 1,

		.anim_direction = tag.anim_direction,
		.ping_pong = tag.ping_pong,
		
		.from_frame = tag.from_frame,
		.to_frame = tag.to_frame,
		
		.repeat = tag.repeat,
		.speed = 1,
		.timer = 0,
		.running = 1
	};

	switch (anim.anim_direction)
	{
		case FORWARDS:
			anim.current_frame = tag.from_frame;
			break;
		case REVERSE:
			anim.current_frame = tag.to_frame;
			break;
	}

	if (tag.repeat == 0)
	{
		anim.loop = 1;
	}

	return anim;
}
Animation CreateSimpleAnimation(Aseprite ase)
{
	return (Animation){
		.ase = ase,
		.ready = 1,

		.current_frame = 0,

		.tag_mode = 0,
		.tag_id = -1,

		.anim_direction = FORWARDS,
		.ping_pong = 0,

		.from_frame = 0,
		.to_frame = 0,

		.repeat = 0,

		.running = 1,
		.speed = 1,
		.timer = 0
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
			return _create_animation_from_tag(ase, current_tag, i);
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

	return _create_animation_from_tag(ase, tag, tag_id);
}

void SetAnimationSpeed(Animation *anim, float speed)
{
	P_ANIMATION_CHECK(anim)
	
	if (speed < 0)
		anim->anim_direction = !anim->anim_direction;

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
	switch (anim->anim_direction)
	{
		case FORWARDS:
			anim->current_frame++;

			if (anim->current_frame > anim->to_frame)
			{
				if (!anim->loop)
					anim->repeat--;

				if (anim->ping_pong)
				{
					anim->anim_direction = REVERSE;

					anim->current_frame -= 2;
				}
				else
					anim->current_frame = anim->from_frame;
			}
			break;
		case REVERSE:
			anim->current_frame--;

			if (anim->current_frame < anim->from_frame)
			{
				if (!anim->loop)
					anim->repeat--;

				if (anim->ping_pong)
				{
					anim->anim_direction = FORWARDS;

					anim->current_frame += 2;
				}
				else
					anim->current_frame = anim->to_frame;
			}
			break;
	}

	if (anim->repeat == 0 && !anim->loop)	// If the tag loops are exhausted
	{
		int next_tag_id = anim->tag_id + 1;
		
		if (next_tag_id >= anim->ase.tag_count)
			next_tag_id = 0;

		anim->tag_id = next_tag_id;

		anim->current_frame = anim->to_frame + 1;

		if (anim->current_frame >= anim->ase.frame_count)
			anim->current_frame = 0;
		
		Tag next_tag = anim->ase.tags[next_tag_id];

		anim->anim_direction = next_tag.anim_direction;
		anim->ping_pong = next_tag.ping_pong;

		anim->from_frame = next_tag.from_frame;
		anim->to_frame = next_tag.to_frame;

		anim->repeat = next_tag.repeat;

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
	
	if (anim->tag_id != -1 && anim->current_frame == anim->from_frame)
	{
		anim->tag_mode = 1;

		switch (anim->anim_direction)
		{
			case FORWARDS:
				anim->current_frame = anim->from_frame;
				break;
			case REVERSE:
				anim->current_frame = anim->to_frame;
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