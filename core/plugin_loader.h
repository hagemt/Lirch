#ifndef __LIRCH_CORE_PLUGIN_LOADER_H__
#define __LIRCH_CORE_PLUGIN_LOADER_H__

#include <string>

#include "message_view.h"
//#include "lirch/core/message_view.h"

//namespace lirch {

extern bool
load_plugin(const std::string &, const plugin_pipe &);

//} // namespace lirch

#endif // __LIRCH_CORE_PLUGIN_LOADER_H__
