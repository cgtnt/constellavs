#include "ConfigurationParser.h"
#include "Utility.h"

#include <string>
#include <string_view>
#include <format>

ConfigurationParser::ConfigurationParser(const args_t& arguments, NSWVectorDatabaseConfiguration& configuration) 
: arguments_(arguments), configuration_(configuration) {};

const std::string& ConfigurationParser::retrieveOptionValue(args_t::const_iterator& it) {
	const std::string& currentOption = *it;
	++it;

	if (it == arguments_.end())
		throw ParsingException(std::format(MISSING_OPTION_VALUE_EX_MESSAGE, currentOption));

	return *it;
}

void ConfigurationParser::processLongOption(args_t::const_iterator& it) {
	std::string_view currentOption = std::string_view(*it).substr(2);

	if (currentOption == OPTION_DISTANCE_METRIC_LONG) {
		parseDistanceMetric(retrieveOptionValue(it));
	}
	else if (currentOption == OPTION_CONNECTIVITY_LONG) {
		parseConnectivity(retrieveOptionValue(it));
	}
	else if (currentOption == OPTION_VECTOR_DIMENSION_LONG) {
		parseVectorDimension(retrieveOptionValue(it));
	}
    else if (currentOption == OPTION_INDEX_Q_BUFFER_MUL_LONG) {
        parseIndexQBufferMul(retrieveOptionValue(it));
    }
    else if (currentOption == OPTION_INDEX_I_BUFFER_MUL_LONG) {
        parseIndexIBufferMul(retrieveOptionValue(it));
    }
    else if (currentOption == OPTION_INDEX_MAX_ENTRYPOINTS_LONG) {
        parseIndexMaxEntrypoints(retrieveOptionValue(it));
    }
	else {
		throw ParsingException(std::format(INVALID_OPTION_EX_MESSAGE, *it));
	}
}

void ConfigurationParser::processShortOption(args_t::const_iterator& it) {
    const std::string& currentRaw = *it;

    if (currentRaw.length() != 2)
		throw ParsingException(std::format(INVALID_OPTION_EX_MESSAGE, currentRaw));

	char currentOption = currentRaw[1];

	switch (currentOption) {
	case OPTION_DISTANCE_METRIC_SHORT:
		parseDistanceMetric(retrieveOptionValue(it));
		break;
	case OPTION_CONNECTIVITY_SHORT:
		parseConnectivity(retrieveOptionValue(it));
		break;
	case OPTION_VECTOR_DIMENSION_SHORT:
		parseVectorDimension(retrieveOptionValue(it));
		break;
	default:
		throw ParsingException(std::format(INVALID_OPTION_EX_MESSAGE, currentRaw));
	}
}

size_t ConfigurationParser::parseSizeTOption(const std::string& rawValue, const char* option, size_t minValue, size_t maxValue) {
	try {
		return NumericParsingHelper::parseSize_T(rawValue, minValue, maxValue);
	}
	catch (const Exception& e) {
		throw ParsingException(std::format(INVALID_OPTION_VALUE_EX_MESSAGE, option, rawValue, e.Message()));
	}
}

void ConfigurationParser::parseDistanceMetric(const std::string& value) {
	if (value == SETTING_DISTANCE_EUCLIDEAN) {
		configuration_.DistanceMetric = DistanceMetricSelection::SQUARED_EUCLIDEAN;
	}
	else if (value == SETTING_DISTANCE_COSINE) {
		configuration_.DistanceMetric = DistanceMetricSelection::COSINE;
	}
	else {
        std::string errorMessage = std::format("Must be '{}' or '{}'", SETTING_DISTANCE_EUCLIDEAN, SETTING_DISTANCE_COSINE);
		throw ParsingException(std::format(INVALID_OPTION_VALUE_EX_MESSAGE, OPTION_DISTANCE_METRIC_LONG, value, errorMessage));
	}
}

void ConfigurationParser::parseConnectivity(const std::string& value) {
    configuration_.TargetConnectivity = parseSizeTOption(value, OPTION_CONNECTIVITY_LONG, MIN_CONNECTIVITY, MAX_CONNECTIVITY);
}

void ConfigurationParser::parseVectorDimension(const std::string& value) {
	configuration_.VectorDimension = parseSizeTOption(value, OPTION_VECTOR_DIMENSION_LONG, MIN_VECTOR_DIMENSION, MAX_VECTOR_DIMENSION);
}

void ConfigurationParser::parseIndexQBufferMul(const std::string& value) {
    configuration_.IndexKQueryBufferSizeMultiplier = parseSizeTOption(value, OPTION_INDEX_Q_BUFFER_MUL_LONG, MIN_K_BUFFERS_SIZE_MUL, MAX_K_BUFFERS_SIZE_MUL);
}

void ConfigurationParser::parseIndexIBufferMul(const std::string& value) {
    configuration_.IndexKInsertBufferSizeMultipler = parseSizeTOption(value, OPTION_INDEX_I_BUFFER_MUL_LONG, MIN_K_BUFFERS_SIZE_MUL, MAX_K_BUFFERS_SIZE_MUL);
}

void ConfigurationParser::parseIndexMaxEntrypoints(const std::string& value) {
	configuration_.IndexKMaxEntrypoints = parseSizeTOption(value, OPTION_INDEX_MAX_ENTRYPOINTS_LONG, MIN_K_ENTRYPOINTS, MAX_K_ENTRYPOINTS);
}

void ConfigurationParser::Parse(const args_t& arguments, NSWVectorDatabaseConfiguration& configuration) {
	ConfigurationParser parser(arguments, configuration);

	for (auto it = arguments.begin(); it != arguments.end(); ++it) {
		std::string_view currentItem = *it;
        if (currentItem.empty())
            continue;

		if (currentItem.substr(0, 2) == "--") {
			parser.processLongOption(it);
		}
		else if (currentItem[0] == '-') {
			parser.processShortOption(it);
		}
		else {
			throw ParsingException(std::format(INVALID_OPTION_EX_MESSAGE, currentItem));
		}
	}
}
