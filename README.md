# Search Engine

A Russian text search engine with boolean query support, built as a web service.

## Features

- Russian text tokenization and stemming
- Inverted index for fast document retrieval
- Boolean search (AND, OR, NOT operations)
- RESTful web API
- MongoDB integration
- Docker support

## Project Structure

```
.
├── include/
│   ├── containers/          # Custom hash map and hash set implementations
│   ├── text_processing/    # Tokenization, stemming, UTF-8 conversion
│   ├── search/             # Boolean search and set operations
│   ├── database/           # MongoDB client wrapper
│   ├── indexing/           # Index building and statistics
│   └── web/                # HTTP server
├── src/                    # Implementation files
├── CMakeLists.txt          # Build configuration
├── Dockerfile              # Docker image definition
└── docker-compose.yml      # Docker Compose configuration
```

## Building

### Local Build

```bash
mkdir build
cd build
cmake ..
cmake --build . -j$(nproc)
```

### Docker Build

```bash
docker-compose up --build
```

## Running

### Local

1. Start MongoDB:
```bash
docker run -d -p 27017:27017 mongo:7.0
```

2. Run the application:
```bash
./build/SearchEngine
```

### Docker

```bash
docker-compose up
```

The service will be available at `http://localhost:8080`

## API Endpoints

### Health Check
```
GET /health
```
Returns: `OK`

### Statistics
```
GET /stats
```
Returns JSON with indexing statistics:
- `docs_count`: Number of indexed documents
- `total_bytes`: Total text size in bytes
- `total_tokens`: Total number of tokens
- `avg_token_length`: Average token length
- `indexing_time_seconds`: Time taken to build index
- `indexing_speed_kb_per_sec`: Indexing speed
- `top_frequencies`: Top 10 term frequencies

### Search
```
POST /search
Content-Type: application/json

{
  "query": "<query>"
}
```
Query format: Russian text with optional boolean operators (`&&`, `||`, `!`) and parentheses for grouping.

Operator precedence: `!` (NOT) > `&&` (AND) > `||` (OR)

Example requests:
```bash
# Simple AND query
curl -X POST http://localhost:8080/search \
  -H "Content-Type: application/json" \
  -d '{"query": "шахматы && турнир"}'

# OR query
curl -X POST http://localhost:8080/search \
  -H "Content-Type: application/json" \
  -d '{"query": "шахматы || шахматист"}'

# Complex query with parentheses
curl -X POST http://localhost:8080/search \
  -H "Content-Type: application/json" \
  -d '{"query": "(шахматы && турнир) || (шахматист && игра)"}'

# NOT query
curl -X POST http://localhost:8080/search \
  -H "Content-Type: application/json" \
  -d '{"query": "шахматы && !турнир"}'
```

Returns JSON with:
- `status`: "success" or "error"
- `count`: Number of matching documents
- `documents`: Array of document objects with `id` and `url`

## Configuration

Environment variables (for Docker):
- `MONGODB_URI`: MongoDB connection string (default: `mongodb://localhost:27017`)
- `DB_NAME`: Database name (default: `documents`)
- `COLLECTION_NAME`: Collection name (default: `documents`)
- `SERVER_PORT`: Server port (default: `8080`)

## Dependencies

- MongoDB C++ Driver (v3.8.0)
- cpp-httplib (v0.14.3)
- jsoncpp (1.9.5)

All dependencies are automatically downloaded by CMake.

