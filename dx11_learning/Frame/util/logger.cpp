
#include <time.h>
#include <assert.h>
#include "Logger.h"

#pragma warning(disable:4996)

using namespace ul;

std::string Logger::fileName_ = "";
FILE* Logger::file_ = nullptr;
char Logger::buffer_[1024];

#define get_parameter(firstParam) \
	memset(buffer_, 0, 1024); \
	va_list ap; \
	va_start(ap, firstParam); \
	vsprintf(buffer_, firstParam, ap); \
	va_end(ap); \


void Logger::init(const std::string& fileName)
{
	Logger::fileName_ = fileName;

	#if defined(DEBUG) | defined(_DEBUG)
		file_ = stdout;
	#else
		file_ = fopen(fileName.data(), "a+");
	#endif

	if (file_ == nullptr)
	{
		assert(0);
		exit(0);
	}
}

void Logger::info(const char* msg, ...)
{
	get_parameter(msg);
	print("INFO", buffer_);
}

void Logger::warn(const char* from, int line, const char* msg, ...)
{
	get_parameter(msg);
	print_file_line("WARNING", buffer_, from, 0);
}

void Logger::error(const char* from, int line, const char* msg, ...)
{
	get_parameter(msg);
	print_file_line("ERROR", buffer_, from, line);
}

void Logger::fatal(const char* from, int line, const char* msg, ...)
{
	get_parameter(msg);
	print_file_line("FATAL", buffer_, from, line);
	exit(0);
}



void Logger::print_file_line(const char* prefixLevel, const char* msg, const char*from, int line)
{
	tm* newTime;
	__time64_t longTime;
	_time64(&longTime);
	newTime = _localtime64(&longTime);

	fprintf(file_, "[ %s %02d:%02d:%02d ]: %s (from file %s: line %d)\n",
		prefixLevel,
		newTime->tm_hour, newTime->tm_min, newTime->tm_sec,
		buffer_ ,
		from,
		line);
	fflush(file_);
}


void Logger::print(const char* prefixLevel, const char* msg)
{
	tm* newTime;
	__time64_t longTime;
	_time64(&longTime);
	newTime = _localtime64(&longTime);

	fprintf(file_, "[ %s %02d:%02d:%02d ]: %s \n",
		prefixLevel,
		newTime->tm_hour, newTime->tm_min, newTime->tm_sec,
		buffer_);
	fflush(file_);
}


