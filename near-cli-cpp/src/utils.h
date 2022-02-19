#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <unistd.h>
#ifdef __linux__
    #include <unistd.h>
    #include <sys/types.h>
    #include <pwd.h>
#endif

#include <sodium/utils.h>

inline static constexpr const uint8_t base58map[] = {
	'1', '2', '3', '4', '5', '6', '7', '8',
	'9', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
	'H', 'J', 'K', 'L', 'M', 'N', 'P', 'Q',
	'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
	'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
	'h', 'i', 'j', 'k', 'm', 'n', 'o', 'p',
	'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
	'y', 'z'};

inline static std::string EncodeBase58(const std::vector<uint8_t> &data)
{
	std::vector<uint8_t> digits((data.size() * 138 / 100) + 1);
	size_t digitslen = 1;
	for (size_t i = 0; i < data.size(); i++)
	{
		uint32_t carry = static_cast<uint32_t>(data[i]);
		for (size_t j = 0; j < digitslen; j++)
		{
			carry = carry + static_cast<uint32_t>(digits[j] << 8);
			digits[j] = static_cast<uint8_t>(carry % 58);
			carry /= 58;
		}
		for (; carry; carry /= 58)
			digits[digitslen++] = static_cast<uint8_t>(carry % 58);
	}
	std::string result;
	for (size_t i = 0; i < (data.size() - 1) && !data[i]; i++)
		result.push_back(base58map[0]);
	for (size_t i = 0; i < digitslen; i++)
		result.push_back(base58map[digits[digitslen - 1 - i]]);
	return result;
}

static std::string GetHomeDirectory()
{
	const char* homeDir;
	#ifdef _WIN32
		homeDir = getenv("USERPROFILE");
	#endif

	#ifdef __linux__
		struct passwd *pw = getpwuid(getuid());
		homeDir = pw->pw_dir;
	#endif

	#ifdef __APPLE__
		struct passwd *pw = getpwuid(getuid());
		homeDir = pw->pw_dir;
	#endif

	return homeDir;
}

// inline static void WaitingAnimation(bool& exp, const char* msg)
// {
// 	std::cout << msg; //<< "Waiting wallet response "
// 	while (!exp) //isServerClose
// 	{
// 		// Es la peor solucion que para un loading
// 		sleep(1);
// 		std::cout << "." << std::flush;
// 		sleep(1);
// 		std::cout << "." << std::flush;
// 		sleep(1);
// 		std::cout << "." << std::flush;
// 		sleep(1);
// 		std::cout << "\b\b\b   \b\b\b" << std::flush;
// 	}
// }