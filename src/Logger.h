#ifndef LOGGER_H
#define LOGGER_H

#include <log4cxx/logger.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/simplelayout.h>
#include "log4cxx/consoleappender.h"

using namespace log4cxx;
using namespace log4cxx::helpers;

extern LoggerPtr GlobalLogger;

#endif
