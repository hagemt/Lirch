#ifndef __LIRCH_PLUGIN_H__
#define __LIRCH_PLUGIN_H__

/*
 * Note: this header is included by plugins, not by code that wants
 * to interact with plugins.
 *
 * Code that wants to interact with plugins should call load_plugin.
 *
 */

#include <string>

#include "lirch/core/message_view.h"
#include "lirch/core/required_messages.h"

extern void
run(plugin_pipe, std::string);

#define LIRCH_CDECL
//Why windows, why?
#if defined(_WIN32)
//We need this to ensure semi-consistent name mangling and APIs
#define LIRCH_CDECL __cdecl
#	if defined(_WIN64)
#		pragma comment(linker, "/EXPORT:plugin_version")
#		pragma comment(linker, "/EXPORT:plugin_init")
#	else
#		pragma comment(linker, "/EXPORT:plugin_version=_plugin_version")
#		pragma comment(linker, "/EXPORT:plugin_init=_plugin_init")
#	endif
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int LIRCH_CDECL
plugin_version(void)
{
	return 0;
}

void LIRCH_CDECL
plugin_init(plugin_pipe p)
{
	message m = p.blocking_read();
	if (m.type != "hello") return;
	auto d = dynamic_cast<hello_message *>(m.getdata());
	if (!d) return;

	// Call the plugin-defined code:
	try { run(p, d->name); }
	catch (...) {
		/*
		 * Catch every exception!
		 *
		 * Uncaught exceptions will terminate the program.
		 * We might want to put some logging here.
		 *
		 */
	}

	// Let the core know we're done:
	p.write(done_message::create(d->name));
}

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // __LIRCH_PLUGIN_H__
