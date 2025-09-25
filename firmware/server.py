import asyncio
import websockets
from zeroconf import ServiceInfo, Zeroconf

PORT = 8080
WS_PATH = "/ws"

clients = {}

async def handler(websocket, path):
    print(f"Client connected: {websocket.remote_address}")
    try:
        async for message in websocket:
            print(f"Received: {message}")
            if websocket not in clients:
                device_id = message.strip()
                clients[websocket] = device_id
                await websocket.send(f"OK:{device_id}")
                print(f"Handshake done with {device_id}")
            else:
                # Здесь можно обрабатывать команды от ESP
                print(f"Message from {clients[websocket]}: {message}")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        print("Client disconnected")
        if websocket in clients:
            del clients[websocket]

async def main():
    async with websockets.serve(handler, "0.0.0.0", PORT):
        print(f"WebSocket server started on ws://0.0.0.0:{PORT}{WS_PATH}")
        await asyncio.Future()  # run forever

if __name__ == "__main__":
    # Регистрируем сервис в mDNS
    desc = {'path': WS_PATH}
    info = ServiceInfo(
        "_command._tcp.local.",
        "MyCommandServer._command._tcp.local.",
        addresses=[b"\x00\x00\x00\x00"],  # 0.0.0.0
        port=PORT,
        properties=desc,
        server="myserver.local."
    )

    zeroconf = Zeroconf()
    zeroconf.register_service(info)
    print("mDNS service registered (_command._tcp)")

    try:
        asyncio.run(main())
    finally:
        zeroconf.unregister_service(info)
        zeroconf.close()
