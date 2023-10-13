import pygame
import socket
from enum import Enum, auto
from constants import *


# define custom events that could happen 
class GameEvent(Enum):
    NULL_EVENT = auto()
    LOGIN_SUCCESS = auto()
    LOGIN_FAIL = auto()
    REGISTER_SUCCESS = auto()
    REGISTER_FAIL = auto()
    FETCH_ROOM_INFO = auto()
    CREATE_ROOM_SUCCESS = auto()
    CREATE_ROOM_FAIL = auto()
    ENTER_ROOM_SUCCESS = auto()
    ENTER_ROOM_FAIL = auto()
    ROOM_SELECT_REPLY = auto()
    ROOM_MOVE_SUCCESS = auto()
    ROOM_MOVE_FAIL = auto()
    ROOM_TURN_CHANGE = auto()
    ROOM_PLAYER_INFO = auto()
    ROOM_GAME_FINISHED = auto()
    ROOM_GAME_TIME_CONTINUE = auto()
    

# Manages Sockets for connecting with the server
class NetworkPygame():
    def __init__(self, HOST, PORT, GAME_EVENT):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((HOST, PORT))

        self.sock.setblocking(False)
        self.data = b''
        self.GAME_EVENT = GAME_EVENT

    def __del__(self):
        self.sock.close()

    # sends info to socket
    def sendall(self, data):
        bytetext = str.encode(data).ljust(MAXLEN, b'\0')
        self.sock.sendall(bytetext)
    
    # has to call this every loop. puts socket info into pygame events!
    def process_network(self):
        try:
            self.data += self.sock.recv(MAXLEN)
        except socket.error as e:
            return

        print(self.data)
        if len(self.data) >= MAXLEN:
            got_data = self.data[:MAXLEN]
            self.data = self.data[MAXLEN:]
            self.post_event_based_on_data(got_data)

    def post_event_based_on_data(self, got_data):
        msg = got_data.decode().split('\n')
        print(msg)

        if msg[0] == 'LOG':
            if msg[1] == 'SUC':
                new_event = pygame.event.Event(self.GAME_EVENT, 
                                               {'utype': GameEvent.LOGIN_SUCCESS})
                pygame.event.post(new_event)  
            elif msg[1] == 'FAL':
                new_event = pygame.event.Event(self.GAME_EVENT, 
                                               {'utype': GameEvent.LOGIN_FAIL})
                pygame.event.post(new_event)  
        elif msg[0] == 'REG':
            if msg[1] == 'SUC':
                new_event = pygame.event.Event(self.GAME_EVENT, {'utype': GameEvent.REGISTER_SUCCESS})
                pygame.event.post(new_event)
            elif msg[1] == 'FAL':
                new_event = pygame.event.Event(self.GAME_EVENT, {'utype': GameEvent.REGISTER_SUCCESS})
                pygame.event.post(new_event)
        elif msg[0] == 'FET':
            arr = []
            for i in range(1, len(msg)-1):
                room = msg[i].split('\\')
                d = {}
                d['room_id'] = room[0]
                d['room_name'] = room[1]
                d['max_user_count'] = room[2]
                d['cur_user_count'] = room[3]
                d['time'] = room[4]
                arr.append(d)
            new_event = pygame.event.Event(self.GAME_EVENT, 
                                           {'utype': GameEvent.FETCH_ROOM_INFO,
                                            'data': arr})
            pygame.event.post(new_event)  
        
        elif msg[0] == 'CRE':
            if msg[1] == "SUC":
                room_id = int(msg[2])
                new_event = pygame.event.Event(self.GAME_EVENT, {'utype': GameEvent.CREATE_ROOM_SUCCESS,
                                                                  'room_id': room_id })
                pygame.event.post(new_event)  

        elif msg[0] == 'ENT':
            if msg[1] == 'SUC':
                room_id = int(msg[2])
                new_event = pygame.event.Event(self.GAME_EVENT, {'utype': GameEvent.ENTER_ROOM_SUCCESS,
                                                                  'room_id': room_id })
                pygame.event.post(new_event)  
        
        elif msg[0] == 'ROO':
            if msg[1] == 'SEL':
                turn = int(msg[2])
                moveable = []
                for i in range(0, len(msg[3]), 2):
                    moveable.append(msg[3][i:i+2])
                new_event = pygame.event.Event(self.GAME_EVENT, {'utype': GameEvent.ROOM_SELECT_REPLY,
                                                                  'turn': turn,
                                                                  'moveable': moveable })
                pygame.event.post(new_event)

            elif msg[1] == 'MOV':
                turn = int(msg[2])
                if msg[3] == 'SUC':
                    new_event = pygame.event.Event(self.GAME_EVENT, {'utype': GameEvent.ROOM_MOVE_SUCCESS,
                                                                     'turn': turn })
                    pygame.event.post(new_event)

            elif msg[1] == 'TUR':
                turn = int(msg[2])
                board_str = msg[3]

                new_event = pygame.event.Event(self.GAME_EVENT, {'utype': GameEvent.ROOM_TURN_CHANGE,
                                                                 'turn': turn,
                                                                 'board_str': board_str })
                pygame.event.post(new_event)

            elif msg[1] == 'INF':
                client_id = int(msg[2])
                p1_username = msg[3]
                p2_username = msg[4]
                new_event = pygame.event.Event(self.GAME_EVENT, {'utype': GameEvent.ROOM_PLAYER_INFO,
                                                                 'client_id': client_id,
                                                                 'p1_username': p1_username,
                                                                 'p2_username': p2_username})
                pygame.event.post(new_event)
            elif msg[1] == 'FIN':
                new_event = pygame.event.Event(self.GAME_EVENT, {'utype': GameEvent.ROOM_GAME_FINISHED,
                                                                 'win_player': msg[2]})
                pygame.event.post(new_event)

            elif msg[1] == 'RES':
                p1_time = int(msg[2])
                p2_time = int(msg[3])
                new_event = pygame.event.Event(self.GAME_EVENT, {'utype': GameEvent.ROOM_GAME_TIME_CONTINUE,
                                                                 'p1_time': p1_time,
                                                                 'p2_time': p2_time })
                pygame.event.post(new_event)


