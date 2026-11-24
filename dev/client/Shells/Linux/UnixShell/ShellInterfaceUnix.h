#pragma once

#ifndef _SHELL_INTERFACE_UNIX_H_
#define _SHELL_INTERFACE_UNIX_H_

#include "ShellInterface.h"
#include "game.h"

namespace GameClient {

	class ShellInterfaceUnix : public ShellInterface
	{
	public:
		ShellInterfaceUnix();
		virtual ~ShellInterfaceUnix();
		virtual void   gameExit(LORD::String params);

		void setWindowHandle(unsigned long handle) { m_windowHandle = handle; }
	private:

		unsigned long m_windowHandle;
	};
}

#endif