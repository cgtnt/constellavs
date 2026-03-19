#ifndef VECTOR_DATABASE_H_ 
#define VECTOR_DATABASE_H_ 

#include "NSWIndexGraph.h"
#include "Vector.h"
#include "Utility.h"

#include <limits>
#include <vector>
#include <format>

enum class DistanceMetricSelection { SQUARED_EUCLIDEAN, COSINE };

constexpr char INVALID_DIMENSION_EX_MESSAGE[] = "Invalid vector dimension <{}>: <{}> was expected";
constexpr char DUPLICATE_VECTOR_EX_MESSAGE[] = "Failed inserting vector - it is a duplicate";
constexpr char INVALID_ID_EX_MESSAGE[] = "Invalid vector id <{}>: database currently contains ids <{}> to <{}>";

// Valid numeric settings
constexpr size_t MIN_K_SEARCH = 1;
constexpr size_t MAX_K_SEARCH = std::numeric_limits<size_t>::max();

constexpr size_t MIN_CONNECTIVITY = 1;
constexpr size_t MAX_CONNECTIVITY = std::numeric_limits<size_t>::max();

constexpr size_t MIN_VECTOR_DIMENSION = 1;
constexpr size_t MAX_VECTOR_DIMENSION = std::numeric_limits<size_t>::max();

constexpr size_t MIN_K_BUFFERS_SIZE_MUL = 1;
constexpr size_t MAX_K_BUFFERS_SIZE_MUL = std::numeric_limits<size_t>::max();

constexpr size_t MIN_K_ENTRYPOINTS = 1;
constexpr size_t MAX_K_ENTRYPOINTS = std::numeric_limits<size_t>::max();

// Default configuration settings
constexpr DistanceMetricSelection DEFAULT_SETTING_DISTANCE = DistanceMetricSelection::SQUARED_EUCLIDEAN;
constexpr size_t DEFAULT_SETTING_CONNECTIVITY = 16;
constexpr size_t DEFAULT_SETTING_VECTOR_DIMENSION = 10;

constexpr size_t INDEX_K_QUERY_BUFFER_SIZE_MULTIPLIER = 3;
constexpr size_t INDEX_K_INSERT_BUFFER_SIZE_MULTIPLIER = 5;
constexpr size_t INDEX_K_MAX_ENTRYPOINTS = 10;

/**
 * @brief Database types 
 */
namespace db {
    using identifier_t = size_t;

    constexpr identifier_t MIN_IDENTIFIER = 0;
    constexpr identifier_t MAX_IDENTIFIER = std::numeric_limits<size_t>::max();

    // Database id parsing

    /// @brief Parses a string into a db::identifier_t.
    class IdentifierParser {
    public:

        /// @brief Parses the input string into a db::identifier_t
        inline static db::identifier_t Parse(const std::string& rawId) {
            return NumericParsingHelper::parseSize_T(rawId, db::MIN_IDENTIFIER, db::MAX_IDENTIFIER);
        };
    };
}

/**
 * @brief Configuration for a NSWVectorDatabase, including its NSWIndexGraph.
 */
class NSWVectorDatabaseConfiguration {
public:
    /// Selects the mathematical distance used by the index graph
	DistanceMetricSelection DistanceMetric = DEFAULT_SETTING_DISTANCE;

    /// Target and maximum degree of each node of the index graph
	size_t TargetConnectivity = DEFAULT_SETTING_CONNECTIVITY;

    /// Dimension of the vectors in the database
	size_t VectorDimension = DEFAULT_SETTING_VECTOR_DIMENSION;

    /// Multiplier applied to k to obtain the size of the search result buffer in the index graph query operation
    size_t IndexKQueryBufferSizeMultiplier = INDEX_K_QUERY_BUFFER_SIZE_MULTIPLIER;

    /// Multiplier applied to k to obtain the size of the search result buffer in the index graph insert operation
    size_t IndexKInsertBufferSizeMultipler = INDEX_K_INSERT_BUFFER_SIZE_MULTIPLIER;

    /// Maximum number of entrypoints used by the approximate k-nearest neighbors search procedure in the index graph
    size_t IndexKMaxEntrypoints = INDEX_K_MAX_ENTRYPOINTS;
};


/// @brief The interface to a vector database.
template<typename VecData>
class VectorDatabase {
public:
    /**
    * @brief Represents a VectorDatabase query result including the vector id, pre-calculated distance measurement from the query vector, and observer pointer to the resulting vector.
    */
    class QueryResult {
    public:
        db::identifier_t Id;
        double Distance;
        const vector_t<VecData>* Vector;

        inline QueryResult(db::identifier_t id, double distance, const vector_t<VecData>* vector)
        : Id(id), Distance(distance), Vector(vector) {};
    };

    /**
    * @brief Inserts the given vector into the database.
    * @param vector The vector to insert
    */
	virtual db::identifier_t Insert(vector_t<VecData>&& vector) =0;

    /**
    * @brief Queries the database for approximate k-nearest neighbors to the given query vector.
    * @param vector The vector to which approximate k-nearest neighbors should be found
    * @param k_nearest How many neighbors to find
    * @returns An std::vector containing the results of the query
    */
	virtual std::vector<QueryResult> Query(const vector_t<VecData>& vector, size_t k_nearest) =0;

    /**
    * @brief Performs soft deletion of the vector with the given ID from the database. The vector will remain in the database memory, but will not be returned by queries.
    * @param id Id of the vector to delete
    * */
	virtual void Delete(db::identifier_t id) =0;

	virtual ~VectorDatabase() = default;
};

/// @brief Implementation of VectorDatabase based on Navigable Small World indexing using NSWIndexGraph.
template<typename VecData, DistanceMetric<vector_t<VecData>> Metric>
class NSWVectorDatabase : public VectorDatabase<VecData> {
private:
    using storage_t = std::vector<vector_t<VecData>>;

	const size_t vectorDimension_;
	storage_t data_;
    std::vector<bool> deleted_;
	NSWIndexGraph<vector_t<VecData>, storage_t, db::identifier_t, Metric> indexGraph_;
 
    /**
    * @brief Asserts the given vector has dimension equal to the database's set VectorDimension.
    * @param vector The vector to assert
    */
	inline void assertVectorDimensionality(const vector_t<VecData>& vector);

public:

	NSWVectorDatabase(NSWVectorDatabaseConfiguration configuration);

    /**
    * @copydoc VectorDatabase::Insert
    */
	db::identifier_t Insert(vector_t<VecData>&& vector) override;  

    /**
    * @copydoc VectorDatabase::Query
    */
	std::vector<typename VectorDatabase<VecData>::QueryResult> Query(const vector_t<VecData>& vector, size_t k_nearest) override; 
	
    /**
    * @copydoc VectorDatabase::Delete
    */
    void Delete(db::identifier_t id) override;
};


template<typename DBData>
using database_storage_t = std::vector<DBData>;

// DEFINITIONS

// NSWVectorDatabase

template<typename VecData, DistanceMetric<vector_t<VecData>> Metric>
NSWVectorDatabase<VecData, Metric>::NSWVectorDatabase(NSWVectorDatabaseConfiguration configuration) 
	: vectorDimension_(configuration.VectorDimension), 
    indexGraph_(
        configuration.TargetConnectivity, 
        configuration.IndexKQueryBufferSizeMultiplier,
        configuration.IndexKInsertBufferSizeMultipler,
        configuration.IndexKMaxEntrypoints,
        &data_,
        &deleted_
    ) {};


template<typename VecData, DistanceMetric<vector_t<VecData>> Metric>
void NSWVectorDatabase<VecData, Metric>::assertVectorDimensionality(const vector_t<VecData>& vector) {
	if (vector.size() != vectorDimension_)
		throw ArgumentException(std::format(INVALID_DIMENSION_EX_MESSAGE, vector.size(), vectorDimension_));
}

template<typename VecData, DistanceMetric<vector_t<VecData>> Metric>
db::identifier_t NSWVectorDatabase<VecData, Metric>::Insert(vector_t<VecData>&& vector) {
	assertVectorDimensionality(vector);

	const vector_t<VecData>& insertedVector = data_.emplace_back(std::move(vector));
    deleted_.push_back(false);
	bool success = indexGraph_.Insert(insertedVector);

	if (!success) {
		data_.pop_back();
		deleted_.pop_back();
        throw ArgumentException(DUPLICATE_VECTOR_EX_MESSAGE);  
    }

    return data_.size() - 1;
}  

template<typename VecData, DistanceMetric<vector_t<VecData>> Metric>
std::vector<typename VectorDatabase<VecData>::QueryResult> NSWVectorDatabase<VecData, Metric>::Query(const vector_t<VecData>& vector, size_t k_nearest) {
	assertVectorDimensionality(vector);

	std::vector<DistanceMeasurement<db::identifier_t>> resultMeasurements = indexGraph_.Query(vector, k_nearest);
	std::vector<typename VectorDatabase<VecData>::QueryResult> result;

	for (const auto& res : resultMeasurements)
		result.emplace_back(res.Id, res.Distance, &(data_[res.Id]));

	return result;
}


template<typename VecData, DistanceMetric<vector_t<VecData>> Metric>
void NSWVectorDatabase<VecData, Metric>::Delete(db::identifier_t id) {
    if (id < 0 || id >= data_.size())
        throw ArgumentException(std::format(INVALID_ID_EX_MESSAGE, id, 0, data_.empty() ? 0 : data_.size() - 1));

    deleted_[id] = true;
}

#endif
