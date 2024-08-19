#include <raylib.h>

#define CUTE_ASEPRITE_IMPLEMENTATION
#include <cute_aseprite.h>

#include <aseprite.h>

#define P_ANIMTAG_CHECK(anim_tag) if (anim_tag == NULL) return; if (!anim_tag->ready) return;
#define P_ANIMTAG_CHECK_RETURN(anim_tag, r) if (anim_tag == NULL) return r; if (!anim_tag->ready) return r;

static Aseprite _load_aseprite(ase_t* cute_ase);
static Texture2D _get_frame_texture(Aseprite ase, int frame);

// Memory management functions

Aseprite _load_aseprite(ase_t* cute_ase)
{
	if (cute_ase == NULL)
		return (Aseprite){0};
	
	Aseprite ase = {0};

	// Load frames

	ase.frame_count = cute_ase->frame_count;
	ase.frames = (Frame *)malloc(sizeof(Frame) * cute_ase->frame_count);

	for (int i = 0; i < cute_ase->frame_count; i++)
	{
		Image image =
		{
			.width = cute_ase->w,
			.height = cute_ase->h,
			.mipmaps = 1,
			.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
			.data = (cute_ase->frames)[i].pixels
		};

		Texture frame_texture = LoadTextureFromImage(image);

		ase.frames[i].texture = frame_texture;
		ase.frames[i].duration_milliseconds = (cute_ase->frames)[i].duration_milliseconds;
	}

	// Load tags

	ase.tag_count = cute_ase->tag_count;
	ase.tags = (Tag *)malloc(sizeof(Tag) * cute_ase->tag_count);

	for (int i = 0; i < cute_ase->tag_count; i++)
	{
		ase_tag_t tag = (cute_ase->tags)[i];

		ase.tags[i].anim_direction = tag.loop_animation_direction & 1;
		ase.tags[i].ping_pong = (tag.loop_animation_direction & 2) >> 1;

		ase.tags[i].from_frame = tag.from_frame;
		ase.tags[i].to_frame = tag.to_frame;

		ase.tags[i].repeat = tag.repeat;

		ase.tags[i].name = strdup(tag.name);
	}

	ase.ready = 1;

	return ase;
}

Aseprite LoadAsepriteFromFile(const char *filename)
{
	ase_t *cute_ase = cute_aseprite_load_from_file(filename, NULL);

	Aseprite ase = _load_aseprite(cute_ase);

	cute_aseprite_free(cute_ase);

	return ase;
}
Aseprite LoadAsepriteFromMemory(const void *data, int size)
{
	ase_t *cute_ase = cute_aseprite_load_from_memory(data, size, NULL);

	Aseprite ase = _load_aseprite(cute_ase);

	cute_aseprite_free(cute_ase);

	return ase;
}
void UnloadAseprite(Aseprite ase)
{
	for (int i = 0; i < ase.frame_count; i++)
	{
		Frame frame = ase.frames[i];

		UnloadTexture(frame.texture);
	}

	free(ase.frames);

	for (int i = 0; i < ase.tag_count; i++)
	{
		Tag tag = ase.tags[i];

		free((void *)tag.name);
	}

	free(ase.tags);
}

// Static draw functions

Texture2D _get_frame_texture(Aseprite ase, int frame)
{
	if ((frame >= ase.frame_count) || !(ase.ready))
		return (Texture2D){0};
	
	return ase.frames[frame].texture;
}

void DrawAseprite(Aseprite ase, int frame, float x, float y, Color tint)
{
	Texture2D texture = _get_frame_texture(ase, frame);

	DrawTexture(texture, x, y, tint);
}
void DrawAsepriteV(Aseprite ase, int frame, Vector2 pos, Color tint)
{
	Texture2D texture = _get_frame_texture(ase, frame);

	DrawTextureV(texture, pos, tint);
}
void DrawAsepriteScale(Aseprite ase, int frame, Vector2 position, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint)
{
	Texture2D texture = _get_frame_texture(ase, frame);
	
	Rectangle source = {0, 0, texture.width, texture.height};
	Rectangle dest = {position.x, position.y, texture.width * x_scale, texture.height * y_scale};

	origin.x *= x_scale;
	origin.y *= y_scale;

	DrawTexturePro(texture, source, dest, origin, rotation, tint);
}

// Animation Tag functions

AnimTag CreateAnimTag(Aseprite ase, const char* tag_name)
{
	if (!ase.ready)
		return (AnimTag){0};

	// Please don't name two tags with the same name.

	for (int i = 0; i < ase.tag_count; i++)
	{
		Tag current_tag = ase.tags[i];
		const char *current_tag_name = current_tag.name;

		if (strcmp(tag_name, current_tag_name) == 0)
		{
			AnimTag anim_tag =
			{
				.ase = ase,

				.ready = 1,
				.id = i,

				.anim_direction = current_tag.anim_direction,
				.ping_pong = current_tag.ping_pong,
				
				.from_frame = current_tag.from_frame,
				.to_frame = current_tag.to_frame,
				
				.repeat = current_tag.repeat,
				.speed = 1,
				.timer = 0,
				.running = 1
			};

			switch (anim_tag.anim_direction)
			{
				case FORWARDS:
					anim_tag.current_frame = current_tag.from_frame;
					break;
				case REVERSE:
					anim_tag.current_frame = current_tag.to_frame;
					break;
				default:
					break;
			}
			
			return anim_tag;
		}
	}

	// If no tags are found, an empty tag is returned as default.

	return (AnimTag){0};
}

void SetAnimTagSpeed(AnimTag* anim_tag, float speed)
{
	P_ANIMTAG_CHECK(anim_tag)
	
	if (speed < 0)
		anim_tag->anim_direction = (anim_tag->anim_direction ^ 1) & 1; // bit magic

	anim_tag->speed = speed;
}

void PlayAnimTag(AnimTag* anim_tag)
{
	P_ANIMTAG_CHECK(anim_tag)
	
	anim_tag->running = 1;
}
void StopAnimTag(AnimTag* anim_tag)
{
	P_ANIMTAG_CHECK(anim_tag)
	
	anim_tag->running = 0;
}
void PauseAnimTag(AnimTag* anim_tag)
{
	P_ANIMTAG_CHECK(anim_tag)

	anim_tag->running = (anim_tag->running ^ 1) & 1; // bit magic pt 2
}

int AdvanceAnimTag(AnimTag *anim_tag, float delta_time)
{
	P_ANIMTAG_CHECK_RETURN(anim_tag, -1)

	if (delta_time == 0 || anim_tag->speed == 0 || !anim_tag->running)
		return anim_tag->current_frame;

	Aseprite ase = anim_tag->ase;

	int frame = anim_tag->current_frame;

	float miliseconds = (int)(1000.f * delta_time * anim_tag->speed);
	anim_tag->timer += miliseconds;

	float frame_duration = (float)ase.frames[frame].duration_milliseconds;

	if (anim_tag->timer >= frame_duration)
	{
		anim_tag->timer -= frame_duration;
	
		switch (anim_tag->anim_direction)
		{
			case FORWARDS:
				anim_tag->current_frame++;

				if (anim_tag->current_frame > anim_tag->to_frame)
				{
					if (anim_tag->ping_pong)
					{
						anim_tag->anim_direction = REVERSE;

						anim_tag->current_frame -= 2;
					}
					else
						anim_tag->current_frame = anim_tag->from_frame;
				}
				break;
			case REVERSE:
				anim_tag->current_frame--;

				if (anim_tag->current_frame < anim_tag->from_frame)
				{
					if (anim_tag->ping_pong)
					{
						anim_tag->anim_direction = FORWARDS;

						anim_tag->current_frame += 2;
					}
					else
						anim_tag->current_frame = anim_tag->to_frame;
				}
				break;
			default:
				break;
		}
	}

	return frame;
}

void DrawAnim(AnimTag *anim_tag, float delta_time, float x, float y, Color tint)
{
	P_ANIMTAG_CHECK(anim_tag)

	int frame = AdvanceAnimTag(anim_tag, delta_time);

	DrawAseprite(anim_tag->ase, frame, x, y, tint);
}
void DrawAnimV(AnimTag *anim_tag, float delta_time, Vector2 pos, Color tint)
{
	P_ANIMTAG_CHECK(anim_tag)

	int frame = AdvanceAnimTag(anim_tag, delta_time);

	DrawAsepriteV(anim_tag->ase, frame, pos, tint);
}
void DrawAnimScale(AnimTag *anim_tag, float delta_time, Vector2 position, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint)
{
	P_ANIMTAG_CHECK(anim_tag)

	int frame = AdvanceAnimTag(anim_tag, delta_time);

	DrawAsepriteScale(anim_tag->ase, frame, position, origin, x_scale, y_scale, rotation, tint);
}