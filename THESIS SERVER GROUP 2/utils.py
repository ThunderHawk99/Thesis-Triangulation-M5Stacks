import numpy as np
from config import Config

def is_tag_2_centered(centroids, box_boundaries, tolerance=5):
    """Check if tag 2 is centered within the bounded box, within a tolerance of 5 units."""
    if len(centroids) < 3:
        raise ValueError("At least three centroids are needed to check tag 2 placement.")

    # Calculate the center of the box
    min_x, max_x, min_y, max_y = box_boundaries
    center_x = (min_x + max_x) / 2
    center_y = (min_y + max_y) / 2

    # Get the position of tag 2
    tag_2 = centroids[1]
    tag_2_x = tag_2['x']
    tag_2_y = tag_2['y']

    # Check if tag 2 is within the tolerance range of the box center
    is_centered_x = abs(tag_2_x - center_x) <= tolerance
    is_centered_y = abs(tag_2_y - center_y) <= tolerance

    return is_centered_x and is_centered_y

def suggest_tag_2_placement(centroids, box_boundaries):
    """Suggest where to place tag 2 so that it is centered within the bounded box."""
    if len(centroids) < 3:
        raise ValueError("At least three centroids are needed to determine placement.")

    # Calculate the center of the box
    min_x, max_x, min_y, max_y = box_boundaries
    center_x = (min_x + max_x) / 2
    center_y = (min_y + max_y) / 2

    # Get the current position of tag 2
    tag_2 = centroids[1]
    tag_2_x = tag_2['x']
    tag_2_y = tag_2['y']

    message = None

    # Suggest movements in the x direction
    if tag_2_x < center_x - 5:
        message = {'index': 2, 'message': 'Move tag 2 to right to center it', 'tag_id': 2}
    elif tag_2_x > center_x + 5:
        message = {'index': 2, 'message': 'Move tag 2 to left to center it', 'tag_id': 2}

    # Suggest movements in the y direction
    if tag_2_y < center_y - 5:
        message =  {'index': 2, 'message': 'Move tag 2 up to center it', 'tag_id': 2}
    elif tag_2_y > center_y + 5:
        message = {'index': 2, 'message': 'Move tag 2 down to center it', 'tag_id': 2}

    return message

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
    if len(centroids) != 3:
        return False
    
    tag_1_x = centroids[0]['x']
    tag_2_x = centroids[1]['x']
    tag_3_x = centroids[2]['x']

    def is_within_range(centroid, anchor, threshold=15):
        return (abs(centroid['x'] - anchor['x']) <= threshold and
                abs(centroid['y'] - anchor['y']) <= threshold)
    
    return (is_within_range(centroids[0], centroids[1]) and is_within_range(centroids[1], centroids[2]) and (tag_1_x < tag_2_x < tag_3_x))

def determine_reorder_steps(centroids):
    def is_within_range(centroid, anchor, threshold=15):
        return (abs(centroid['x'] - anchor['x']) <= threshold and
                abs(centroid['y'] - anchor['y']) <= threshold)

    if len(centroids) < 3:
        raise ValueError("At least three centroids are needed to ensure the correct order of tags.")
    
    tag_1_x = centroids[0]['x']
    tag_2_x = centroids[1]['x']
    tag_3_x = centroids[2]['x']
    tag_1_y = centroids[0]['y']
    tag_2_y = centroids[1]['y']
    tag_3_y = centroids[2]['y']
    
    messages = []

    # Check if tag 1 is on the left side of tag 2
    if not (tag_1_x < tag_2_x and is_within_range(centroids[0], centroids[1])):
        # Check if tag 1 is above or below tag 2
        if tag_1_y < tag_2_y:
            messages.append({'index': 1, 'message': 'Move it left, up next to tag 2', 'tag_id': 1})
        else:
            messages.append({'index': 1, 'message': 'Move it left, down next to tag 2', 'tag_id': 1})
        
    # Check if tag 3 is on the right side of tag 2
    if not (tag_3_x > tag_2_x and is_within_range(centroids[2], centroids[1])):
        # Check if tag 3 is above or below tag 2
        if tag_3_y < tag_2_y:
            messages.append({'index': 3, 'message': 'Move it right, up next to tag 2', 'tag_id': 3})
        else:
            messages.append({'index': 3, 'message': 'Move it right, down next to tag 2', 'tag_id': 3})

    return messages

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

