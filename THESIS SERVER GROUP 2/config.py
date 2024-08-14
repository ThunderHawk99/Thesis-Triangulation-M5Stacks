class Config:
    SOLVED = False
    ERROR_MARGIN = 0
    BOX_SIZE = (40, 10)
    ANCHORS = [(0, 86), (100, 86), (100, 0), (0, 0)]
    TAG_DISTANCES = [
        [114, 108, 67, 61],  # Distances for Tag 1
        [59, 66, 104, 95],   # Distances for Tag 2
        [80,104,102,56.99999999999999]     # Distances for Tag 3
    ]
    
