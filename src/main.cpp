#include <iostream>
#include <string>
#include <vector>
#include <bitset>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <Shared/FSH/FshFile.h>


static const SDL_DialogFileFilter filters[] = {
  { "FSH archive",  "fsh;FSH;qfs;QFS" }
};


static void SDLCALL callback(void* userdata, const char* const* filelist, int filter) {
  std::string * file_name = (std::string *) userdata;
    if (!filelist) {
        SDL_Log("An error occured: %s", SDL_GetError());
        return;
    } else if (!*filelist) {
        SDL_Log("The user did not select any file.");
        SDL_Log("Most likely, the dialog was canceled.");
        return;
    }

    while (*filelist) {
        SDL_Log("Full path to selected file: '%s'", *filelist);
        *file_name = *filelist;
        filelist++;
    }

    if (filter < 0) {
        SDL_Log("The current platform does not support fetching "
                "the selected filter, or the user did not select"
                " any filter.");
        return;
    } else if (filter < SDL_arraysize(filters)) {
        SDL_Log("The filter selected by the user is '%s' (%s).",
                filters[filter].pattern, filters[filter].name);
        return;
    }
}

SDL_Surface * getSDLSurface(LibOpenNFS::Shared::FshTexture fshTex) {
  std::vector<uint32_t> pixels = fshTex.ToARGB32();
  SDL_Surface * surface = SDL_CreateSurfaceFrom((int) fshTex.Width(), (int) fshTex.Height(), SDL_PIXELFORMAT_ARGB8888, pixels.data(), (int) fshTex.Width() * 4);
  return surface;
}

SDL_Texture * getSDLTexture(SDL_Renderer * renderer, LibOpenNFS::Shared::FshTexture fshTex) {
SDL_Surface * surface = getSDLSurface(fshTex);
  if (surface == nullptr)
    return nullptr;

  SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_DestroySurface(surface);

  return texture;
}


int main(int argc, char ** argv) {
  size_t current_image = 0;
  std::string file_name = "";

  if (argc == 3) {
    LibOpenNFS::Shared::FshArchive fsh;
    fsh.Load(std::string(argv[1]));
    SDL_Surface * surface = getSDLSurface(fsh.GetTexture(0));
    SDL_SavePNG(surface, argv[2]);
    SDL_DestroySurface(surface);
    return 0;
  }

  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window * window = nullptr;
  SDL_Renderer * renderer = nullptr;
  if (!SDL_CreateWindowAndRenderer("example", 100, 100, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE, &window, &renderer)) {
    return 2;
  }

  if (argc > 1) {
    file_name = argv[1];
  } else {
    SDL_ShowOpenFileDialog(callback, (void *) &file_name, window, filters, 1, NULL, false);
  }

  while (file_name.empty()) {
    SDL_PumpEvents();
  }
  SDL_Log("Selected %s", file_name.c_str());

  LibOpenNFS::Shared::FshArchive fsh;
  fsh.Load(file_name);
  std::vector<SDL_Texture *> textures;
  for(size_t i = 0; i < fsh.TextureCount(); i++) {
    textures.push_back(nullptr);
  }

  SDL_SetWindowSize(window, fsh.GetTexture(current_image).Width(), fsh.GetTexture(current_image).Height());
  SDL_ShowWindow(window);

  bool running = true;
  SDL_Event event;
  while (running) { 
    // Process input
    if (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_EVENT_QUIT:
          // End the loop if the programs is being closed
          running = false;
          break;
        case SDL_EVENT_KEY_DOWN:
          if( event.key.key == SDLK_RIGHT ) {
              current_image++;
              if (current_image >= textures.size())
                current_image = 0;
              SDL_SetWindowSize(window, fsh.GetTexture(current_image).Width(), fsh.GetTexture(current_image).Height());
          } else if( event.key.key == SDLK_LEFT ) {
              if (current_image == 0) {
                current_image = textures.size() - 1;
              } else {
                current_image--;
              }
              SDL_SetWindowSize(window, fsh.GetTexture(current_image).Width(), fsh.GetTexture(current_image).Height());
          }
          break;
      }
    }
    // Clear the screen
    SDL_RenderClear(renderer);

    // Draw a red square
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    // Load texture
    if (textures[current_image] == nullptr)
      textures[current_image] = getSDLTexture(renderer, fsh.GetTexture(current_image));

    if (textures[current_image] != nullptr) {
      SDL_RenderTexture(renderer, textures[current_image], NULL, NULL);
    }


    // Draw everything on a white background
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderPresent(renderer);
  }
  for (SDL_Texture * texture : textures)
    SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
