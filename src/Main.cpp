#include "ConfigurationParser.h"
#include "VectorDatabase.h"
#include "Utility.h"
#include "CommandHandler.h"

#include <iostream>
#include <memory>

constexpr int SUCCESS_EXIT_CODE = 0;
constexpr int FAILURE_EXIT_CODE = 1;

constexpr char CSV_VECTOR_FIELD_DELIMITER = ',';

int main(int argc, char** argv) {
	try {
		args_t arguments(argv + 1, argv + argc);

		std::shared_ptr<VectorDatabase<double>> database = nullptr;

		NSWVectorDatabaseConfiguration vectorDBConfiguration;
		ConfigurationParser::Parse(arguments, vectorDBConfiguration);

		auto vectorParser = std::make_unique<CSVDoubleVectorParser>(CSV_VECTOR_FIELD_DELIMITER, vectorDBConfiguration.VectorDimension);
		auto vectorSerializer = std::make_unique<CSVDoubleVectorSerializer>(CSV_VECTOR_FIELD_DELIMITER);

		switch (vectorDBConfiguration.DistanceMetric) {
		case DistanceMetricSelection::SQUARED_EUCLIDEAN:
			database = std::make_shared<NSWVectorDatabase<double, SquaredEuclideanDistanceMetric<double>>>(vectorDBConfiguration);
			break;
		case DistanceMetricSelection::COSINE:
			database = std::make_shared<NSWVectorDatabase<double, CosineDistanceMetric<double>>>(vectorDBConfiguration);
			break;
		default:
			throw NotImplementedException{"Corresponding DistanceMetricSelection not implemented in main"};
		}
		
		CommandHandler<double> commandHandler(database, std::move(vectorParser), std::move(vectorSerializer));
		commandHandler.StartCommandLoop(std::cin, std::cout);

		return SUCCESS_EXIT_CODE;
	}
	catch (const Exception& e) {
		std::cout << e.Message() << std::endl;
		return FAILURE_EXIT_CODE;
	}
}
