const express = require('express');
const swaggerUi = require('swagger-ui-express');
const swaggerJsdoc = require('swagger-jsdoc');

let QuantumRNG;

// Swagger definition
const swaggerOptions = {
    definition: {
        openapi: '3.0.0',
        info: {
            title: 'Quantum RNG API',
            version: '1.0.0',
            description: 'A REST API for generating true random numbers using quantum mechanics',
            license: {
                name: 'ISC',
                url: 'https://opensource.org/licenses/ISC',
            },
        },
        servers: [
            {
                url: 'https://api.tsotchke.net',
                description: 'Production server',
            },
        ],
    },
    apis: ['./server.js'], // files containing annotations
};

const swaggerDocs = swaggerJsdoc(swaggerOptions);

try {
    console.log('Current directory:', process.cwd());
    console.log('Attempting to load quantum_rng module...');
    const quantum_rng = require('./build/Release/quantum_rng.node');
    console.log('Module loaded:', quantum_rng);
    QuantumRNG = quantum_rng.QuantumRNG;
    console.log('QuantumRNG constructor:', QuantumRNG);
} catch (err) {
    console.error('Failed to load quantum_rng module:', err);
    process.exit(1);
}

const app = express();
app.use(express.json());

// Initialize quantum RNG
console.log('Initializing QuantumRNG instance...');
const rng = new QuantumRNG();
console.log('QuantumRNG instance created successfully');

// Add boolean and choice functions to rng object
rng.boolean = function(probability = 0.5) {
    return this.getDouble() < probability;
};

rng.choice = function(array) {
    if (!Array.isArray(array) || array.length === 0) {
        throw new Error('Array must be non-empty');
    }
    const index = Math.floor(this.getDouble() * array.length);
    return array[index];
};

// Welcome message
app.get('/', (req, res) => {
    res.json({
        message: 'Welcome to the Quantum RNG API',
        version: '1.0.0',
        documentation: 'https://api.tsotchke.net/v1/docs'
    });
});

// API v1 router
const v1Router = express.Router();

// Swagger UI
v1Router.use('/docs', swaggerUi.serve, swaggerUi.setup(swaggerDocs));

/**
 * @swagger
 * /v1/health:
 *   get:
 *     summary: Health check endpoint
 *     description: Returns the API status, version, and current entropy estimate
 *     tags: [System]
 *     responses:
 *       200:
 *         description: System health information
 *         content:
 *           application/json:
 *             schema:
 *               type: object
 *               properties:
 *                 status:
 *                   type: string
 *                   example: ok
 *                 version:
 *                   type: string
 *                   example: 1.0.0
 *                 entropy:
 *                   type: number
 *                   example: 8.2
 */
v1Router.get('/health', (req, res) => {
    res.json({
        status: 'ok',
        version: QuantumRNG.getVersion(),
        entropy: rng.getEntropyEstimate()
    });
});

/**
 * @swagger
 * /v1/qrng/bytes/{count}:
 *   get:
 *     summary: Get random bytes
 *     description: Returns the specified number of random bytes
 *     tags: [Random]
 *     parameters:
 *       - in: path
 *         name: count
 *         required: true
 *         schema:
 *           type: integer
 *           minimum: 1
 *           maximum: 1024
 *         description: Number of random bytes to generate
 *     responses:
 *       200:
 *         description: Random bytes in hex and raw format
 *         content:
 *           application/json:
 *             schema:
 *               type: object
 *               properties:
 *                 bytes:
 *                   type: string
 *                   description: Hex string representation of random bytes
 *                 raw:
 *                   type: array
 *                   items:
 *                     type: integer
 *       400:
 *         description: Invalid count parameter
 *       500:
 *         description: Server error
 */
v1Router.get('/qrng/bytes/:count', (req, res) => {
    try {
        const count = parseInt(req.params.count);
        if (isNaN(count) || count <= 0 || count > 1024) {
            return res.status(400).json({
                error: 'Count must be a number between 1 and 1024'
            });
        }
        
        const bytes = rng.getBytes(count);
        res.json({
            bytes: bytes.toString('hex'),
            raw: Array.from(bytes)
        });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

/**
 * @swagger
 * /v1/qrng/number:
 *   get:
 *     summary: Get random number
 *     description: Returns a random number in either float or uint64 format
 *     tags: [Random]
 *     parameters:
 *       - in: query
 *         name: type
 *         schema:
 *           type: string
 *           enum: [float, uint64]
 *           default: float
 *         description: Type of random number to generate
 *     responses:
 *       200:
 *         description: Random number
 *         content:
 *           application/json:
 *             schema:
 *               type: object
 *               properties:
 *                 value:
 *                   oneOf:
 *                     - type: number
 *                     - type: string
 *       400:
 *         description: Invalid type parameter
 *       500:
 *         description: Server error
 */
v1Router.get('/qrng/number', (req, res) => {
    try {
        const { type = 'float' } = req.query;
        
        let result;
        switch (type) {
            case 'float':
                result = rng.getDouble();
                break;
            case 'uint64':
                // Convert BigInt to string for JSON serialization
                result = rng.getUInt64().toString();
                break;
            default:
                return res.status(400).json({
                    error: 'Type must be either "float" or "uint64"'
                });
        }
        
        res.json({ value: result });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

/**
 * @swagger
 * /v1/qrng/range:
 *   get:
 *     summary: Get random number in range
 *     description: Returns a random integer within the specified range
 *     tags: [Random]
 *     parameters:
 *       - in: query
 *         name: min
 *         required: true
 *         schema:
 *           type: integer
 *         description: Minimum value (inclusive)
 *       - in: query
 *         name: max
 *         required: true
 *         schema:
 *           type: integer
 *         description: Maximum value (inclusive)
 *     responses:
 *       200:
 *         description: Random number within range
 *         content:
 *           application/json:
 *             schema:
 *               type: object
 *               properties:
 *                 value:
 *                   type: integer
 *       400:
 *         description: Invalid parameters
 *       500:
 *         description: Server error
 */
v1Router.get('/qrng/range', (req, res) => {
    try {
        const min = parseInt(req.query.min);
        const max = parseInt(req.query.max);
        
        if (isNaN(min) || isNaN(max)) {
            return res.status(400).json({
                error: 'Min and max must be numbers'
            });
        }
        
        if (min > max) {
            return res.status(400).json({
                error: 'Min must be less than or equal to max'
            });
        }
        
        const value = rng.getRange32(min, max);
        res.json({ value });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

/**
 * @swagger
 * /v1/qrng/boolean:
 *   get:
 *     summary: Get random boolean
 *     description: Returns a random boolean with specified probability of being true
 *     tags: [Random]
 *     parameters:
 *       - in: query
 *         name: probability
 *         schema:
 *           type: number
 *           minimum: 0
 *           maximum: 1
 *           default: 0.5
 *         description: Probability of returning true
 *     responses:
 *       200:
 *         description: Random boolean
 *         content:
 *           application/json:
 *             schema:
 *               type: object
 *               properties:
 *                 value:
 *                   type: boolean
 *       400:
 *         description: Invalid probability parameter
 *       500:
 *         description: Server error
 */
v1Router.get('/qrng/boolean', (req, res) => {
    try {
        const probability = parseFloat(req.query.probability) || 0.5;
        
        if (probability < 0 || probability > 1) {
            return res.status(400).json({
                error: 'Probability must be between 0 and 1'
            });
        }
        
        const value = rng.boolean(probability);
        res.json({ value });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

/**
 * @swagger
 * /v1/qrng/choice:
 *   post:
 *     summary: Get random array element
 *     description: Returns a random element from the provided array
 *     tags: [Random]
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             required:
 *               - array
 *             properties:
 *               array:
 *                 type: array
 *                 items:
 *                   type: string
 *                 minItems: 1
 *                 example: ["apple", "banana", "orange"]
 *     responses:
 *       200:
 *         description: Random element from array
 *         content:
 *           application/json:
 *             schema:
 *               type: object
 *               properties:
 *                 value:
 *                   type: any
 *       400:
 *         description: Invalid request body
 *       500:
 *         description: Server error
 */
v1Router.post('/qrng/choice', (req, res) => {
    try {
        const { array } = req.body;
        
        if (!Array.isArray(array) || array.length === 0) {
            return res.status(400).json({
                error: 'Request body must contain a non-empty array'
            });
        }
        
        const value = rng.choice(array);
        res.json({ value });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

// Mount v1 router
app.use('/v1', v1Router);

// Error handler
app.use((err, req, res, next) => {
    console.error(err.stack);
    res.status(500).json({
        error: 'Internal server error',
        message: err.message
    });
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
    console.log(`Quantum RNG API server running on port ${PORT}`);
    console.log(`Version: ${QuantumRNG.getVersion()}`);
    console.log(`Current entropy estimate: ${rng.getEntropyEstimate()}`);
});
