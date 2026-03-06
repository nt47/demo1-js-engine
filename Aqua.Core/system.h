#pragma once
#include <fmt/color.h>

enum class LEVEL//class限域
{
	LOG,
	INFO,
	WARNING,
	ERR,
	SUCCESS,
	DEBUG
};

class System
{
public:
	template<typename... Args>
	static void PrintLn(LEVEL level, fmt::format_string<Args...> fmt_str, Args&&... args)
	{
		fmt::color color = fmt::color::white;
		switch (level) {
		case LEVEL::LOG:     color = fmt::color::wheat; break;
		case LEVEL::INFO:    color = fmt::color::aqua; break;
		case LEVEL::WARNING: color = fmt::color::yellow; break;
		case LEVEL::ERR:     color = fmt::color::red; break;
		case LEVEL::SUCCESS:     color = fmt::color::green; break;
		case LEVEL::DEBUG:     color = fmt::color::pink; break;
		default:     color = fmt::color::white; break;
		}

		fmt::print(fg(color), "{}\n", fmt::format(fmt_str, std::forward<Args>(args)...));
	}
};
