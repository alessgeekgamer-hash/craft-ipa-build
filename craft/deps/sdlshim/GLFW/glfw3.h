/*
 * glfw3.h - CAPA DE COMPATIBILIDAD GLFW sobre SDL2 (puerto multiplataforma)
 * ============================================================
 * Este NO es el GLFW original. Es un reemplazo que implementa el subconjunto
 * de la API de GLFW que usa Craft, encima de SDL2, para que el mismo codigo
 * compila en Windows (EXE), Android (APK) e iOS (IPA).
 *
 * Autor del shim: puerto de Minecraft-Port. Licencia MIT (igual que Craft).
 *
 * Como funciona:
 *  - glfwInit/glfwCreateWindow/etc. -> SDL_Init/SDL_CreateWindow/etc.
 *  - Teclado/raton -> eventos SDL mapeados a codigos GLFW (mismos numeros).
 *  - Cursor "exclusivo" (mirar alrededor) -> SDL relative mouse mode.
 *  - Tiempo (glfwGetTime/glfwSetTime) -> reloj propio de alta precision.
 *  - En movil: incluye SDL_main.h (renombra main -> SDL_main, requerido por
 *    SDL en Windows/Android/iOS) y expone funciones extra glfwshim_* para los
 *    controles tactiles definidos en main.c.
 */
#ifndef _glfw3_h_
#define _glfw3_h_

#include <SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------- tipos ---------------- */

typedef struct GLFWwindow GLFWwindow;
typedef struct GLFWmonitor GLFWmonitor;

typedef struct GLFWvidmode {
    int width;
    int height;
    int redBits;
    int greenBits;
    int blueBits;
    int refreshRate;
} GLFWvidmode;

typedef void (*GLFWkeyfun)(GLFWwindow *window, int key, int scancode,
    int action, int mods);
typedef void (*GLFWcharfun)(GLFWwindow *window, unsigned int codepoint);
typedef void (*GLFWmousebuttonfun)(GLFWwindow *window, int button,
    int action, int mods);
typedef void (*GLFWscrollfun)(GLFWwindow *window, double xoffset,
    double yoffset);
/* tipos de accion tactil para glfwshim_set_touch_handler */
#define GLFW_TOUCH_DOWN  0
#define GLFW_TOUCH_MOVE  1
#define GLFW_TOUCH_UP    2
typedef void (*GLFWtouchfun)(int type, long finger, double x, double y,
    double dx, double dy);

/* ---------------- acciones ---------------- */

#define GLFW_RELEASE 0
#define GLFW_PRESS 1
#define GLFW_REPEAT 2

/* ---------------- modos de input ---------------- */

#define GLFW_CURSOR 0x00033001
#define GLFW_CURSOR_NORMAL 0x00034001
#define GLFW_CURSOR_HIDDEN 0x00034002
#define GLFW_CURSOR_DISABLED 0x00034003

/* ---------------- mods ---------------- */

#define GLFW_MOD_SHIFT 0x0001
#define GLFW_MOD_CONTROL 0x0002
#define GLFW_MOD_ALT 0x0004
#define GLFW_MOD_SUPER 0x0008

/* ---------------- botones de raton ---------------- */

#define GLFW_MOUSE_BUTTON_1 0
#define GLFW_MOUSE_BUTTON_2 1
#define GLFW_MOUSE_BUTTON_3 2
#define GLFW_MOUSE_BUTTON_LEFT GLFW_MOUSE_BUTTON_1
#define GLFW_MOUSE_BUTTON_RIGHT GLFW_MOUSE_BUTTON_2
#define GLFW_MOUSE_BUTTON_MIDDLE GLFW_MOUSE_BUTTON_3

/* ---------------- teclas (valores oficiales de GLFW 3.x) ---------------- */

#define GLFW_KEY_SPACE 32
#define GLFW_KEY_APOSTROPHE 39
#define GLFW_KEY_COMMA 44
#define GLFW_KEY_MINUS 45
#define GLFW_KEY_PERIOD 46
#define GLFW_KEY_SLASH 47
#define GLFW_KEY_0 48
#define GLFW_KEY_1 49
#define GLFW_KEY_2 50
#define GLFW_KEY_3 51
#define GLFW_KEY_4 52
#define GLFW_KEY_5 53
#define GLFW_KEY_6 54
#define GLFW_KEY_7 55
#define GLFW_KEY_8 56
#define GLFW_KEY_9 57
#define GLFW_KEY_SEMICOLON 59
#define GLFW_KEY_EQUAL 61
#define GLFW_KEY_A 65
#define GLFW_KEY_B 66
#define GLFW_KEY_C 67
#define GLFW_KEY_D 68
#define GLFW_KEY_E 69
#define GLFW_KEY_F 70
#define GLFW_KEY_G 71
#define GLFW_KEY_H 72
#define GLFW_KEY_I 73
#define GLFW_KEY_J 74
#define GLFW_KEY_K 75
#define GLFW_KEY_L 76
#define GLFW_KEY_M 77
#define GLFW_KEY_N 78
#define GLFW_KEY_O 79
#define GLFW_KEY_P 80
#define GLFW_KEY_Q 81
#define GLFW_KEY_R 82
#define GLFW_KEY_S 83
#define GLFW_KEY_T 84
#define GLFW_KEY_U 85
#define GLFW_KEY_V 86
#define GLFW_KEY_W 87
#define GLFW_KEY_X 88
#define GLFW_KEY_Y 89
#define GLFW_KEY_Z 90
#define GLFW_KEY_ESCAPE 256
#define GLFW_KEY_ENTER 257
#define GLFW_KEY_TAB 258
#define GLFW_KEY_BACKSPACE 259
#define GLFW_KEY_INSERT 260
#define GLFW_KEY_DELETE 261
#define GLFW_KEY_RIGHT 262
#define GLFW_KEY_LEFT 263
#define GLFW_KEY_DOWN 264
#define GLFW_KEY_UP 265
#define GLFW_KEY_PAGE_UP 266
#define GLFW_KEY_PAGE_DOWN 267
#define GLFW_KEY_HOME 268
#define GLFW_KEY_END 269
#define GLFW_KEY_CAPS_LOCK 280
#define GLFW_KEY_SCROLL_LOCK 281
#define GLFW_KEY_NUM_LOCK 282
#define GLFW_KEY_PRINT_SCREEN 283
#define GLFW_KEY_PAUSE 284
#define GLFW_KEY_F1 290
#define GLFW_KEY_F2 291
#define GLFW_KEY_F3 292
#define GLFW_KEY_F4 293
#define GLFW_KEY_F5 294
#define GLFW_KEY_F6 295
#define GLFW_KEY_F7 296
#define GLFW_KEY_F8 297
#define GLFW_KEY_F9 298
#define GLFW_KEY_F10 299
#define GLFW_KEY_F11 300
#define GLFW_KEY_F12 301
#define GLFW_KEY_LEFT_SHIFT 340
#define GLFW_KEY_LEFT_CONTROL 341
#define GLFW_KEY_LEFT_ALT 342
#define GLFW_KEY_LEFT_SUPER 343
#define GLFW_KEY_RIGHT_SHIFT 344
#define GLFW_KEY_RIGHT_CONTROL 345
#define GLFW_KEY_RIGHT_ALT 346
#define GLFW_KEY_RIGHT_SUPER 347
#define GLFW_KEY_MENU 348

/* ---------------- API GLFW (subconjunto usado por Craft) ---------------- */

int glfwInit(void);
void glfwTerminate(void);

GLFWwindow *glfwCreateWindow(int width, int height, const char *title,
    GLFWmonitor *monitor, GLFWwindow *share);
void glfwDestroyWindow(GLFWwindow *window);
void glfwMakeContextCurrent(GLFWwindow *window);
void glfwSwapBuffers(GLFWwindow *window);
void glfwSwapInterval(int interval);
void glfwPollEvents(void);
int glfwWindowShouldClose(GLFWwindow *window);
void glfwSetWindowShouldClose(GLFWwindow *window, int value);
void glfwGetFramebufferSize(GLFWwindow *window, int *width, int *height);
void glfwGetWindowSize(GLFWwindow *window, int *width, int *height);

GLFWmonitor *glfwGetPrimaryMonitor(void);
const GLFWvidmode *glfwGetVideoModes(GLFWmonitor *monitor, int *count);

void glfwGetCursorPos(GLFWwindow *window, double *xpos, double *ypos);
void glfwSetCursorPos(GLFWwindow *window, double xpos, double ypos);
void glfwSetInputMode(GLFWwindow *window, int mode, int value);
int glfwGetInputMode(GLFWwindow *window, int mode);
int glfwGetKey(GLFWwindow *window, int key);
int glfwGetMouseButton(GLFWwindow *window, int button);
const char *glfwGetClipboardString(GLFWwindow *window);
void glfwSetClipboardString(GLFWwindow *window, const char *string);

double glfwGetTime(void);
void glfwSetTime(double time);

GLFWkeyfun glfwSetKeyCallback(GLFWwindow *window, GLFWkeyfun cbfun);
GLFWcharfun glfwSetCharCallback(GLFWwindow *window, GLFWcharfun cbfun);
GLFWmousebuttonfun glfwSetMouseButtonCallback(GLFWwindow *window,
    GLFWmousebuttonfun cbfun);
GLFWscrollfun glfwSetScrollCallback(GLFWwindow *window, GLFWscrollfun cbfun);

/* ---------------- API extra del shim (controles tactiles / utilidades) ---- */

/* Registra el manejador de eventos tactiles (movil). type: DOWN/MOVE/UP. */
void glfwshim_set_touch_handler(GLFWtouchfun cbfun);
/* Fuerza una tecla (para el joystick virtual y botones en pantalla). */
void glfwshim_set_key(int key, int pressed);
/* Acumula movimiento relativo del cursor (drag de camara en tactil). */
void glfwshim_add_relative(double dx, double dy);
/* Tamano de la ventana en pixeles de ventana (para mapear touch -> pixeles) */
void glfwshim_get_window_size(int *width, int *height);

#ifdef __cplusplus
}
#endif

#endif /* _glfw3_h_ */
