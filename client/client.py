import pygame
import pygame_gui
import socket

ADDR = '13.209.83.147:58024'
HOST, PORT = ADDR.split(':')
PORT = int(PORT)
MAXLEN = 2048


class ChessSprite():
    def __init__(self, color, x, y, width, height, chess_sprites, board_coord):
        super(ChessSprite, self).__init__()
        self.color = color
        self.rect = pygame.Rect(x, y, width, height)
        self.chess_sprites = chess_sprites
        self.surface = pygame.Surface((width, height))
        self.surface.fill(self.color)
        self.board_coord = board_coord
        self.moveable = False

    def draw(self, window, num):
        if num in self.chess_sprites:
            self.surface.blit(self.chess_sprites[num], (0, 0, self.rect.width,self.rect.height))
        if self.moveable:
            self.surface.blit(self.chess_sprites[99], (0, 0, self.rect.width,self.rect.height))
        window.blit(self.surface, self.rect)

    


class LoginScreen():
    def __init__(self, window: pygame.Surface, sock: socket, clock: pygame.time.Clock):
        # login screen
        self.manager = pygame_gui.UIManager((1280, 720))
        self.login_button = pygame_gui.elements.UIButton(relative_rect=pygame.Rect(200, 0, 200, 120), 
                                                    text='login', manager=self.manager,
                                                    anchors={'center': 'center'})
        self.id_text_box = pygame_gui.elements.UITextEntryLine(relative_rect=pygame.Rect(-50, -30, 200, 50), manager=self.manager,
                                                        anchors={'center': 'center'})
        self.pw_text_box = pygame_gui.elements.UITextEntryLine(relative_rect=pygame.Rect(-50, 30, 200, 50), manager=self.manager,
                                                        anchors={'center': 'center'})
        self.background = pygame.Surface((1280, 720))
        self.background.fill('#FFFFFF')

        # socket to connect to the main server
        self.sock = sock
        self.running = True
        self.next_window = ''
        self.clock = clock
        self.window = window
    def run(self):
        # run the app
        while self.running:
            delta = self.clock.tick(120)/1000
            self.handle_events()
            self.manager.update(delta)
            self.render()
        return self.next_window, True
    
    def handle_events(self):
        # handle events in login screen
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                quit()

            if event.type == pygame_gui.UI_BUTTON_PRESSED:
                if event.ui_element == self.login_button:
                    print('sending login information')
                    id_text = self.id_text_box.get_text()
                    pw_text = self.pw_text_box.get_text()
                    sendtext = f'LOG\n{id_text}\n{pw_text}\n'
                    bytetext = str.encode(sendtext).ljust(MAXLEN, b'\0')
                    self.sock.sendall(bytetext)
                    
                    data = recvall(self.sock, MAXLEN).split(b'\n')
                    head = data[0].decode()
                    if head == 'SUC':
                        self.running = False
                        self.next_window = 'lobby'
                        return
                    else:
                        # things to do if login fails
                        pass

            self.manager.process_events(event)

    def render(self):
        # render objects in login screen
        self.window.blit(self.background, (0, 0))
        self.manager.draw_ui(self.window)
        pygame.display.flip()
    
    def quit(self):
        # close the window, close the sockets, exit program
        pygame.quit()
        self.sock.close()
        exit()


class LobbyScreen():
    def __init__(self, window: pygame.Surface, sock: socket, clock: pygame.time.Clock):
        # lobby screen
        self.manager = pygame_gui.UIManager((1280, 720))

        self.background = pygame.Surface((1280, 720))
        self.background.fill('#EEEEEE')

        self.lobby_refresh_bt_rect = pygame.Rect(0, 0, 50, 50)
        self.lobby_refresh_bt_rect.bottomright = (-100, -50)
        self.lobby_refresh_bt = pygame_gui.elements.UIButton(relative_rect=self.lobby_refresh_bt_rect,
                                                            text='refresh', manager=self.manager,
                                                            anchors={'right': 'right',
                                                                    'bottom': 'bottom'})

        self.create_room_bt_rect = pygame.Rect(0, 0, 50, 50)
        self.create_room_bt_rect.bottomright = (-200, -50)
        self.create_room_bt = pygame_gui.elements.UIButton(relative_rect=self.create_room_bt_rect,
                                                            text='create room', manager=self.manager,
                                                            anchors={'right': 'right',
                                                                    'bottom': 'bottom'})

        self.rooms_panel_display = pygame_gui.elements.UIPanel(pygame.Rect(30, 30, 900, 600),
                                                manager=self.manager)


        self.create_room_window_rect = pygame.Rect(300, 100, 700, 500)

        self.create_room_window_color = "#21282D"
        self.create_room_window = pygame_gui.elements.UIWindow(rect=self.create_room_window_rect, 
                                                        window_display_title='create room',
                                                        manager=self.manager)

        self.create_room_window.on_close_window_button_pressed = self.create_room_window.hide
        self.create_room_window.hide()



        # create room window
        self.room_name_label = pygame_gui.elements.UILabel(text='Room Name', relative_rect=pygame.Rect(50, 50, 150, 50),
                                                        manager=self.manager, container=self.create_room_window)
        self.room_name_text_box = pygame_gui.elements.UITextEntryLine(relative_rect=pygame.Rect(200, 50, 400, 50), manager=self.manager,
                                                                container=self.create_room_window)

        self.max_user_label = pygame_gui.elements.UILabel(text='Max User', relative_rect=pygame.Rect(50, 120, 150, 50),
                                                    manager=self.manager, container=self.create_room_window)
        self.max_user_menu = pygame_gui.elements.UIDropDownMenu(options_list=['2', '3', '4'],
                                                            starting_option='2',
                                                            relative_rect=pygame.Rect(200, 120, 100, 50),
                                                            manager=self.manager, container=self.create_room_window)

        self.time_label = pygame_gui.elements.UILabel(text='Time (min)', relative_rect=pygame.Rect(50, 190, 150, 50),
                                                manager=self.manager, container=self.create_room_window)
        self.time_menu = pygame_gui.elements.UIDropDownMenu(options_list=['10', '20', '30'],
                                                            starting_option='10',
                                                            relative_rect=pygame.Rect(200, 190, 100, 50),
                                                            manager=self.manager, container=self.create_room_window)
        self.final_create_room_bt_rect = pygame.Rect(0, 0, 200, 50)
        self.final_create_room_bt_rect.bottomright = (-50, -50)
        self.final_create_room_bt = pygame_gui.elements.UIButton(relative_rect=self.final_create_room_bt_rect,
                                                            text='Create Room!',
                                                            manager=self.manager,
                                                            container=self.create_room_window,
                                                            anchors={'right': 'right',
                                                                    'bottom': 'bottom'})
        
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

    def run(self):
        self.rooms = self.fetch_room()
        while self.running:
            delta = self.clock.tick(120)/1000
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
                    self.rooms = self.fetch_room()
                    self.display_rooms_panel()

                if event.ui_element == self.create_room_bt:
                    print('create room')
                    self.create_room_window.show()
                if event.ui_element == self.final_create_room_bt:
                    print('create room final')
                    rname = self.room_name_text_box.text
                    rmax_user = self.max_user_menu.selected_option
                    rtime = self.time_menu.selected_option

                    sendtext = f'CRE\n{rname}\n{rmax_user}\n{rtime}\n'
                    bytetext = str.encode(sendtext).ljust(MAXLEN, b'\0')
                    self.sock.sendall(bytetext)
                    
                    data = recvall(self.sock, MAXLEN).split(b'\n')
                    print(data, len(data[0]))

                    if data[0] == b'ENT':
                        self.running = False
                        self.next_window = 'game'
                        self.room_id = int(data[1].decode())
                        return
                    else:
                        print('wrong data')

                for i, (p, b, room) in enumerate(self.rooms_panel):
                    if event.ui_element == b:
                        cur_idx, _ = self.get_room_range()
                        cur_idx += i

                        sendtext = f'ENT\n{room[cur_idx]["room_id"]}\n'
                        bytetext = str.encode(sendtext).ljust(MAXLEN, b'\0')
                        self.sock.sendall(bytetext)

                        data = recvall(self.sock, MAXLEN).split(b'\n')
                        if data[0] == b'SUC':
                            self.running = False
                            self.next_window = 'game'
                            self.room_id = int(data[1].decode())
                            return
                        else:
                            print('failed because of unknown reason')
                        

            self.manager.process_events(event)
    
    def render(self):
        self.window.blit(self.background, (0, 0))
        self.manager.draw_ui(self.window)
        pygame.display.flip()

    def make_rooms_panel(self):
        self.rooms_panel = []
        for room in range(self.room_display_count):
            
            new_panel = pygame_gui.elements.UIPanel(relative_rect=pygame.Rect(0, 0, 300, 200),
                                                    manager=self.manager,
                                                    container=self.rooms_panel_display)
            new_bt = pygame_gui.elements.UIButton(relative_rect=pygame.Rect(0, 150, 300, 50),
                                                text='enter', manager=self.manager,
                                                container=new_panel)
            text_box = pygame_gui.elements.UITextBox(html_text='',
                                        relative_rect=pygame.Rect(0, 0, 300, 150),
                                        manager=self.manager,
                                        container=new_panel)
            new_panel.hide()

            self.rooms_panel.append([new_panel, new_bt, text_box])

    def display_rooms_panel(self):
        cur_idx, last_idx = self.get_room_range()

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
            if i >= len(self.room):
                break
            cur_text_box = self.rooms_panel[i][1]
            cur_text_box.set_text = 'Name: {}\nUser: {}/{}\nTime: {}\n'.format(
                                                                self.room[i]['room_name'], 
                                                                self.room[i]['cur_user_count'],
                                                                self.room[i]['max_user_count'],
                                                                self.room[i]['time'])
            
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
        arr = []
        sendtext = "FET\n"
        bytetext = sendtext.encode().ljust(MAXLEN, b'\0')
        self.sock.sendall(bytetext)
        data = recvall(self.sock, MAXLEN).rstrip(b'\x00')

        # no rooms
        if data == b'':
            return []

        data = data.split(b'\n')[:-1]
        

        for room in data:
            room = room.decode().split('\\')
            print(room)
            d = {}
            d['room_id'] = room[0]
            d['room_name'] = room[1]
            d['max_user_count'] = room[2]
            d['cur_user_count'] = room[3]
            d['time'] = room[4]
            d['address'] = room[5]
            arr.append(d)

        return arr
    
    def quit(self):
        # close the window, close the sockets, exit program
        pygame.quit()
        self.sock.close()
        exit()

class GameScreen():
    def __init__(self, window: pygame.Surface, sock: socket, clock: pygame.time.Clock):
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
        self.start_x = 60
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

        self.board[0][0] = 25

        self.turn = 0
        self.cur_select = [-1, -1]

        self.window = window
        self.sock = sock
        self.clock = clock

    def run(self):
        while True:
            delta = self.clock.tick(120)/1000
            self.handle_events()
            self.manager.update(delta)
            self.render()

    def handle_events(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.quit()
            
            if event.type == pygame.MOUSEBUTTONDOWN:
                pos = pygame.mouse.get_pos()
                if pos[0] > self.start_x and pos[0] < self.start_x+self.size*8 and pos[1] > self.start_y and pos[1] < self.start_y+self.size*8:
                    # TODO: if turn
                    for i in self.board_gui:
                        for j in i:
                            if j.rect.collidepoint(pos):
                                print(j.board_coord)
                                coord = j.board_coord
                                if self.board_gui[coord[1]][coord[0]].moveable == False:
                                    sendtext = f'SEL\n{self.turn}\n{coord[1]}{coord[0]}\n'
                                    bytetext = str.encode(sendtext)
                                    self.sock.sendall(bytetext)

                                    data = recvall(self.sock, MAXLEN).split(b'\n')
                                    if data[0] == b'SEL' and int(data[1]) == self.turn:
                                        moveable = data[2].split()
                                        self.disable_moveable(self.board_gui)
                                        for m in moveable:
                                            r = int(m[0])
                                            c = int(m[1])
                                            cur_select = [c, r]
                                            self.board_gui[r][c].moveable = True
                                else:
                                    sendtext = f'MOV\n{cur_select[1]}{cur_select[0]}{coord[1]}{coord[0]}\n'
                                    bytetext = str.encode(sendtext)
                                    self.sock.sendall(bytetext)

                                    data = recvall(self.sock, MAXLEN).split(b'\n')
                                    

            if event.type == pygame_gui.UI_BUTTON_PRESSED:
                pass
                    
            self.manager.process_events(event)

    def render(self):   
        self.window.blit(self.background, (0, 0))
        self.display_board()
        self.manager.draw_ui(self.window)
        pygame.display.flip()

    def display_board(self):
        for i in range(len(self.board)):
            for j in range(len(self.board[i])):
                self.board_gui[i][j].draw(self.window, self.board[i][j])

    def disable_moveable(self):
        for i in self.board_gui:
            for j in i:
                j.moveable = False

    def quit(self):
        # close the window, close the sockets, exit program
        pygame.quit()
        self.sock.close()
        exit()

def start_game():
    # pygame init
    pygame.init()
    window = pygame.display.set_mode((1280,720))
    pygame.display.set_caption("Chess")
    clock = pygame.time.Clock()

    # socket interface
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # connect to main_server with client_socket
    client_socket.connect((HOST, PORT))
    login_screen = LoginScreen(window, client_socket, clock)
    lobby_screen = LobbyScreen(window, client_socket, clock)


    window_state = 'login'
    info = 0
    while True:
        if window_state == 'login':
            window_state, _ = login_screen.run()
        elif window_state == 'lobby':
            window_state, info = lobby_screen.run()
        elif window_state == 'game':
            game_screen = GameScreen(window, client_socket, clock, info)
            window_state, info = game_screen.run()


def recvall(sock, size):
    data = b""
    while len(data) < size:
        chunk = sock.recv(size - len(data))
        if not chunk:
            break
        data += chunk
    return data







if __name__ == '__main__':
    start_game()






