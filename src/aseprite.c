#include <raylib.h>

#define CUTE_ASEPRITE_IMPLEMENTATION
#include <cute_aseprite.h>

#include <aseprite.h>

static Texture2D _get_frame_texture(Aseprite ase, int frame);
static int _advance_animtag(AnimTag *anim_tag, float delta_time, Aseprite ase);

// Memory management functions

Aseprite LoadAsepriteFromFile(const char *filename)
{
	ase_t *cute_ase = cute_aseprite_load_from_file(filename, NULL);

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

		ase.tags[i].anim_direction = tag.loop_animation_direction; // Caution: the enum types might not be compatible in the future

		ase.tags[i].from_frame = tag.from_frame;
		ase.tags[i].to_frame = tag.to_frame;

		ase.tags[i].repeat = tag.repeat;

		ase.tags[i].name = strdup(tag.name);
	}

	// End

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
	if (frame >= ase.frame_count)
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
	// Please don't name two tags with the same name.

	for (int i = 0; i < ase.tag_count; i++)
	{
		Tag current_tag = ase.tags[i];
		const char *current_tag_name = current_tag.name;

		if (strcmp(tag_name, current_tag_name) == 0)
		{
			AnimTag anim_tag =
			{
				.id = i,
				.anim_direction = current_tag.anim_direction,
				.current_direction = current_tag.anim_direction,
				.from_frame = current_tag.from_frame,
				.to_frame = current_tag.to_frame,
				.frame_count = current_tag.to_frame - current_tag.from_frame + 1,
				.repeat = current_tag.repeat,
				.timer = 0
			};

			switch (current_tag.anim_direction)
			{
				case FORWARDS:
					anim_tag.current_frame = current_tag.from_frame;
					break;

				case REVERSE:
					anim_tag.current_frame = current_tag.to_frame;
					break;

				case PINGPONG:
					anim_tag.current_direction = FORWARDS;
					anim_tag.current_frame = current_tag.from_frame;
					break;
				case REVERSEPINGPONG:
					anim_tag.anim_direction = PINGPONG;
					anim_tag.current_direction = REVERSE;
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

int _advance_animtag(AnimTag *anim_tag, float delta_time, Aseprite ase)
{
	if (ase.frame_count == 0 || anim_tag->frame_count == 0)
		return -1;

	int frame = anim_tag->current_frame;

	if (anim_tag->frame_count == 1)
		return frame;

	float miliseconds = (int)(1000.f * delta_time);
	anim_tag->timer += miliseconds;

	float frame_duration = (float)ase.frames[frame].duration_milliseconds;

	if (anim_tag->timer >= frame_duration)
	{
		anim_tag->timer -= frame_duration;
	
		switch (anim_tag->current_direction)
		{
			case FORWARDS:
				anim_tag->current_frame++;

				if (anim_tag->current_frame > anim_tag->to_frame)
				{
					if (anim_tag->anim_direction == PINGPONG)
					{
						anim_tag->current_direction = REVERSE;

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
					if (anim_tag->anim_direction == PINGPONG)
					{
						anim_tag->current_direction = FORWARDS;

						anim_tag->current_frame += 2;
					}
					else
						anim_tag->current_frame = anim_tag->to_frame;
				}
				break;
			case PINGPONG:
			default:
				break;
		}
	}

	return frame;
}

void DrawAnim(Aseprite ase, AnimTag *anim_tag, float delta_time, float x, float y, Color tint)
{
	int frame = _advance_animtag(anim_tag, delta_time, ase);

	DrawAseprite(ase, frame, x, y, tint);
}
void DrawAnimV(Aseprite ase, AnimTag *anim_tag, float delta_time, Vector2 pos, Color tint)
{
	int frame = _advance_animtag(anim_tag, delta_time, ase);

	DrawAsepriteV(ase, frame, pos, tint);
}
void DrawAnimScale(Aseprite ase, AnimTag *anim_tag, float delta_time, Vector2 position, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint)
{
	int frame = _advance_animtag(anim_tag, delta_time, ase);

	DrawAsepriteScale(ase, frame, position, origin, x_scale, y_scale, rotation, tint);
}