from shapely.geometry import Point
from shapely.ops import unary_union
import numpy as np
from config import Config
from utils import are_tags_in_order, determine_reorder_steps, get_box_coordinates, is_tag_2_centered, suggest_tag_2_placement

def intersect_multiple_circles(circles):
    if not circles:
        return None
    intersection_polygon = circles[0]
    for circle in circles[1:]:
        intersection_polygon = intersection_polygon.intersection(circle)
        if intersection_polygon.is_empty:
            return None
    return intersection_polygon

def compute_plot_data(box_boundaries):
    anchors = Config.ANCHORS
    tag_distances = Config.TAG_DISTANCES
    error_margin = Config.ERROR_MARGIN
    
    data = {'tags': [], 'anchors': anchors, 'box': get_box_coordinates(box_boundaries)}
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

    if not is_tag_2_centered(all_centroids, box_boundaries):
        message = suggest_tag_2_placement(all_centroids, box_boundaries)
        data['tag_2_placement'] = message


    if are_tags_in_order(all_centroids):
        if is_tag_2_centered(all_centroids, box_boundaries):
            data['status'] = 'NEXT TO EACH OTHER AND IN ORDER'
    else:
        data['status'] = 'NOT NEXT TO EACH OTHER AND NOT IN BOX'
        messages = determine_reorder_steps(all_centroids)
        data['movement_messages'] = messages
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
    
    if tag_id is not None and 1 <= tag_id < len(tag_distances) + 1:
        tag_distances[tag_id - 1] = distances
        return {'status': 'success', 'message': 'Distances updated successfully.'}
    else:
        return {'status': 'error', 'message': 'Invalid tagID.'}
