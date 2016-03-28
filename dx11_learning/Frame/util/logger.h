#ifndef LOGGER_HEADER__
#define LOGGER_HEADER__

#include <string>
#include <stdarg.h>

namespace ul
{
	class Logger
	{
	public:
		static void init(const std::string& fileName);

		static void info(const char* msg, ...);

		static void warn(const char*from, int line, const char* msg, ...);

		static void error(const char*from, int line, const char* msg, ...);

		static void fatal(const char*from, int line, const char* msg, ...);

		static void destroy()
		{
			if (file_)
			{
				fclose(file_);
				file_ = nullptr;
			}
		}

	protected:
		static void print_file_line(const char* prefixLevel, const char* msg, const char*from, int line);
		static void print(const char* prefixLevel, const char* msg);

		static std::string fileName_;
		static FILE* file_;
		static char buffer_[1024];
	};

};

#endif
