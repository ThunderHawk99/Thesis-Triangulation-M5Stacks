import asyncio
import websockets
import json
from tag_processing import compute_plot_data, update_tag_distances
from utils import calculate_box_boundaries
from collections import deque
import logging
logging.basicConfig(level=logging.INFO)

connected_clients = set()
movement_queue = deque()
current_tag = None
solved = False
box_boundaries = calculate_box_boundaries(40, 20, 100, 85)
async def broadcast(message):
    if connected_clients:
        message_data = json.dumps(message)
        # Create tasks for each send operation
        send_tasks = [asyncio.create_task(client.send(message_data)) for client in connected_clients]
        # Wait for all tasks to complete
        await asyncio.gather(*send_tasks)

async def process_queue():
    global current_tag  # Ensure we use the global variable
    if current_tag is None and movement_queue:
        current_tag = movement_queue.popleft()
        print(f'Tag {current_tag} is now allowed to move.')
        await broadcast({'type': 'do_move', 'tag_id': current_tag})
        return  # Exit after processing the queue

async def handle_client(websocket, path):
    global current_tag, box_boundaries  # Ensure we use the global variable
    # Register client
    connected_clients.add(websocket)
    try:
        # Notify when a new client connects
        print('A new client has connected.')
        print(f'Total connected clients: {len(connected_clients)}')
        
        async for message in websocket:
            data = json.loads(message)

            if data['type'] == 'start_new_game' and solved:
                solved = False
                print("Received start_new_game")
                box_boundaries = calculate_box_boundaries(40, 20, 100, 85)
                await broadcast({'type': 'new_game_started'})

            if data['type'] == 'allow_move_confirmed':
                print("Received allow_move_confirmed")
                await process_queue()
            
            if data['type'] == 'request_data':
                response_data = compute_plot_data(box_boundaries)
                await websocket.send(json.dumps({'type': 'response_data', 'data': response_data}))
            
            if data['type'] == 'update_measurements':
                result = update_tag_distances(data['data'])
                # await websocket.send(json.dumps({'type': 'update_result', 'result': result}))

            if data['type'] == 'request_move':
                tag_id = data['tag_id']
                # Check if the tag is already in the queue
                if tag_id not in movement_queue:
                    movement_queue.append(tag_id)
                print(f'Tag {tag_id} requested to move. Queue: {list(movement_queue)}')
                if(movement_queue.__len__() == 1 and current_tag is None):
                    tag = movement_queue[0]
                    print(f'Tag {tag} is now allowed to move.')
                    await broadcast({'type': 'allow_move', 'tag_id': tag})

            if data['type'] == 'movement_done':
                if data['tag_id'] == current_tag:
                    print(f'Tag {current_tag} finished moving.')
                    current_tag = None
                    response_data = compute_plot_data(box_boundaries)
                    # Check if response_data['move_steps'] exists
                    if 'status' in response_data:
                        if response_data['status'] == 'NEXT TO EACH OTHER AND IN ORDER' and movement_queue.__len__() == 0:
                            solved = True
                            await broadcast({'type': 'direction', 'direction': 'finish_game', 'tag_id': 1})
                    if 'move_steps' in response_data:
                        directions = response_data['move_steps']
                        direction = [item for item in directions if item['index'] == data['tag_id']]
                        if(direction.__len__() == 1):
                            # Remove ' and ' from the string
                            directionForTag = direction[0]["direction"].replace(' and ', '')
                            print(directionForTag)
                            await broadcast({'type': 'direction', 'direction': directionForTag, 'tag_id': data['tag_id']})
                    if 'reorder_steps' in response_data:
                        reorder_steps = response_data['reorder_steps']
                        if reorder_steps:
                            # next_step = (0, 2) example
                            next_step = reorder_steps.pop(0)
                            first_tag = next_step[0]
                            second_tag = next_step[1]
                            first_tag_x = response_data['tags'][first_tag]['centroids'][0]['x']
                            second_tag_x = response_data['tags'][second_tag]['centroids'][0]['x']
                            if(movement_queue.__len__() == 0):
                                if(first_tag_x < second_tag_x):
                                    await broadcast({'type': 'swap', 'tag_id': (first_tag + 1), 'direction': 'East'})
                                    await broadcast({'type': 'swap', 'tag_id': (second_tag + 1), 'direction': 'West'})
                                else:
                                    await broadcast({'type': 'swap', 'tag_id': (first_tag + 1), 'direction': 'West'})
                                    await broadcast({'type': 'swap', 'tag_id': (second_tag + 1), 'direction': 'East'})
                            # await websocket.send(json.dumps({'type': 'reorder', 'steps': reorder_steps, 'next_step': next_step}))
                    else:
                        if(movement_queue.__len__() == 0):
                            await broadcast({'type': 'direction', 'direction': 'inplace', 'tag_id': -1})
                if(movement_queue.__len__() > 0 and current_tag is None):
                    tag = movement_queue[0]
                    await broadcast({'type': 'allow_move', 'tag_id': tag})
        
    except websockets.ConnectionClosed:
        print('A client has disconnected.')
    except OSError as e:
        if e.errno == 121:  # WinError 121 specific handling
            logging.warning(f"Ignored OSError: ")
        else:
            logging.error(f"Unexpected OSError:")
    except Exception as e:
        logging.error(f"Unexpected error: {e}")
    finally:
        connected_clients.remove(websocket)

async def main():
    # Start the WebSocket server
    server = await websockets.serve(handle_client, "192.168.178.182", 5000, ping_timeout = None)

    await server.wait_closed()

# Run the WebSocket server
if __name__ == "__main__":
    asyncio.run(main())
