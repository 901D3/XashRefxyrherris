/**
 * glsl2header - small program for convert shader files to header
 * By 901D3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GL21_VERSION  "#version 120"
#define GLES2_VERSION "#version 100 es"

enum {
  TARGET_GL21,
  TARGET_GLES2
};

typedef int GLTarget;

GLTarget target = TARGET_GL21;

#define WRITE_SPACE(charPtr, count) \
  { \
    for ( int i = 0; i < count; i++ ) \
      charPtr[i] = ' '; \
  }

static inline int startsWith(const char *s, const char *p) {
  return strncmp(s, p, strlen(p)) == 0;
}

static inline int containString(const char *s, const char *p) {
  return strstr(s, p) != NULL;
}

static int getLine(FILE *file, char *buffer, size_t size) {
  if ( !file || !buffer || size == 0 )
    return 0;

  size_t i = 0;
  int c;

  while ( i + 1 < size && (c = fgetc(file)) != EOF ) {
    if ( c == '\n' )
      break;

    buffer[i++] = (char)c;
  }

  buffer[i] = '\0';

  if ( c == EOF && i == 0 )
    return 0;

  return 1;
}

void stripPrecision(char *src) {
  char *r = src;
  char *w = src;

  while ( *r ) {

    if ( strncmp(r, "lowp", 4) == 0 ) {
      WRITE_SPACE(w, 4);
      r += 4;
      w += 4;
    } else if ( strncmp(r, "mediump", 7) == 0 ) {
      WRITE_SPACE(w, 7);
      r += 7;
      w += 7;
    } else if ( strncmp(r, "highp", 5) == 0 ) {
      WRITE_SPACE(w, 5);
      r += 5;
      w += 5;
    } else {
      *w++ = *r++;
    }
  }

  *w = '\0';
}

static void emitFile(FILE *out, FILE *in) {
  int skip = 0;
  char line[8192];
  while ( getLine(in, line, sizeof(line)) ) {
    if ( startsWith(line, "#ifdef") ) {
      if ( containString(line, "GLES") )
        skip = (target != TARGET_GLES2);
      else if ( containString(line, "GL21") )
        skip = (target != TARGET_GL21);
      continue;
    }

    if ( startsWith(line, "#endif") ) {
      skip = 0;
      continue;
    }

    if ( skip )
      continue;

    if ( startsWith(line, "#version") )
      continue;

    if ( line[0] == '\0' )
      fputs("\"\\n\"\n", out);

    else {

      if ( target == TARGET_GL21 )
        stripPrecision(line);

      fputs("\"", out);

      for ( int i = 0; line[i]; i++ ) {
        if ( line[i] == '"' )
          fputs("\"", out);
        else
          fputc(line[i], out);
      }

      fputs("\\n\"\n", out);
    }
  }
}

int main(int argc, char *argv[]) {
  if ( argc < 3 )
    return 1;

  const char *inName = NULL;
  int include        = 0;

  const char *includeName = NULL;

  for ( int i = 1; i < argc; i++ ) {
    if ( !strcmp(argv[i], "-i") && i + 1 < argc )
      inName = argv[++i];
    else if ( !strcmp(argv[i], "-gles") )
      target = TARGET_GLES2;
    else if ( !strcmp(argv[i], "-include") ) {
      include     = 1;
      includeName = argv[++i];
    }
  }

  if ( !inName )
    return 1;

  size_t len    = strlen(inName);
  char *outName = calloc(1, len + 4);
  memcpy(outName, inName, len);
  memcpy(outName + len, ".h", 3);

  FILE *in  = fopen(inName, "r");
  FILE *out = fopen(outName, "w");
  FILE *util;

  if ( include ) {
    util = fopen(includeName, "r");

    if ( !util )
      include = 0;
  }

  if ( !in ) {
    printf("FAILED to open input: %s\n", inName);
    return 1;
  }

  if ( !out ) {
    printf("FAILED to open output: %s\n", outName);
    return 1;
  }

  fputs("\"", out);
  fputs(target == TARGET_GLES2 ? GLES2_VERSION : GL21_VERSION, out);
  fputs("\\n\"\n", out);

  if ( include ) {
    emitFile(out, util);
    fclose(util);
  }

  emitFile(out, in);

  fclose(in);
  fclose(out);
  free(outName);

  return 0;
}
