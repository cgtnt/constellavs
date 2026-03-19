#ifndef NSW_INDEX_GRAPH_H_
#define NSW_INDEX_GRAPH_H_

#include "Vector.h"

#include <queue>
#include <vector>
#include <algorithm>
#include <utility>
#include <concepts>

constexpr size_t STARTING_SEARCH_EPOCH = 0;

// Duplicate vector detection
constexpr double DUPLICATE_IDENTIFICATION_EPSILON = 1e-9;

template<typename Id>
concept Identifier = std::integral<Id>;

/// @brief Represents a pre-calculated distance measurement from a database object labeled with the given Id to some other object.
template<Identifier d_identifier_t>
class DistanceMeasurement {
public:
	d_identifier_t Id;
	double Distance;
};

/**
* @brief A comparator for DistanceMeasurement.
* @tparam MaxComparator If true, the comparator will return true if the first element is greater than the second.
* If false, returns true if second element is greater than the first. 
*/
template <Identifier d_identifier_t, bool MaxComparator>
class DistanceComparator {
public:
	bool operator()(const DistanceMeasurement<d_identifier_t>& a, const DistanceMeasurement<d_identifier_t>& b) const noexcept;
};

/// @brief A node of the NSWIndexGraph.
template<Identifier database_identifier_t>
class Node {
private:
	database_identifier_t id_;

public:
	std::vector<database_identifier_t> Neighbors;
    
    /// @brief Getter for the node ID.
	inline database_identifier_t Id() const noexcept {
		return id_;
	}

	inline Node(database_identifier_t id, const std::vector<database_identifier_t>& neighbors) : id_(id), Neighbors(neighbors) {};
	inline Node(database_identifier_t id, std::vector<database_identifier_t>&& neighbors) : id_(id), Neighbors(std::move(neighbors)) {};
};

/**
* @brief An index of a given database_storage_t based on Navigable Small World graphs. 
* @tparam DBData Data type of objects stored in the provided database_storage_t
* @tparam Metric A functor used to calculate distance (double) between two DBData objects
*/
template<typename DBData, typename d_storage_t, Identifier d_identifier_t, DistanceMetric<DBData> Metric>
class NSWIndexGraph {
private:
    // Type aliasing
    using measurement_t = DistanceMeasurement<d_identifier_t>;

    using candidates_queue_t = std::priority_queue<measurement_t, std::vector<measurement_t>, DistanceComparator<d_identifier_t, true>>;
    using search_buffer_t = std::priority_queue<measurement_t, std::vector<measurement_t>, DistanceComparator<d_identifier_t, false>>;

    // Data members
	const Metric distanceMetric_;
	const size_t targetConnectivity_;
    size_t kQueryBufferSizeMultiplier_;
    size_t kInsertBufferSizeMultiplier_;
    size_t kSearchMaxEntrypoints_;

	const d_storage_t* dbData_;
    const std::vector<bool>* deleted_;
	
    std::vector<Node<d_identifier_t>> nodes_;

    /** 
    * @brief Instead of creating a new set to store visited nodes on each invocation of getApproxKNearest, we maintain a data member std::vector and count getApproxKNearest invocations
    * to indicate if a node has been visited in the current search invocation or not (epoch system)
    */
    std::vector<size_t> searchVisited_;
    size_t searchEpoch_ = STARTING_SEARCH_EPOCH;

    /// @brief Checks if the node with the given ID has been visited in the current search epoch.
    /// @return bool True if the node was visited, false otherwise.
    inline bool isVisited(d_identifier_t id) {
        return searchVisited_[id] == searchEpoch_;
    }

    /// @brief Marks the node with the given ID as visited in the current search epoch.
    inline void markVisited(d_identifier_t id) {
        searchVisited_[id] = searchEpoch_;
    }

    /**
    * @brief Calculates the distance of the candidate to the query data, then inserts the candidate to both the searchQueue and resultBuffer if 
    * the resultBuffer is not full or the calculated distance is smaller than the worst distance currently in the resultBuffer 
    * @param queryData The query data to which k nearest neighbors need to be found
    * @param candidateId Id of the current search node
    * @param searchQueue The queue of nodes to visit
    * @param resultBuffer The buffer of search results so far
    * @param targetSearchBufferSize The targeted size of the search result buffer
    */
    inline void evaluateSearchCandidate(
        const DBData& queryData, 
        d_identifier_t candidateId,
        candidates_queue_t& searchQueue,
        search_buffer_t& resultBuffer,
        size_t targetSearchBufferSize
    );

    /**
    * @brief Finds the approximate k-nearest neighbors in the database_storage_t to the queryData
    * @param queryData The query data to which k nearest neighbors need to be found
    * @param k How many nearest neighbors to find
    * @param kBufferSize The size of the search results buffer to maintain during the search
    * @return std::vector of records corresponding to approximate k-nearest neighbors where each record contains an id of a databse object and measured distance to query data
    */
	std::vector<measurement_t> getApproxKNearest(const DBData& queryData, size_t k, size_t kBufferSize);

    /**
    * @brief If the neighbor count of the given node is greater than the targetConnectivity_, keeps targetConnectivity_ neighbors closest to the node and discard the rest.
    * @param node Node which's neighbors to prune
    */
    void pruneNodeNeighbors(Node<d_identifier_t>& node);

public:

    /**
    * @brief Constructs the index graph.
    * @param targetConnectivity The targeted and maximum degree of nodes of the graph
    * @param kQueryBufferSizeMultiplier Determines how much bigger than k the resultBuffer will be during the query operation
    * @param kInsertBufferSizeMultiplier Determines how much bigger than k the resultBuffer will be during the insert operation
    * @param kSearchMaxEntrypoints Maximum number of search entrypoints that will be used during the getApproxKNearest operation
    * @param dbData Pointer to a database_storage_t which the graph should index
    */
	NSWIndexGraph(
        size_t targetConnectivity,
        size_t kQueryBufferSizeMultiplier,
        size_t kInsertBufferSizeMultiplier,
        size_t kSearchMaxEntrypoints,
        const d_storage_t* dbData,
        const std::vector<bool>* deleted
    );

    /**
    * @brief Inserts a data point into the index.
    * @param data Data to insert
    * @return true if the insert is successful, false if the provided data is a duplicate
    */
	bool Insert(const DBData& data); 

    /**
    * @brief Queries the index for approximate k-nearest neighbors of the provided queryData
    * @param queryData Data to which approximate k-nearest neighbors should be found
    * @return std::vector of records corresponding to approximate k-nearest neighbors where each record contains an id of a databse object and measured distance to query data
    */
	inline std::vector<measurement_t> Query(const DBData& queryData, size_t approx_k_nearest); 
};

// DEFINITIONS

// DistanceComparator

template <Identifier d_identifier_t, bool MaxComparator>
bool DistanceComparator<d_identifier_t, MaxComparator>::operator()(const DistanceMeasurement<d_identifier_t>& a, const DistanceMeasurement<d_identifier_t>& b) const noexcept {
	if constexpr (MaxComparator) {
		return a.Distance > b.Distance;
	}
	else {
		return a.Distance < b.Distance;
	}
}

// NSWIndexGraph

template<typename DBData, typename d_storage_t, Identifier d_identifier_t, DistanceMetric<DBData> Metric>
NSWIndexGraph<DBData, d_storage_t, d_identifier_t, Metric>::NSWIndexGraph(
        size_t targetConnectivity,
        size_t kQueryBufferSizeMultiplier,
        size_t kInsertBufferSizeMultiplier,
        size_t kSearchMaxEntrypoints,
        const d_storage_t* dbData,
        const std::vector<bool>* deleted
) : distanceMetric_(Metric()), 
    targetConnectivity_(targetConnectivity), 
    kQueryBufferSizeMultiplier_(kQueryBufferSizeMultiplier), 
    kInsertBufferSizeMultiplier_(kInsertBufferSizeMultiplier),
    kSearchMaxEntrypoints_(kSearchMaxEntrypoints),
    dbData_(dbData),
    deleted_(deleted)
{};

template<typename DBData, typename d_storage_t, Identifier d_identifier_t, DistanceMetric<DBData> Metric>
void NSWIndexGraph<DBData, d_storage_t, d_identifier_t, Metric>::evaluateSearchCandidate(
    const DBData& queryData,
    d_identifier_t candidateId,
    candidates_queue_t& candidates,
    search_buffer_t& resultBuffer,
    size_t targetSearchBufferSize
) {
    double candidateDistance = distanceMetric_(queryData, (*dbData_)[candidateId]);

    if (resultBuffer.size() < targetSearchBufferSize || resultBuffer.top().Distance > candidateDistance) {
        // We always traverse deleted nodes as well but do not incldue them in results
        candidates.emplace(candidateId, candidateDistance);
        
        if (!(*deleted_)[candidateId]) {
        // Maintain only kBufferSize best results
            if (resultBuffer.size() >= targetSearchBufferSize)
                resultBuffer.pop();

            resultBuffer.emplace(candidateId, candidateDistance);
        }
        
    }
}

template<typename DBData, typename d_storage_t, Identifier d_identifier_t, DistanceMetric<DBData> Metric>
typename std::vector<DistanceMeasurement<d_identifier_t>> NSWIndexGraph<DBData, d_storage_t, d_identifier_t, Metric>::getApproxKNearest(const DBData& queryData, size_t k, size_t kBufferSize) {
	if (dbData_->empty() || nodes_.empty())
		return std::vector<measurement_t>();

    ++searchEpoch_;

	candidates_queue_t candidates;
	search_buffer_t kBuffer;

	size_t numOfEntrypoints = std::min<size_t>(kSearchMaxEntrypoints_, nodes_.size());

    // Enqueue search entrypoints
	for (size_t i = 0; i < numOfEntrypoints; ++i) {
		d_identifier_t entrypointId = i * (nodes_.size() / numOfEntrypoints);

		if (isVisited(entrypointId))
			continue;

		markVisited(entrypointId);
        evaluateSearchCandidate(
            queryData,
            entrypointId,
            candidates,
            kBuffer,
            kBufferSize
        );
	}

    // Search
	while (!candidates.empty()) {
		DistanceMeasurement candidate = candidates.top();
		candidates.pop();

		// If the search buffer is full and this node is worse than worst node in the buffer, we have found the (approximate) best result and can break
		if (kBuffer.size() >= kBufferSize && candidate.Distance > kBuffer.top().Distance)
			break;

		for (d_identifier_t neighborId : nodes_[candidate.Id].Neighbors) {
            if (isVisited(neighborId))
                continue;

		    markVisited(neighborId);
            evaluateSearchCandidate(
                queryData,
                neighborId,
                candidates,
                kBuffer,
                kBufferSize
            );
		}
	}

	// Trunctate search buffer to k values to return 
	while (kBuffer.size() > k)
		kBuffer.pop();

	std::vector<measurement_t> result;
    result.reserve(k);
	while (kBuffer.size() > 0) {
		result.push_back(kBuffer.top());
		kBuffer.pop();
	}
	std::ranges::reverse(result);

	return result;
}
	
template<typename DBData, typename d_storage_t, Identifier d_identifier_t, DistanceMetric<DBData> Metric>
void NSWIndexGraph<DBData, d_storage_t, d_identifier_t, Metric>::pruneNodeNeighbors(Node<d_identifier_t>& node) {
	if (node.Neighbors.size() <= targetConnectivity_)
		return;

	const DBData& nodeData = (*dbData_)[node.Id()];
	
	std::vector<measurement_t> neighborCandidates;
    neighborCandidates.reserve(node.Neighbors.size());
	for (auto&& neighborId : node.Neighbors) {
		double distance = distanceMetric_(nodeData, (*dbData_)[neighborId]);
		neighborCandidates.emplace_back(neighborId, distance);
	}
    std::ranges::nth_element(neighborCandidates, neighborCandidates.begin() + targetConnectivity_, DistanceComparator<d_identifier_t, false>());

    node.Neighbors.clear();
	for (size_t i = 0; i < targetConnectivity_; ++i)
		node.Neighbors.push_back(neighborCandidates[i].Id);
}

template<typename DBData, typename d_storage_t, Identifier d_identifier_t, DistanceMetric<DBData> Metric>
bool NSWIndexGraph<DBData, d_storage_t, d_identifier_t, Metric>::Insert(const DBData& data) {
	std::vector<measurement_t> neighborCandidates = getApproxKNearest(data, targetConnectivity_, targetConnectivity_ * kInsertBufferSizeMultiplier_);

	// If the inserted vector is a duplicate, we do not insert it again
	if (!neighborCandidates.empty()) {
		d_identifier_t closestId = neighborCandidates[0].Id;

		if (distanceMetric_(data, (*dbData_)[closestId]) < DUPLICATE_IDENTIFICATION_EPSILON)
			return false; 
	}

	d_identifier_t newNodeId = nodes_.size();
    std::vector<d_identifier_t> newNodeNeighbors;
    newNodeNeighbors.reserve(neighborCandidates.size());

	for (DistanceMeasurement candidate : neighborCandidates) {
        newNodeNeighbors.push_back(candidate.Id);

        std::vector<d_identifier_t>& candidateNeighbors = nodes_[candidate.Id].Neighbors;
	    
        candidateNeighbors.push_back(newNodeId);
	    if (candidateNeighbors.size() > targetConnectivity_)
		    pruneNodeNeighbors(nodes_[candidate.Id]);
	}

	nodes_.emplace_back(newNodeId, std::move(newNodeNeighbors));
    searchVisited_.push_back(searchEpoch_);
	return true;
}

template<typename DBData, typename d_storage_t, Identifier d_identifier_t, DistanceMetric<DBData> Metric>
std::vector<DistanceMeasurement<d_identifier_t>> NSWIndexGraph<DBData, d_storage_t, d_identifier_t, Metric>::Query(const DBData& object, size_t approx_k_nearest) {
	return getApproxKNearest(object, approx_k_nearest, approx_k_nearest * kQueryBufferSizeMultiplier_);
}

#endif
