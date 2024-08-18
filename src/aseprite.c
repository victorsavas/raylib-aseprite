#include <raylib.h>

#define CUTE_ASEPRITE_IMPLEMENTATION
#include <cute_aseprite.h>

#include <aseprite.h>

static Texture2D _get_frame_texture(Aseprite ase, int frame);
static int _advance_animtag(AnimTag *anim_tag, float delta_time, Aseprite ase);

Aseprite LoadAsepriteFromFile(const char *filename)
{
	ase_t *cute_ase = cute_aseprite_load_from_file(filename, NULL);

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
		ase.tags[i].from_frame = (cute_ase->tags)[i].from_frame;
		ase.tags[i].to_frame = (cute_ase->tags)[i].to_frame;

		ase.tags[i].repeat = (cute_ase->tags)[i].repeat;

		// Transfer of ownership of tag name pointer

		ase.tags[i].name = strdup((cute_ase->tags)[i].name);
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
				.current_frame = current_tag.from_frame,
				.from_frame = current_tag.from_frame,
				.to_frame = current_tag.to_frame,
				.repeat = current_tag.repeat,
				.timer = 0
			};

			return anim_tag;
		}
	}

	// If no tags are found, -1 is returned as default.

	return (AnimTag){0};
}

Texture2D _get_frame_texture(Aseprite ase, int frame)
{
	if (frame >= ase.frame_count)
		frame = 0;
	
	return ase.frames[frame].texture;
}

void DrawAseprite(Aseprite ase, int frame, float x, float y, Color tint)
{
	Texture texture = _get_frame_texture(ase, frame);

	DrawTexture(texture, x, y, tint);
}

void DrawAsepriteScale(Aseprite ase, int frame, Vector2 position, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint)
{
	Texture texture = _get_frame_texture(ase, frame);
	
	Rectangle source = {0, 0, texture.width, texture.height};
	Rectangle dest = {position.x, position.y, texture.width * x_scale, texture.height * y_scale};

	origin.x *= x_scale;
	origin.y *= y_scale;

	DrawTexturePro(texture, source, dest, origin, rotation, tint);
}

int _advance_animtag(AnimTag *anim_tag, float delta_time, Aseprite ase)
{
	int frame = anim_tag->current_frame;

	float miliseconds = (int)(1000.f * delta_time);
	anim_tag->timer += miliseconds;

	float frame_duration = (float)ase.frames[frame].duration_milliseconds;

	if (anim_tag->timer >= frame_duration)
	{
		anim_tag->timer -= frame_duration;
		anim_tag->current_frame++;

		if (anim_tag->current_frame > anim_tag->to_frame)
			anim_tag->current_frame = anim_tag->from_frame;
	}

	return frame;
}

void DrawAnim(Aseprite ase, AnimTag *anim_tag, float delta_time, float x, float y, Color tint)
{
	int frame = _advance_animtag(anim_tag, delta_time, ase);

	DrawAseprite(ase, frame, x, y, tint);
}

void DrawAnimScale(Aseprite ase, AnimTag *anim_tag, float delta_time, Vector2 position, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint)
{
	int frame = _advance_animtag(anim_tag, delta_time, ase);

	DrawAsepriteScale(ase, frame, position, origin, x_scale, y_scale, rotation, tint);
}