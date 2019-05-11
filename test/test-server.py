import msgpack
import socket

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind(('localhost', 8080))
server.listen(1)

aggregation = {}

# print("Listening")
while True:
    client, address = server.accept()
    # print("Accepted client connection")
    unpacker = msgpack.Unpacker()

    while True:
        unpacker.feed(client.recv(1))

        for value in unpacker:
            value = dict(value)
            # print("Got message: ", value)
            key = value[b'id']
            if key in aggregation:
            	values = aggregation[key]
            	values.append(value[b'value'])
            	if len(values) == 3:
            		print("id:", str(key) + "," , "values:", \
        		 	str(values[0]) + ",", str(values[1]) + ",", values[2])
            		del aggregation[key]
            else:
            	aggregation[key] = [value[b'value']]

    # print("Client connection closed.")
