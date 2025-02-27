#pragma once

// TODO: remove after boost fixes issue in boost 1.78
#define BOOST_USE_WINAPI_VERSION BOOST_WINAPI_VERSION_WIN7

//#define BOOST_LOG_DYN_LINK // necessary when linking the boost_log library dynamically

#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/trivial.hpp>

// the logs are also written to LOGFILE
#define LOGFILE "logfile.log"

// just log messages with severity >= SEVERITY_THRESHOLD are written
#define SEVERITY_THRESHOLD boost::log::trivial::trace

// register a global logger
BOOST_LOG_GLOBAL_LOGGER(logger, boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level>)

// just a helper macro used by the macros below - don't use it in your code
#define LOG(severity) BOOST_LOG_SEV(logger::get(), boost::log::trivial::severity)

// ===== log macros =====
#define LOG_TRACE LOG(trace)
#define LOG_DEBUG LOG(debug)
#define LOG_INFO LOG(info)
#define LOG_WARNING LOG(warning)
#define LOG_ERROR LOG(error)
#define LOG_FATAL LOG(fatal)

namespace Logging {

// must be called early at app startup to ensure safety.
void
Init();

void
Enable(bool enabled);

};
