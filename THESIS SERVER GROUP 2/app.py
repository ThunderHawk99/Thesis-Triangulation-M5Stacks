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
                if 'tag_2_placement' in response_data and response_data['tag_2_placement'] != None and response_data['tag_2_placement'] != "":
                    if(movement_messages in response_data):
                        response_data['movement_messages'].append(response_data['tag_2_placement'])
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
                    if 'movement_messages_box' in response_data:
                        print('movement_messages_box')
                        directions = response_data['movement_messages_box']
                        # For every movement message box, broadcast it to the clients
                        for message in directions:
                            await broadcast({'type': 'movement_message_box', 'message': message['message'], 'tag_id': message['index']})
                    if response_data is not None and 'tag_2_placement' in response_data and response_data['tag_2_placement'] is not None:
                        print('tag_2_placement')
                        message = response_data['tag_2_placement']
                        print(f"Broadcasting message: {message['message']} to tag {message['index']}")
                        if message is not None and message != "":
                            print(f"Broadcasting message: {message['message']} to tag {message['index']}")
                        await broadcast({'type': 'tag_2_placement', 'message': message['message'], 'tag_id': message['index']})
                    if 'movement_messages' in response_data:
                        print('movement_messages')
                        movement_messages = response_data['movement_messages']
                        if movement_messages:
                            # For every movement message, broadcast it to the clients
                            for message in movement_messages:
                                print(f"Broadcasting message: {message['message']} to tag {message['index']}")  
                                await broadcast({'type': 'movement_message', 'message': message['message'], 'tag_id': message['index']})
                    else:
                        print('No movement messages')
                        if(movement_queue.__len__() == 0):
                            await broadcast({'type': 'direction', 'direction': 'inplace', 'tag_id': -1})
                if(movement_queue.__len__() > 0 and current_tag is None):
                    print('Processing queue')
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
