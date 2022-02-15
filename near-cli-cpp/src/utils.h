#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <unistd.h>

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

inline static std::string base64_encode(std::string bin_str)
{

	// bytes len
	const size_t bin_len = bin_str.size();

	// base64_max_len
	const size_t base64_max_len = sodium_base64_encoded_len(bin_len, sodium_base64_VARIANT_ORIGINAL);

	// std::cout << base64_max_len << std::endl;

	// base64 encoded var
	std::string base64_str(base64_max_len - 1, 0);

	char *encoded_str_char = sodium_bin2base64(
		(char *)base64_str.data(),
		base64_max_len,
		(unsigned char *)bin_str.data(),
		bin_len,
		sodium_base64_VARIANT_ORIGINAL);

	if (encoded_str_char == NULL)
	{
		throw "Base64 Error: Failed to encode string";
	}

	// std::cout << sizeof(encoded_str_char) << "---" << base64_str.size() << std::endl;

	return base64_str;
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