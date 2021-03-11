#ifndef U2_OPENGL_H_
#define U2_OPENGL_H_

/* #include <GL/gl.h> */
/* #include <GLES2/gl2.h> */
#include <SDL2/SDL_opengl.h>

extern GLAPI void GLAPIENTRY glAttachShader(GLuint program, GLuint shader);
extern GLAPI void GLAPIENTRY glBindAttribLocation(GLuint program, GLuint index, const GLchar *name);
extern GLAPI void GLAPIENTRY glBindFramebuffer(GLenum target, GLuint framebuffer);
extern GLAPI void GLAPIENTRY glCompileShader(GLuint shader);
extern GLAPI GLuint GLAPIENTRY glCreateProgram(void);
extern GLAPI GLuint GLAPIENTRY glCreateShader(GLenum shaderType);
extern GLAPI void GLAPIENTRY glDisableVertexAttribArray(GLuint index);
extern GLAPI void GLAPIENTRY glEnableVertexAttribArray(GLuint index);
extern GLAPI void GLAPIENTRY glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern GLAPI void GLAPIENTRY glGenerateMipmap(GLenum target);
extern GLAPI void GLAPIENTRY glGenFramebuffers(GLsizei n, GLuint * framebuffers);
extern GLAPI GLint GLAPIENTRY glGetAttribLocation(GLuint program, const GLchar *name);
extern GLAPI void GLAPIENTRY glGetProgramInfoLog(GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
extern GLAPI void GLAPIENTRY glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
extern GLAPI GLint GLAPIENTRY glGetUniformLocation(GLuint program, const GLchar *name);
extern GLAPI void GLAPIENTRY glLinkProgram(GLuint program);
extern GLAPI void GLAPIENTRY glShaderSource(GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
extern GLAPI void GLAPIENTRY glUniform1f(GLint location, GLfloat v0);
extern GLAPI void GLAPIENTRY glUniform1fv(GLint location, GLsizei count, const GLfloat *value);
extern GLAPI void GLAPIENTRY glUniform1i(GLint location, GLint v0);
extern GLAPI void GLAPIENTRY glUniform2fv(GLint location, GLsizei count, const GLfloat *value);
extern GLAPI void GLAPIENTRY glUniform3fv(GLint location, GLsizei count, const GLfloat *value);
extern GLAPI void GLAPIENTRY glUniform4fv (GLint location, GLsizei count, const GLfloat *value);
extern GLAPI void GLAPIENTRY glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern GLAPI void GLAPIENTRY glUseProgram (GLuint program);
extern GLAPI void GLAPIENTRY glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride,	const void * pointer);

#include <string.h>
#include "matrix.h"

struct u2gl_program {
	GLuint program;
	GLuint pMatrix_location;
	GLuint uMatrix_location;
	GLuint aPosition_location;
	GLuint aNormal_location;
	GLuint uLight_location;
	GLuint uColor_location;
	GLuint uPal_location;
	GLuint aTexPosition_location;
};

static inline void u2gl_set_color(struct u2gl_program *p, GLfloat *c) {
	glUseProgram(p->program);
	glUniform4fv(p->uColor_location, 1, c);
}

static inline void u2gl_set_matrix(struct u2gl_program *p, float *m) {
	glUseProgram(p->program);
	glUniformMatrix4fv(p->uMatrix_location, 1, 0, m);
}

void	u2gl_set_tex_coords(float *);
void	u2gl_check_error(char *);
void	u2gl_draw_triangle_strip(struct u2gl_program *, float *, int);
void	u2gl_draw_triangle_fan(struct u2gl_program *, float *, int);
void	u2gl_draw_textured_triangle_strip(struct u2gl_program *, float *, int);
void	u2gl_draw_diffuse_triangle_fan(struct u2gl_program *, float *, float *, int);
void	u2gl_set_light_position(struct u2gl_program *, float *);
void	u2gl_projection(float, float, float, float, struct u2gl_program *);
int	u2gl_create_program(struct u2gl_program *, GLuint, GLuint);
GLuint	u2gl_compile_vertex_shader(const char *);
GLuint	u2gl_compile_fragment_shader(const char *);

#endif
