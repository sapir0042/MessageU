import struct
import uuid
from database import db
SERVER_VERSION = 2
CLIENT_VERSION = 1

CODE_REQUESTS = {'Registration': 100, 'Users list': 101, 'Retrieval client\'s public key': 102,
                 'Sending a message to the client': 103, 'Retrieving waiting messages': 104}
MESSAGE_TYPE = {'Symmetric key request': 1, 'Sending a symmetric key': 2, 'Send a text message': 3, 'Send a file': 4}
CODE_RESPONSE = {'Registration was successful': 1000, 'Users list': 1001, 'Public key': 1002,
                 'Client message sent': 1003, 'Retrieving waiting messages': 1004, 'General error': 9000}


def handle_request(sock):
    # Receives header
    data_header = sock.recv(struct.calcsize('<16s B B I'))
    if not data_header:
        return
    client_id, srv_version, request_code, payload_size = decode_request(data_header)
    # Receives payload
    data_payload = b''
    for i in range(payload_size):
        data_payload += sock.recv(1)

    if srv_version != CLIENT_VERSION:
        error_response(sock)
        return
    elif request_code == CODE_REQUESTS['Registration']:
        handle_registration(sock, data_payload)
        return
    elif request_code == CODE_REQUESTS['Users list']:
        send_users_list(sock, client_id)
    elif request_code == CODE_REQUESTS['Retrieval client\'s public key']:
        send_public_key(sock, data_payload)
    elif request_code == CODE_REQUESTS['Sending a message to the client']:
        send_message_to_client(sock, data_payload, client_id)
    elif request_code == CODE_REQUESTS['Retrieving waiting messages']:
        retrieving_waiting_messages(sock, client_id)
    else:
        error_response(sock)
    db.update_time_for_new_request(client_id)


def handle_registration(sock, data_payload):
    payload_request = struct.unpack('<255s 160s', data_payload)
    name = (payload_request[0].split(b'\x00'))[0]
    public_key = payload_request[1]
    if db.name_exists_in_the_table(name):
        error_response(sock)
    else:
        gen_uuid = uuid.uuid4()
        db.insert_new_client_to_the_table(gen_uuid, name, public_key)

        # send response - header and payload
        sock.sendall(struct.pack('<B H L', SERVER_VERSION, CODE_RESPONSE['Registration was successful'], 16))
        sock.sendall(struct.pack('<16s', gen_uuid.bytes))


def send_users_list(sock, client_id):
    users_list = db.get_users_list(client_id)
    users_count = len(users_list)
    # send response - header and payload
    sock.sendall(struct.pack('<B H L', SERVER_VERSION, CODE_RESPONSE['Users list'], users_count*(16+255)))
    for u in users_list:
        buffer = u[0]  # user id
        buffer += u[1]  # user name
        sock.sendall(buffer)


def send_public_key(sock, data_payload):
    client_id = struct.unpack('<16s', data_payload)
    public_key = db.get_public_key(client_id[0])
    # send response - header and payload
    sock.sendall(struct.pack('<B H L', SERVER_VERSION, CODE_RESPONSE['Public key'], 16+160))
    sock.sendall(struct.pack('<16s 160s', client_id[0], public_key))


def send_message_to_client(sock, data_payload, from_client_id):
    payload_request = struct.unpack_from('<16s B I', data_payload, 0)
    client_id = payload_request[0]
    message_type = payload_request[1]
    content_size = payload_request[2]
    print(client_id, message_type, content_size)
    msg_len = '<' + str(content_size) + 's'
    message_content = struct.unpack_from(msg_len, data_payload, 21)
    message_id = db.insert_new_message_to_the_table(client_id, from_client_id, message_type, message_content[0])
    # send response - header and payload
    sock.sendall(struct.pack('<B H L', SERVER_VERSION, CODE_RESPONSE['Client message sent'], 16+4))
    sock.sendall(struct.pack('<16s L', client_id, message_id))


def retrieving_waiting_messages(sock, client_id):
    msg_list = db.get_messages(client_id)
    if not msg_list:
        sock.sendall(struct.pack('<B H L', SERVER_VERSION, CODE_RESPONSE['Retrieving waiting messages'], 0))
        return
    buffer = b''
    for m in msg_list:
        buffer += encode_message(m)
    payload_size = len(buffer)
    # send response - header and payload
    sock.sendall(struct.pack('<B H L', SERVER_VERSION, CODE_RESPONSE['Retrieving waiting messages'], payload_size))
    buffer_len = '<' + str(payload_size) + 's'
    sock.sendall(struct.pack(buffer_len, buffer))
    db.delete_messages_waiting_to_client(client_id)


def decode_request(data_header):
    header_request = struct.unpack('<16s B B I', data_header)
    client_id = header_request[0]
    srv_version = header_request[1]
    request_code = header_request[2]
    payload_size = header_request[3]
    return client_id, srv_version, request_code, payload_size


def error_response(sock):
    sock.sendall(struct.pack('<B H L', SERVER_VERSION, CODE_RESPONSE['General error'], 0))


def encode_message(data_msg):
    buffer = struct.pack('<16s I B I', data_msg[0], int(data_msg[1]), int(data_msg[2]), len(data_msg[3]))
    msg_len = '<' + str(len(data_msg[3])) + 's'
    buffer += struct.pack(msg_len, data_msg[3])
    return buffer
