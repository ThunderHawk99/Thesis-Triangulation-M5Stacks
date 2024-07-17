document.addEventListener("DOMContentLoaded", () => {
    // Function to draw an anchor or point
    function drawNode(x, y, label, color) {
        x = canvas.width / 2 + x;  // Adjusted to add x instead of subtracting
        y = canvas.height / 2 - y;
        ctx.fillStyle = color;
        ctx.beginPath();
        ctx.arc(x, y, 5, 0, 2 * Math.PI);
        ctx.fill();
        ctx.fillText(label, x + 10, y - 10);
    }

    const canvas = document.getElementById("myCanvas");
    const ctx = canvas.getContext("2d");

    // Function to draw the grid and axes
    function drawGrid() {
        // Clear canvas
        ctx.clearRect(0, 0, canvas.width, canvas.height);

        // Draw grid or background
        ctx.beginPath();
        ctx.strokeStyle = '#ddd';

        // Draw vertical grid lines
        for (let x = 0; x <= canvas.width; x += 50) {
            ctx.moveTo(x, 0);
            ctx.lineTo(x, canvas.height);
        }

        // Draw horizontal grid lines
        for (let y = 0; y <= canvas.height; y += 50) {
            ctx.moveTo(0, y);
            ctx.lineTo(canvas.width, y);
        }
        ctx.stroke();

        // Draw X and Y axes with labels
        ctx.beginPath();
        ctx.strokeStyle = '#000';
        ctx.lineWidth = 2;

        // X-axis
        ctx.moveTo(0, canvas.height / 2);
        ctx.lineTo(canvas.width, canvas.height / 2);
        ctx.fillText('X', canvas.width - 10, canvas.height / 2 - 10);

        // Y-axis
        ctx.moveTo(canvas.width / 2, 0);
        ctx.lineTo(canvas.width / 2, canvas.height);
        ctx.fillText('Y', canvas.width / 2 + 10, 10);

        ctx.stroke();
    }

    // Draw initial grid and anchors
    drawGrid();
    drawNode(0, 0, 'Anchor A', 'blue');
    drawNode(0, 81, 'Anchor B', 'blue');
    drawNode(128, 0, 'Anchor C', 'blue');

    function fetchPosition() {
        // Fetch tag positions from server
        fetch('http://localhost:3000/positions')
        .then(response => response.json())
        .then(data => {
            console.log(data);
            // Redraw grid and anchors
            drawGrid();
            drawNode(0, 0, 'Anchor A', 'blue');
            drawNode(0, 81, 'Anchor B', 'blue');
            drawNode(128, 0, 'Anchor C', 'blue');
            
            // Draw the fetched positions
            for (const [tagId, position] of Object.entries(data)) {
                const { x, y } = position;
                drawNode(x, y, tagId, 'red');
            }
        })
        .catch(error => {
            console.error('Error fetching positions:', error);
        });
    }

    // Fetch position every second
    setInterval(fetchPosition, 1000);
});
