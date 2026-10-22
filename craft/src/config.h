#ifndef _config_h_
#define _config_h_

// app parameters
#define DEBUG 0
#define FULLSCREEN 0
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#define VSYNC 1
#define SCROLL_THRESHOLD 0.1
#define MAX_MESSAGES 4
#define DB_PATH "craft.db"
#define USE_CACHE 1
#define DAY_LENGTH 600
#define INVERT_MOUSE 0

// ============================================================
// PERSONALIZACION (edita y recompila - ver LEEME.txt)
// ============================================================
// Nombre de la ventana / titulo del juego (en movil se usa el del
// manejador: AndroidManifest.xml y Info.plist)
#define APP_NAME "Craft"
// Carga reloj dia/noche en segundos desde el inicio (3 = amanecer)
#define START_TIME_FACTOR 3
// Radios de generacion/renderizado de chunks (mas grande = mas mundo visible)
// (CREATE_CHUNK_RADIUS / RENDER_CHUNK_RADIUS estan mas abajo)
// Controles tactiles en Android/iOS (0 = desactivar, raton virtual)
#define TOUCH_CONTROLS 1
// Sensibilidad del raton (mas bajo = mas lenta la camara en escritorio)
#define MOUSE_SENSITIVITY 0.0012
// Escala extra para los gestos tactiles de la camara (compensa la
// sensibilidad lenta del raton en movil)
#define TOUCH_LOOK_SCALE 2.0
// Tecla para alternar entre modo creativo y modo supervivencia
#define CRAFT_KEY_GAMEMODE 'G'
// Log de depuracion detallado (0 = silencioso; 1 = imprime cada evento
// tactil/raton en la consola y en craft_log.txt)
#define DEBUG_TOUCH 0
// Radios de chunks en movil (menos = mas FPS en telefonos)
#define MOBILE_CREATE_CHUNK_RADIUS 7
#define MOBILE_RENDER_CHUNK_RADIUS 7
#define MOBILE_DELETE_CHUNK_RADIUS 10
// ============================================================

// rendering options
#define SHOW_LIGHTS 1
#define SHOW_PLANTS 1
#define SHOW_CLOUDS 1
#define SHOW_TREES 1
#define SHOW_ITEM 1
#define SHOW_CROSSHAIRS 1
#define SHOW_WIREFRAME 1
#define SHOW_INFO_TEXT 1
#define SHOW_CHAT_TEXT 1
#define SHOW_PLAYER_NAMES 1

// key bindings
#define CRAFT_KEY_FORWARD 'W'
#define CRAFT_KEY_BACKWARD 'S'
#define CRAFT_KEY_LEFT 'A'
#define CRAFT_KEY_RIGHT 'D'
#define CRAFT_KEY_JUMP GLFW_KEY_SPACE
#define CRAFT_KEY_FLY GLFW_KEY_TAB
#define CRAFT_KEY_OBSERVE 'O'
#define CRAFT_KEY_OBSERVE_INSET 'P'
#define CRAFT_KEY_ITEM_NEXT 'E'
#define CRAFT_KEY_ITEM_PREV 'R'
#define CRAFT_KEY_ZOOM GLFW_KEY_LEFT_SHIFT
#define CRAFT_KEY_ORTHO 'F'
#define CRAFT_KEY_CHAT 't'
#define CRAFT_KEY_COMMAND '/'
#define CRAFT_KEY_SIGN '`'

// advanced parameters
#define CREATE_CHUNK_RADIUS 10
#define RENDER_CHUNK_RADIUS 10
#define RENDER_SIGN_RADIUS 4
#define DELETE_CHUNK_RADIUS 14
#define CHUNK_SIZE 32
#define COMMIT_INTERVAL 5

#endif
