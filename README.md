# raylib-aseprite
A small C library which allows the user to import and draw [Aseprite](https://www.aseprite.org) pixel arts and animations with [raylib](https://www.raylib.com/).

# simple example
``` c
#include <raylib.h>
#include <aseprite.h>

int main()
{
	InitWindow(640, 360, "Aseprite example");
	SetTargetFPS(60);

	// Load an Aseprite file
	Aseprite frog = LoadAsepriteFromFile("sprites/frog.ase", ASEPRITE_LOAD_ALL);

	// Create an animation from an Aseprite file tag
	AseAnimation tongue = CreateAnimationTag(&frog, "Tongue");

	while (!WindowShouldClose())
	{
		// Update animation
		AdvanceAnimation(&tongue);

		BeginDrawing();
		ClearBackground(WHITE);

		// Draw the current animation frame to the screen
		DrawAnimation(tongue, 312, 172, WHITE);
    
		EndDrawing();
	}

	// Unload the Aseprite file
	UnloadAseprite(frog);

	CloseWindow();

	return 0;
}
```

# usage
Simply include cute_aseprite.h, aseprite.c and aseprite.h in your source files and include the aseprite.h header in the relevant files. It is also necessary to make sure the [raylib.h](https://github.com/raysan5/raylib) header is linked.

The [cute_aseprite.h](https://github.com/RandyGaul/cute_headers/blob/master/cute_aseprite.h) header file used to parse .ase and .aseprite files was provided by [Randy Gaul's cute_headers repository](https://github.com/RandyGaul/cute_headers/tree/master).

# cheatsheet
``` c
// Load functions.

Aseprite LoadAsepriteFromFile(const char *filename, AseLoadFlags flags);
Aseprite LoadAsepriteFromMemory(const void *data, int size, AseLoadFlags flags);
void UnloadAseprite(Aseprite ase);
int IsAsepriteReady(Aseprite ase);

// Motionless frame draw functions.

void DrawFrame(Aseprite ase, int frame, float x, float y, Color tint);
void DrawFrameV(Aseprite ase, int frame, Vector2 position, Color tint);
void DrawFrameEx(Aseprite ase, int frame, Vector2 position, float scale, float rotation, Color tint);
void DrawFrameScale(Aseprite ase, int frame, Vector2 position, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint);

// Montionless cel draw functions.

void DrawCel(Aseprite ase, int layer, int frame, float x, float y, Color tint);
void DrawCelV(Aseprite ase, int layer, int frame, Vector2 position, Color tint);
void DrawCelEx(Aseprite ase, int layer, int frame, Vector2 position, float scale, float rotation, Color tint);
void DrawCelScale(Aseprite ase, int layer, int frame, Vector2 position, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint);

// Animation load functions.

AseAnimation CreateSimpleAnimation(Aseprite *ase);
AseAnimation CreateAnimationTag(Aseprite *ase, const char *tag_name);
AseAnimation CreateAnimationTagId(Aseprite *ase, int tag_id);
int IsAnimationReady(AseAnimation anim);

// Animation update functions.

void SetAnimationSpeed(AseAnimation *anim, float speed);
void PlayAnimation(AseAnimation *anim);
void StopAnimation(AseAnimation *anim);
void PauseAnimation(AseAnimation *anim);
void AdvanceAnimation(AseAnimation *anim);

// Animated frame draw functions.

void DrawAnimation(AseAnimation anim, float x, float y, Color tint);
void DrawAnimationV(AseAnimation anim, Vector2 position, Color tint);
void DrawAnimationEx(AseAnimation anim, Vector2 position, float scale, float rotation, Color tint);
void DrawAnimationScale(AseAnimation anim, Vector2 position, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint);

// Animated layer draw functions.

void DrawAnimLayer(AseAnimation anim, int layer, float x, float y, Color tint);
void DrawAnimLayerV(AseAnimation anim, int layer, Vector2 position, Color tint);
void DrawAnimLayerEx(AseAnimation anim, int layer, Vector2 position, float scale, float rotation, Color tint);
void DrawAnimLayerScale(AseAnimation anim, int layer, Vector2 position, Vector2 origin, float x_scale, float y_scale, float rotation, Color tint);
```

# limitations
Randy Gaul's cute_aseprite.h header has no support for tilesets. Attempting to load a file with tileset data will result in an error.

Also, it only supports the normal blending mode. Layers will only be displayed in it. A palliative solution is to merge layers that contain alternative blending before using them.
