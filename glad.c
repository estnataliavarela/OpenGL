#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/glad.h>

struct gladGLversionStruct GLVersion = { 0, 0 };

PFNGLGENBUFFERSPROC glad_glGenBuffers = NULL;
PFNGLBINDBUFFERPROC glad_glBindBuffer = NULL;
PFNGLBUFFERDATAPROC glad_glBufferData = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC glad_glEnableVertexAttribArray = NULL;
PFNGLVERTEXATTRIBPOINTERPROC glad_glVertexAttribPointer = NULL;
PFNGLGENVERTEXARRAYSPROC glad_glGenVertexArrays = NULL;
PFNGLBINDVERTEXARRAYPROC glad_glBindVertexArray = NULL;
PFNGLCREATESHADERPROC glad_glCreateShader = NULL;
PFNGLSHADERSOURCEPROC glad_glShaderSource = NULL;
PFNGLCOMPILESHADERPROC glad_glCompileShader = NULL;
PFNGLCREATEPROGRAMPROC glad_glCreateProgram = NULL;
PFNGLATTACHSHADERPROC glad_glAttachShader = NULL;
PFNGLLINKPROGRAMPROC glad_glLinkProgram = NULL;
PFNGLUSEPROGRAMPROC glad_glUseProgram = NULL;
PFNGLGETUNIFORMLOCATIONPROC glad_glGetUniformLocation = NULL;
PFNGLUNIFORMMATRIX4FVPROC glad_glUniformMatrix4fv = NULL;
PFNGLDRAWARRAYSPROC glad_glDrawArrays = NULL;
PFNGLDRAWELEMENTSPROC glad_glDrawElements = NULL;
PFNGLDELETESHADERPROC glad_glDeleteShader = NULL;
PFNGLDELETEPROGRAMPROC glad_glDeleteProgram = NULL;
PFNGLDELETEVERTEXARRAYSPROC glad_glDeleteVertexArrays = NULL;
PFNGLDELETEBUFFERSPROC glad_glDeleteBuffers = NULL;

// Funciones adicionales faltantes que causaban el error LNK2001
PFNGLVIEWPORTPROC glad_glViewport = NULL;
PFNGLCLEARPROC glad_glClear = NULL;
PFNGLCLEARCOLORPROC glad_glClearColor = NULL;
PFNGLENABLEPROC glad_glEnable = NULL;
PFNGLBLENDFUNCPROC glad_glBlendFunc = NULL;
PFNGLGENTEXTURESPROC glad_glGenTextures = NULL;
PFNGLBINDTEXTUREPROC glad_glBindTexture = NULL;
PFNGLTEXPARAMETERIPROC glad_glTexParameteri = NULL;
PFNGLTEXIMAGE2DPROC glad_glTexImage2D = NULL;
PFNGLGENERATEMIPMAPPROC glad_glGenerateMipmap = NULL;
PFNGLPIXELSTOREIPROC glad_glPixelStorei = NULL;

static void load_GL_version_33(GLADloadproc load) {
    glad_glGenBuffers = (PFNGLGENBUFFERSPROC)load("glGenBuffers");
    glad_glBindBuffer = (PFNGLBINDBUFFERPROC)load("glBindBuffer");
    glad_glBufferData = (PFNGLBUFFERDATAPROC)load("glBufferData");
    glad_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)load("glEnableVertexAttribArray");
    glad_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)load("glVertexAttribPointer");
    glad_glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)load("glGenVertexArrays");
    glad_glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)load("glBindVertexArray");
    glad_glCreateShader = (PFNGLCREATESHADERPROC)load("glCreateShader");
    glad_glShaderSource = (PFNGLSHADERSOURCEPROC)load("glShaderSource");
    glad_glCompileShader = (PFNGLCOMPILESHADERPROC)load("glCompileShader");
    glad_glCreateProgram = (PFNGLCREATEPROGRAMPROC)load("glCreateProgram");
    glad_glAttachShader = (PFNGLATTACHSHADERPROC)load("glAttachShader");
    glad_glLinkProgram = (PFNGLLINKPROGRAMPROC)load("glLinkProgram");
    glad_glUseProgram = (PFNGLUSEPROGRAMPROC)load("glUseProgram");
    glad_glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)load("glGetUniformLocation");
    glad_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)load("glUniformMatrix4fv");
    glad_glDrawArrays = (PFNGLDRAWARRAYSPROC)load("glDrawArrays");
    glad_glDrawElements = (PFNGLDRAWELEMENTSPROC)load("glDrawElements");
    glad_glDeleteShader = (PFNGLDELETESHADERPROC)load("glDeleteShader");
    glad_glDeleteProgram = (PFNGLDELETEPROGRAMPROC)load("glDeleteProgram");
    glad_glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)load("glDeleteVertexArrays");
    glad_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)load("glDeleteBuffers");

    // Asignación de punteros para las funciones faltantes
    glad_glViewport = (PFNGLVIEWPORTPROC)load("glViewport");
    glad_glClear = (PFNGLCLEARPROC)load("glClear");
    glad_glClearColor = (PFNGLCLEARCOLORPROC)load("glClearColor");
    glad_glEnable = (PFNGLENABLEPROC)load("glEnable");
    glad_glBlendFunc = (PFNGLBLENDFUNCPROC)load("glBlendFunc");
    glad_glGenTextures = (PFNGLGENTEXTURESPROC)load("glGenTextures");
    glad_glBindTexture = (PFNGLBINDTEXTUREPROC)load("glBindTexture");
    glad_glTexParameteri = (PFNGLTEXPARAMETERIPROC)load("glTexParameteri");
    glad_glTexImage2D = (PFNGLTEXIMAGE2DPROC)load("glTexImage2D");
    glad_glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)load("glGenerateMipmap");
    glad_glPixelStorei = (PFNGLPIXELSTOREIPROC)load("glPixelStorei");
}

int gladLoadGLLoader(GLADloadproc load) {
    GLVersion.major = 3;
    GLVersion.minor = 3;
    load_GL_version_33(load);
    return 1;
}

int gladLoadGL(void) {
    return 1;
}