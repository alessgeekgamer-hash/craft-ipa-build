/*
 * GL/glew.h - STUB movil (Android / iOS)
 * =======================================
 * En escritorio se usa el GLEW real (deps/glew). En movil no existe GLEW:
 * OpenGL ES 2.0 ya trae todas las funciones de GL2 incluidas, asi que este
 * stub solo incluye los headers de ES2 y anula glewInit().
 *
 * Orden de includes en CMake: en movil este directorio (deps/sdlshim) va
 * PRIMERO, por eso resuelve aqui; en escritorio deps/glew/include va primero.
 */
#ifndef _glew_stub_movil_h_
#define _glew_stub_movil_h_

#if defined(__APPLE__)
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#define GLEW_OK 0
#define glewInit() GLEW_OK

/* glLogicOp / GL_COLOR_LOGIC_OP no existen en OpenGL ES 2.0.
 * El codigo de Craft los protege con #ifndef CRAFT_MOBILE. */

#endif /* _glew_stub_movil_h_ */
