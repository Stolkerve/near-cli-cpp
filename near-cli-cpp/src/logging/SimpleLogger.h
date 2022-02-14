/*
Name            FG  BG
Black           30  40
Red             31  41
Green           32  42
Yellow          33  43
Blue            34  44
Magenta         35  45
Cyan            36  46
White           37  47
Bright Black    90  100
Bright Red      91  101
Bright Green    92  102
Bright Yellow   93  103
Bright Blue     94  104
Bright Magenta  95  105
Bright Cyan     96  106
Bright White    97  107
*/

constexpr auto BRIGHT_GREEN = "\x1B[92m";
constexpr auto BRIGHT_YELLOW = "\x1B[93m";
constexpr auto YELLOW = "\x1B[33m";
constexpr auto RED_BACKGROUND = "\033[41;1m";
constexpr auto BRIGHT_RED_BACKGROUND = "\033[101;1m";
constexpr auto NORMAL_BACKGROUND = "\033[0m\n";
constexpr auto NORMAL_COLOR = "\x1B[0m";

#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>

static auto getDate() {
	auto in_time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::stringstream buffer;
	buffer << "[" << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S") << "] ";
	return buffer.str();
}

#ifdef ENABLE_SIMPLE_NAMESPACE
namespace simple {
#endif
    class Logger
    {
    public:
        template<typename ... Ts>
        static void Info(Ts&& ... Args) {
            std::ostringstream stream;
            ((stream << Args), ...);

            if (stream.str().empty()) {
                std::cout << '\n';
                return;
            }

            std::cout << BRIGHT_GREEN << getDate() << stream.str() << NORMAL_COLOR << '\n';
        }

        template<typename ... Ts>
        static void Warn(Ts&& ... Args) {
            std::ostringstream stream;
            ((stream << Args << ' '), ...);

            if (stream.str().empty()) {
                std::cout << '\n';
                return;
            }

            std::cout << BRIGHT_YELLOW << getDate() << stream.str() << NORMAL_COLOR << '\n';
        }

        template<typename ... Ts>
        static void Error(Ts&& ... Args) {
            std::ostringstream stream;
            ((stream << Args << ' '), ...);
            if (stream.str().empty()) {
                std::cout << '\n';
                return;
            }
            std::cout << RED_BACKGROUND << YELLOW << getDate() << stream.str() << NORMAL_COLOR << NORMAL_BACKGROUND;

        }
    };
    
#ifdef ENABLE_SIMPLE_NAMESPACE
}
#endif