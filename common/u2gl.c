#include <stdio.h>
#include <math.h>
#include "u2gl.h"

static float tex_coords[8];

void u2gl_set_tex_coords(float *coords)
{
	memcpy(tex_coords, coords, 8 * sizeof(float));
}

void u2gl_check_error(char *t)
{
	int e = glGetError();
	if (e != GL_NO_ERROR) {
		fprintf(stderr, "[%s] glGetError: 0x%X\n", t, e);
	}
}

void u2gl_draw_triangle_strip(struct u2gl_program *p, float *obj, int num)
{
	glEnableVertexAttribArray(p->aPosition_location);
	glVertexAttribPointer(p->aPosition_location, 3, GL_FLOAT,
				GL_FALSE, 3 * sizeof(float), obj);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, num);
	glDisableVertexAttribArray(p->aPosition_location);
}

void u2gl_draw_triangle_fan(struct u2gl_program *p, float *obj, int num)
{
	glEnableVertexAttribArray(p->aPosition_location);
	glVertexAttribPointer(p->aPosition_location, 3, GL_FLOAT,
				GL_FALSE, 3 * sizeof(float), obj);
	glDrawArrays(GL_TRIANGLE_FAN, 0, num);
	glDisableVertexAttribArray(p->aPosition_location);
}

void u2gl_set_light_position(struct u2gl_program *p, float *pos)
{
	// Normalize position
	float vec[3];
	float size = sqrt(pos[0] * pos[0] + pos[1] * pos[1] + pos[2] * pos[2]);

	vec[0] = pos[0] / size;
	vec[1] = pos[1] / size;
	vec[2] = pos[2] / size;


	glUniform3fv(p->uLight_location, 1, vec);
}

void u2gl_draw_diffuse_triangle_fan(struct u2gl_program *p, float *obj, float *norm, int num)
{
	glEnableVertexAttribArray(p->aPosition_location);
	glVertexAttribPointer(p->aPosition_location, 3, GL_FLOAT,
				GL_FALSE, 3 * sizeof(float), obj);
	glEnableVertexAttribArray(p->aNormal_location);
	glVertexAttribPointer(p->aNormal_location, 3, GL_FLOAT,
				GL_FALSE, 3 * sizeof(float), norm);
	glDrawArrays(GL_TRIANGLE_FAN, 0, num);
	glDisableVertexAttribArray(p->aPosition_location);
}

void u2gl_draw_textured_triangle_strip(struct u2gl_program *p, float *obj, int num)
{
	glEnableVertexAttribArray(p->aPosition_location);
	glVertexAttribPointer(p->aPosition_location, 3, GL_FLOAT,
					0, 3 * sizeof(float), obj);

        glEnableVertexAttribArray(p->aTexPosition_location);
        glVertexAttribPointer(p->aTexPosition_location, 2, GL_FLOAT,
				GL_FALSE, 2 * sizeof(float), (void*)tex_coords);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, num);
	glDisableVertexAttribArray(p->aPosition_location);
	//u2gl_check_error("DRAW_TEXTURED");
}

void u2gl_projection(float left, float right, float bottom, float top, struct u2gl_program *p)
{
	float far = 1000.0f, near = -1000.0f;
	float a = 2.0f / (right - left);
	float b = 2.0f / (top - bottom);
	float c = -2.0f / (far - near);

	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	float tz = -(far + near) / (far - near);

	float ortho[16] = {
		a, 0, 0, 0,
		0, b, 0, 0,
		0, 0, c, 0,
		tx, ty, tz, 1
	};

	glUseProgram(p->program);
	glUniformMatrix4fv(p->pMatrix_location, 1, 0, ortho);
}

int u2gl_create_program(struct u2gl_program *p, GLuint v, GLuint f)
{
	char msg[512];
	Matrix m;

	p->program = glCreateProgram();
	glAttachShader(p->program, v);
	glAttachShader(p->program, f);
	glBindAttribLocation(p->program, 0, "position");
	u2gl_check_error("ATTACH");

	glLinkProgram(p->program);
	glGetProgramInfoLog(p->program, sizeof msg, NULL, msg);
	printf("program info: %s\n", msg);

	p->pMatrix_location = glGetUniformLocation(p->program, "pMatrix");
	p->uMatrix_location = glGetUniformLocation(p->program, "uMatrix");
	p->aPosition_location = glGetAttribLocation(p->program, "aPosition");
	p->uColor_location = glGetUniformLocation(p->program, "uColor");
	p->aTexPosition_location = glGetAttribLocation(p->program, "aTexPosition");

	matrix_identity(m);
        u2gl_set_matrix(p, m);

	return 0;
}

GLuint u2gl_compile_vertex_shader(const char *p)
{
	GLuint v = glCreateShader(GL_VERTEX_SHADER);
	char msg[512];

	glShaderSource(v, 1, &p, NULL);
	glCompileShader(v);
	glGetShaderInfoLog(v, sizeof msg, NULL, msg);
	printf("vertex shader info: %s\n", msg);

	return v;
}

GLuint u2gl_compile_fragment_shader(const char *p)
{
	GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
	char msg[512];

	glShaderSource(f, 1, &p, NULL);
	glCompileShader(f);
	glGetShaderInfoLog(f, sizeof msg, NULL, msg);
	printf("fragment shader info: %s\n", msg);

	return f;
}

