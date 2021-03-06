#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

typedef struct Client {
	struct Client *next, *previous;
	Window window;
} Client;

int main(int argc, char *argv[]) {
	XEvent event;
	Display *display = XOpenDisplay(0);
	int screen = DefaultScreen(display);
	Client *client_current = (Client *) calloc(1, sizeof(Client));
	client_current->window = DefaultRootWindow(display);
	client_current->next = client_current;
	client_current->previous = client_current;
	XSelectInput(display, client_current->window, SubstructureRedirectMask);
	signal(SIGCHLD, SIG_IGN);
	if (argc < 2) {;}
	XGrabKey(display, XKeysymToKeycode(display, XK_m), Mod4Mask, client_current->window, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_j), Mod4Mask, client_current->window, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_k), Mod4Mask, client_current->window, True, GrabModeAsync, GrabModeAsync);
	while (!XNextEvent(display, &event)) {
		if (event.type == KeyPress) {
			KeySym keysym = XkbKeycodeToKeysym(display, event.xkey.keycode, 0, 0);
			if (keysym == XK_m) {
				if (!fork())
					execvp(argv[1], &argv[1]);
			} else if (keysym == XK_j)
				XRaiseWindow(display, client_current->next->window);
			else if (keysym == XK_k)
				XRaiseWindow(display, client_current->previous->window);
		} else if (event.type == MapRequest) {
			Client *client_new = (Client *) calloc(1, sizeof(Client));
			client_new->window = event.xmaprequest.window;
			client_current->next->previous = client_new;
			client_new->previous = client_current;
			client_new->next = client_current->next;
			client_current->next = client_new;
			XSelectInput(display, client_new->window, StructureNotifyMask|EnterWindowMask);
			XMoveResizeWindow(display, client_new->window, 0, 0, XDisplayWidth(display, screen), XDisplayHeight(display, screen));
			XMapWindow(display, client_new->window);
		} else if (event.type == EnterNotify) {
			while (client_current->window != event.xcrossing.window)
				client_current = client_current->next;
			XSetInputFocus(display, client_current->window, RevertToParent, CurrentTime);
		} else if (event.type == DestroyNotify) {
			Client *client_destroyed = client_current;
			while (client_destroyed->window != event.xdestroywindow.window)
				client_destroyed = client_destroyed->next;
			if (client_destroyed == client_destroyed->previous)
				client_current = 0;
			if (client_destroyed == client_current)
				client_current = client_destroyed->next;
			if (client_destroyed->next)
				client_destroyed->next->previous = client_destroyed->previous;
			if (client_destroyed->previous)
				client_destroyed->previous->next = client_destroyed->next;
			free(client_destroyed);
		}
	}
	return 1;
}
