import asyncio
import websockets
import json
from tag_processing import compute_plot_data, update_tag_distances
from collections import deque

connected_clients = set()
movement_queue = deque()
current_tag = None

async def broadcast(message):
    if connected_clients:
        message_data = json.dumps(message)
        # Create tasks for each send operation
        send_tasks = [asyncio.create_task(client.send(message_data)) for client in connected_clients]
        # Wait for all tasks to complete
        await asyncio.gather(*send_tasks)

async def process_queue():
    global current_tag
    if current_tag is None and movement_queue:
        current_tag = movement_queue.popleft()
        print(f'Tag {current_tag} is now allowed to move.')
        await broadcast({'type': 'allow_move', 'tag_id': current_tag})


async def handle_client(websocket, path):
    global current_tag  # Ensure we use the global variable
    # Register client
    connected_clients.add(websocket)
    try:
        # Notify when a new client connects
        print('A new client has connected.')
        print(f'Total connected clients: {len(connected_clients)}')
        
        async for message in websocket:
            data = json.loads(message)
            
            if data['type'] == 'request_data':
                response_data = compute_plot_data()
                await websocket.send(json.dumps({'type': 'response_data', 'data': response_data}))
            
            elif data['type'] == 'update_measurements':
                result = update_tag_distances(data['data'])
                # await websocket.send(json.dumps({'type': 'update_result', 'result': result}))
            
            elif data['type'] == 'test':
                print('Test event received.')
                await websocket.send(json.dumps({'type': 'test', 'message': 'Test event acknowledged.'}))

            if data['type'] == 'request_move':
                tag_id = data['tag_id']
                movement_queue.append(tag_id)
                print(f'Tag {tag_id} requested to move. Queue: {list(movement_queue)}')
                if(movement_queue.__len__() == 1):
                    await process_queue()

            elif data['type'] == 'movement_done':
                if data['tag_id'] == current_tag:
                    print(f'Tag {current_tag} finished moving.')
                    current_tag = None
                    response_data = compute_plot_data()
                    directions = response_data['move_steps']
                    direction = [item for item in directions if item['index'] == data['tag_id']]
                    if(direction.__len__() == 1):
                        await websocket.send(json.dumps({'type': 'direction', 'direction': direction[0]["direction"]}))
                    await process_queue()
        
    except websockets.ConnectionClosed:
        print('A client has disconnected.')
    finally:
        # Unregister client
        connected_clients.remove(websocket)

async def main():
    # Start the WebSocket server
    async with websockets.serve(handle_client, "192.168.178.182", 5000):
        await asyncio.Future()  # Run forever

# Run the WebSocket server
if __name__ == "__main__":
    asyncio.run(main())
