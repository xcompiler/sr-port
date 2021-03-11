#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <SOIL/SOIL.h>
#include "u2gl.h"

static int view_width;
static int view_height;

static struct u2gl_program triangle_program;
static struct u2gl_program diffuse_triangle_program;

float obj[16 * 3];
float norm[16 * 3];

float fc_obj[12] = {
	0.0f, 0.0f, 0.0f,
	320.0f, 0.0f, 0.0f,
	0.0f, 50.0f, 0.0f,
	320.0f, 50.0f, 0.0f
};

float rectangle1_obj[12] = {
	70.0f, 0.0f, 0.0f,
	250.0f, 0.0f, 0.0f,
	70.0f, 200.0f, 0.0f,
	250.0f, 200.0f, 0.0f
};

float rectangle2_obj[12] = {
	0.0f, 24.0f, 0.0f,
	320.0f, 24.0f, 0.0f,
	0.0f, 174.0f, 0.0f,
	320.0f, 174.0f, 0.0f
};

float rectangle3_obj[12] = {
	0.0f, 0.0f, 0.0f,
	320.0f, 9.0f, 0.0f,
	0.0f, 200.0f, 0.0f,
	320.0f, 200.0f, 0.0f
};

static const char vertex_shader[] =
"uniform mat4 pMatrix;\n"
"uniform mat4 uMatrix;\n"
"attribute vec4 aPosition;\n"
"varying vec3 vPosition;\n"
"\n"
"void main(){\n"
"    mat4 Matrix = pMatrix * uMatrix;\n"
"    vec4 position = Matrix * aPosition;\n"
"    gl_Position = position;\n"
"    vPosition = vec3(position);\n"
"}\n";

static const char fragment_shader[] =
//"precision mediump float;\n"
"uniform vec4 uColor;\n"
"varying vec3 vPosition;\n"
"\n"
"void main() {\n"
"    gl_FragColor = uColor;\n"
"}\n";

static const char vertex_shader_diffuse[] =
"uniform mat4 pMatrix;\n"
"uniform mat4 uMatrix;\n"
"attribute vec4 aPosition;\n"		// projected vertex coordinates
"attribute vec3 aNormal;\n"
"uniform vec3 uLight;\n"		// light source position
"varying vec3 vPosition;\n"
"varying vec3 vNormal;\n"
"\n"
"void main(){\n"
"    mat4 Matrix = pMatrix * uMatrix;\n"
"    vec4 position = Matrix * aPosition;\n"
"    gl_Position = position;\n"
"    vPosition = vec3(position);\n"
"    vNormal = normalize(aNormal);\n"
"}\n";

static const char fragment_shader_diffuse[] =
//"precision mediump float;\n"
"uniform vec4 uColor;\n"
"uniform vec3 uLight;\n"
"varying vec3 vPosition;\n"
"varying vec3 vNormal;\n"
"\n"
"void main() {\n"
"    float d = dot(vNormal, uLight);\n"
"    vec4 c = 0.1 + 0.45 * uColor * (1.0 + d);\n"
"    gl_FragColor = vec4(c.xyz, 1.0);\n"
"}\n";

/*
static const char vertex_shader_texture[] =
"uniform mat4 pMatrix;\n"
"uniform mat4 uMatrix;\n"
"attribute vec4 aPosition;\n"
"varying vec3 vPosition;\n"
"attribute vec2 aTexPosition;\n"
"varying vec2 vTexPosition;\n"
"\n"
"void main() {\n"
"    mat4 Matrix = pMatrix * uMatrix;\n"
"    vec4 position = Matrix * aPosition;\n"
"    gl_Position = position;\n"
"    vTexPosition = aTexPosition;\n"
"    vPosition = vec3(position);\n"
"}\n";

static const char fragment_shader_texture[] =
//"precision mediump float;\n"
"uniform sampler2D uTexture;\n"
"uniform vec4 uColor;\n"
"varying vec3 vPosition;\n"
"varying vec2 vTexPosition;\n"
"\n"
"void main() {\n"
"    gl_FragColor = texture2D(uTexture, vTexPosition).rgba * uColor.x;\n"
"}\n";
*/


#if 0
static float tex_coords[] = {
	0.0f, 0.5f,
	1.0f, 0.5f,
	0.0f, 0.0f,
	1.0f, 0.0f
};
#endif


static float color[256][4];
#define CC 63

void setrgb(int c, int r, int g, int b, float a)
{
	color[c][0] = (float)r / CC;
	color[c][1] = (float)g / CC;
	color[c][2] = (float)b / CC;
	color[c][3] = a;
}

void getrgb(int c, char *p)
{
	p[0] = color[c][0] * CC;
	p[1] = color[c][1] * CC;
	p[2] = color[c][2] * CC;
}

void draw_poly(short *f, int sides, int c)
{
	int i;

	glUseProgram(triangle_program.program);
	u2gl_set_color(&triangle_program, color[c]);

	for (i = 0; i < sides; i++) {
		obj[i * 3 + 0] = *f++;
		obj[i * 3 + 1] = *f++;
	}

	u2gl_draw_triangle_fan(&triangle_program, obj, sides);
}

void draw_poly_diffuse(short *f, int *n, int sides, int c)
{
	int i;

	glUseProgram(diffuse_triangle_program.program);
	u2gl_set_color(&diffuse_triangle_program, color[c]);

	for (i = 0; i < sides; i++) {
		obj[i * 3 + 0] = *f++;
		obj[i * 3 + 1] = *f++;

		norm[i * 3 + 0] = *n++;
		norm[i * 3 + 1] = *n++;
		norm[i * 3 + 2] = *n++;
	}

	u2gl_draw_diffuse_triangle_fan(&diffuse_triangle_program, obj, norm, sides);
}

void draw_palette()
{
	int i;

	glUseProgram(triangle_program.program);
	for (i = 0; i < 256; i++) {
		u2gl_set_color(&triangle_program, color[i]);
	
		obj[0] = 20 + (i - 64 * (i / 64)) * 4;
		obj[1] = 70 -10 * (i / 64) ;
		obj[3] = obj[0];
		obj[4] = obj[1] - 10;
		obj[6] = obj[0] + 4;
		obj[7] = obj[1] - 10;
	
		u2gl_draw_triangle_strip(&triangle_program, obj, 3);
	}
}

void draw_rectangle1()
{
	glUseProgram(triangle_program.program);
	u2gl_set_color(&triangle_program, color[1]);
	u2gl_draw_triangle_strip(&triangle_program, rectangle1_obj, 4);
}

void draw_rectangle2()
{
	glUseProgram(triangle_program.program);
	u2gl_set_color(&triangle_program, color[2]);
	u2gl_draw_triangle_strip(&triangle_program, rectangle2_obj, 4);
}

void draw_rectangle3()
{
	glUseProgram(triangle_program.program);
	u2gl_set_color(&triangle_program, color[255]);
	u2gl_draw_triangle_strip(&triangle_program, rectangle3_obj, 4);
}

#if 0
static void init_texture()
{
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	u2gl_set_tex_coords(tex_coords);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height;
	unsigned char* image = SOIL_load_image("fc_2x.png",
				&width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
				GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);

	glGenerateMipmap(GL_TEXTURE_2D);
	u2gl_check_error("init_texture");
}
#endif

extern int window_width;
extern int window_height;

void set_window(int x, int y, int width, int height)
{
	y = 199 - y - height;

	glEnable(GL_SCISSOR_TEST);
	glScissor(window_width * x / view_width, window_height * y / view_height, window_width * width / view_width, window_height * height / view_height);
}

extern int newlight[3];

int init_opengl()
{
	GLuint v, f;
	float lightpos[3];

	view_width = 320;
	view_height = 200;

	v = u2gl_compile_vertex_shader(vertex_shader);
	f = u2gl_compile_fragment_shader(fragment_shader);
	u2gl_create_program(&triangle_program, f, v);

	v = u2gl_compile_vertex_shader(vertex_shader_diffuse);
	f = u2gl_compile_fragment_shader(fragment_shader_diffuse);
	u2gl_create_program(&diffuse_triangle_program, f, v);
	diffuse_triangle_program.aNormal_location =
		glGetAttribLocation(diffuse_triangle_program.program, "aNormal");
	diffuse_triangle_program.uLight_location =
		glGetUniformLocation(diffuse_triangle_program.program, "uLight");

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(.0, .0, .0, 0);

	lightpos[0] = newlight[0];
	lightpos[1] = newlight[1];
	lightpos[2] = newlight[2];
	u2gl_set_light_position(&diffuse_triangle_program, lightpos);

	u2gl_check_error("init_opengl");

	//init_texture();

	glClear(GL_COLOR_BUFFER_BIT);

	u2gl_projection(0, view_width, 0, view_height, &triangle_program);
	u2gl_projection(0, view_width, 0, view_height, &diffuse_triangle_program);

	return 0;
}

void blend_alpha()
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

void blend_color()
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void clear_screen()
{
	glClear(GL_COLOR_BUFFER_BIT);
}
