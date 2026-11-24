#include "UnixShell.h"
#include "Game.h"
#include "UI/Touch.h"
#include "Core.h"
#include "Object/Root.h"
#include "UnixSoundEngine.h"
#include "Audio/SoundSystem.h"
#include "ShellInterfaceUnix.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <cstring>
#include <cstdint>
#include <cstdio>

using namespace LORD;

static Display*   g_display = nullptr;
static Window     g_window = 0;
static EGLContext g_eglContext = nullptr;
static EGLSurface g_eglSurface = nullptr;
static EGLDisplay g_eglDisplay = nullptr;
static Atom       g_wmDeleteMessage;

static int g_nWindowWidth = 896;
static int g_nWindowHeight = 504;

static GameClient::CGame* g_pGame = nullptr;
static SoundEngine*       g_pSoundEngine = nullptr;
static GameClient::ShellInterfaceUnix* g_pShellInterface = nullptr;

static bool g_bMousePressed = false;
static bool g_bDestroy = false;

static void LoadWindowSize()
{
    std::ifstream _if("win_config.txt");
    std::string width, height;
    if (_if) {
        _if >> width >> height;
        auto split = [](const std::string& s, char delim) -> std::vector<std::string> {
            std::vector<std::string> result;
            std::string token;
            for (char c : s) {
                if (c == delim) {
                    result.push_back(token);
                    token.clear();
                } else {
                    token += c;
                }
            }
            result.push_back(token);
            return result;
        };
        auto wparts = split(width, '=');
        auto hparts = split(height, '=');
        if (wparts.size() > 1) g_nWindowWidth = std::stoi(wparts[1]);
        if (hparts.size() > 1) g_nWindowHeight = std::stoi(hparts[1]);
    }
    _if.close();
}

static bool InitEGL()
{
    g_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (g_eglDisplay == EGL_NO_DISPLAY) {
        std::cerr << "eglGetDisplay failed" << std::endl;
        return false;
    }
    if (!eglInitialize(g_eglDisplay, nullptr, nullptr)) {
        std::cerr << "eglInitialize failed" << std::endl;
        return false;
    }
    if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        std::cerr << "eglBindAPI failed" << std::endl;
        return false;
    }

    EGLint configAttributes[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_NONE
    };
    EGLConfig config;
    EGLint numConfigs;
    if (!eglChooseConfig(g_eglDisplay, configAttributes, &config, 1, &numConfigs) || numConfigs == 0) {
        std::cerr << "eglChooseConfig failed" << std::endl;
        return false;
    }

    EGLint contextAttributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    g_eglContext = eglCreateContext(g_eglDisplay, config, EGL_NO_CONTEXT, contextAttributes);
    if (g_eglContext == EGL_NO_CONTEXT) {
        std::cerr << "eglCreateContext failed" << std::endl;
        return false;
    }

    g_eglSurface = eglCreateWindowSurface(g_eglDisplay, config, reinterpret_cast<EGLNativeWindowType>(g_window), nullptr);
    if (g_eglSurface == EGL_NO_SURFACE) {
        std::cerr << "eglCreateWindowSurface failed" << std::endl;
        return false;
    }

    if (!eglMakeCurrent(g_eglDisplay, g_eglSurface, g_eglSurface, g_eglContext)) {
        std::cerr << "eglMakeCurrent failed" << std::endl;
        return false;
    }

    // Set vsync
    eglSwapInterval(g_eglDisplay, 1);

    return true;
}

// Load unixshell.ico (BMP frames) and set _NET_WM_ICON so window managers
// show the icon in the taskbar / alt-tab switcher.
static void SetWindowIcon()
{
    // Candidates: next to the executable, then the current working directory
    std::vector<std::string> candidates;
    {
        char buf[1024];
        ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
        if (len > 0) {
            buf[len] = '\0';
            std::string exePath(buf);
            size_t pos = exePath.find_last_of('/');
            if (pos != std::string::npos) {
                candidates.push_back(exePath.substr(0, pos + 1) + "unixshell.ico");
            }
        }
    }
    candidates.push_back("unixshell.ico");
    candidates.push_back("./unixshell.ico");

    std::ifstream f;
    for (size_t i = 0; i < candidates.size(); i++) {
        f.open(candidates[i], std::ios::binary);
        if (f) break;
    }
    if (!f) {
        std::cerr << "unixshell.ico not found" << std::endl;
        return;
    }
    std::vector<unsigned char> data((std::istreambuf_iterator<char>(f)),
                                    std::istreambuf_iterator<char>());
    f.close();
    if (data.size() < 6) return;

    // ICO header: reserved(2), type(2), count(2)
    uint16_t type = (uint16_t)(data[2] | (data[3] << 8));
    uint16_t count = (uint16_t)(data[4] | (data[5] << 8));
    if (type != 1 || count == 0) return;

    // Pick the largest frame
    size_t bestOffset = 0, bestSize = 0;
    int bestW = 0, bestH = 0, bestBpp = 0;
    for (uint16_t i = 0; i < count; i++) {
        size_t e = 6 + i * 16;
        if (e + 16 > data.size()) break;
        int w = data[e]; if (w == 0) w = 256;
        int h = data[e + 1]; if (h == 0) h = 256;
        uint16_t bpp = (uint16_t)(data[e + 6] | (data[e + 7] << 8));
        uint32_t sz  = (uint32_t)data[e + 8] | ((uint32_t)data[e + 9] << 8) |
                       ((uint32_t)data[e + 10] << 16) | ((uint32_t)data[e + 11] << 24);
        uint32_t off = (uint32_t)data[e + 12] | ((uint32_t)data[e + 13] << 8) |
                       ((uint32_t)data[e + 14] << 16) | ((uint32_t)data[e + 15] << 24);
        if (off + sz > data.size()) continue;
        if ((int)sz > (int)bestSize) {
            bestSize = sz; bestOffset = off; bestW = w; bestH = h; bestBpp = bpp;
        }
    }
    if (bestSize == 0 || bestW <= 0 || bestH <= 0) return;

    // BMP-in-ICO: BITMAPINFOHEADER(40 bytes) then pixel data
    if (bestSize < 40) return;
    uint32_t biSize = (uint32_t)data[bestOffset] | ((uint32_t)data[bestOffset+1] << 8) |
                      ((uint32_t)data[bestOffset+2] << 16) | ((uint32_t)data[bestOffset+3] << 24);
    if (biSize < 40) return;
    int32_t bmpH = (int32_t)((uint32_t)data[bestOffset+8] | ((uint32_t)data[bestOffset+9] << 8) |
                             ((uint32_t)data[bestOffset+10] << 16) | ((uint32_t)data[bestOffset+11] << 24));
    // biHeight includes the AND mask, so it's 2x the visible height
    int height = bestH;
    int width = bestW;
    (void)bmpH;

    size_t pixelOffset = bestOffset + biSize;
    // Skip palette if any (indexed formats) — we only handle 32/24bpp
    if (bestBpp == 32 || bestBpp == 24) {
        int rowSize = ((bestBpp * width + 31) / 32) * 4;
        size_t needed = pixelOffset + (size_t)rowSize * height;
        if (needed > data.size()) return;

        // Build _NET_WM_ICON: [width, height, ARGB pixels...] (bottom-up BMP → top-down)
        std::vector<unsigned long> icon;
        icon.reserve(2 + (size_t)width * height);
        icon.push_back((unsigned long)width);
        icon.push_back((unsigned long)height);
        bool anyAlpha = false;
        for (int y = height - 1; y >= 0; y--) {
            const unsigned char* row = &data[pixelOffset + (size_t)y * rowSize];
            for (int x = 0; x < width; x++) {
                const unsigned char* px = row + x * (bestBpp / 8);
                unsigned char b = px[0], g = px[1], r = px[2];
                unsigned char a = (bestBpp == 32) ? px[3] : 255;
                if (a != 0) anyAlpha = true;
                icon.push_back(((unsigned long)a << 24) | ((unsigned long)r << 16) |
                               ((unsigned long)g << 8) | (unsigned long)b);
            }
        }
        // Some ICOs leave the alpha channel zeroed out — force opaque in that case
        if (!anyAlpha) {
            for (size_t i = 2; i < icon.size(); i++) {
                icon[i] |= 0xFF000000ul;
            }
        }

        Atom netWmIcon = XInternAtom(g_display, "_NET_WM_ICON", False);
        XChangeProperty(g_display, g_window, netWmIcon, XA_CARDINAL, 32,
                        PropModeReplace,
                        reinterpret_cast<const unsigned char*>(icon.data()),
                        (int)icon.size());
    }

    // Also set WM_CLASS / icon name for good measure
    XClassHint classHint;
    classHint.res_name = const_cast<char*>("unixshell");
    classHint.res_class = const_cast<char*>("UnixShell");
    XSetClassHint(g_display, g_window, &classHint);

    XSetIconName(g_display, g_window, "UnixShell-Blockman");
}

static bool CreateX11Window()
{
    g_display = XOpenDisplay(nullptr);
    if (!g_display) {
        std::cerr << "Cannot open X display" << std::endl;
        return false;
    }
    int screen = DefaultScreen(g_display);

    XSetWindowAttributes swa;
    swa.background_pixel = BlackPixel(g_display, screen);
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
                     ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
                     StructureNotifyMask;

    g_window = XCreateWindow(g_display, RootWindow(g_display, screen),
                             0, 0, g_nWindowWidth, g_nWindowHeight, 0,
                             CopyFromParent, InputOutput, CopyFromParent,
                             CWBackPixel | CWEventMask, &swa);
    if (!g_window) {
        std::cerr << "XCreateWindow failed" << std::endl;
        return false;
    }

    // Center window
    XMoveWindow(g_display, g_window, 100, 100);

    XStoreName(g_display, g_window, "LinShell-Blockman");

    // Set the app icon from unixshell.ico
    SetWindowIcon();

    // Handle window close
    g_wmDeleteMessage = XInternAtom(g_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(g_display, g_window, &g_wmDeleteMessage, 1);

    XMapWindow(g_display, g_window);
    XFlush(g_display);

    return true;
}

static void HandleKeyPress(XKeyEvent* event)
{
    KeySym key = XLookupKeysym(event, 0);
    if (key == XK_F1) {
        if (g_pSoundEngine) g_pSoundEngine->pauseBackgroundMusic();
    } else if (key == XK_F2) {
        if (g_pSoundEngine) g_pSoundEngine->resumeBackgroundMusic();
    } else if (key == XK_F8) {
        g_bDestroy = true;
    } else {
        if (g_pGame) g_pGame->handleKeyDown(static_cast<int>(key));
    }
}

static void HandleKeyRelease(XKeyEvent* event)
{
    KeySym key = XLookupKeysym(event, 0);
    if (g_pGame) g_pGame->handleKeyUp(static_cast<int>(key));
}

static void HandleButtonPress(XButtonEvent* event)
{
    if (!g_pGame) return;
    float x = static_cast<float>(event->x);
    float y = static_cast<float>(event->y);
    int num = 1;
    intptr_t ids[] = {0};
    float xs[] = {x};
    float ys[] = {y};
    g_pGame->handleTouchesBegin(num, ids, xs, ys);
    g_bMousePressed = true;
}

static void HandleButtonRelease(XButtonEvent* event)
{
    if (!g_pGame || !g_bMousePressed) return;
    float x = static_cast<float>(event->x);
    float y = static_cast<float>(event->y);
    int num = 1;
    intptr_t ids[] = {0};
    float xs[] = {x};
    float ys[] = {y};
    g_pGame->handleTouchesEnd(num, ids, xs, ys);
    g_bMousePressed = false;
}

static void HandleMotionNotify(XMotionEvent* event)
{
    if (!g_pGame || !g_bMousePressed) return;
    float x = static_cast<float>(event->x);
    float y = static_cast<float>(event->y);
    int num = 1;
    intptr_t ids[] = {0};
    float xs[] = {x};
    float ys[] = {y};
    g_pGame->handleTouchesMove(num, ids, xs, ys);
}

static void ProcessEvents()
{
    while (XPending(g_display)) {
        XEvent event;
        XNextEvent(g_display, &event);
        switch (event.type) {
            case DestroyNotify:
                g_bDestroy = true;
                break;
            case ClientMessage:
                if (event.xclient.data.l[0] == static_cast<long>(g_wmDeleteMessage)) {
                    g_bDestroy = true;
                }
                break;
            case KeyPress:
                HandleKeyPress(&event.xkey);
                break;
            case KeyRelease:
                HandleKeyRelease(&event.xkey);
                break;
            case ButtonPress:
                HandleButtonPress(&event.xbutton);
                break;
            case ButtonRelease:
                HandleButtonRelease(&event.xbutton);
                break;
            case MotionNotify:
                HandleMotionNotify(&event.xmotion);
                break;
            case ConfigureNotify:
                if (g_pGame) {
                    g_pGame->OnSize(event.xconfigure.width, event.xconfigure.height);
                }
                break;
            default:
                break;
        }
    }
}

static void ConstructGame()
{
    g_pGame = new GameClient::CGame;
    g_pGame->setPlatformInfo("test");

    // create document dir if needed
    // PathUtil::CreateDir handled by engine

    g_pSoundEngine = LordNew UnixSoundEngine;
    g_pShellInterface = LordNew GameClient::ShellInterfaceUnix;
}

static void DestructGame()
{
    if (g_pGame) {
        g_pGame->DestroyGame();
    }
    if (g_pSoundEngine) {
        g_pSoundEngine->end();
        LordSafeDelete(g_pSoundEngine);
    }
    if (g_pShellInterface) {
        LordSafeDelete(g_pShellInterface);
    }
    if (g_pGame) {
        g_pGame->setShellInterface(nullptr);
        delete g_pGame;
        g_pGame = nullptr;
    }
}

int main(int argc, char** argv)
{
    (void)argc; (void)argv;

    LoadWindowSize();

    if (!CreateX11Window()) {
        return 1;
    }
    if (!InitEGL()) {
        XDestroyWindow(g_display, g_window);
        XCloseDisplay(g_display);
        return 1;
    }

    ConstructGame();

    g_pGame->InitConfig("./");
    g_pGame->setShellInterface(g_pShellInterface);
    g_pGame->InitGame("./", "./document/", g_nWindowWidth, g_nWindowHeight, g_window, g_pSoundEngine);

    while (!g_bDestroy) {
        ProcessEvents();
        if (g_pGame) {
            g_pGame->MainTick();
        }
        eglSwapBuffers(g_eglDisplay, g_eglSurface);
        usleep(10000); // ~100 fps cap
    }

    DestructGame();

    eglMakeCurrent(g_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(g_eglDisplay, g_eglSurface);
    eglDestroyContext(g_eglDisplay, g_eglContext);
    eglTerminate(g_eglDisplay);

    XDestroyWindow(g_display, g_window);
    XCloseDisplay(g_display);

    return 0;
}
