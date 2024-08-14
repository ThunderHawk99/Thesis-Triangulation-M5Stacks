import numpy as np
from config import Config
# TODO : Instead of always placing a box, generate a random box and check if the centroids are within the box
def calculate_box_boundaries(box_width, box_height, field_width, field_height):
    """Calculate the boundaries of a randomly positioned box within the field."""
    # Define the maximum possible x and y coordinates for the box's center to keep it within the field
    max_center_x = field_width - box_width / 2
    max_center_y = field_height - box_height / 2

    # Generate a random position for the box's center within allowed ranges
    center_x = np.random.uniform(box_width / 2, max_center_x)
    center_y = np.random.uniform(box_height / 2, max_center_y)

    # Calculate the boundaries of the box based on the center position
    min_x = center_x - box_width / 2
    max_x = center_x + box_width / 2
    min_y = center_y - box_height / 2
    max_y = center_y + box_height / 2

    return min_x, max_x, min_y, max_y

def get_box_coordinates(box_boundaries):
    """Define the box coordinates and appearance."""
    min_x, max_x, min_y, max_y = box_boundaries

    return {
        'x': [min_x, max_x, max_x, min_x, min_x],
        'y': [min_y, min_y, max_y, max_y, min_y],
        'color': 'rgba(0, 255, 0, 0.2)',
        'line': {'color': 'rgba(0, 255, 0, 0.8)'}
    }

def are_centroids_within_box(centroids, box_boundaries):
    """Check if all centroids are within the specified box."""
    if not centroids:
        raise ValueError("Centroids list cannot be empty")

    min_x, max_x, min_y, max_y = box_boundaries
    for centroid in centroids:
        try:
            x = centroid['x']
            y = centroid['y']
        except KeyError as e:
            raise ValueError(f"Each centroid must have the key: {e}")

        if not (min_x <= x <= max_x and min_y <= y <= max_y):
            return False

    return True

def are_tags_in_order(centroids):
    """Check if centroids are ordered by increasing x values."""
    if len(centroids) != 3:
        return False

    return centroids[0]['x'] < centroids[1]['x'] < centroids[2]['x']

def determine_reorder_steps(centroids):
    """Determine the steps to reorder centroids based on their x values."""
    x_values = [(centroid['x'], index) for index, centroid in enumerate(centroids)]
    sorted_x_values = sorted(x_values, key=lambda item: item[0])
    index_mapping = {original_index: target_index for target_index, (_, original_index) in enumerate(sorted_x_values)}

    steps = []
    visited = [False] * len(centroids)

    for i in range(len(centroids)):
        if visited[i] or index_mapping[i] == i:
            continue

        cycle_start = i
        cycle = []

        while not visited[cycle_start]:
            visited[cycle_start] = True
            cycle.append(cycle_start)
            cycle_start = index_mapping[cycle_start]

        steps.extend((cycle[j], cycle[j + 1]) for j in range(len(cycle) - 1))

    return steps

def determine_move_steps(centroids, box_boundaries):
    """Determine movement steps for centroids that are outside the specified box."""
    min_x, max_x, min_y, max_y = box_boundaries

    def get_direction(x, y):
        directions = []
        if x < min_x:
            directions.append("East")
        elif x > max_x:
            directions.append("West")

        if y < min_y:
            directions.append("North")
        elif y > max_y:
            directions.append("South")

        return " and ".join(directions) or "Inside"

    move_steps = []

    for index, centroid in enumerate(centroids):
        try:
            x = centroid['x']
            y = centroid['y']
        except KeyError as e:
            raise ValueError(f"Each centroid must have the key: {e}")

        direction = get_direction(x, y)

        if direction != "Inside":
            move_steps.append({
                'index': index + 1,
                'direction': direction
            })

    return move_steps

