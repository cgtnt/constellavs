#ifndef UTILITY_H_
#define UTILITY_H_

#include <string>
#include <utility>

// Exceptions

/// @brief Base exception class of the application.
class Exception {
public:
	virtual const char* Message() const =0;

	virtual ~Exception() = default;
};

// Technical separation 

/// @brief Exception for which the message is known at compile time.
class CompileTimeTextException : public Exception {
private:
	const char* message_;

public:
	inline CompileTimeTextException(const char* message) : message_(message) {};

	inline const char* Message() const override {
		return message_;
	}
};

/// @brief Exception for which the message is not known at compile time, and depends on runtime arguments.
class VariableTextException : public Exception {
private:
	std::string message_;

public:
	inline VariableTextException(const std::string& messageText) : message_(messageText) {};
	inline VariableTextException(std::string&& messageText) : message_(std::move(messageText)) {};

	inline const char* Message() const override {
		return message_.c_str();
	}
};

// Semantic separation

class ParsingException : public VariableTextException {
	using VariableTextException::VariableTextException;
};

class NotImplementedException : public CompileTimeTextException {
	using CompileTimeTextException::CompileTimeTextException;
};

class FileException : public VariableTextException {
	using VariableTextException::VariableTextException;
};

class ArgumentException : public VariableTextException {
	using VariableTextException::VariableTextException;
};

// Numeric Parsing

constexpr char INVALID_NUMBER_EX_MESSAGE[] = "Invalid <{}>: <{}>";
constexpr char OVERFLOW_NUMBER_EX_MESSAGE[] = "Overflow <{}>: <{}>";
constexpr char MALFORMED_NUMBER_EX_MESSAGE[] = "Malformed <{}>: <{}>";
constexpr char NUMBER_OUT_OF_RANGE_EX_MESSAGE[] = "<{}> <{}> out of range <{}, {}>";

/// @brief Utility function containing static parsing functions for needed numeric types.
class NumericParsingHelper {
public:

    /** 
     * @brief Parses a string into size_t and checks if the resulting value is within provided range.
     * @param rawValue The raw value to parse.
     * @param minValue Lower bound of the acceptable valeu range (inclusive).
     * @param maxValue Upper bound of the acceptable valeu range (inclusive).
    */ 
	static size_t parseSize_T(const std::string& rawValue, size_t minValue, size_t maxValue); 

    /** 
     * @brief Parses a string into double and checks if the resulting value is within provided range.
     * @param rawValue The raw value to parse.
     * @param minValue Lower bound of the acceptable valeu range (inclusive).
     * @param maxValue Upper bound of the acceptable valeu range (inclusive).
    */ 
	static double parseDouble(const std::string& rawValue, double minValue, double maxValue); 
};

#endif
