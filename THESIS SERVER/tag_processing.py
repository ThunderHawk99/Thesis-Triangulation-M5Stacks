from shapely.geometry import Point
from shapely.ops import unary_union
import numpy as np
from config import Config
from utils import are_centroids_within_box, are_tags_in_order, determine_reorder_steps, get_box_coordinates, determine_move_steps

def intersect_multiple_circles(circles):
    if not circles:
        return None
    intersection_polygon = circles[0]
    for circle in circles[1:]:
        intersection_polygon = intersection_polygon.intersection(circle)
        if intersection_polygon.is_empty:
            return None
    return intersection_polygon

def compute_plot_data():
    anchors = Config.ANCHORS
    tag_distances = Config.TAG_DISTANCES
    error_margin = Config.ERROR_MARGIN
    
    data = {'tags': [], 'anchors': anchors, 'box': get_box_coordinates()}
    all_centroids = []

    for tag_index, distances in enumerate(tag_distances):
        circles = []
        tag_data = {'color': f'rgb({np.random.randint(255)}, {np.random.randint(255)}, {np.random.randint(255)})', 'overlaps': [], 'centroids': []}
        
        for (x, y), distance in zip(anchors, distances):
            circle = Point(x, y).buffer(distance + error_margin)
            circles.append(circle)
        
        common_overlap = intersect_multiple_circles(circles)
        if common_overlap and not common_overlap.is_empty:
            x, y = common_overlap.exterior.xy
            tag_data['overlaps'].append({'x': x.tolist(), 'y': y.tolist()})
        
        if common_overlap and not common_overlap.is_empty:
            centroid = common_overlap.centroid
            centroid_data = {'x': centroid.x, 'y': centroid.y}
            tag_data['centroids'].append(centroid_data)
            all_centroids.append(centroid_data)
        
        data['tags'].append(tag_data)
    if are_centroids_within_box(all_centroids):
        if are_tags_in_order(all_centroids):
            data['status'] = 'NEXT TO EACH OTHER AND IN ORDER'

        else:
            reorder_steps = determine_reorder_steps(all_centroids)
            data['status'] = 'NEXT TO EACH OTHER BUT NOT IN ORDER'
            data['reorder_steps'] = reorder_steps
    else:
        # Compute target box and movement steps for not next to each other scenario
        data['status'] = 'NOT NEXT TO EACH OTHER'
        move_steps = determine_move_steps(all_centroids)
        data['move_steps'] = move_steps

    
    return data

def update_tag_distances(content):
    tag_distances = Config.TAG_DISTANCES
    tag_id = content.get('tagID')
    distances = [
        content.get('distance1'),
        content.get('distance2'),
        content.get('distance3'),
        content.get('distance4')
    ]
    
    if tag_id is not None and 0 <= tag_id < len(tag_distances):
        tag_distances[tag_id] = distances
        return {'status': 'success', 'message': 'Distances updated successfully.'}
    else:
        return {'status': 'error', 'message': 'Invalid tagID.'}
