/*
 * glfw_sdl.c - Implementacion de la capa GLFW -> SDL2
 * ====================================================
 * Ver deps/sdlshim/GLFW/glfw3.h para la documentacion general.
 *
 * Resumen de mapping:
 *   glfwInit              -> SDL_Init(VIDEO|TIMER|EVENTS)
 *   glfwCreateWindow      -> SDL_CreateWindow(+contexto GL)
 *   glfwPollEvents        -> SDL_PollEvent + dispatch de callbacks
 *   cursor exclusivo      -> SDL_SetRelativeMouseMode
 *   glfwGetTime/SetTime   -> reloj propio sobre SDL_GetPerformanceCounter
 *   teclas                -> tabla scancode SDL -> codigo GLFW, mas teclas
 *                            sinteticas para los controles tactiles
 *   tactil                -> SDL_FINGER* (normalizado -> pixeles) -> callback
 */
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>

struct GLFWwindow {
    SDL_Window *sdl;
    SDL_GLContext gl;
    int should_close;
    int cursor_mode;
    double cx, cy;
    unsigned char keys[512];
    GLFWkeyfun key_cb;
    GLFWcharfun char_cb;
    GLFWmousebuttonfun mouse_cb;
    GLFWscrollfun scroll_cb;
};

static GLFWwindow g_window;
static GLFWwindow *g_win = NULL;
static GLFWtouchfun g_touch_cb = NULL;
static GLFWvidmode g_modes[8];

static double g_time = 0.0;
static Uint64 g_freq = 1;
static Uint64 g_last = 0;

static Uint64 perf_now(void) {
    return SDL_GetPerformanceCounter();
}

static void time_sync(void) {
    g_last = perf_now();
}

/* ------------------------------------------------------------------ */
/* scancode SDL -> codigo GLFW (numeros identicos a GLFW de verdad)    */
/* ------------------------------------------------------------------ */
static int scancode_to_glfw(SDL_Scancode sc) {
    if (sc >= SDL_SCANCODE_A && sc <= SDL_SCANCODE_Z) {
        return 'A' + (sc - SDL_SCANCODE_A);
    }
    if (sc >= SDL_SCANCODE_1 && sc <= SDL_SCANCODE_9) {
        return '1' + (sc - SDL_SCANCODE_1);
    }
    if (sc == SDL_SCANCODE_0) {
        return '0';
    }
    switch (sc) {
    case SDL_SCANCODE_SPACE: return GLFW_KEY_SPACE;
    case SDL_SCANCODE_APOSTROPHE: return GLFW_KEY_APOSTROPHE;
    case SDL_SCANCODE_COMMA: return GLFW_KEY_COMMA;
    case SDL_SCANCODE_MINUS: return GLFW_KEY_MINUS;
    case SDL_SCANCODE_PERIOD: return GLFW_KEY_PERIOD;
    case SDL_SCANCODE_SLASH: return GLFW_KEY_SLASH;
    case SDL_SCANCODE_SEMICOLON: return GLFW_KEY_SEMICOLON;
    case SDL_SCANCODE_EQUALS: return GLFW_KEY_EQUAL;
    case SDL_SCANCODE_ESCAPE: return GLFW_KEY_ESCAPE;
    case SDL_SCANCODE_RETURN: return GLFW_KEY_ENTER;
    case SDL_SCANCODE_TAB: return GLFW_KEY_TAB;
    case SDL_SCANCODE_BACKSPACE: return GLFW_KEY_BACKSPACE;
    case SDL_SCANCODE_INSERT: return GLFW_KEY_INSERT;
    case SDL_SCANCODE_DELETE: return GLFW_KEY_DELETE;
    case SDL_SCANCODE_RIGHT: return GLFW_KEY_RIGHT;
    case SDL_SCANCODE_LEFT: return GLFW_KEY_LEFT;
    case SDL_SCANCODE_DOWN: return GLFW_KEY_DOWN;
    case SDL_SCANCODE_UP: return GLFW_KEY_UP;
    case SDL_SCANCODE_PAGEUP: return GLFW_KEY_PAGE_UP;
    case SDL_SCANCODE_PAGEDOWN: return GLFW_KEY_PAGE_DOWN;
    case SDL_SCANCODE_HOME: return GLFW_KEY_HOME;
    case SDL_SCANCODE_END: return GLFW_KEY_END;
    case SDL_SCANCODE_CAPSLOCK: return GLFW_KEY_CAPS_LOCK;
    case SDL_SCANCODE_SCROLLLOCK: return GLFW_KEY_SCROLL_LOCK;
    case SDL_SCANCODE_NUMLOCKCLEAR: return GLFW_KEY_NUM_LOCK;
    case SDL_SCANCODE_PRINTSCREEN: return GLFW_KEY_PRINT_SCREEN;
    case SDL_SCANCODE_PAUSE: return GLFW_KEY_PAUSE;
    case SDL_SCANCODE_F1: return GLFW_KEY_F1;
    case SDL_SCANCODE_F2: return GLFW_KEY_F2;
    case SDL_SCANCODE_F3: return GLFW_KEY_F3;
    case SDL_SCANCODE_F4: return GLFW_KEY_F4;
    case SDL_SCANCODE_F5: return GLFW_KEY_F5;
    case SDL_SCANCODE_F6: return GLFW_KEY_F6;
    case SDL_SCANCODE_F7: return GLFW_KEY_F7;
    case SDL_SCANCODE_F8: return GLFW_KEY_F8;
    case SDL_SCANCODE_F9: return GLFW_KEY_F9;
    case SDL_SCANCODE_F10: return GLFW_KEY_F10;
    case SDL_SCANCODE_F11: return GLFW_KEY_F11;
    case SDL_SCANCODE_F12: return GLFW_KEY_F12;
    case SDL_SCANCODE_LSHIFT: return GLFW_KEY_LEFT_SHIFT;
    case SDL_SCANCODE_LCTRL: return GLFW_KEY_LEFT_CONTROL;
    case SDL_SCANCODE_LALT: return GLFW_KEY_LEFT_ALT;
    case SDL_SCANCODE_LGUI: return GLFW_KEY_LEFT_SUPER;
    case SDL_SCANCODE_RSHIFT: return GLFW_KEY_RIGHT_SHIFT;
    case SDL_SCANCODE_RCTRL: return GLFW_KEY_RIGHT_CONTROL;
    case SDL_SCANCODE_RALT: return GLFW_KEY_RIGHT_ALT;
    case SDL_SCANCODE_RGUI: return GLFW_KEY_RIGHT_SUPER;
    case SDL_SCANCODE_AC_BACK: return GLFW_KEY_ESCAPE;
    default: return -1;
    }
}

static int mods_from_sdl(SDL_Keymod mod) {
    int result = 0;
    if (mod & KMOD_SHIFT) result |= GLFW_MOD_SHIFT;
    if (mod & KMOD_CTRL) result |= GLFW_MOD_CONTROL;
    if (mod & KMOD_ALT) result |= GLFW_MOD_ALT;
    if (mod & KMOD_GUI) result |= GLFW_MOD_SUPER;
    return result;
}

/* decodifica UTF-8 simple y llama al char callback (Craft usa ASCII) */
static void dispatch_text(const char *text) {
    const unsigned char *p = (const unsigned char *)text;
    while (*p) {
        unsigned int cp;
        int extra;
        if (*p < 0x80) {
            cp = *p;
            extra = 0;
        }
        else if ((*p & 0xE0) == 0xC0) {
            cp = *p & 0x1F;
            extra = 1;
        }
        else if ((*p & 0xF0) == 0xE0) {
            cp = *p & 0x0F;
            extra = 2;
        }
        else if ((*p & 0xF8) == 0xF0) {
            cp = *p & 0x07;
            extra = 3;
        }
        else {
            p++;
            continue;
        }
        p++;
        while (extra-- > 0 && (*p & 0xC0) == 0x80) {
            cp = (cp << 6) | (*p & 0x3F);
            p++;
        }
        if (g_win && g_win->char_cb) {
            g_win->char_cb(g_win, cp);
        }
    }
}

/* ------------------------------------------------------------------ */
/* Init / ventana                                                      */
/* ------------------------------------------------------------------ */
int glfwInit(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 0;
    }
    SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
    /* Los toques los procesa NUESTRO codigo (controles tactiles); si SDL
     * ademas los emula como raton se generan giros y clics fantasma. */
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
#ifdef CRAFT_MOBILE
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif
    g_freq = SDL_GetPerformanceFrequency();
    if (g_freq == 0) {
        g_freq = 1;
    }
    g_time = 0.0;
    time_sync();
    SDL_StartTextInput();
    return 1;
}

void glfwTerminate(void) {
    SDL_Quit();
}

GLFWwindow *glfwCreateWindow(int width, int height, const char *title,
    GLFWmonitor *monitor, GLFWwindow *share)
{
    (void)share;
    Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
        SDL_WINDOW_ALLOW_HIGHDPI;
    if (monitor) {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
#if defined(CRAFT_MOBILE)
    flags |= SDL_WINDOW_FULLSCREEN;
#endif
    memset(&g_window, 0, sizeof(g_window));
    g_window.sdl = SDL_CreateWindow(
        title ? title : "Craft",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height, flags);
    if (!g_window.sdl) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return NULL;
    }
    g_window.gl = SDL_GL_CreateContext(g_window.sdl);
    if (!g_window.gl) {
        fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(g_window.sdl);
        g_window.sdl = NULL;
        return NULL;
    }
    g_window.cursor_mode = GLFW_CURSOR_NORMAL;
    g_win = &g_window;
    return &g_window;
}

void glfwDestroyWindow(GLFWwindow *window) {
    if (window && window->sdl) {
        if (window->gl) {
            SDL_GL_DeleteContext(window->gl);
            window->gl = NULL;
        }
        SDL_DestroyWindow(window->sdl);
        window->sdl = NULL;
    }
    if (window == g_win) {
        g_win = NULL;
    }
}

void glfwMakeContextCurrent(GLFWwindow *window) {
    if (window && window->sdl && window->gl) {
        SDL_GL_MakeCurrent(window->sdl, window->gl);
    }
}

void glfwSwapBuffers(GLFWwindow *window) {
    if (window && window->sdl) {
        SDL_GL_SwapWindow(window->sdl);
    }
}

void glfwSwapInterval(int interval) {
    SDL_GL_SetSwapInterval(interval);
}

int glfwWindowShouldClose(GLFWwindow *window) {
    return window ? window->should_close : 1;
}

void glfwSetWindowShouldClose(GLFWwindow *window, int value) {
    if (window) {
        window->should_close = value;
    }
}

void glfwGetFramebufferSize(GLFWwindow *window, int *width, int *height) {
    if (window && window->sdl) {
        SDL_GL_GetDrawableSize(window->sdl, width, height);
    }
    else {
        *width = 0;
        *height = 0;
    }
}

void glfwGetWindowSize(GLFWwindow *window, int *width, int *height) {
    if (window && window->sdl) {
        SDL_GetWindowSize(window->sdl, width, height);
    }
    else {
        *width = 0;
        *height = 0;
    }
}

GLFWmonitor *glfwGetPrimaryMonitor(void) {
    return (GLFWmonitor *)1; /* monitor dummy: solo se usa con FULLSCREEN */
}

const GLFWvidmode *glfwGetVideoModes(GLFWmonitor *monitor, int *count) {
    (void)monitor;
    int n = SDL_GetNumDisplayModes(0);
    if (n < 0 || n > 8) {
        n = 1;
    }
    for (int i = 0; i < n; i++) {
        SDL_DisplayMode mode;
        if (SDL_GetDisplayMode(0, i, &mode) != 0) {
            memset(&g_modes[i], 0, sizeof(GLFWvidmode));
            continue;
        }
        g_modes[i].width = mode.w;
        g_modes[i].height = mode.h;
        g_modes[i].redBits = 8;
        g_modes[i].greenBits = 8;
        g_modes[i].blueBits = 8;
        g_modes[i].refreshRate = mode.refresh_rate;
    }
    if (count) {
        *count = n;
    }
    return g_modes;
}

/* ------------------------------------------------------------------ */
/* Cursor / input                                                      */
/* ------------------------------------------------------------------ */
void glfwGetCursorPos(GLFWwindow *window, double *xpos, double *ypos) {
    if (window) {
        if (xpos) *xpos = window->cx;
        if (ypos) *ypos = window->cy;
    }
}

void glfwSetCursorPos(GLFWwindow *window, double xpos, double ypos) {
    if (window) {
        window->cx = xpos;
        window->cy = ypos;
    }
}

void glfwSetInputMode(GLFWwindow *window, int mode, int value) {
    if (!window || mode != GLFW_CURSOR) {
        return;
    }
    if (value == GLFW_CURSOR_DISABLED) {
        SDL_SetRelativeMouseMode(SDL_TRUE);
        SDL_ShowCursor(SDL_DISABLE);
    }
    else {
        SDL_SetRelativeMouseMode(SDL_FALSE);
        SDL_ShowCursor(SDL_ENABLE);
    }
    window->cursor_mode = value;
}

int glfwGetInputMode(GLFWwindow *window, int mode) {
    if (!window || mode != GLFW_CURSOR) {
        return GLFW_CURSOR_NORMAL;
    }
    return window->cursor_mode;
}

int glfwGetKey(GLFWwindow *window, int key) {
    if (!window || key < 0 || key >= 512) {
        return GLFW_RELEASE;
    }
    return window->keys[key] ? GLFW_PRESS : GLFW_RELEASE;
}

int glfwGetMouseButton(GLFWwindow *window, int button) {
    (void)window;
    Uint32 mask = SDL_GetMouseState(NULL, NULL);
    switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT: return (mask & SDL_BUTTON_LMASK) ? 1 : 0;
    case GLFW_MOUSE_BUTTON_RIGHT: return (mask & SDL_BUTTON_RMASK) ? 1 : 0;
    case GLFW_MOUSE_BUTTON_MIDDLE: return (mask & SDL_BUTTON_MMASK) ? 1 : 0;
    default: return 0;
    }
}

const char *glfwGetClipboardString(GLFWwindow *window) {
    (void)window;
    static char copy[4096];
    char *text = SDL_GetClipboardText();
    if (!text) {
        return "";
    }
    snprintf(copy, sizeof(copy), "%s", text);
    SDL_free(text);
    return copy;
}

void glfwSetClipboardString(GLFWwindow *window, const char *string) {
    (void)window;
    SDL_SetClipboardText(string);
}

/* ------------------------------------------------------------------ */
/* Tiempo                                                              */
/* ------------------------------------------------------------------ */
double glfwGetTime(void) {
    Uint64 now = perf_now();
    g_time += (double)(now - g_last) / (double)g_freq;
    g_last = now;
    return g_time;
}

void glfwSetTime(double time) {
    g_time = time;
    time_sync();
}

/* ------------------------------------------------------------------ */
/* Callbacks                                                           */
/* ------------------------------------------------------------------ */
GLFWkeyfun glfwSetKeyCallback(GLFWwindow *window, GLFWkeyfun cbfun) {
    GLFWkeyfun old = window ? window->key_cb : NULL;
    if (window) window->key_cb = cbfun;
    return old;
}

GLFWcharfun glfwSetCharCallback(GLFWwindow *window, GLFWcharfun cbfun) {
    GLFWcharfun old = window ? window->char_cb : NULL;
    if (window) window->char_cb = cbfun;
    return old;
}

GLFWmousebuttonfun glfwSetMouseButtonCallback(GLFWwindow *window,
    GLFWmousebuttonfun cbfun)
{
    GLFWmousebuttonfun old = window ? window->mouse_cb : NULL;
    if (window) window->mouse_cb = cbfun;
    return old;
}

GLFWscrollfun glfwSetScrollCallback(GLFWwindow *window, GLFWscrollfun cbfun) {
    GLFWscrollfun old = window ? window->scroll_cb : NULL;
    if (window) window->scroll_cb = cbfun;
    return old;
}

/* ------------------------------------------------------------------ */
/* API extra del shim                                                  */
/* ------------------------------------------------------------------ */
void glfwshim_set_touch_handler(GLFWtouchfun cbfun) {
    g_touch_cb = cbfun;
}

void glfwshim_set_key(int key, int pressed) {
    if (g_win && key >= 0 && key < 512) {
        g_win->keys[key] = pressed ? 1 : 0;
    }
}

void glfwshim_add_relative(double dx, double dy) {
    if (g_win) {
        g_win->cx += dx;
        g_win->cy += dy;
    }
}

void glfwshim_get_window_size(int *width, int *height) {
    if (g_win && g_win->sdl) {
        SDL_GetWindowSize(g_win->sdl, width, height);
    }
    else {
        *width = 0;
        *height = 0;
    }
}

/* ------------------------------------------------------------------ */
/* Bucle de eventos                                                    */
/* ------------------------------------------------------------------ */
void glfwPollEvents(void) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_QUIT:
#ifdef CRAFT_MOBILE
            /* Boton Atras de Android: primero actua como ESC (salir del
             * juego/menus), despues cierra. */
            if (g_win && g_win->cursor_mode == GLFW_CURSOR_DISABLED) {
                if (g_win->key_cb) {
                    g_win->key_cb(g_win, GLFW_KEY_ESCAPE, 0, GLFW_PRESS, 0);
                }
            }
            else if (g_win) {
                g_win->should_close = 1;
            }
#else
            if (g_win) {
                g_win->should_close = 1;
            }
#endif
            break;
        case SDL_WINDOWEVENT:
            if (e.window.event == SDL_WINDOWEVENT_CLOSE) {
                if (g_win) g_win->should_close = 1;
            }
            break;
        case SDL_APP_TERMINATING:
            if (g_win) g_win->should_close = 1;
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            if (!g_win) break;
            int key = scancode_to_glfw(e.key.keysym.scancode);
            if (key < 0 || key >= 512) break;
            int action = (e.type == SDL_KEYUP) ? GLFW_RELEASE :
                (e.key.repeat ? GLFW_REPEAT : GLFW_PRESS);
            if (action != GLFW_RELEASE) {
                g_win->keys[key] = 1;
            }
            else {
                g_win->keys[key] = 0;
            }
            if (g_win->key_cb) {
                g_win->key_cb(g_win, key, e.key.keysym.scancode, action,
                    mods_from_sdl(e.key.keysym.mod));
            }
            break;
        }
        case SDL_TEXTINPUT:
            dispatch_text(e.text.text);
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            if (!g_win) break;
            int button;
            switch (e.button.button) {
            case SDL_BUTTON_LEFT: button = GLFW_MOUSE_BUTTON_LEFT; break;
            case SDL_BUTTON_RIGHT: button = GLFW_MOUSE_BUTTON_RIGHT; break;
            case SDL_BUTTON_MIDDLE: button = GLFW_MOUSE_BUTTON_MIDDLE; break;
            default: button = -1;
            }
            if (button >= 0 && g_win->mouse_cb) {
                g_win->mouse_cb(g_win, button,
                    e.type == SDL_MOUSEBUTTONDOWN ? GLFW_PRESS : GLFW_RELEASE,
                    mods_from_sdl(SDL_GetModState()));
            }
            break;
        }
        case SDL_MOUSEWHEEL:
            if (g_win && g_win->scroll_cb) {
                g_win->scroll_cb(g_win, 0, e.wheel.y);
            }
            break;
        case SDL_MOUSEMOTION:
            if (g_win) {
                if (g_win->cursor_mode == GLFW_CURSOR_DISABLED) {
                    g_win->cx += e.motion.xrel;
                    g_win->cy += e.motion.yrel;
                }
                else {
                    g_win->cx = e.motion.x;
                    g_win->cy = e.motion.y;
                }
            }
            break;
        case SDL_FINGERDOWN:
        case SDL_FINGERMOTION:
        case SDL_FINGERUP:
            if (g_touch_cb) {
                int ww, wh;
                glfwshim_get_window_size(&ww, &wh);
                int type = e.type == SDL_FINGERDOWN ? GLFW_TOUCH_DOWN :
                    (e.type == SDL_FINGERUP ? GLFW_TOUCH_UP : GLFW_TOUCH_MOVE);
                g_touch_cb(type, (long)e.tfinger.fingerId,
                    e.tfinger.x * ww, e.tfinger.y * wh,
                    e.tfinger.dx * ww, e.tfinger.dy * wh);
            }
            break;
        default:
            break;
        }
    }
}
