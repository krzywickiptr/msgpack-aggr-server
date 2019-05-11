import argparse
import random
import socket
import time

import msgpack

__description__ = 'Generator danych dla zadania rekrutacyjnego z C.'


MSG_ID_SCALER = 4000000
MSG_INT_LO = 0.01
MSG_INT_HI = 0.035

def make_message(id_, value):
    body = {'id': id_, 'value': value}
    return msgpack.packb(body, use_bin_type=True)


def main():
    parser = argparse.ArgumentParser(description=__description__)
    parser.add_argument('--host', default='127.0.0.1', help='adres IP serwera')
    parser.add_argument('--port', default=8080, type=int, help='port serwera')
    args = parser.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    print('connecting to %s:%d' % (args.host, args.port))
    sock.connect((args.host, args.port))

    print('starting stream')
    try:
        while True:
            msg_id = round(time.time_ns() // MSG_ID_SCALER, -3) + random.randint(1, 5)
            msg_value = random.randint(0, 1000) + random.randint(0, 6) * 1000000000 
            msg = make_message(msg_id, msg_value)
            sock.send(msg)
            time.sleep(random.uniform(MSG_INT_LO, MSG_INT_HI))
    except KeyboardInterrupt:
        exit()


if __name__ == '__main__':
    main()
