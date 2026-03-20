# ConstellaVS
## Overview
ConstellaVS is a vector similarity search engine indexed by Navigable Small World (NSW) graphs, capable of returning approximate k-nearest neighbors of an arbitrary query vector. The executable processes commands on the standard input and prints results to the standard output, allowing it to be used interactively in the terminal or programmatically as part of a larger pipeline.

## Build Instructions
### Prerequisites

To build ConstellaVS from source, ensure your system meets the following requirements:
    - CMake: Version 3.15 or higher.
    - Compiler: A C++20 compatible compiler (e.g., GCC 10+, Clang 10+, or MSVC 19.29+).

### Building from Source

ConstellaVS uses a standard out-of-source CMake build process. Open your terminal and execute the following commands:

```Bash

# 1. Clone the repository
git clone https://github.com/cgtnt/constellavs.git
cd constellavs

# 2. Create a build directory and navigate into it
mkdir build && cd build

# 3. Generate the build system files
cmake ..

# 4. Compile the executable (Release mode is highly recommended for performance)
cmake --build . --config Release
```

Once compiled, the constella executable will be located in your build directory (or build/Release on Windows). You can verify the installation by running:

```Bash
./constella 
```

## Configuration
The engine is configured at startup using command-line arguments. Usage: ```./constella <options>```. Any options which are not provided as CLI arguments will assume their default values listed below.

| Short |      Long      | Description                                                                                                                                                                                                                                                                                                                                  |  Range                     | Default        |
| ----- | -------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------- | -------------- |
|   -d  |   --distance   | The metric to be used to calculate distance between vectors both during indexing and query execution.                                                                                                                                                                                                                                        | ``squared_euclidean`` or ``cosine``| ``squared_euclidean``  |
|   -n  |  --dimension   | Dimension of the vectors which the engine will work with.                                                                                                                                                                                                                                                                                    | 1 .. max_size_t            |  10            |     
|   -c  | --connectivity | Degree of each vertex of the NSW index graph will be equal to this value. Higher values increase search accuracy but slow down insert and query operations. Lower values speed up both operations but increase risk of graph fragmentation and decrease search accuracy.                                                                     | 1 .. max_size_t            |  16            |       
|  N/A  | --qbuffer-mul  | During a query, the search operation maintains a buffer of best results. The size of this buffer when searching for k-nearest neighbors will be ``k * qbuffer-mul``. Higher values increase search accuracy but slow down query operations.                                                                                                  | 1 .. max_size_t            |  3             |      
|  N/A  | --ibuffer-mul  | Similarly, the insert operation maintains a buffer of closest vectors to the inserted vector before settling on which vectors to connect as neighbors. The size of this buffer will be ``connectivity * ibuffer-mul``. Higher values increase indexing accuracy (and therefore later search accuracy), but slow down insert operations.      | 1 .. max_size_t            |  5             |       
|  N/A  | --max-entry-pt | The number of entrypoints which will be used for search operations during both insertion and querying. Higher values decrease risk of search "getting stuck" in local minima and returning suboptimal results - which is especially present in fragmented index graphs or low connectivity graphs, but might increase search time.           | 1 .. max_size_t            |  10            |     

Example:
```./constella --dimension 50 --distance cosine -c 24```

## Commands - interface 
Once the executable starts, the command interface will start on standard input, printing ``>`` which indicates a command prompt. Commands are executed sequentially, one per line.

#### Vector formatting rule
Each field marked as ``<vector>`` must be in the CSV format ``x1,x2,x3,...`` (with optional spaces after each comma). The vector dimension must match the value set by ``--dimension`` during configuration.

### help 
Prints a list of all supported commands and their usage syntax. 
- Usage: ``help``

### insert
Inserts a single vector into the engine and prints its assigned ID.
- Usage: ``insert <vector>``

*Example:* 
```cmd
> insert 1, 5, 6, 3, 3.0
Vector inserted with id: <0>
> ...
```

### import
Reads a file line-by-line, interpreting each line as a vector which is inserted into the engine, and then prints the range of IDs that was assigned sequentially to each read vector. Each line of the file must follow the vector formatting rule. The command prints a progress update every 5000 processed lines.
- Usage: ``import <filepath>``

*Example:* 
```cmd
> import myvectors.txt
Starting import
PROGRESS: Processed <5000> lines
PROGRESS: Processed <10000> lines
Inserted vectors with ids <0> to <9999>
> ...
```

### query
Queries the engine for approximate k-nearest neighbors of the input query vector and prints the ID, distance from query vector, and vector value of each result. *Note:* the query vector can be an arbitrary vector that adheres to the formatting rule, it does not need to be already in the engine! 
- Usage: ``query <k nearest to get> <vector>``

*Example:* 
```cmd
> query 5 4,5,6.1,2.34,5
Query result:
Id: <1> Distance: <25.4856> Vector: <1,3,4,5,6>
Id: <0> Distance: <39.7256> Vector: <1,5,6,7,8>
Id: <4> Distance: <49.0452> Vector: <1,5,2.24,6.5,7.8>
Id: <2> Distance: <61.9656> Vector: <1,7,8,9,6>
Id: <3> Distance: <347501> Vector: <1,592,60,3,9.2>
> ...

```
### delete
Performs a soft-deletion of the vector with the given ID from the engine. This means the vector will not be returned as a result for any queries in the future, but will remain in the engine.
- Usage: ``delete <id>``

*Example:* 
```cmd
> delete 5
> ...
```

## Out of scope / architectural roadmap
While ConstellaVS meets its scope of requirements, the implementation can be further improved by making the following changes:
- Memory locality: The current implementation stores vectors as individual objects - transitioning to a flat-array style of storage would significantly reduce cache misses during the k-NN search.
- Hierarchical Navigable Small World indexing: an algorithmic improvement that could be made is building a hierarchy of NSW index graphs instead of relying on one graph.
- Concurrency: the current implementation is single-threaded, parallelizing for example the distance calculations of the candidates in the k-NN search would allow for faster search.
- Index persistance: adding the ability to save the NSW index graph to disk would allow users to repeatedly load a dataset to memory while avoiding the computational cost of building the index.
