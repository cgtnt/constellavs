#ifndef CONFIGURATION_PARSER_H_ 
#define CONFIGURATION_PARSER_H_

#include "VectorDatabase.h"

#include <vector>
#include <string>

using args_t = std::vector<std::string>;

// Configuration options 
constexpr char OPTION_DISTANCE_METRIC_LONG[] = "distance";
constexpr char OPTION_DISTANCE_METRIC_SHORT = 'd';

constexpr char OPTION_CONNECTIVITY_LONG[] = "connectivity";
constexpr char OPTION_CONNECTIVITY_SHORT = 'c';

constexpr char OPTION_VECTOR_DIMENSION_LONG[] = "dimension";
constexpr char OPTION_VECTOR_DIMENSION_SHORT = 'n';

constexpr char OPTION_INDEX_Q_BUFFER_MUL_LONG[] = "qbuffer-mul";
constexpr char OPTION_INDEX_I_BUFFER_MUL_LONG[] = "ibuffer-mul";
constexpr char OPTION_INDEX_MAX_ENTRYPOINTS_LONG[] = "max-entry-pt";

// Valid distance settings
constexpr char SETTING_DISTANCE_EUCLIDEAN[] = "squared_euclidean";
constexpr char SETTING_DISTANCE_COSINE[] = "cosine";

// Configuration exception messages
constexpr char INVALID_OPTION_EX_MESSAGE[] = "Invalid option <{}>";

constexpr char MISSING_OPTION_VALUE_EX_MESSAGE[] = "Value for option <{}> is missing!";
constexpr char INVALID_OPTION_VALUE_EX_MESSAGE[] = "Invalid <{}> setting <{}>: <{}>";

/// @brief Parses CLI arguments into a NSWVectorDatabaseConfiguration.
class ConfigurationParser {
private:
	const args_t& arguments_;
	NSWVectorDatabaseConfiguration& configuration_;

    /**
     * @brief Retrieves the next argument by incrementing the provided iterator and reading the result or throws an exception if there is no such argument.
     * @param it Iterator pointing to the current argument i.e. argument before the value to retrieve
     */
	inline const std::string& retrieveOptionValue(args_t::const_iterator& it);
    
    /**
     * @brief Processes the given argument as a short option i.e. option prefixed by '-'.
     * @param it Iterator pointing to the argument to process
     */
	void processShortOption(args_t::const_iterator& it);

    /**
     * @brief Processes the given argument as a long option i.e. option prefixed by "--".
     * @param it Iterator pointing to the argument to process
     */
	void processLongOption(args_t::const_iterator& it);

    /**
     * @brief Parses the given rawValue into a size_t and checks the result is within given range.
     * @param rawValue Raw value to parse.
     * @param option Name of option which the value is being parsed for. Used to construct exception messages if any happen.
     * @param minValue Lower bound of acceptable value range (inclusive).
     * @param minValue Upper bound of acceptable value range (inclusive).
     */
    size_t parseSizeTOption(const std::string& rawValue, const char* option, size_t minValue, size_t maxValue);

    /// @brief Parses NSWVectorDatabaseConfiguration::DistanceMetric 
	void parseDistanceMetric(const std::string& value);

    /// @brief Parses NSWVectorDatabaseConfiguration::TargetConnectivity and checks it is within range MIN_CONNECTIVITY to MAX_CONNECTIVITY
	inline void parseConnectivity(const std::string& value);

    /// @brief Parses NSWVectorDatabaseConfiguration::VectorDImension and checks it is within range MIN_VECTOR_DIMENSION to MAX_VECTOR_DIMENSION 
	inline void parseVectorDimension(const std::string& value);

    /// @brief Parses NSWVectorDatabaseConfiguration::IndexKQueryBufferSizeMultiplier and checks it is within range MIN_K_BUFFERS_SIZE_MUL to MAX_K_BUFFERS_SIZE_MUL
	inline void parseIndexQBufferMul(const std::string& value);

    /// @brief Parses NSWVectorDatabaseConfiguration::IndexKInsertBufferSizeMultipler and checks it is within range MIN_K_BUFFERS_SIZE_MUL to MAX_K_BUFFERS_SIZE_MUL
	inline void parseIndexIBufferMul(const std::string& value);

    /// @brief Parses NSWVectorDatabaseConfiguration::IndexKMaxEntrypoints and checks it is within range MIN_K_ENTRYPOINTS to MAX_K_ENTRYPOINTS 
	inline void parseIndexMaxEntrypoints(const std::string& value);

	ConfigurationParser(const args_t& arguments_, NSWVectorDatabaseConfiguration& configuration);

public:

    /**
     * @brief Parses the provided arguments into the provided NSWVectorDatabaseConfiguration.
     * @param arguments The arguments to parse
     * @param configuration The configuration to update based on parsed arguments
     */
	static void Parse(const args_t& arguments, NSWVectorDatabaseConfiguration& configuration);
};

#endif
