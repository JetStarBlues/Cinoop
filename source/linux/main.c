#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "rom.h"
#include "cpu.h"
#include "gpu.h"
#include "interrupts.h"
#include "debug.h"
#include "keys.h"

#include "main.h"

Display *dpy;
Window root;
GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
XVisualInfo *vi;
Colormap cmap;
XSetWindowAttributes swa;
Window win;
GLXContext glc;
XWindowAttributes gwa;
XEvent xev;
KeySym sym;

void quit(void) {
	glXMakeCurrent(dpy, None, NULL);
	glXDestroyContext(dpy, glc);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
	exit(0);
}

int main(int argc, char **argv) {
	char *filename = NULL;
	
	printf("Starting...\n");
	
	dpy = XOpenDisplay(NULL);
	
	if(dpy == NULL) {
		printf("Cannot connect to X server!\n");
		exit(0);
	}
	
	root = DefaultRootWindow(dpy);
	
	vi = glXChooseVisual(dpy, 0, att);
	
	if(vi == NULL) {
		printf("No appropriate visual found!\n");
		exit(0);
	}
	
	cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask;
	
	win = XCreateWindow(dpy, root, 0, 0, 160, 144, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "Cinoop");
	
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
	
	printf("argc = %d\n", argc);
	int i;
	for(i = 1; i < argc; i++) {
		filename = argv[i];
	}
	
	if(filename == NULL) {
		printf("No ROM input\n");
		
		quit();
		return 1;
	}
	
	printf("Loading file \"%s\"...\n", filename);
	
	if(!loadROM(filename)) {
		printf("Failed!\n");
		
		quit();
		return 1;
	}
	
	printf("Passed!\n");
	
	srand(time(NULL));
	reset();
	
	while(1) {
		if(XPending(dpy)) {
			XNextEvent(dpy, &xev);
			
			if(xev.type == KeyPress || xev.type == KeyRelease) {
				//printf("%d\n", xev.xkey.keycode);
				sym = XLookupKeysym( &xev.xkey, 0 );
				switch(sym) {
					case XK_BackSpace:
						keys.select = (xev.type == KeyPress) ? 0 : 1;
						break;
					
					case XK_Return:
						keys.start = (xev.type == KeyPress) ? 0 : 1;
						break;
					
					case XK_z:
						keys.b = (xev.type == KeyPress) ? 0 : 1;
						break;
					
					case XK_x:
						keys.a = (xev.type == KeyPress) ? 0 : 1;
						break;
					
					case XK_Left:
						keys.left = (xev.type == KeyPress) ? 0 : 1;
						break;
					
					case XK_Right:
						keys.right = (xev.type == KeyPress) ? 0 : 1;
						break;
					
					case XK_Up:
						keys.up = (xev.type == KeyPress) ? 0 : 1;
						break;
					
					case XK_Down:
						keys.down = (xev.type == KeyPress) ? 0 : 1;
						break;
				}
			}
		}
		
		cpuStep();
		gpuStep();
		interruptStep();
	}
	
	quit();
	
	return 0;
}
