#include "Utility.h"

#include <format>
#include <cmath>

size_t NumericParsingHelper::parseSize_T(const std::string& rawValue, size_t minValue, size_t maxValue) {
    if(!rawValue.empty() && rawValue[0] == '-')
		throw ParsingException{ std::format(INVALID_NUMBER_EX_MESSAGE, "size_t", rawValue)};
	
    size_t pos;
	try {
		size_t value = std::stoull(rawValue, &pos);

		if (pos != rawValue.size())
			throw ParsingException{ std::format(MALFORMED_NUMBER_EX_MESSAGE, "size_t", rawValue)};

		if (value < minValue || value > maxValue) {
			throw ParsingException{ std::format(NUMBER_OUT_OF_RANGE_EX_MESSAGE, "size_t", rawValue, minValue, maxValue)};
		}

		return value;
	}
	catch (std::out_of_range&) {
		throw ParsingException{ std::format(OVERFLOW_NUMBER_EX_MESSAGE, "size_t", rawValue)};
	}
	catch (std::invalid_argument&) {
		throw ParsingException{ std::format(INVALID_NUMBER_EX_MESSAGE, "size_t", rawValue)};
	}
}

double NumericParsingHelper::parseDouble(const std::string& rawValue, double minValue, double maxValue) {
	size_t pos;
	try {
		double value = std::stod(rawValue, &pos);

		if (pos != rawValue.size())
			throw ParsingException{ std::format(MALFORMED_NUMBER_EX_MESSAGE, "double", rawValue)};

		if (value < minValue || value > maxValue) {
			throw ParsingException{ std::format(NUMBER_OUT_OF_RANGE_EX_MESSAGE, "double", rawValue, minValue, maxValue)};
		}

		if (std::isnan(value))
			throw ParsingException{ std::format(INVALID_NUMBER_EX_MESSAGE, "double", rawValue)};

		return value;
	}
	catch (std::out_of_range&) {
		throw ParsingException{ std::format(OVERFLOW_NUMBER_EX_MESSAGE, "double", rawValue)};
	}
	catch (std::invalid_argument&) {
		throw ParsingException{ std::format(INVALID_NUMBER_EX_MESSAGE, "double", rawValue)};
	}
}
