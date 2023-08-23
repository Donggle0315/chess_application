import pygame
import pygame_gui
import socket
from pygame import Rect
from enum import Enum, auto

ADDR = '52.78.117.250:53938'
HOST, PORT = ADDR.split(':')
PORT = int(PORT)
MAXLEN = 2048


class ChessSprite():
    def __init__(self, color, x, y, width, height, chess_sprites, board_coord):
        super(ChessSprite, self).__init__()
        self.color = color
        self.rect = Rect(x, y, width, height)
        self.chess_sprites = chess_sprites
        self.surface = pygame.Surface((width, height))
        self.surface.fill(self.color)
        self.board_coord = board_coord
        self.moveable = False

    def draw(self, window, num):
        self.surface.fill(self.color)
        if num in self.chess_sprites:
            self.surface.blit(self.chess_sprites[num], (0, 0, self.rect.width,self.rect.height))
        if self.moveable:
            self.surface.blit(self.chess_sprites[99], (0, 0, self.rect.width,self.rect.height))
        window.blit(self.surface, self.rect)

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
                p1_time = int(msg[3])
                p2_time = int(msg[4])
                board_str = msg[5]

                new_event = pygame.event.Event(self.GAME_EVENT, {'utype': GameEvent.ROOM_TURN_CHANGE,
                                                                 'turn': turn,
                                                                 'p1_time': p1_time,
                                                                 'p2_time': p2_time,
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
                new_event = pygame.event.Event(self.GAME_EVENT, {'utype': GameEvent.ROOM_GAME_FINISHED})

class LoginScreen():
    def __init__(self, window: pygame.Surface, sock: NetworkPygame, clock: pygame.time.Clock, GAME_EVENT):
        # login screen
        self.manager = pygame_gui.UIManager((1280, 720))
        self.login_button = pygame_gui.elements.UIButton(relative_rect=Rect(200, 0, 200, 120), 
                                                    text='login', manager=self.manager,
                                                    anchors={'center': 'center'})
        self.id_text_box = pygame_gui.elements.UITextEntryLine(relative_rect=Rect(-50, -30, 200, 50), manager=self.manager,
                                                        anchors={'center': 'center'})
        self.pw_text_box = pygame_gui.elements.UITextEntryLine(relative_rect=Rect(-50, 30, 200, 50), manager=self.manager,
                                                        anchors={'center': 'center'})
        self.alert_label = pygame_gui.elements.UILabel(text='', relative_rect=Rect(-50, 80, 200, 50), manager=self.manager,
                                                        anchors={'center': 'center'})
        self.reg_bt = pygame_gui.elements.UIButton(relative_rect=Rect(200, 100, 100, 40),
                                                   text='register', manager=self.manager,
                                                   anchors={'center': 'center'})
        self.background = pygame.Surface((1280, 720))
        self.background.fill('#FFFFFF')



        self.register_window = pygame_gui.elements.UIWindow(rect=Rect(300, 100, 700, 500), 
                                                        window_display_title='Register',
                                                        manager=self.manager)

        self.register_window.on_close_window_button_pressed = self.register_window.hide
        self.register_window.hide()
        self.register_id_label = pygame_gui.elements.UILabel(text='ID: ', relative_rect=Rect(50, 50, 150, 50),
                                                        manager=self.manager, container=self.register_window)
        self.register_id_text = pygame_gui.elements.UITextEntryLine(relative_rect=Rect(200, 50, 400, 50), manager=self.manager,
                                                                container=self.register_window)

        self.register_pw_label = pygame_gui.elements.UILabel(text='PW: ', relative_rect=Rect(50, 120, 150, 50),
                                                    manager=self.manager, container=self.register_window)

        self.register_pw_text = pygame_gui.elements.UITextEntryLine(relative_rect=Rect(200, 120, 400, 50), manager=self.manager,
                                                                container=self.register_window)

        self.user_name_label = pygame_gui.elements.UILabel(text='Username: ', relative_rect=Rect(50, 190, 150, 50),
                                                    manager=self.manager, container=self.register_window)

        self.user_name_text = pygame_gui.elements.UITextEntryLine(relative_rect=Rect(200, 190, 400, 50), manager=self.manager,
                                                                container=self.register_window)
        self.reg_alert_label = pygame_gui.elements.UILabel(text='', relative_rect=Rect(200, 260, 400, 50), manager=self.manager,
                                                           container=self.register_window)
        self.final_reg_bt_rect = Rect(0, 0, 200, 50)
        self.final_reg_bt_rect.bottomright = (-50, -50)
        self.final_reg_bt = pygame_gui.elements.UIButton(relative_rect=self.final_reg_bt_rect,
                                                            text='Register',
                                                            manager=self.manager,
                                                            container=self.register_window,
                                                            anchors={'right': 'right',
                                                                    'bottom': 'bottom'})
        # socket to connect to the main server
        self.sock = sock
        self.running = True
        self.next_window = ''
        self.clock = clock
        self.window = window
        self.GAME_EVENT = GAME_EVENT

    def run(self):
        # run the app
        while self.running:
            delta = self.clock.tick(120)/1000
            self.sock.process_network()
            self.handle_events()
            self.manager.update(delta)
            self.render()
        return self.next_window, True
    
    def handle_events(self):
        # handle events in login screen
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                quit()

            elif event.type == pygame_gui.UI_BUTTON_PRESSED:
                if event.ui_element == self.login_button:
                    print('sending login information')
                    id_text = self.id_text_box.get_text()
                    pw_text = self.pw_text_box.get_text()
                    sendtext = f'LOG\n{id_text}\n{pw_text}\n'
                    self.sock.sendall(sendtext)
                elif event.ui_element == self.final_reg_bt:
                    id_text = self.register_id_text.get_text()
                    pw_text = self.register_pw_text.get_text()
                    username_text = self.user_name_text.get_text()
                    sendtext = f'REG\n{id_text}\n{pw_text}\n{username_text}\n'
                    self.sock.sendall(sendtext)
                elif event.ui_element == self.reg_bt:
                    print('regwindow')
                    self.register_window.show()
            elif event.type == self.GAME_EVENT:
                if hasattr(event, 'utype'):
                    if event.utype == GameEvent.LOGIN_SUCCESS:
                        self.running = False
                        self.next_window = 'lobby'
                        return
                    elif event.utype == GameEvent.LOGIN_FAIL:
                        self.alert_label.set_text("Login Failed")
                    elif event.utype == GameEvent.REGISTER_SUCCESS:
                        self.register_window.hide()
                        self.alert_label.set_text('Register Success!')
                    elif event.utype == GameEvent.REGISTER_FAIL:
                        self.reg_alert_label.set_text("Register Fail")
                    
            self.manager.process_events(event)

    def render(self):
        # render objects in login screen
        self.window.blit(self.background, (0, 0))
        self.manager.draw_ui(self.window)
        pygame.display.flip()
    
    def quit(self):
        # close the window, close the sockets, exit program
        pygame.quit()
        exit()


class LobbyScreen():
    def __init__(self, window: pygame.Surface, sock: NetworkPygame, clock: pygame.time.Clock, GAME_EVENT):
        # lobby screen
        self.manager = pygame_gui.UIManager((1280, 720))

        self.background = pygame.Surface((1280, 720))
        self.background.fill('#EEEEEE')

        self.lobby_refresh_bt_rect = Rect(0, 0, 50, 50)
        self.lobby_refresh_bt_rect.bottomright = (-100, -50)
        self.lobby_refresh_bt = pygame_gui.elements.UIButton(relative_rect=self.lobby_refresh_bt_rect,
                                                            text='refresh', manager=self.manager,
                                                            anchors={'right': 'right',
                                                                    'bottom': 'bottom'})

        self.create_room_bt_rect = Rect(0, 0, 50, 50)
        self.create_room_bt_rect.bottomright = (-200, -50)
        self.create_room_bt = pygame_gui.elements.UIButton(relative_rect=self.create_room_bt_rect,
                                                            text='create room', manager=self.manager,
                                                            anchors={'right': 'right',
                                                                    'bottom': 'bottom'})

        self.rooms_panel_display = pygame_gui.elements.UIPanel(Rect(30, 30, 900, 650),
                                                manager=self.manager)


        self.create_room_window_rect = Rect(300, 100, 700, 500)

        self.create_room_window_color = "#21282D"
        self.create_room_window = pygame_gui.elements.UIWindow(rect=self.create_room_window_rect, 
                                                        window_display_title='create room',
                                                        manager=self.manager)

        self.create_room_window.on_close_window_button_pressed = self.create_room_window.hide
        self.create_room_window.hide()
        self.create_room_window.layer = 100


        # create room window
        self.room_name_label = pygame_gui.elements.UILabel(text='Room Name', relative_rect=Rect(50, 50, 150, 50),
                                                        manager=self.manager, container=self.create_room_window)
        self.room_name_text_box = pygame_gui.elements.UITextEntryLine(relative_rect=Rect(200, 50, 400, 50), manager=self.manager,
                                                                container=self.create_room_window)

        self.max_user_label = pygame_gui.elements.UILabel(text='Max User', relative_rect=Rect(50, 120, 150, 50),
                                                    manager=self.manager, container=self.create_room_window)
        self.max_user_menu = pygame_gui.elements.UIDropDownMenu(options_list=['2', '3', '4'],
                                                            starting_option='2',
                                                            relative_rect=Rect(200, 120, 100, 50),
                                                            manager=self.manager, container=self.create_room_window)

        self.time_label = pygame_gui.elements.UILabel(text='Time (min)', relative_rect=Rect(50, 190, 150, 50),
                                                manager=self.manager, container=self.create_room_window)
        self.time_menu = pygame_gui.elements.UIDropDownMenu(options_list=['10', '20', '30'],
                                                            starting_option='10',
                                                            relative_rect=Rect(200, 190, 100, 50),
                                                            manager=self.manager, container=self.create_room_window)
        self.final_create_room_bt_rect = Rect(0, 0, 200, 50)
        self.final_create_room_bt_rect.bottomright = (-50, -50)
        self.final_create_room_bt = pygame_gui.elements.UIButton(relative_rect=self.final_create_room_bt_rect,
                                                            text='Create Room!',
                                                            manager=self.manager,
                                                            container=self.create_room_window,
                                                            anchors={'right': 'right',
                                                                    'bottom': 'bottom'})
        
        self.next_page_bt_rect = Rect(0, 0, 200, 50)
        self.next_page_bt_rect.topright = (-50, 120)
        self.prev_page_bt_rect = Rect(0, 0, 200, 50)
        self.prev_page_bt_rect.topright = (-50, 50)

        self.next_page_bt = pygame_gui.elements.UIButton(relative_rect=self.next_page_bt_rect,
                                                         text='next page',
                                                         manager=self.manager,
                                                         anchors={'right': 'right',
                                                                  'top': 'top'})
        self.prev_page_bt = pygame_gui.elements.UIButton(relative_rect=self.prev_page_bt_rect,
                                                         text='prev page',
                                                         manager=self.manager ,
                                                         anchors={'right': 'right',
                                                                  'top': 'top'})
        self.clock = clock
        self.window = window
        self.page = 0
        self.rooms = []
        self.rooms_panel = []
    
        self.sock = sock
        self.room_display_count = 6
        self.running = True
        self.next_window = ''
        self.room_id = -1
        self.make_rooms_panel()
        self.GAME_EVENT = GAME_EVENT
        
        self.creating_room = False

    def run(self):
        self.fetch_room()
        while self.running:
            delta = self.clock.tick(120)/1000

            self.sock.process_network()
            self.handle_events()
            
            self.manager.update(delta)
            self.render()

        return self.next_window, self.room_id
    
    def handle_events(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.quit()
            
            if event.type == pygame_gui.UI_BUTTON_PRESSED:
                if event.ui_element == self.lobby_refresh_bt:
                    print('refresh')
                    self.fetch_room()

                elif event.ui_element == self.create_room_bt:
                    print('create room')
                    self.create_room_window.show()

                elif event.ui_element == self.final_create_room_bt:
                    print('create room final')
                    rname = self.room_name_text_box.text
                    rmax_user = self.max_user_menu.selected_option
                    rtime = self.time_menu.selected_option

                    sendtext = f'CRE\n{rname}\n{rmax_user}\n{rtime}\n'
                    self.sock.sendall(sendtext)
                    
                    self.creating_room = True
                    # disable all gui later
                elif event.ui_element == self.prev_page_bt:
                    self.page -= 1
                    if self.page < 0:
                        self.page = 0
                    self.display_rooms_panel()
                elif event.ui_element == self.next_page_bt:
                    self.page += 1
                    self.display_rooms_panel()

                for i, (_, b, _) in enumerate(self.rooms_panel):
                    if event.ui_element == b:
                        cur_idx, _ = self.get_room_range()
                        cur_idx += i
                        room_id = self.rooms[cur_idx]['room_id']

                        self.try_enter_room(room_id)

            elif event.type == self.GAME_EVENT:
                if hasattr(event, 'utype'):
                    if event.utype == GameEvent.FETCH_ROOM_INFO:
                        self.rooms = event.data
                        self.display_rooms_panel()

                    elif event.utype == GameEvent.CREATE_ROOM_SUCCESS:
                        self.creating_room = False
                        self.try_enter_room(event.room_id)

                    elif event.utype == GameEvent.ENTER_ROOM_SUCCESS:
                        self.running = False
                        self.next_window = 'game'
                        self.room_id = event.room_id
                        return

                    elif event.utype == GameEvent.ENTER_ROOM_FAIL:
                        print('failed entering room')
                

            self.manager.process_events(event)

    def try_enter_room(self, room_id):
        sendtext = f'ENT\n{room_id}\n'
        self.sock.sendall(sendtext)
        

    def render(self):
        self.window.blit(self.background, (0, 0))
        self.manager.draw_ui(self.window)
        pygame.display.flip()

    def make_rooms_panel(self):
        self.rooms_panel = []
        for _ in range(self.room_display_count):
            
            new_panel = pygame_gui.elements.UIPanel(relative_rect=Rect(0, 0, 300, 200),
                                                    manager=self.manager,
                                                    container=self.rooms_panel_display)
            new_bt = pygame_gui.elements.UIButton(relative_rect=Rect(0, 150, 300, 50),
                                                text='enter', manager=self.manager,
                                                container=new_panel)
            text_box = pygame_gui.elements.UITextBox(html_text='',
                                        relative_rect=Rect(0, 0, 300, 150),
                                        manager=self.manager,
                                        container=new_panel)
            new_panel.hide()

            self.rooms_panel.append([new_panel, new_bt, text_box])

    def display_rooms_panel(self):
        cur_idx, last_idx = self.get_room_range()
        print(self.rooms, cur_idx, last_idx)
        xpos = 50
        ypos = 10
        positions = []
        for i in range(self.room_display_count):
            positions.append((xpos, ypos))
            xpos = (xpos + 400) % 800
            if i % 2 == 1:
                ypos += 200

        for p, _, _ in self.rooms_panel:
            p.hide()

        for i in range(cur_idx, last_idx):
            if i >= len(self.rooms):
                break
            cur_text_box = self.rooms_panel[i][2]
            cur_text_box.set_text('Name: {}\nUser: {}/{}\nTime: {}\n'.format(
                                                                self.rooms[i]['room_name'], 
                                                                self.rooms[i]['cur_user_count'],
                                                                self.rooms[i]['max_user_count'],
                                                                self.rooms[i]['time']))
            
            cur_panel = self.rooms_panel[i][0]
            cur_panel.set_relative_position((positions[i]))
            cur_panel.show()

    def get_room_range(self):
        cur_idx = (self.page) * self.room_display_count
        last_idx = cur_idx+self.room_display_count if cur_idx+self.room_display_count < len(self.rooms) else len(self.rooms)
        return cur_idx, last_idx

    # fetch room information
    def fetch_room(self):
        print('fet')
        sendtext = "FET\n"
        self.sock.sendall(sendtext)

    def quit(self):
        # close the window, close the sockets, exit program
        pygame.quit()
        exit()

class PlayerInfoPanel():
    def __init__(self, manager):
        self.manager = manager
        self.panel = pygame_gui.elements.UIPanel(relative_rect=Rect(0, 0, 300, 350),
                                                 manager=self.manager)
        self.username = pygame_gui.elements.UILabel(text='???',
                                                    relative_rect=Rect(10, 10, 200, 50),
                                                    manager=self.manager,
                                                    container=self.panel)

    def set_position(self, left, top):
        self.panel.set_relative_position((left, top))

    def set_username(self, username):
        self.username.set_text(username)

                                                    
        
class GameScreen():
    def __init__(self, window: pygame.Surface, sock: NetworkPygame, clock: pygame.time.Clock, room_id, GAME_EVENT):
        # room
        self.manager = pygame_gui.UIManager((1280, 720))
        self.background = pygame.Surface((1280, 720))
        self.background.fill('#EEEEEE')

        # load sprites
        self.chess_sprites = {}
        self.chess_sprites[21] = pygame.image.load('./img/rook_b.png')
        self.chess_sprites[22] = pygame.image.load('./img/knight_b.png')
        self.chess_sprites[23] = pygame.image.load('./img/bishop_b.png')
        self.chess_sprites[24] = pygame.image.load('./img/queen_b.png')
        self.chess_sprites[25] = pygame.image.load('./img/king_b.png')
        self.chess_sprites[26] = pygame.image.load('./img/pawn_b.png')
        self.chess_sprites[31] = pygame.image.load('./img/rook_w.png')
        self.chess_sprites[32] = pygame.image.load('./img/knight_w.png')
        self.chess_sprites[33] = pygame.image.load('./img/bishop_w.png')
        self.chess_sprites[34] = pygame.image.load('./img/queen_w.png')
        self.chess_sprites[35] = pygame.image.load('./img/king_w.png')
        self.chess_sprites[36] = pygame.image.load('./img/pawn_w.png')
        self.chess_sprites[99] = pygame.image.load('./img/checker.png')

        self.size = 60
        self.start_x = 400
        self.start_y = 60

        for k in self.chess_sprites:
            self.chess_sprites[k] = pygame.transform.scale(self.chess_sprites[k], (self.size, self.size))

        self.board = []
        self.board_gui = []
        for i in range(8):
            row = []
            row_gui = []
            for j in range(8):
                row.append(-1)
                color = '#7A9D54' if (i+j)%2 else '#EEEEEE'
                row_gui.append(ChessSprite(color, self.start_x+self.size*j, self.start_y+self.size*i, self.size, self.size, self.chess_sprites, (j,i)))
            self.board.append(row)
            self.board_gui.append(row_gui)

        # gui elements 
        self.start_game_bt_rect = Rect(500, 600, 200, 50)
        
        self.start_game_bt = pygame_gui.elements.UIButton(relative_rect=self.start_game_bt_rect, 
                                                          text='Start Game!',
                                                          manager=self.manager)

        self.p1_time_rect = Rect(0, 0, 200, 100)
        self.p2_time_rect = Rect(0, 0, 200, 100)
        self.p1_time_rect.topleft = (50, 100)
        self.p2_time_rect.topright = (1230, 100)

        self.p1_time_indicator = pygame_gui.elements.UILabel(text='99:99',
                                                             relative_rect=self.p1_time_rect,
                                                             manager=self.manager)
        self.p2_time_indicator = pygame_gui.elements.UILabel(text='99:99',
                                                             relative_rect=self.p2_time_rect,
                                                             manager=self.manager)

        self.p1_time = 100.0
        self.p2_time = 100.0

        self.p1_info = PlayerInfoPanel(self.manager)
        self.p2_info = PlayerInfoPanel(self.manager)

        self.p1_info.set_position(20, 350)
        self.p2_info.set_position(960, 350)


        self.board[0][0] = 25

        self.turn = 0
        self.cur_select = [-1, -1]

        self.window = window
        self.sock = sock
        self.clock = clock
        self.room_id = room_id
        self.GAME_EVENT = GAME_EVENT
        self.client_id = -1

        # ROO\nINF\n
        self.get_player_info()

    def run(self):
        while True:
            delta = self.clock.tick(120)/1000
            if self.turn%2 == 1:
                self.p1_time -= delta
                if self.p1_time < 0:
                    self.check_game_end()
            else:
                self.p2_time -= delta
                if self.p2_time < 0:
                    self.check_game_end()

            self.update_time()

            self.sock.process_network()
            self.handle_events()
            self.manager.update(delta)
            self.render()
        return

    def handle_events(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.quit()
            
            elif event.type == pygame.MOUSEBUTTONDOWN:
                pos = pygame.mouse.get_pos()
                if pos[0] > self.start_x and pos[0] < self.start_x+self.size*8 and pos[1] > self.start_y and pos[1] < self.start_y+self.size*8:
                    print(self.client_id, self.turn, self.turn%2)
                    if self.client_id == self.turn%2:
                        # TODO: if turn
                        for i in self.board_gui:
                            for j in i:
                                if j.rect.collidepoint(pos):
                                    print(j.board_coord)
                                    coord = j.board_coord
                                    # when clicked on location where is not movable (no marker)
                                    if self.board_gui[coord[1]][coord[0]].moveable == False:
                                        sendtext = f'ROO\n{self.room_id}\nSEL\n{self.turn}\n{coord[1]}{coord[0]}\n'
                                        self.sock.sendall(sendtext)
                                        self.cur_select = [coord[0], coord[1]]

                                    # when clicking on marker, move the piece
                                    else:
                                        sendtext = f'ROO\n{self.room_id}\nMOV\n{self.turn}\n{self.cur_select[1]}{self.cur_select[0]}{coord[1]}{coord[0]}\n'
                                        print(sendtext)
                                        self.sock.sendall(sendtext)
                                    

            elif event.type == pygame_gui.UI_BUTTON_PRESSED:
                if event.ui_element == self.start_game_bt:
                    sendtext = f'ROO\n{self.room_id}\nPLY\n'
                    self.sock.sendall(sendtext)

            elif event.type == self.GAME_EVENT:
                if hasattr(event, 'utype'):
                    if event.utype == GameEvent.ROOM_SELECT_REPLY and event.turn == self.turn:
                        self.disable_moveable()
                        for m in event.moveable:
                            r = int(m[0])
                            c = int(m[1])
                            self.board_gui[r][c].moveable = True
                            print(r, c, self.board_gui[r][c].moveable)
                    elif event.utype == GameEvent.ROOM_TURN_CHANGE:
                        self.turn = event.turn
                        self.disable_moveable()
                        self.p1_time = float(event.p1_time)
                        self.p2_time = float(event.p2_time)
                        self.update_time()
                        # parse board_str
                        board_str = event.board_str
                        print(board_str)
                        for i in range(8):
                            for j in range(8):
                                s = board_str[(i*8+j)*2] + board_str[(i*8+j)*2+1]
                                print(s)
                                self.board[i][j] = int(s)
                    elif event.utype == GameEvent.ROOM_PLAYER_INFO:
                        self.client_id = event.client_id
                        self.p1_info.username.set_text(event.p1_username)
                        self.p2_info.username.set_text(event.p2_username)
                    elif event.utype == GameEvent.ROOM_GAME_FINISHED:
                        print("someone won")
                    
            self.manager.process_events(event)

    def render(self):   
        self.window.blit(self.background, (0, 0))
        self.display_board()
        self.manager.draw_ui(self.window)
        pygame.display.flip()

    def update_time(self):
        self.p1_time_indicator.set_text(f'{int(self.p1_time)//60:02d}:{int(self.p1_time)%60:02d}')
        self.p2_time_indicator.set_text(f'{int(self.p2_time)//60:02d}:{int(self.p2_time)%60:02d}')

    def check_game_end(self):
        self.sock.sendall(f'ROO\n{self.room_id}\nCHK\n')

    def display_board(self):
        for i in range(len(self.board)):
            for j in range(len(self.board[i])):
                self.board_gui[i][j].draw(self.window, self.board[i][j])

    def disable_moveable(self):
        for i in self.board_gui:
            for j in i:
                j.moveable = False
    
    def get_player_info(self):
        self.sock.sendall(f"ROO\n{self.room_id}\nINF\n")

    def quit(self):
        # close the window, close the sockets, exit program
        pygame.quit()
        exit()


def start_game():
    # pygame init
    pygame.init()
    window = pygame.display.set_mode((1280,720))
    pygame.display.set_caption("Chess")
    clock = pygame.time.Clock()
    GAME_EVENT = pygame.event.custom_type()

    network = NetworkPygame(HOST, PORT, GAME_EVENT)

    login_screen = LoginScreen(window, network, clock, GAME_EVENT)
    lobby_screen = LobbyScreen(window, network, clock, GAME_EVENT)

    

    window_state = 'login'
    info = 0
    while True:
        if window_state == 'login':
            window_state, _ = login_screen.run()
        elif window_state == 'lobby':
            window_state, info = lobby_screen.run()
        elif window_state == 'game':
            game_screen = GameScreen(window, network, clock, info, GAME_EVENT)
            window_state, info = game_screen.run()



if __name__ == '__main__':
    start_game()






