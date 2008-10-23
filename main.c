/* Written by Nick Welch in the year 2008.  Author disclaims copyright. */

#include <SDL/SDL.h>
#include <SDL/SDL_joystick.h>
#include <SDL/SDL_events.h>

#include <X11/extensions/XTest.h>
#include <X11/Xlib.h>
#include <X11/X.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>

#define THRESHOLD 2000

#define HORZ 0
#define VERT 1

SDL_Joystick ** sticks = NULL;
int nsticks = 0;

Display * dpy = NULL;

void quit(int return_code)
{
    for(int i = 0; i < nsticks; ++i)
        SDL_JoystickClose(sticks[i]);
    if(sticks)
        free(sticks);
    if(dpy)
        XCloseDisplay(dpy);
    SDL_Quit();
    exit(return_code);
}

void die(const char * message)
{
    fprintf(stderr, "fatal: %s\n", message);
    quit(1);
}

int open_joysticks(void)
{
    int nreported = SDL_NumJoysticks();
    if(!nreported)
        return 0;
    sticks = malloc(sizeof(SDL_Joystick *) * nreported);
    SDL_Joystick ** current = sticks;
    for(int i = 0; i < nreported; ++i)
    {
        SDL_Joystick * stick = SDL_JoystickOpen(i);
        if(!stick)
        {
            fprintf(stderr, "could not open joystick %d: %s\n",
                i, SDL_JoystickName(i));
            continue;
        }
        *current = stick;
        current++;
        nsticks++;
    }
    return !!sticks;
}

int main(void)
{
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
        die("could not init SDL");

    SDL_JoystickEventState(SDL_ENABLE);

    if(!open_joysticks())
        die("could not initialize any joysticks");

    Display * dpy = XOpenDisplay(NULL);
    if(!dpy)
        die("could not connect to X display");

    int _;
    if(!XQueryExtension(dpy, "XTEST", &_, &_, &_))
        die("XTEST extension is not supported by X server");

    SDL_Event ev;
    int horz = 0;
    int vert = 0;
    for(;;)
    {
        for(int i = 0; i < 10; ++i)
        {
            while(SDL_PollEvent(&ev))
            {
                if(ev.type == SDL_QUIT)
                    quit(0);
                if(ev.type != SDL_JOYAXISMOTION)
                    continue;

                int val = ev.jaxis.value;
                if(abs(val) < THRESHOLD)
                    val = 0;

                if(ev.jaxis.axis % 2 == 0)
                    horz = val;
                else
                    vert = val;
            }
            usleep(1000);
        }
        // abs() the value passed to pow() since it can't handle a negative
        // with a non-int exponent, and then turn the result back into a
        // negative if needed
        int horz_px = (int)pow(abs(horz) * 0.0001, 2.7) * (horz >= 0 ? 1 : -1);
        int vert_px = (int)pow(abs(vert) * 0.0001, 2.7) * (vert >= 0 ? 1 : -1);
        if(horz_px != 0 || vert_px != 0)
        {
            //fprintf(stderr, "ev %d %d -> %d %d\n", horz, vert, horz_px, vert_px);
            XTestFakeRelativeMotionEvent(dpy, horz_px, vert_px, CurrentTime);
            XSync(dpy, False);
        }
    }
    quit(0);
}

