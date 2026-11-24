#include "ShellInterfaceUnix.h"
#include <X11/Xlib.h>

namespace GameClient {

	GameClient::ShellInterfaceUnix::ShellInterfaceUnix()
		: m_windowHandle(0)
	{

	}

	GameClient::ShellInterfaceUnix::~ShellInterfaceUnix()
	{

	}

	void GameClient::ShellInterfaceUnix::gameExit(LORD::String params)
	{
		// Send WM_DELETE_WINDOW to the X11 window to request close
		Display* display = XOpenDisplay(nullptr);
		if (display) {
			Atom wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
			XEvent event;
			event.xclient.type = ClientMessage;
			event.xclient.window = static_cast<Window>(m_windowHandle);
			event.xclient.message_type = wmDeleteMessage;
			event.xclient.format = 32;
			event.xclient.data.l[0] = static_cast<long>(wmDeleteMessage);
			event.xclient.data.l[1] = CurrentTime;
			XSendEvent(display, static_cast<Window>(m_windowHandle), False, NoEventMask, &event);
			XFlush(display);
			XCloseDisplay(display);
		}
	}
};