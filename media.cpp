#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")


#include <vlc/libvlc.h>
#include <vlc/libvlc_media.h>
#include <vlc/libvlc_media_player.h>

#include <SDL.h>
#include <SDL_syswm.h>

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"
#include <GL/gl3w.h>

#include <windows.h>
#include <commdlg.h>
#include <iostream>

int videoWidth = 800, videoHeight = 600;
unsigned char* videoBuffer = nullptr;
GLuint videoTexture = 0;

void* lock(void* opaque, void** planes) {
    *planes = opaque;
    return nullptr;
}

void unlock(void* opaque, void* picture, void* const* planes) {}
void display(void* opaque, void* picture) {}

std::string OpenVideoFileDialog(HWND hwnd) {
    char filepath[MAX_PATH] = "";
    OPENFILENAMEA ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "Video Files\0*.mp4;*.mkv;*.avi;*.mov;*.webm\0All Files\0*.*\0";
    ofn.lpstrFile = filepath;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    return GetOpenFileNameA(&ofn) ? std::string(filepath) : "";
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return -1;

    SDL_Window* window = SDL_CreateWindow("ILTD Player",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        videoWidth, videoHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    gl3wInit();

    SDL_Surface* logoSurface = SDL_LoadBMP("logo.bmp");
    GLuint logoTexture = 0;

    if (logoSurface) {
        glGenTextures(1, &logoTexture);
        glBindTexture(GL_TEXTURE_2D, logoTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
            logoSurface->w, logoSurface->h, 0,
            GL_BGR, GL_UNSIGNED_BYTE, logoSurface->pixels);
        SDL_FreeSurface(logoSurface);
    }

    // ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 130");

    // VLC
    const char* const vlc_args[] = {
    "--aout=directsound"
    };

    libvlc_instance_t* vlc = libvlc_new(1, vlc_args);
    if (!vlc) {
        MessageBoxA(NULL, "libvlc_new failed!", "Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    //libvlc_instance_t* vlc = libvlc_new(0, nullptr);
    libvlc_media_player_t* player = nullptr;
    bool is_playing = false;
    int volume = 100;
    float slider = 0.0f;

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);
    HWND hwnd = wmInfo.info.win.window;

    bool running = true;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                running = false;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // --- Video Rendering ---
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("VideoWindow", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoBringToFrontOnFocus);

        if (videoTexture && videoBuffer) {
            glBindTexture(GL_TEXTURE_2D, videoTexture);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, videoWidth, videoHeight, GL_RGB, GL_UNSIGNED_BYTE, videoBuffer);
            ImVec2 size = ImGui::GetContentRegionAvail();
            //ImGui::Image((ImTextureID)(intptr_t)videoTexture, size);
        }

        if (!videoTexture || !is_playing) {
            ImVec2 size = ImGui::GetContentRegionAvail();
            ImGui::Image((ImTextureID)(intptr_t)logoTexture, size);
        }
        else {
            glBindTexture(GL_TEXTURE_2D, videoTexture);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, videoWidth, videoHeight, GL_RGB, GL_UNSIGNED_BYTE, videoBuffer);
            ImVec2 size = ImGui::GetContentRegionAvail();
            ImGui::Image((ImTextureID)(intptr_t)videoTexture, size);
        }


        ImGui::End();

        // --- Controls ---
        ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 120), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 120));
        ImGui::Begin("Controls", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        if (ImGui::Button("Open Video")) {
            std::string path = OpenVideoFileDialog(hwnd);
            if (!path.empty()) {
                if (player) {
                    libvlc_media_player_stop(player);
                    libvlc_media_player_release(player);
                }

                libvlc_media_t* media = libvlc_media_new_path(vlc, path.c_str());
                player = libvlc_media_player_new(vlc);
                libvlc_media_player_set_media(player, media);
                libvlc_media_release(media);

                if (!videoBuffer)
                    videoBuffer = new unsigned char[videoWidth * videoHeight * 3];

                if (!videoTexture) {
                    glGenTextures(1, &videoTexture);
                    glBindTexture(GL_TEXTURE_2D, videoTexture);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, videoWidth, videoHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
                }

                libvlc_video_set_callbacks(player, lock, unlock, display, videoBuffer);
                libvlc_video_set_format(player, "RV24", videoWidth, videoHeight, videoWidth * 3);
                libvlc_media_player_play(player);
                libvlc_audio_set_volume(player, volume);
                is_playing = true;
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Play") && player) {
            if (!libvlc_media_player_is_playing(player)) {
                libvlc_media_player_set_pause(player, false);
                libvlc_media_player_play(player);
                is_playing = true;
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Pause") && player && is_playing) {
            if (libvlc_media_player_is_playing(player))
                libvlc_media_player_set_pause(player, true);
        }

        ImGui::SameLine();
        if (ImGui::Button("Stop") && player && is_playing) {
            libvlc_media_player_stop(player);
            is_playing = false;
        }

        // Volume Slider
        if (player) {
            ImGui::Text("Volume");
            if (ImGui::SliderInt("##volume", &volume, 0, 100)) {
                libvlc_audio_set_volume(player, volume);
            }
        }

        // Time Slider
        if (player) {
            libvlc_time_t length = libvlc_media_player_get_length(player);
            libvlc_time_t current = libvlc_media_player_get_time(player);
            if (length > 0) {
                slider = (float)current / length;
                if (ImGui::SliderFloat("##seek", &slider, 0.0f, 1.0f)) {
                    libvlc_media_player_set_time(player, (libvlc_time_t)(slider * length));
                }
                ImGui::Text("Time: %.2f / %.2f sec", current / 1000.0, length / 1000.0);
            }
        }

        ImGui::End();

        // --- Render everything ---
        ImGui::Render();
        SDL_GL_MakeCurrent(window, gl_context);
        glViewport(0, 0, videoWidth, videoHeight);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    if (player) libvlc_media_player_release(player);
    libvlc_release(vlc);
    if (videoBuffer) delete[] videoBuffer;
    if (videoTexture) glDeleteTextures(1, &videoTexture);
    if (logoTexture) glDeleteTextures(1, &logoTexture);


    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
