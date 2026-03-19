#ifndef VECTOR_H_ 
#define VECTOR_H_ 

#include <cmath>
#include <vector>
#include <concepts>
#include <string>


/**
* @brief Data type used to represent vectors in VectorDatabase
*/
template<typename VecData>
using vector_t = std::vector<VecData>;

// Distance
template<typename Metric, typename Object>
concept DistanceMetric = requires(Metric m, const Object& object1, const Object& object2) {
	{ m(object1, object2) } -> std::same_as<double>;
};

/**
* @brief Calculates squared euclidean distance between two vector_t.
*/
template<typename VecData>
class SquaredEuclideanDistanceMetric {
public:
	double operator()(const vector_t<VecData>& vector1, const vector_t<VecData>& vector2) const;
};

/**
* @brief Calculates cosine distance between two vector_t.
*/
template<typename VecData>
class CosineDistanceMetric {
public:
	double operator()(const vector_t<VecData>& vector1, const vector_t<VecData>& vector2) const;
};

// Parsing
constexpr char FAILED_PARSING_VECTOR_EX_MESSAGE[] = "Failed parsing vector <{}>: <{}>";

/**
 * @brief Parser used to parse a string into a vector_t<VecData>.
 * @tparam VecData The data type stored in the resulting vector
 */
template<typename VecData>
class VectorParser {
public:

    /// @brief Parses the given string into a vector_t<VecData>
	virtual vector_t<VecData> Parse(const std::string& rawVector) =0;

	virtual ~VectorParser() = default;
};


/**
* @brief Parses a string of form x1, x2, x3, ... to vector_t.
*/
class CSVDoubleVectorParser : public VectorParser<double> {
private:
	char fieldDelimiter_;
    size_t vectorDimension_;

public:
    /**
    * @brief Constructs the parser.
    * @param fieldDelimiter The delimiter used to separate individual values of the resulting vetor_t in the input string
    * @param vectorDimension The dimension of the vector_t
    */
	CSVDoubleVectorParser(char fieldDelimiter, size_t vectorDimension);

    /// @brief Parses the given string into a vector_t<double>
	vector_t<double> Parse(const std::string& rawVector) override;
};

// Serialization 

/**
 * @brief Serializer used to serialize a vector_t<VecData> into a string.
 * @tparam VecData The data type stored in the input vector
 */
template<typename VecData>
class VectorSerializer {
public:

    /// @brief Serializes the given vector_t<VecData> into a string
	virtual std::string Serialize(const vector_t<VecData>& vector) =0;
	virtual ~VectorSerializer() = default;
};

/**
* @brief Serializes a vector_t to a string of form x1, x2, x3, ... 
*/
class CSVDoubleVectorSerializer : public VectorSerializer<double> {
private:
	char fieldDelimiter_;

public:
    /**
    * @brief Constructs the serializer.
    * @param fieldDelimiter The delimiter used to separate individual values from the vector_t in the final string
    */
	CSVDoubleVectorSerializer(char fieldDelimiter);

    /// @brief Serializes the input vector_t<double> into a string
	std::string Serialize(const vector_t<double>& vector) override;
};

// Definitions

template<typename VecData>
double SquaredEuclideanDistanceMetric<VecData>::operator()(const vector_t<VecData>& vector1, const vector_t<VecData>& vector2) const {
	double sum = 0;

	for (size_t i = 0; i < vector1.size(); ++i) {
		double difference = vector2[i] - vector1[i];
		sum += difference * difference;
	}

	return sum;
}

template<typename VecData>
double CosineDistanceMetric<VecData>::operator()(const vector_t<VecData>& vector1, const vector_t<VecData>& vector2) const {
	double scalarProduct = 0;
	double vector1Sum = 0;
	double vector2Sum = 0;

	for (size_t i = 0; i < vector1.size(); ++i) {
		scalarProduct += (vector1[i] * vector2[i]);
		vector1Sum += vector1[i] * vector1[i];
		vector2Sum += vector2[i] * vector2[i];
	}

    double denominator = (std::sqrt(vector1Sum) * std::sqrt(vector2Sum));

	if (denominator == 0.0) // if one of the vectors is a zero vector, it makes no sense to compare the angle between the vectors
		return (vector1Sum == 0.0 && vector2Sum == 0.0) ? 0.0 : 1.0; // if we are comparing the zero vector to itself, dist should be 0, otw 1
    
	return 1 - scalarProduct/denominator;
}

#endif
