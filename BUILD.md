# Build Instructions

## Option 1: Using Docker (Recommended - No CMake needed locally)

### Prerequisites
- Docker and Docker Compose installed

### Build and Run
```bash
# Build and start all services (MongoDB + Search Engine)
docker-compose up --build

# Or run in detached mode
docker-compose up -d --build
```

### Stop Services
```bash
docker-compose down
```

### View Logs
```bash
docker-compose logs -f search_engine
```

## Option 2: Local Build (Requires CMake and MongoDB C++ Driver)

### Install Dependencies on macOS

#### MongoDB C++ Driver
```bash
brew install mongo-cxx-driver
```

This will install both `libmongocxx` and `libbsoncxx` which are required.

### Install CMake on macOS

#### Using Homebrew (Recommended)
```bash
brew install cmake
```

#### Using MacPorts
```bash
sudo port install cmake
```

#### Manual Installation
Download from: https://cmake.org/download/

### Build Locally
```bash
# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
cmake --build . -j$(sysctl -n hw.ncpu)

# Run (requires MongoDB running locally)
./SearchEngine
```

### Start MongoDB Locally
```bash
# Using Docker
docker run -d -p 27017:27017 --name mongodb mongo:7.0

# Or using Homebrew
brew install mongodb-community
brew services start mongodb-community
```

## Testing the API

Once running, test the endpoints:

```bash
# Health check
curl http://localhost:8080/health

# Get statistics
curl http://localhost:8080/stats

# Search (POST with JSON)
curl -X POST http://localhost:8080/search \
  -H "Content-Type: application/json" \
  -d '{"query": "шахматы"}'
```

