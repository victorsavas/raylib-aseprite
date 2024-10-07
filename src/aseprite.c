#include <stdlib.h>
#include <raylib.h>

#define CUTE_ASEPRITE_IMPLEMENTATION
#include <cute_aseprite.h>

#include <aseprite.h>

static Aseprite _load_aseprite(ase_t *cute_ase, AseLoadFlags flags);

static void _load_aseprite_frames(ase_t *cute_ase, Aseprite *ase);
static void _load_aseprite_layers(ase_t *cute_ase, Aseprite *ase);
static void _load_aseprite_tags(ase_t *cute_ase, Aseprite *ase);
static void _load_aseprite_palette(ase_t *cute_ase, Aseprite *ase);

static int _aseprite_flags_check(AseLoadFlags flags, AseLoadFlags check);

static AseAnimation _create_animation_from_tag(Aseprite *ase, AseTag tag);
static void _advance_animation_tag_mode(AseAnimation *anim);

#define P_ANIMATION_CHECK(anim) if (anim == NULL) return; \
								if (!anim->ready || anim->ase == NULL) return; \
								if (!_aseprite_flags_check(anim->ase->flags, ASEPRITE_LOAD_TAGS)) return;

#define ANIMATION_CHECK(anim) 	if (!anim.ready || anim.ase == NULL) return; \
								if (!_aseprite_flags_check(anim.ase->flags, ASEPRITE_LOAD_TAGS)) return;

// Memory management functions

Aseprite _load_aseprite(ase_t *cute_ase, AseLoadFlags flags)
{
	if (cute_ase == NULL || flags == 0)
		return (Aseprite){0};
	
	Aseprite ase = {0};

	ase.flags = flags;

	ase.width = cute_ase->w;
	ase.height = cute_ase->h;

	if (flags & ASEPRITE_LOAD_FRAMES)
		_load_aseprite_frames(cute_ase, &ase);

	if (flags & ASEPRITE_LOAD_LAYERS)
		_load_aseprite_layers(cute_ase, &ase);

	if (flags & ASEPRITE_LOAD_TAGS)
		_load_aseprite_tags(cute_ase, &ase);

	if (flags & ASEPRITE_LOAD_PALETTE)
		_load_aseprite_palette(cute_ase, &ase);

	return ase;
}

void _load_aseprite_frames(ase_t *cute_ase, Aseprite *ase)
{
	ase->frame_count = cute_ase->frame_count;
	ase->frames = (AseFrame *)malloc(sizeof(AseFrame) * cute_ase->frame_count);

	Image image = GenImageColor(cute_ase->w * cute_ase->frame_count, cute_ase->h, BLANK);

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

		Image frame_image =
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

		ImageDraw(&image, frame_image, source, ase->frames[i].source, WHITE);
	}

	ase->frames_texture = LoadTextureFromImage(image);

	UnloadImage(image);
}
void _load_aseprite_layers(ase_t *cute_ase, Aseprite *ase)
{
	ase->layer_count = cute_ase->layer_count;
	ase->layers = (AseLayer *)malloc(cute_ase->layer_count * sizeof(AseLayer));

	ase->layer_cel_count = cute_ase->frame_count;

	for (int j = 0; j < cute_ase->layer_count; j++)
	{
		AseLayer *layer = &ase->layers[j];
		
		*layer = (AseLayer){
			.id = j,
			.name = strdup(cute_ase->layers[j].name),

			.opacity = cute_ase->layers[j].opacity,

			.cels = calloc(cute_ase->frame_count, sizeof(AseCel))
		};
	}

	for (int i = 0; i < ase->frame_count; i++)
	{
		ase_frame_t *cute_frame = &cute_ase->frames[i];

		for (int k = 0; k < cute_frame->cel_count; k++)
		{
			ase_cel_t cute_cel = cute_frame->cels[k];

			int j = cute_cel.layer - cute_ase->layers;

			AseCel cel;

			cel.active = 1;

			cel.opacity = cute_cel.opacity;

			Image image = {
				.width = cute_cel.w,
				.height = cute_cel.h,
				.mipmaps = 1,
				.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
				.data = cute_cel.pixels
			};

			cel.texture = LoadTextureFromImage(image);

			float x_offset = cute_cel.x;
			float y_offset = cute_cel.y;

			Rectangle visible_area =
			{
				0, 0, cute_cel.w, cute_cel.h
			};

			if (x_offset < 0)
			{
				x_offset = 0;

				visible_area.x = -cute_cel.x;
				visible_area.width += cute_cel.x;
			}

			if (y_offset < 0)
			{
				y_offset = 0;

				visible_area.y = -cute_cel.y;
				visible_area.height += cute_cel.y;
			}

			if (x_offset + visible_area.width > cute_ase->w)
			{
				visible_area.width = cute_ase->w - x_offset;
			}

			if (y_offset + visible_area.height > cute_ase->h)
			{
				visible_area.height = cute_ase->h - y_offset;
			}

			cel.x_offset = x_offset;
			cel.y_offset = y_offset;

			cel.visible_area = visible_area;

			ase->layers[j].cels[i] = cel;
		}
	}
}
void _load_aseprite_tags(ase_t *cute_ase, Aseprite *ase)
{
	ase->tag_count = cute_ase->tag_count;
	ase->tags = (AseTag *)malloc(sizeof(AseTag) * cute_ase->tag_count);

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
void _load_aseprite_palette(ase_t *cute_ase, Aseprite *ase)
{
	ase->color_count = cute_ase->palette.entry_count;
	ase->palette = (Color *)malloc(ase->color_count * sizeof(Color));

	for (int u = 0; u < ase->color_count; u++)
	{
		Color *color = &ase->palette[u];
		ase_color_t cute = cute_ase->palette.entries[u].color;

		color->r = cute.r;
		color->g = cute.g;
		color->b = cute.b;
		color->a = cute.a;
	}
}

Aseprite LoadAsepriteFromFile(const char *filename, AseLoadFlags flags)
{
	if (!FileExists(filename))
		return (Aseprite){0};

	ase_t *cute_ase = cute_aseprite_load_from_file(filename, NULL);

	Aseprite ase = _load_aseprite(cute_ase, flags);

	cute_aseprite_free(cute_ase);

	return ase;
}
Aseprite LoadAsepriteFromMemory(const void *data, int size, AseLoadFlags flags)
{
	ase_t *cute_ase = cute_aseprite_load_from_memory(data, size, NULL);

	Aseprite ase = _load_aseprite(cute_ase, flags);

	cute_aseprite_free(cute_ase);

	return ase;
}
void UnloadAseprite(Aseprite *ase)
{
	if (ase == NULL)
		return;

	if (ase->flags & ASEPRITE_LOAD_FRAMES)
	{
		UnloadTexture(ase->frames_texture);
	}

	if (ase->flags & ASEPRITE_LOAD_LAYERS)
	{
		for (int j = 0; j < ase->layer_count; j++)
		{
			AseLayer layer = ase->layers[j];

			free((void *)layer.name);

			for (int i = 0; i < ase->layer_cel_count; i++)
			{
				AseCel cel = layer.cels[i];

				UnloadTexture(cel.texture);
			}

			free((void *)layer.cels);
		}

		free((void *)ase->layers);
	}

	if (ase->flags & ASEPRITE_LOAD_TAGS)
	{
		for (int i = 0; i < ase->tag_count; i++)
		{
			AseTag tag = ase->tags[i];

			free((void *)tag.name);
		}

		free((void *)ase->tags);
	}

	if (ase->flags & ASEPRITE_LOAD_PALETTE)
	{
		free((void *)ase->palette);
	}

	free((void *)ase->frames);

	*ase = (Aseprite){0};
}

int IsAsepriteReady(Aseprite ase)
{
	return ase.flags;
}

int _aseprite_flags_check(AseLoadFlags flags, AseLoadFlags check)
{
	return (flags & check) == check;
}

// Motionless draw functions

void DrawFrame(Aseprite ase, int frame, float x, float y, Color tint)
{
	if (!(ase.flags & ASEPRITE_LOAD_FRAMES))
		return;

	if (frame < 0 || frame >= ase.frame_count)
		return;

	Texture2D texture = ase.frames_texture;
	Rectangle source = ase.frames[frame].source;

	DrawTextureRec(texture, source, (Vector2){x, y}, tint);
}
void DrawFrameV(Aseprite ase, int frame, Vector2 position, Color tint)
{
	if (!(ase.flags & ASEPRITE_LOAD_FRAMES))
		return;

	if (frame < 0 || frame >= ase.frame_count)
		return;

	Texture2D texture = ase.frames_texture;
	Rectangle source = ase.frames[frame].source;

	DrawTextureRec(texture, source, position, tint);
}
void DrawFrameEx(Aseprite ase, int frame, Vector2 position, float scale, float rotation, Color tint)
{
	if (!(ase.flags & ASEPRITE_LOAD_FRAMES))
		return;

	if (frame < 0 || frame >= ase.frame_count)
		return;

	Texture2D texture = ase.frames_texture;
	Rectangle source = ase.frames[frame].source;

	if (scale < 0)
	{
		source.width *= -1;
		source.height *= -1;
	}

	Rectangle dest = 
	{
		.x = position.x,
		.y = position.y,
		.width = source.width * scale,
		.height = source.height * scale
	};

	DrawTexturePro(texture, source, dest, (Vector2){0,0}, rotation, tint);
}
void DrawFrameScale(Aseprite ase, int frame, Vector2 position, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint)
{
	if (!(ase.flags & ASEPRITE_LOAD_FRAMES))
		return;

	if (frame < 0 || frame >= ase.frame_count)
		return;

	Texture2D texture = ase.frames_texture;
	Rectangle source = ase.frames[frame].source;

	origin.x *= x_scale;
	origin.y *= y_scale;

	if (x_scale < 0)
	{
		source.width *= -1;
		origin.x *= -1;
	}
	
	if (y_scale < 0)
	{
		source.height *= -1;
		origin.y *= -1;
	}

	Rectangle dest = 
	{
		.x = position.x,
		.y = position.y,
		.width = source.width * x_scale,
		.height = source.height * y_scale
	};

	DrawTexturePro(texture, source, dest, origin, rotation, tint);
}

void DrawCel(Aseprite ase, int layer, int frame, float x, float y, Color tint)
{
	if (!(ase.flags & ASEPRITE_LOAD_LAYERS))
		return;

	if (layer < 0 || layer >= ase.layer_count)
		return;

	if (frame < 0 || frame >= ase.layer_cel_count)
		return;
	
	AseLayer ase_layer = ase.layers[layer];
	AseCel cel = ase_layer.cels[frame];

	if (!cel.active)
		return;

	Texture2D texture = cel.texture;
	Rectangle source = cel.visible_area;
	Vector2 position = {cel.x_offset + x, cel.y_offset + y};
	tint.a *= ase_layer.opacity * cel.opacity;

	DrawTextureRec(texture, source, position, tint);
}
void DrawCelV(Aseprite ase, int layer, int frame, Vector2 position, Color tint)
{
	if (!(ase.flags & ASEPRITE_LOAD_LAYERS))
		return;

	if (layer < 0 || layer >= ase.layer_count)
		return;

	if (frame < 0 || frame >= ase.layer_cel_count)
		return;
	
	AseLayer ase_layer = ase.layers[layer];
	AseCel cel = ase_layer.cels[frame];

	if (!cel.active)
		return;

	Texture2D texture = cel.texture;
	Rectangle source = cel.visible_area;

	position.x += cel.x_offset;
	position.y += cel.y_offset;

	tint.a *= ase_layer.opacity * cel.opacity;

	DrawTextureRec(texture, source, position, tint);
}
void DrawCelEx(Aseprite ase, int layer, int frame, Vector2 position, float scale, float rotation, Color tint)
{
	if (!(ase.flags & ASEPRITE_LOAD_LAYERS))
		return;

	if (layer < 0 || layer >= ase.layer_count)
		return;

	if (frame < 0 || frame >= ase.layer_cel_count)
		return;
	
	AseLayer ase_layer = ase.layers[layer];
	AseCel cel = ase_layer.cels[frame];

	if (!cel.active)
		return;

	Texture2D texture = cel.texture;
	Rectangle source = cel.visible_area;

	Vector2 origin;

	if (scale < 0)
	{
		origin.x = (ase.width - cel.x_offset - source.width) * scale;
		origin.y = (ase.height - cel.y_offset - source.height) * scale;

		source.width *= -1;
		source.height *= -1;
	}
	else
	{
		origin.x = -cel.x_offset * scale;
		origin.y = -cel.y_offset * scale;
	}

	Rectangle dest = 
	{
		.x = position.x,
		.y = position.y,
		.width = source.width * scale,
		.height = source.height * scale
	};

	tint.a *= ase_layer.opacity * cel.opacity;

	DrawTexturePro(texture, source, dest, origin, rotation, tint);
}
void DrawCelScale(Aseprite ase, int layer, int frame, Vector2 position, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint)
{
	if (!(ase.flags & ASEPRITE_LOAD_LAYERS))
		return;

	if (layer < 0 || layer >= ase.layer_count)
		return;

	if (frame < 0 || frame >= ase.layer_cel_count)
		return;
	
	AseLayer ase_layer = ase.layers[layer];
	AseCel cel = ase_layer.cels[frame];

	if (!cel.active)
		return;

	Texture2D texture = cel.texture;
	Rectangle source = cel.visible_area;

	origin.x *= x_scale;

	if (x_scale < 0)
	{

		origin.x = -origin.x + (ase.width - cel.x_offset - source.width) * x_scale;

		source.width *= -1;
	}
	else
	{
		origin.x += -cel.x_offset * x_scale;
	}

	origin.y *= y_scale;

	if (y_scale < 0)
	{
		origin.y = -origin.y + (ase.height - cel.y_offset - source.height) * y_scale;

		source.height *= -1;
	}
	else
	{
		origin.y += -cel.y_offset * y_scale;
	}

	Rectangle dest = 
	{
		.x = position.x,
		.y = position.y,
		.width = source.width * x_scale,
		.height = source.height * y_scale
	};

	tint.a *= ase_layer.opacity * cel.opacity;

	DrawTexturePro(texture, source, dest, origin, rotation, tint);
}

// Animation Tag functions

AseAnimation _create_animation_from_tag(Aseprite *ase, AseTag tag)
{
	AseAnimation anim =
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
		case ASEPRITE_ANIM_FORWARDS:
			anim.current_frame = tag.from_frame;
			break;
		case ASEPRITE_ANIM_REVERSE:
			anim.current_frame = tag.to_frame;
			break;
	}

	return anim;
}

AseAnimation CreateSimpleAnimation(Aseprite *ase)
{
	if (ase == NULL)
		return (AseAnimation){0};
	
	AseAnimation anim =
	{
		.ase = ase,
		.ready = 1,

		.current_frame = 0,

		.running = 1,
		.speed = 1,
		.timer = 0,

		.tag_mode = 0,
		.current_tag = {0}
	};

	return anim;
}
AseAnimation CreateAnimationTag(Aseprite *ase, const char *tag_name)
{
	if (ase == NULL)
		return (AseAnimation){0};

	if (!_aseprite_flags_check(ase->flags, ASEPRITE_LOAD_TAGS))
		return (AseAnimation){0};

	// Please don't name two tags with the same name.

	for (int i = 0; i < ase->tag_count; i++)
	{
		AseTag current_tag = ase->tags[i];
		const char *current_tag_name = current_tag.name;

		// Is this safe?

		if (strcmp(tag_name, current_tag_name) == 0)
		{	
			return _create_animation_from_tag(ase, current_tag);
		}
	}

	// If no tags are found, an empty tag is returned as default.

	return (AseAnimation){0};
}
AseAnimation CreateAnimationTagId(Aseprite *ase, int tag_id)
{
	if (ase == NULL)
		return (AseAnimation){0};

	if (!_aseprite_flags_check(ase->flags, ASEPRITE_LOAD_TAGS))
		return (AseAnimation){0};

	AseTag tag = ase->tags[tag_id];

	return _create_animation_from_tag(ase, tag);
}

int IsAnimationReady(AseAnimation anim)
{
	if (anim.ase == NULL)
		return 0;

	return anim.ready;
}

void SetAnimationSpeed(AseAnimation *anim, float speed)
{
	P_ANIMATION_CHECK(anim)
	
	if (speed < 0)
		anim->current_tag.anim_direction = !anim->current_tag.anim_direction;

	anim->speed = speed;
}

void PlayAnimation(AseAnimation *anim)
{
	P_ANIMATION_CHECK(anim)

	anim->running = 1;
}
void StopAnimation(AseAnimation *anim)
{
	P_ANIMATION_CHECK(anim)

	anim->running = 0;
}
void PauseAnimation(AseAnimation *anim)
{
	P_ANIMATION_CHECK(anim)

	anim->running = !anim->running;
}

void _advance_animation_tag_mode(AseAnimation *anim)
{
	switch (anim->current_tag.anim_direction)
	{
		case ASEPRITE_ANIM_FORWARDS:
			anim->current_frame++;

			if (anim->current_frame > anim->current_tag.to_frame)
			{
				if (!anim->current_tag.loop)
					anim->current_tag.repeat--;

				if (anim->current_tag.ping_pong)
				{
					anim->current_tag.anim_direction = ASEPRITE_ANIM_REVERSE;

					anim->current_frame -= 2;
				}
				else
					anim->current_frame = anim->current_tag.from_frame;
			}
			break;
		case ASEPRITE_ANIM_REVERSE:
			anim->current_frame--;

			if (anim->current_frame < anim->current_tag.from_frame)
			{
				if (!anim->current_tag.loop)
					anim->current_tag.repeat--;

				if (anim->current_tag.ping_pong)
				{
					anim->current_tag.anim_direction = ASEPRITE_ANIM_FORWARDS;

					anim->current_frame += 2;
				}
				else
					anim->current_frame = anim->current_tag.to_frame;
			}
			break;
	}

	if (anim->current_tag.repeat == 0 && !anim->current_tag.loop)	// If the tag loops are exhausted
	{
		Aseprite ase = *anim->ase;

		int next_tag_id = anim->current_tag.id + 1;
		
		if (next_tag_id >= ase.tag_count)
			next_tag_id = 0;

		anim->current_tag.id = next_tag_id;

		anim->current_frame = anim->current_tag.to_frame + 1;

		if (anim->current_frame >= ase.frame_count)
			anim->current_frame = 0;
		
		AseTag next_tag = ase.tags[next_tag_id];

		anim->current_tag.anim_direction = next_tag.anim_direction;
		anim->current_tag.ping_pong = next_tag.ping_pong;

		anim->current_tag.from_frame = next_tag.from_frame;
		anim->current_tag.to_frame = next_tag.to_frame;

		anim->current_tag.repeat = next_tag.repeat;

		if (next_tag.from_frame <= anim->current_frame && anim->current_frame <= next_tag.to_frame)
		{
			switch (next_tag.anim_direction)
			{
				case ASEPRITE_ANIM_FORWARDS:
					anim->current_frame = next_tag.from_frame;
					break;
				case ASEPRITE_ANIM_REVERSE:
					anim->current_frame = next_tag.to_frame;
					break;
			}
		}
		else
			anim->tag_mode = 0;
	}
}

void AdvanceAnimation(AseAnimation *anim)
{
	P_ANIMATION_CHECK(anim)

	float delta_time = GetFrameTime();

	if (delta_time == 0 || anim->speed == 0 || !anim->running)
		return;

	Aseprite ase = *anim->ase;

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

	if (anim->current_frame >= ase.frame_count)
		anim->current_frame = 0;
	
	if (anim->current_tag.id != -1 && anim->current_frame == anim->current_tag.from_frame)
	{
		anim->tag_mode = 1;

		switch (anim->current_tag.anim_direction)
		{
			case ASEPRITE_ANIM_FORWARDS:
				anim->current_frame = anim->current_tag.from_frame;
				break;
			case ASEPRITE_ANIM_REVERSE:
				anim->current_frame = anim->current_tag.to_frame;
				break;
		}
	}
}

void DrawAnimation(AseAnimation anim, float x, float y, Color tint)
{
	ANIMATION_CHECK(anim)
	
	DrawFrame(*anim.ase, anim.current_frame, x, y, tint);
}
void DrawAnimationV(AseAnimation anim, Vector2 position, Color tint)
{
	ANIMATION_CHECK(anim)
	
	DrawFrameV(*anim.ase, anim.current_frame, position, tint);
}
void DrawAnimationEx(AseAnimation anim, Vector2 position, float rotation, float scale, Color tint)
{
	ANIMATION_CHECK(anim)
	
	DrawFrameEx(*anim.ase, anim.current_frame, position, rotation, scale, tint);
}
void DrawAnimationScale(AseAnimation anim, Vector2 position, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint)
{
	ANIMATION_CHECK(anim)

	DrawFrameScale(*anim.ase, anim.current_frame, position, origin, x_scale, y_scale, rotation, tint);
}

void DrawAnimLayer(AseAnimation anim, int layer, float x, float y, Color tint)
{
	ANIMATION_CHECK(anim)
	
	DrawCel(*anim.ase, layer, anim.current_frame, x, y, tint);
}
void DrawAnimLayerV(AseAnimation anim, int layer, Vector2 position, Color tint)
{
	ANIMATION_CHECK(anim)
	
	DrawCelV(*anim.ase, layer, anim.current_frame, position, tint);
}
void DrawAnimLayerEx(AseAnimation anim, int layer, Vector2 position, float scale, float rotation, Color tint)
{
	ANIMATION_CHECK(anim)
	
	DrawCelEx(*anim.ase, layer, anim.current_frame, position, scale, rotation, tint);
}
void DrawAnimLayerScale(AseAnimation anim, int layer, Vector2 position, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint)
{
	ANIMATION_CHECK(anim)
	
	DrawCelScale(*anim.ase, layer, anim.current_frame, position, origin, x_scale, y_scale, rotation, tint);
}