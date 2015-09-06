// GL utilities, bare essentials
// By Ingemar Ragnemalm

// August 2012:
// FBO creation/usage routines.
// Geometry shader support synched with preliminary version.
// September 2012: Improved infolog printouts with file names.
// 120910: Clarified error messages from shader loader.
// 120913: Re-activated automatic framebuffer checks for UseFBO().
// Fixed FUBAR in InitFBO().
// 130228: Changed most printf's to stderr.
// 131014: Added tesselation shader support
// 150812: Added a NULL check on file names in readFile, makes Visual Studio happier.

//#define GL3_PROTOTYPES
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "GL_utilities.h"

// Shader loader

char* readFile(char *file)
{
	FILE *fptr;
	long length;
	char *buf;

	if (file == NULL)
			return NULL;

	fptr = fopen(file, "rb"); /* Open file for reading */
	if (!fptr) /* Return NULL on failure */
		return NULL;
	fseek(fptr, 0, SEEK_END); /* Seek to the end of the file */
	length = ftell(fptr); /* Find out how many bytes into the file we are */
	buf = (char*)malloc(length+1); /* Allocate a buffer for the entire length of the file and a null terminator */
	fseek(fptr, 0, SEEK_SET); /* Go back to the beginning of the file */
	fread(buf, length, 1, fptr); /* Read the contents of the file in to the buffer */
	fclose(fptr); /* Close the file */
	buf[length] = 0; /* Null terminator */
	
	return buf; /* Return the buffer */
}

// Infolog: Show result of shader compilation
void printShaderInfoLog(GLuint obj, const char *fn)
{
	GLint infologLength = 0;
	GLint charsWritten  = 0;
	char *infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 2)
	{
		fprintf(stderr, "[From %s:]\n", fn);
		infoLog = (char *)malloc(infologLength);
		glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		fprintf(stderr, "%s\n",infoLog);
		free(infoLog);
	}
}

void printProgramInfoLog(GLuint obj, const char *vfn, const char *ffn,
					const char *gfn, const char *tcfn, const char *tefn)
{
	GLint infologLength = 0;
	GLint charsWritten  = 0;
	char *infoLog;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

	if (infologLength > 2)
	{
		if (gfn == NULL)
			fprintf(stderr, "[From %s+%s:]\n", vfn, ffn);
		else
		if (tcfn == NULL || tefn == NULL)
			fprintf(stderr, "[From %s+%s+%s:]\n", vfn, ffn, gfn);
		else
			fprintf(stderr, "[From %s+%s+%s+%s+%s:]\n", vfn, ffn, gfn, tcfn, tefn);
		infoLog = (char *)malloc(infologLength);
		glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		fprintf(stderr, "%s\n",infoLog);
		free(infoLog);
	}
}

// Compile a shader, return reference to it
GLuint compileShaders(const char *vs, const char *fs, const char *gs, const char *tcs, const char *tes,
								const char *vfn, const char *ffn, const char *gfn, const char *tcfn, const char *tefn)
{
	GLuint v,f,g,tc,te,p;
	
	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(v, 1, &vs, NULL);
	glShaderSource(f, 1, &fs, NULL);
	glCompileShader(v);
	glCompileShader(f);
	if (gs != NULL)
	{
		g = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(g, 1, &gs, NULL);
		glCompileShader(g);
	}
#ifdef GL_TESS_CONTROL_SHADER
	if (tcs != NULL)
	{
		tc = glCreateShader(GL_TESS_CONTROL_SHADER);
		glShaderSource(tc, 1, &tcs, NULL);
		glCompileShader(tc);
	}
	if (tes != NULL)
	{
		te = glCreateShader(GL_TESS_EVALUATION_SHADER);
		glShaderSource(te, 1, &tes, NULL);
		glCompileShader(te);
	}
#endif	
	p = glCreateProgram();
	glAttachShader(p,v);
	glAttachShader(p,f);
	if (gs != NULL)
		glAttachShader(p,g);
	if (tcs != NULL)
		glAttachShader(p,tc);
	if (tes != NULL)
		glAttachShader(p,te);
	glLinkProgram(p);
	glUseProgram(p);
	
	printShaderInfoLog(v, vfn);
	printShaderInfoLog(f, ffn);
	if (gs != NULL)	printShaderInfoLog(g, gfn);
	if (tcs != NULL)	printShaderInfoLog(tc, tcfn);
	if (tes != NULL)	printShaderInfoLog(te, tefn);
	
	printProgramInfoLog(p, vfn, ffn, gfn, tcfn, tefn);
	
	return p;
}

GLuint loadShaders(const char *vertFileName, const char *fragFileName)
{
	return loadShadersGT(vertFileName, fragFileName, NULL, NULL, NULL);
}

GLuint loadShadersG(const char *vertFileName, const char *fragFileName, const char *geomFileName)
// With geometry shader support
{
	return loadShadersGT(vertFileName, fragFileName, geomFileName, NULL, NULL);
}

GLuint loadShadersGT(const char *vertFileName, const char *fragFileName, const char *geomFileName,
						const char *tcFileName, const char *teFileName)
// With tesselation shader support
{
	char *vs, *fs, *gs, *tcs, *tes;
	GLuint p = 0;
	
	vs = readFile((char *)vertFileName);
	fs = readFile((char *)fragFileName);
	gs = readFile((char *)geomFileName);
	tcs = readFile((char *)tcFileName);
	tes = readFile((char *)teFileName);
	if (vs==NULL)
		fprintf(stderr, "Failed to read %s from disk.\n", vertFileName);
	if (fs==NULL)
		fprintf(stderr, "Failed to read %s from disk.\n", fragFileName);
	if ((gs==NULL) && (geomFileName != NULL))
		fprintf(stderr, "Failed to read %s from disk.\n", geomFileName);
	if ((tcs==NULL) && (tcFileName != NULL))
		fprintf(stderr, "Failed to read %s from disk.\n", tcFileName);
	if ((tes==NULL) && (teFileName != NULL))
		fprintf(stderr, "Failed to read %s from disk.\n", teFileName);
	if ((vs!=NULL)&&(fs!=NULL))
		p = compileShaders(vs, fs, gs, tcs, tes, vertFileName, fragFileName, geomFileName, tcFileName, teFileName);
	if (vs != NULL) free(vs);
	if (fs != NULL) free(fs);
	if (gs != NULL) free(gs);
	if (tcs != NULL) free(tcs);
	if (tes != NULL) free(tes);
	return p;
}

// End of Shader loader

void dumpInfo(void)
{
   printf ("Vendor: %s\n", glGetString (GL_VENDOR));
   printf ("Renderer: %s\n", glGetString (GL_RENDERER));
   printf ("Version: %s\n", glGetString (GL_VERSION));
   printf ("GLSL: %s\n", glGetString (GL_SHADING_LANGUAGE_VERSION));
   printError ("dumpInfo");
}

static GLenum lastError = 0;
static char lastErrorFunction[1024] = "";

/* report GL errors, if any, to stderr */
void printError(const char *functionName)
{
   GLenum error;
   while (( error = glGetError() ) != GL_NO_ERROR)
   {
       if ((lastError != error) || (strcmp(functionName, lastErrorFunction)))
       {
	       fprintf (stderr, "GL error 0x%X detected in %s\n", error, functionName);
	       strcpy(lastErrorFunction, functionName);
	       lastError = error;
       }
   }
}


// Keymap mini manager
// Important! Uses glutKeyboardFunc/glutKeyboardUpFunc so you can't use them
// elsewhere or they will conflict.
// (This functionality is built-in in MicroGlut, as "glutKeyIsDown" where this conflict should not exist.)

char keymap[256];

char keyIsDown(unsigned char c)
{
	return keymap[(unsigned int)c];
}

void keyUp(unsigned char key, int x, int y)
{
	keymap[(unsigned int)key] = 0;
}

void keyDown(unsigned char key, int x, int y)
{
	keymap[(unsigned int)key] = 1;
}

void initKeymapManager()
{
	int i;
	for (i = 0; i < 256; i++) keymap[i] = 0;

	glutKeyboardFunc(keyDown);
	glutKeyboardUpFunc(keyUp);
}


// FBO

//----------------------------------FBO functions-----------------------------------
void CHECK_FRAMEBUFFER_STATUS()
{
	GLenum status;
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		fprintf(stderr, "Framebuffer not complete\n");
}

// create FBO
// FP buffer, suitable for HDR
FBOstruct *initFBO(int width, int height, int int_method)
{
	FBOstruct *fbo = malloc(sizeof(FBOstruct));

	fbo->width = width;
	fbo->height = height;

	// create objects
	glGenFramebuffers(1, &fbo->fb); // frame buffer id
	glBindFramebuffer(GL_FRAMEBUFFER, fbo->fb);
	glGenTextures(1, &fbo->texid);
	fprintf(stderr, "%i \n",fbo->texid);
	glBindTexture(GL_TEXTURE_2D, fbo->texid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	if (int_method == 0)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo->texid, 0);

	// Renderbuffer
	// initialize depth renderbuffer
    glGenRenderbuffers(1, &fbo->rb);
    glBindRenderbuffer(GL_RENDERBUFFER, fbo->rb);
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, fbo->width, fbo->height );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo->rb );
    CHECK_FRAMEBUFFER_STATUS();

	fprintf(stderr, "Framebuffer object %d\n", fbo->fb);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return fbo;
}

// create FBO, optionally with depth
// Integer buffer, not suitable for HDR!
FBOstruct *initFBO2(int width, int height, int int_method, int create_depthimage)
{
    FBOstruct *fbo = malloc(sizeof(FBOstruct));

    fbo->width = width;
    fbo->height = height;

    // create objects
    glGenRenderbuffers(1, &fbo->rb);
    glGenFramebuffers(1, &fbo->fb); // frame buffer id
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->fb);
    glGenTextures(1, &fbo->texid);
    fprintf(stderr, "%i \n",fbo->texid);
    glBindTexture(GL_TEXTURE_2D, fbo->texid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if (int_method == 0)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo->texid, 0);
    if (create_depthimage!=0)
    {
      glGenTextures(1, &fbo->depth);
      glBindTexture(GL_TEXTURE_2D, fbo->depth);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0L);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glBindTexture(GL_TEXTURE_2D, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fbo->depth, 0);	
      fprintf(stderr, "depthtexture: %i\n",fbo->depth);
    }

    // Renderbuffer
    // initialize depth renderbuffer
    glBindRenderbuffer(GL_RENDERBUFFER, fbo->rb);
    CHECK_FRAMEBUFFER_STATUS();

    fprintf(stderr, "Framebuffer object %d\n", fbo->fb);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return fbo;
}

static int lastw = 0;
static int lasth = 0;

// Obsolete
void updateScreenSizeForFBOHandler(int w, int h)
{
	lastw = w;
	lasth = h;
}

// choose input (textures) and output (FBO)
void useFBO(FBOstruct *out, FBOstruct *in1, FBOstruct *in2)
{
	GLint curfbo;

// This was supposed to catch changes in viewport size and update lastw/lasth.
// It worked for me in the past, but now it causes problems to I have to
// fall back to manual updating.
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &curfbo);
	if (curfbo == 0)
	{
		GLint viewport[4] = {0,0,0,0};
		GLint w, h;
		glGetIntegerv(GL_VIEWPORT, viewport);
		w = viewport[2] - viewport[0];
		h = viewport[3] - viewport[1];
		if ((w > 0) && (h > 0) && (w < 65536) && (h < 65536)) // I don't believe in 64k pixel wide frame buffers for quite some time
		{
			lastw = viewport[2] - viewport[0];
			lasth = viewport[3] - viewport[1];
		}
	}
	
	if (out != 0L)
		glViewport(0, 0, out->width, out->height);
	else
		glViewport(0, 0, lastw, lasth);

	if (out != 0L)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, out->fb);
		glViewport(0, 0, out->width, out->height);
	}
	else
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE1);
	if (in2 != 0L)
		glBindTexture(GL_TEXTURE_2D, in2->texid);
	else
		glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	if (in1 != 0L)
		glBindTexture(GL_TEXTURE_2D, in1->texid);
	else
		glBindTexture(GL_TEXTURE_2D, 0);
}
