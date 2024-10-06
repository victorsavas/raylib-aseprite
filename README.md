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
  Aseprite frog = LoadAsepriteFromFile("sprites/frog.ase", ASEPRITE_LOAD_FRAMES | ASEPRITE_LOAD_TAGS);

  // Create an animation from an Aseprite file tag
  AseAnimTag tongue = CreateAnimTag(frog, "Tongue");

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

# limitations
Randy Gaul's cute_aseprite.h header has no support for tilesets. Attempting to load a file with tileset data will result in an error. 
