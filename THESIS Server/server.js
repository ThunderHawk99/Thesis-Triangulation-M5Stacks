const express = require('express');
const cors = require('cors');
const app = express();
const port = 3000;

// Middleware to enable CORS
app.use(cors());

// Middleware to parse JSON data
app.use(express.json());

// Store received distances
let distances = {

};

// Absolute positions of anchors
const anchors = [
    { id: 'A', x: 0, y: 0, label: 'Anchor A' },
    { id: 'B', x: 0, y: 81, label: 'Anchor B' },
    { id: 'C', x: 128, y: 0, label: 'Anchor C' }
];

// Function to calculate tag position using trilateration
function calculateTagPosition(dA, dB, dC, anchorA, anchorB, anchorC) {
    // Extract anchor coordinates
    const xa = anchorA.x;
    const ya = anchorA.y;
    const xb = anchorB.x;
    const yb = anchorB.y;
    const xc = anchorC.x;
    const yc = anchorC.y;

    // Square the distances to avoid taking square roots later
    const dA2 = dA * dA;
    const dB2 = dB * dB;
    const dC2 = dC * dC;

    // Calculate tag position using trilateration
    const A = 2 * (xb - xa);
    const B = 2 * (yb - ya);
    const C = dA2 - dB2 - xa * xa + xb * xb - ya * ya + yb * yb;
    const D = 2 * (xc - xb);
    const E = 2 * (yc - yb);
    const F = dB2 - dC2 - xb * xb + xc * xc - yb * yb + yc * yc;

    // Check if denominator is zero (shouldn't be, unless anchors are coincident or distances are incorrect)
    const denominator = A * E - B * D;
    if (Math.abs(denominator) < 1e-6) {
        console.error('Denominator is zero or very close to zero. Cannot trilaterate accurately.');
        return { x: null, y: null };
    }

    // Calculate tag coordinates
    const x = (C * E - F * B) / denominator;
    const y = (A * F - D * C) / denominator;

    return { x, y };
}


// Endpoint to receive distances from tags
app.post('/distance', (req, res) => {
    var { tagId, dA, dB, dC } = req.body;
    dA = (dA * 100) - 15;
    dB = (dB * 100) - 15;
    dC = (dC * 100) - 15;
    distances[tagId] = { dA, dB, dC };
    console.log(`Received distances from tag ${tagId}: ${JSON.stringify(distances[tagId])}`);
    res.send('Distance received');
});

// Endpoint to get current time in seconds
app.get('/current-time', (req, res) => {
    const currentTime = new Date();
    const seconds = currentTime.getSeconds();
    console.log(`Current time in seconds: ${seconds}`);
    res.json({ seconds });
});

// Endpoint to calculate and return coordinates of tags
app.get('/positions', (req, res) => {
    if (Object.keys(distances).length === 0) {
        return res.status(400).send('No data received');
    }

    const tags = {};

    // Calculate positions for each tag based on distances using trilateration
    for (const tagId in distances) {
        const { dA, dB, dC } = distances[tagId];
        const { x, y } = calculateTagPosition(dA, dB, dC, anchors[0], anchors[1], anchors[2]);
        tags[tagId] = { x, y };
    }

    res.json(tags);
});

// Start the server
app.listen(port, () => {
    console.log(`Server is running on http://localhost:${port}`);
});
