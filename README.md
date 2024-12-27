# Quantum RNG API

A high-performance REST API for true random number generation using quantum mechanical processes. This API provides cryptographically secure random numbers suitable for various applications including cryptography, scientific simulations, gaming, and statistical sampling.

## Features

- True random number generation using quantum mechanical processes
- Multiple output formats (bytes, integers, floats, booleans)
- Range-based random number generation
- Random element selection from arrays
- Real-time entropy monitoring
- Comprehensive API documentation with Swagger UI
- Production-ready with Railway deployment support

## API Documentation

The API comes with a built-in Swagger UI that provides interactive documentation and testing capabilities. Access the Swagger UI at:

```
https://api.tsotchke.net/v1/docs
```

This interface allows you to:
- Explore all available endpoints
- Test API calls directly from the browser
- View request/response schemas
- Understand parameter requirements and constraints

## Deployment to Railway

1. Create a new project on [Railway](https://railway.app)

2. Connect your GitHub repository

3. Deploy the service

The API will automatically build and deploy. Railway will handle building the native module in the container environment.

## API Endpoints

### Base URL
```
https://api.tsotchke.net
```

### Root Endpoint
```
GET /
```
Returns a welcome message and API information.

### API Version 1

All v1 endpoints are prefixed with `/v1`

#### Health Check
```
GET /v1/health
```

#### Random Number Generation

All random number generation endpoints are prefixed with `/v1/qrng/`

##### Random Bytes
```
GET /v1/qrng/bytes/:count
```
Parameters:
- count: Number of random bytes (1-1024)

##### Random Number
```
GET /v1/qrng/number?type=float|uint64
```
Parameters:
- type: "float" (0-1) or "uint64" (optional, defaults to float)

##### Random Range
```
GET /v1/qrng/range?min=0&max=100
```
Parameters:
- min: Minimum value (inclusive)
- max: Maximum value (inclusive)

##### Random Boolean
```
GET /v1/qrng/boolean?probability=0.5
```
Parameters:
- probability: Chance of true (0-1, optional, defaults to 0.5)

##### Random Choice
```
POST /v1/qrng/choice
Content-Type: application/json

{
    "array": ["apple", "banana", "orange"]
}
```
Parameters:
- array: Array of items to choose from

## Local Development

### Prerequisites

- Node.js >= 18.0.0
- npm or yarn
- C++ build tools (for native module compilation)

### Setup

1. Install dependencies:
```bash
npm install
```

2. Build the native module:
```bash
npm run build
```

3. Start the server:
```bash
npm start
```

The server will run on port 3000 by default. Set the PORT environment variable to change this.

## Technical Details

### Quantum Random Number Generation

This API uses hardware-level quantum mechanical processes to generate true random numbers. The implementation:

- Leverages quantum mechanical noise as a source of entropy
- Provides cryptographically secure random numbers
- Continuously monitors entropy levels
- Implements various post-processing techniques to ensure uniform distribution

### Performance and Security

- High-performance C implementation with Node.js bindings
- Continuous entropy monitoring and quality assurance
- Input validation and error handling
- Rate limiting and security best practices

## Error Handling

The API uses standard HTTP status codes and returns detailed error messages in JSON format:

```json
{
    "error": "Error description"
}
```

Common status codes:
- 200: Successful request
- 400: Invalid input parameters
- 500: Internal server error

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Citation

If you use this software in your research, please cite it as:

```bibtex
@software{quantum_rng,
  title = {Semi-Classical Quantum Random Number Generator With Examples},
  author = {tsotchke},
  year = {2024},
  url = {https://github.com/tsotchke/quantum_rng}
}
```
