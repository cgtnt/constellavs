#include "Vector.h"
#include "Utility.h"

#include <sstream>
#include <format>

// Vector parsing 

CSVDoubleVectorParser::CSVDoubleVectorParser(char fieldDelimiter, size_t vectorDimension) : fieldDelimiter_(fieldDelimiter), vectorDimension_(vectorDimension) {};

vector_t<double> CSVDoubleVectorParser::Parse(const std::string& rawVector) {
	vector_t<double> result;
    result.reserve(vectorDimension_);

	std::string currentField;
	std::istringstream stream(rawVector);

    try {
        while (std::getline(stream, currentField, fieldDelimiter_))
                result.emplace_back(NumericParsingHelper::parseDouble(currentField, std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max()));
    }
    catch (const Exception& e) {
        throw ParsingException(std::format(FAILED_PARSING_VECTOR_EX_MESSAGE, rawVector, e.Message()));
    }

	return result;
}

// Vector serialization

CSVDoubleVectorSerializer::CSVDoubleVectorSerializer(char fieldDelimiter) : fieldDelimiter_(fieldDelimiter) {};

std::string CSVDoubleVectorSerializer::Serialize(const vector_t<double>& vector) {
	std::ostringstream stringStream;

	bool first = true;
	for (const auto& element : vector) {
		if (!first)
			stringStream << fieldDelimiter_;

		stringStream << element;
		first = false;
	}

	return stringStream.str();
}
