#ifndef COMMAND_HANDLER_H_
#define COMMAND_HANDLER_H_

#include "VectorDatabase.h"
#include "Utility.h"

#include <memory>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <format>

constexpr size_t IMPORT_PROGRESS_FREQUENCY = 5000;

constexpr char HELP_MESSAGE[] = "The following commands are supported:";
constexpr char STARTUP_MESSAGE[] = R"(
_________                   _____     ___________      ___    _________
__  ____/_____________________  /________  /__  /_____ __ |  / /_  ___/
_  /    _  __ \_  __ \_  ___/  __/  _ \_  /__  /_  __ `/_ | / /_____ \ 
/ /___  / /_/ /  / / /(__  )/ /_ /  __/  / _  / / /_/ /__ |/ / ____/ / 
\____/  \____//_/ /_//____/ \__/ \___//_/  /_/  \__,_/ _____/  /____/  
                                                                       
------------------- Vector Similarity Search Engine -------------------
Type 'help' for a list of commands.
)";

constexpr char USAGE_MESSAGE[] = "Usage: {} {}";
constexpr char COMMAND_PROMPT[] = "> ";
constexpr char LINE_NUMBER_PREFIX[] = "Line {}: ";

constexpr char COMMAND_IMPORT_FILE[] = "import";
constexpr char COMMAND_INSERT_VECTOR[] = "insert";
constexpr char COMMAND_QUERY_VECTOR[] = "query";
constexpr char COMMAND_DELETE_VECTOR[] = "delete";
constexpr char COMMAND_HELP[] = "help";

constexpr char USAGE_ARGS_IMPORT[] = "<filename>";
constexpr char USAGE_ARGS_INSERT[] = "<vector>";
constexpr char USAGE_ARGS_QUERY[] = "<k nearest to get> <vector>";
constexpr char USAGE_ARGS_DELETE[] = "<id of vector>";

constexpr char USAGE_DESC_IMPORT[] = "Imports vectors from the provided file. Each vector must match format expected by the 'insert' command.";
constexpr char USAGE_DESC_INSERT[] = "Inserts a vector into the database.";
constexpr char USAGE_DESC_QUERY[] = "Searches for approximate k-nearest neighbors of the provided query vector and prints them to the standard output.";
constexpr char USAGE_DESC_DELETE[] = "Performs a soft delete of the vector with the associated id.";

constexpr char MISSING_FILENAME_EX_MESSAGE[] = "Missing filename";
constexpr char MISSING_K_EX_MESSAGE[] = "Missing k-nearest parameter";
constexpr char MISSING_VECTOR_QUERY_EX_MESSAGE[] = "Missing query vector";
constexpr char MISSING_VECTOR_INSERT_EX_MESSAGE[] = "Missing vector to insert";
constexpr char MISSING_ID_DELETE_EX_MESSAGE[] = "Missing id of vector to delete";

constexpr char FILE_EX_MESSAGE[] = "Could not open file <{}>";
constexpr char FAILED_PARSING_COMMAND_MESSAGE[] = "Failed parsing command <{}>: <{}>";
constexpr char UNKNOWN_COMMAND_MESSAGE[] = "Unknown command - type 'help' to list commands";

/**
*   @brief An input stream handler which continuously parses and executes commands on the underlying VectorDatabase and prints the results.
*   @tparam VecData The data type stored in vectors of the underlying VectorDatabase
*/
template<typename VecData>
class CommandHandler {
private:
	std::shared_ptr<VectorDatabase<VecData>> database_;
	std::unique_ptr<VectorParser<VecData>> vectorParser_;
	std::unique_ptr<VectorSerializer<VecData>> vectorSerializer_;

	// Dispatch 
    
    /**
     *  @brief Parses the rawVector using the injected VectorParser and inserts the vector into the database.
     *  @param rawVector The string to be parsed into a vector_t
     *  @param output The output stream where the id of the inserted vector is printed
     */
	inline void Insert(const std::string& rawVector, std::ostream& output);

    /**
     *  @brief Parses the rawVector using the injected VectorParser, queries the database for approximate k-nearest vectors and prints the result to the output.
     *  @param rawVector The string to be parsed into a vector_t and queried
     *  @param k_nearest The number of nearest neighbors of the query vector to get
     *  @param output The output stream where the serialized query results are printed
     */
	void Query(const std::string& rawVector, size_t k_nearest, std::ostream& output);

    /**
     *  @brief Sequentially parses each line of the input stream as a vector and inserts it into the database.
     *  @param stream The input stream from which vectors are parsed
     *  @param output The output stream where import progress and errors are printed
     */
	void Import(std::istream& stream, std::ostream& output); 

    /**
     *  @brief Opens the provided file, sequentially parses each line as a vector and inserts it into the database.
     *  @param filename Name of the file from which vectors are parsed
     *  @param output The output stream where import progress and errors are printed
     */
	void Import(const std::string& filename, std::ostream& output); 
    
    /**
     *  @brief Marks the vector with the corresponding id as deleted in the database. The vector will not be returned by queries but will stay in memory.
     *  @param id Id of the vector to delete
     */
	inline void Delete(const std::string& rawId); 

    // User help 
    
    /**
     * @brief Prints usage help for the user to the provided output stream.
     * @param output THe stream to print to 
     * @param command The name of the command
     * @param commandArgs Command arguments for the provided command
     */
    inline void printUsage(std::ostream& output, const char* command, const char* commandArgs) {
        output << std::format(USAGE_MESSAGE, command, commandArgs) << std::endl;
    }

    /**
     * @brief Prints usage help for the user to the provided output stream for each command the handler recognizes.
     * @param output THe stream to print to 
     */
    void printHelp(std::ostream& output);

	// Parsing
    
    /**
     * @brief Parses the arguments of the 'insert' command from the given stream and calls the Insert method.
     * @param stream The input stream containing the arguments.
     * @param output Output stream where any result or error of the command is printed.
     */
	void parseInsert(std::istream& stream, std::ostream& output);

    /**
     * @brief Parses the arguments of the 'import' command from the given stream and calls the Import method.
     * @param stream The input stream containing the arguments.
     * @param output Output stream where any result or error of the command is printed.
     */
	void parseImport(std::istream& stream, std::ostream& output);

    /**
     * @brief Parses the arguments of the 'query' command from the given stream and calls the Query method.
     * @param stream The input stream containing the arguments.
     * @param output Output stream where any result or error of the command is printed.
     */
	void parseQuery(std::istream& stream, std::ostream& output);

    /**
     * @brief Parses the arguments of the 'delete' command from the given stream and calls the Delete method.
     * @param stream The input stream containing the arguments.
     * @param output Output stream where any result or error of the command is printed.
     */
	void parseDelete(std::istream& stream);

    /**
     * @brief Parses the beginning of the provided line and dispatches parsing of the rest of the line to secondary command parser functions, or prints an error.
     * @param line The line to parse
     * @param output The output stream to which command results and errors should be printed
     */
	void parseLine(const std::string& line, std::ostream& output);

public:	

    /**
    *   @param database Shared pointer to the VectorDatabase which the parsed commands should be executed on
    *   @param parser Parser for vector_t used by the handler to parse vectors from the input before passing them to the VectorDatabase
    *   @param serializer Serializer for vector_t used by the handler to print vectors to the output
    */
	CommandHandler(
        std::shared_ptr<VectorDatabase<VecData>> database, 
        std::unique_ptr<VectorParser<VecData>> parser,
        std::unique_ptr<VectorSerializer<VecData>> serializer
    );

    /**
    *   @brief Starts an interactive loop parsing commands from the input stream and printing their results to the output stream until the input ends.
     *  @param input The input stream from which commands are parsed
     *  @param output The output stream where results of parsed commands and any errors are printed
    */
	void StartCommandLoop(std::istream& input, std::ostream& output);
};

// DEFINITIONS

template<typename VecData>
CommandHandler<VecData>::CommandHandler(
    std::shared_ptr<VectorDatabase<VecData>> database,
    std::unique_ptr<VectorParser<VecData>> parser,
    std::unique_ptr<VectorSerializer<VecData>> serializer
) : database_(database), vectorParser_(std::move(parser)), vectorSerializer_(std::move(serializer)) {};

// User help commands 
template<typename VecData>
void CommandHandler<VecData>::printHelp(std::ostream& output) {
    output << HELP_MESSAGE << std::endl;
    output << COMMAND_IMPORT_FILE << " " << USAGE_ARGS_IMPORT << " -- " << USAGE_DESC_IMPORT << std::endl;
    output << COMMAND_INSERT_VECTOR << " " << USAGE_ARGS_INSERT << " -- " << USAGE_DESC_INSERT << std::endl;
    output << COMMAND_QUERY_VECTOR << " " << USAGE_ARGS_QUERY << " -- " << USAGE_DESC_QUERY << std::endl;
    output << COMMAND_DELETE_VECTOR << " " << USAGE_ARGS_DELETE << " -- " << USAGE_DESC_DELETE << std::endl;
}

// Handling commands

template<typename VecData>
void CommandHandler<VecData>::Insert(const std::string& rawVector, std::ostream& output) {
    db::identifier_t id = database_->Insert(vectorParser_->Parse(rawVector));

    output << "Vector inserted with id: <" << id << ">" << std::endl;
}

template<typename VecData>
void CommandHandler<VecData>::Query(const std::string& rawVector, size_t k_nearest, std::ostream& output) {
    vector_t<VecData> queryVector = vectorParser_->Parse(rawVector);
    auto result = database_->Query(queryVector, k_nearest);

    output << "Query result:" << std::endl;
    for (const auto& res : result)
        output << "Id: <" << res.Id << "> Distance: <" << res.Distance << "> Vector: <" << vectorSerializer_->Serialize(*(res.Vector)) << ">" << std::endl;
}

template<typename VecData>
void CommandHandler<VecData>::Import(std::istream& stream, std::ostream& output) {
	std::string currentLine;
    size_t lineNumber = 0;

    bool firstVector = true;
    db::identifier_t firstInsertedId;
    db::identifier_t currentInsertedId;

    output << "Starting import" << std::endl;

	while (std::getline(stream, currentLine)) {
        ++lineNumber;

        if (lineNumber % IMPORT_PROGRESS_FREQUENCY == 0)
            output << "PROGRESS: Processed <" << lineNumber << "> lines" << std::endl;

        if (currentLine.empty()) 
            continue;

        try {
            currentInsertedId = database_->Insert(vectorParser_->Parse(currentLine)); 
            
            if (firstVector) {
                firstVector = false;
                firstInsertedId = currentInsertedId;
            }
        } 
        catch (const ArgumentException& e) {
            output << std::format(LINE_NUMBER_PREFIX, lineNumber) << e.Message() << std::endl;
        }
        catch (const ParsingException& e) {
            output << std::format(LINE_NUMBER_PREFIX, lineNumber) << e.Message() << std::endl;
        }
    }

    if (!firstVector)
        output << "Inserted vectors with ids <" << firstInsertedId << "> to <" << currentInsertedId << ">" << std::endl;
}

template<typename VecData>
void CommandHandler<VecData>::Import(const std::string& filename, std::ostream& output) {
	std::ifstream stream;
	stream.open(filename);

	if (!stream.good())
		throw FileException(std::format(FILE_EX_MESSAGE, filename));

	Import(stream, output);
	stream.close();
}

template<typename VecData>
void CommandHandler<VecData>::Delete(const std::string& rawId) {
	database_->Delete(db::IdentifierParser::Parse(rawId));
}

// Parsing commands

template<typename VecData>
void CommandHandler<VecData>::parseInsert(std::istream& stream, std::ostream& output) {
	std::string vectorCandidate;
	std::getline(stream, vectorCandidate);

    if (vectorCandidate.empty())
        throw ParsingException(MISSING_VECTOR_INSERT_EX_MESSAGE);

	Insert(vectorCandidate, output);
}

template<typename VecData>
void CommandHandler<VecData>::parseQuery(std::istream& stream, std::ostream& output) {
	std::string kCandidate;
	stream >> kCandidate;	

    if (kCandidate.empty())
        throw ParsingException(MISSING_K_EX_MESSAGE);

	size_t k = NumericParsingHelper::parseSize_T(kCandidate, MIN_K_SEARCH, MAX_K_SEARCH);

	stream >> std::ws;

	std::string vectorCandidate;
	std::getline(stream, vectorCandidate);

    if (vectorCandidate.empty())
        throw ParsingException(MISSING_VECTOR_QUERY_EX_MESSAGE);

	Query(vectorCandidate, k, output);
}

template<typename VecData>
void CommandHandler<VecData>::parseImport(std::istream& stream, std::ostream& output) {
	std::string filenameCandidate;
	std::getline(stream, filenameCandidate);

    if (filenameCandidate.empty())
        throw ParsingException(MISSING_FILENAME_EX_MESSAGE);

	Import(filenameCandidate, output);
}

template<typename VecData>
void CommandHandler<VecData>::parseDelete(std::istream& stream) {
	std::string idCandidate;
	std::getline(stream, idCandidate);

    if (idCandidate.empty())
        throw ParsingException(MISSING_ID_DELETE_EX_MESSAGE);

	Delete(idCandidate);
}

template<typename VecData>
void CommandHandler<VecData>::parseLine(const std::string& line, std::ostream& output) {
	std::istringstream lineStream(line);
	lineStream >> std::ws;

	std::string command;
	lineStream >> command;

	lineStream >> std::ws;

    try {
        if (command == COMMAND_QUERY_VECTOR) {
            parseQuery(lineStream, output);
        }
        else if (command == COMMAND_INSERT_VECTOR) {
            parseInsert(lineStream, output);
        }
        else if (command == COMMAND_IMPORT_FILE) {
            parseImport(lineStream, output);
        }
        else if (command == COMMAND_DELETE_VECTOR) {
            parseDelete(lineStream);
        }
        else if (command == COMMAND_HELP) {
            printHelp(output);
        } else {
            throw ParsingException(UNKNOWN_COMMAND_MESSAGE);
        }
    } catch (const ParsingException& e) {
        output << std::format(FAILED_PARSING_COMMAND_MESSAGE, command, e.Message()) << std::endl;
    
        if (command == COMMAND_INSERT_VECTOR)
            printUsage(output, COMMAND_INSERT_VECTOR, USAGE_ARGS_INSERT);
        if (command == COMMAND_QUERY_VECTOR)
            printUsage(output, COMMAND_QUERY_VECTOR, USAGE_ARGS_QUERY);
        if (command == COMMAND_IMPORT_FILE)
            printUsage(output, COMMAND_IMPORT_FILE, USAGE_ARGS_IMPORT);
        if (command == COMMAND_DELETE_VECTOR)
            printUsage(output, COMMAND_DELETE_VECTOR, USAGE_ARGS_DELETE);

    } catch (const ArgumentException& e) {
        output << e.Message() << std::endl;
    } catch (const FileException& e) {
        output << e.Message() << std::endl;
    }
}


template<typename VecData>
void CommandHandler<VecData>::StartCommandLoop(std::istream& input, std::ostream& output) {
	std::string currentLine;

    output << STARTUP_MESSAGE << std::endl;
	output << COMMAND_PROMPT << std::flush;
	while (std::getline(input, currentLine)) {
		if (currentLine.empty()) {
			output << COMMAND_PROMPT << std::flush;
			continue;
		}

		parseLine(currentLine, output);
		output << COMMAND_PROMPT << std::flush;
	}
}

#endif
