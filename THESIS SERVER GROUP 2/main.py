import matplotlib.pyplot as plt
import numpy as np
from matplotlib.patches import Circle
from shapely.geometry import Point, Polygon as ShapelyPolygon
from shapely.ops import unary_union

# Define the anchor points and distances for multiple tags
anchors = [(0, 85), (85, 85), (85, 0), (0, 0)]
tag_distances = [
    [114, 108, 67, 61],  # Distances for Tag 1
    [59, 66, 104, 95],   # Distances for Tag 2
    [80,104,102,56.99999999999999]     # Distances for Tag 3
]
error_margin = 0

# Define a list of colors for the tags
tag_colors = ['blue', 'green', 'purple']

# Create a plot
fig, ax = plt.subplots()
ax.set_aspect('equal')

# Function to calculate the intersection of multiple circles
def intersect_multiple_circles(circles):
    if not circles:
        return None
    intersection_polygon = circles[0]
    for circle in circles[1:]:
        intersection_polygon = intersection_polygon.intersection(circle)
        if intersection_polygon.is_empty:
            return None
    return intersection_polygon

# Function to visualize each tag
def visualize_tag(tag_index, distances, color):
    circles = []
    for (x, y), distance in zip(anchors, distances):
        circle_outer = plt.Circle((x, y), distance + error_margin, color=color, fill=False, linestyle='--', alpha=0.5)
        circle_inner = plt.Circle((x, y), distance - error_margin, color=color, fill=False, linestyle='--', alpha=0.5)
        ax.add_patch(circle_outer)
        ax.add_patch(circle_inner)
        ax.plot(x, y, 'o', color=color, label=f'Anchor ({x},{y})')  # Anchor points with color
        
        # Create a Shapely circle
        circle = Point(x, y).buffer(distance + error_margin)
        circles.append(circle)
    
    # Compute the intersection of all circles for this tag
    common_overlap = intersect_multiple_circles(circles)
    
    # Plot the common overlap if it exists
    if common_overlap and not common_overlap.is_empty:
        x, y = common_overlap.exterior.xy
        ax.fill(x, y, alpha=0.3, color=color, label=f'Overlap Tag {tag_index + 1}')
        
        # Compute and plot the centroid of the common overlap
        centroid = common_overlap.centroid
        ax.plot(centroid.x, centroid.y, 'x', color='black', label=f'Centroid Tag {tag_index + 1}')
        print(f'Centroid of Overlap for Tag {tag_index + 1}: ({centroid.x:.2f}, {centroid.y:.2f})')

# Visualize each tag's circles and overlaps
for tag_index, distances in enumerate(tag_distances):
    visualize_tag(tag_index, distances, tag_colors[tag_index])

# Set limits and labels
plt.xlim(-10, 100)
plt.ylim(-10, 100)
plt.xlabel('X (cm)')
plt.ylabel('Y (cm)')
plt.title('Trilateration and Common Overlaps')
plt.grid(True)

# Add legend
plt.legend()

plt.show()
