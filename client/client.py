import pygame
import pygame_gui
import socket

HOST = '127.0.0.1'
PORT = 12345
MAXLEN = 2048

pygame.init()
window = pygame.display.set_mode((1280,720))
pygame.display.set_caption("Chess")
clock = pygame.time.Clock()

# login screen
manager = pygame_gui.UIManager((1280, 720))
login_button = pygame_gui.elements.UIButton(relative_rect=pygame.Rect(200, 0, 200, 120), 
                                            text='login', manager=manager,
                                            anchors={'center': 'center'})
id_text_box = pygame_gui.elements.UITextEntryLine(relative_rect=pygame.Rect(-50, -30, 200, 50), manager=manager,
                                                  anchors={'center': 'center'})
pw_text_box = pygame_gui.elements.UITextEntryLine(relative_rect=pygame.Rect(-50, 30, 200, 50), manager=manager,
                                                  anchors={'center': 'center'})
background = pygame.Surface((1280, 720))
background.fill('#FFFFFF')



# lobby screen

lobby_manager = pygame_gui.UIManager((1280, 720))

lobby_background = pygame.Surface((1280, 720))
lobby_background.fill('#EEEEEE')

lobby_refresh_bt_rect = pygame.Rect(0, 0, 50, 50)
lobby_refresh_bt_rect.bottomright = (-100, -50)
lobby_refresh_bt = pygame_gui.elements.UIButton(relative_rect=lobby_refresh_bt_rect,
                                                    text='refresh', manager=lobby_manager,
                                                    anchors={'right': 'right',
                                                             'bottom': 'bottom'})

create_room_bt_rect = pygame.Rect(0, 0, 50, 50)
create_room_bt_rect.bottomright = (-200, -50)
create_room_bt = pygame_gui.elements.UIButton(relative_rect=create_room_bt_rect,
                                                    text='create room', manager=lobby_manager,
                                                    anchors={'right': 'right',
                                                             'bottom': 'bottom'})

rooms_panel_display = pygame_gui.elements.UIPanel(pygame.Rect(30, 30, 900, 600),
                                          manager=lobby_manager)


create_room_window_rect = pygame.Rect(300, 100, 700, 500)

create_room_window_color = "#21282D"
create_room_window = pygame_gui.elements.UIWindow(rect=create_room_window_rect, 
                                                  window_display_title='create room',
                                                  manager=lobby_manager)

create_room_window.on_close_window_button_pressed = create_room_window.hide
create_room_window.hide()



# create room window
room_name_label = pygame_gui.elements.UILabel(text='Room Name', relative_rect=pygame.Rect(50, 50, 150, 50),
                                                manager=lobby_manager, container=create_room_window)
room_name_text_box = pygame_gui.elements.UITextEntryLine(relative_rect=pygame.Rect(200, 50, 400, 50), manager=lobby_manager,
                                                         container=create_room_window)

max_user_label = pygame_gui.elements.UILabel(text='Max User', relative_rect=pygame.Rect(50, 120, 150, 50),
                                             manager=lobby_manager, container=create_room_window)
max_user_menu = pygame_gui.elements.UIDropDownMenu(options_list=['2', '3', '4'],
                                                    starting_option='2',
                                                    relative_rect=pygame.Rect(200, 120, 100, 50),
                                                    manager=lobby_manager, container=create_room_window)

time_label = pygame_gui.elements.UILabel(text='Time (min)', relative_rect=pygame.Rect(50, 190, 150, 50),
                                         manager=lobby_manager, container=create_room_window)
time_menu = pygame_gui.elements.UIDropDownMenu(options_list=['10', '20', '30'],
                                                    starting_option='10',
                                                    relative_rect=pygame.Rect(200, 190, 100, 50),
                                                    manager=lobby_manager, container=create_room_window)
final_create_room_bt_rect = pygame.Rect(0, 0, 200, 50)
final_create_room_bt_rect.bottomright = (-50, -50)
final_create_room_bt = pygame_gui.elements.UIButton(relative_rect=final_create_room_bt_rect,
                                                    text='Create Room!',
                                                    manager=lobby_manager,
                                                    container=create_room_window,
                                                    anchors={'right': 'right',
                                                             'bottom': 'bottom'})

# room
room_manager = pygame_gui.UIManager((1280, 720))
room_background = pygame.Surface((1280, 720))
room_background.fill('#EEEEEE')




# chess sprites
chess_sprites = {}
chess_sprites[21] = pygame.image.load('./img/rook_b.png')
chess_sprites[22] = pygame.image.load('./img/knight_b.png')
chess_sprites[23] = pygame.image.load('./img/bishop_b.png')
chess_sprites[24] = pygame.image.load('./img/queen_b.png')
chess_sprites[25] = pygame.image.load('./img/king_b.png')
chess_sprites[26] = pygame.image.load('./img/pawn_b.png')

chess_sprites[31] = pygame.image.load('./img/rook_w.png')
chess_sprites[32] = pygame.image.load('./img/knight_w.png')
chess_sprites[33] = pygame.image.load('./img/bishop_w.png')
chess_sprites[34] = pygame.image.load('./img/queen_w.png')
chess_sprites[35] = pygame.image.load('./img/king_w.png')
chess_sprites[36] = pygame.image.load('./img/pawn_w.png')

chess_sprites[99] = pygame.image.load('./img/checker.png')

# socket interface
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect((HOST, PORT))
room_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)




# exit window
def quit():
    pygame.quit()
    client_socket.close()
    exit()

# fetch room information
def fetch_room():
    print('fet')
    arr = []
    sendtext = "FET\n"
    bytetext = sendtext.encode().ljust(MAXLEN, b'\0')
    client_socket.sendall(bytetext)
    data = client_socket.recv(MAXLEN).rstrip(b'\x00')

    # no rooms
    if data == b'':
        return []

    data = data.split(b'\n')
    
    
    for room in data:
        room = room.decode().split('\\')
        d = {}
        d['room_id'] = room[0]
        d['room_name'] = room[1]
        d['max_user_count'] = room[2]
        d['cur_user_count'] = room[3]
        d['time'] = room[4]
        d['address'] = room[5]
        arr.append(d)

    return arr

def make_rooms_panel(rooms, rooms_panel: list):
    for room in rooms_panel:
        room.kill()
    
    rooms_panel.clear()

    for room in rooms:
        text = 'Name: {}\nUser: {}/{}\nTime: {}\n'.format(room['room_name'], 
                                                            room['cur_user_count'],
                                                            room['max_user_count'],
                                                            room['time'])
        
        new_panel = pygame_gui.elements.UIPanel(relative_rect=pygame.Rect(0, 0, 300, 200),
                                                manager=lobby_manager,
                                                container=rooms_panel_display)
        new_bt = pygame_gui.elements.UIButton(relative_rect=pygame.Rect(0, 150, 300, 50),
                                              text='enter', manager=lobby_manager,
                                              container=new_panel)
        pygame_gui.elements.UITextBox(html_text=text,
                                      relative_rect=pygame.Rect(0, 0, 300, 150),
                                      manager=lobby_manager,
                                      container=new_panel)
        new_panel.disable()

        rooms_panel.append([new_panel, new_bt, room])


def display_rooms_panel(page, rooms_panel):
    cur_idx = (page) * 4

    xpos = 50
    ypos = 10
    positions = []
    for i in range(4):
        positions.append((xpos, ypos))
        xpos = (xpos + 400) % 800
        if i % 2 == 1:
            ypos += 200

    for i, b in rooms_panel:
        i.disable()

    for i in range(cur_idx, len(rooms_panel)):
        cur = rooms_panel[i][0]
        cur.set_relative_position((positions[i]))
        cur.enable()



def login_screen():
    while True:
        delta = clock.tick(120)/1000
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                quit()

            if event.type == pygame_gui.UI_BUTTON_PRESSED:
                if event.ui_element == login_button:
                    print('sending login information')
                    id_text = id_text_box.get_text()
                    pw_text = pw_text_box.get_text()
                    sendtext = f'LOG\n{id_text}\n{pw_text}\n'
                    bytetext = str.encode(sendtext).ljust(MAXLEN, b'\0')
                    client_socket.sendall(bytetext)
                    
                    data = client_socket.recv(MAXLEN).split(b'\n')
                    head = data[0].decode()
                    print(head)
                    if head == 'SUC':
                        print(head)
                        return 'lobby'
                    else:
                        continue

            manager.process_events(event)
        
        manager.update(delta)

        window.blit(background, (0, 0))
        manager.draw_ui(window)
        pygame.display.flip()


def lobby_screen():
    page = 0
    rooms = fetch_room()

    rooms_panel = []
    make_rooms_panel(rooms, rooms_panel)
    display_rooms_panel(page, rooms_panel)

    print(rooms)
    while True:
        delta = clock.tick(120)/1000
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                quit()
            
            if event.type == pygame_gui.UI_BUTTON_PRESSED:
                if event.ui_element == lobby_refresh_bt:
                    print('refresh')
                    room = fetch_room()
                    make_rooms_panel(rooms, rooms_panel)
                    display_rooms_panel(page, rooms_panel)

                if event.ui_element == create_room_bt:
                    print('create room')
                    create_room_window.show()
                if event.ui_element == final_create_room_bt:
                    print('create room final')
                    rname = room_name_text_box.text
                    rmax_user = max_user_menu.selected_option
                    rtime = time_menu.selected_option

                    sendtext = f'CRE\n{rname}\n{rmax_user}\n{rtime}\n'
                    bytetext = str.encode(sendtext).ljust(MAXLEN, b'\0')
                    client_socket.sendall(bytetext)
                    
                    data = client_socket.recv(MAXLEN).split(b'\n')
                    print(data)
                    
                    if data[0] == b'ENT':
                        room_socket.connect((data[1], data[2]))
                        return 'game'
                    else:
                        print('wrong data')
                for p, b, room in rooms_panel:
                    if event.ui_element == b:
                        # TODO: enter

                        sendtext = f'ENT\n{room["room_id"]}\n'
                        bytetext = str.encode(sendtext).ljust(MAXLEN, b'\0')
                        client_socket.sendall(bytetext)

                        data = client_socket.recv(MAXLEN).split(b'\n')
                        if data[0] == b'ENT':
                            address, port = data[1].split(b':')
                            room_socket.connect((address, int(port)))
                            return 'game'
                        
                    


            lobby_manager.process_events(event)
        
        lobby_manager.update(delta)

        window.blit(lobby_background, (0, 0))

        lobby_manager.draw_ui(window)
        pygame.display.flip()


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
        if num in chess_sprites:
            self.surface.blit(self.chess_sprites[num], (0, 0, self.rect.width,self.rect.height))
        if self.moveable:
            self.surface.blit(self.chess_sprites[99], (0, 0, self.rect.width,self.rect.height))
        window.blit(self.surface, self.rect)

def display_board(window, board_gui, board):
    for i in range(len(board)):
        for j in range(len(board[i])):
            board_gui[i][j].draw(window, board[i][j])

size = 60
start_x = 60
start_y = 60

for k in chess_sprites:
        chess_sprites[k] = pygame.transform.scale(chess_sprites[k], (size, size))

def disable_moveable(board_gui):
    for i in board_gui:
        for j in i:
            j.moveable = False
def game_screen():
    # room_socket connect
    #room_socket.connect()

    # 8x8 board
    
    board = []
    board_gui = []
    for i in range(8):
        row = []
        row_gui = []
        for j in range(8):
            row.append(-1)
            color = '#7A9D54' if (i+j)%2 else '#EEEEEE'
            row_gui.append(ChessSprite(color, start_x+size*j, start_y+size*i, size, size, chess_sprites, (j,i)))
        board.append(row)
        board_gui.append(row_gui)

    board[0][0] = 25

    turn = 0
    cur_select = [-1, -1]
    while True:
        delta = clock.tick(120)/1000
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                quit()
            
            if event.type == pygame.MOUSEBUTTONDOWN:
                pos = pygame.mouse.get_pos()
                if pos[0] > start_x and pos[0] < start_x+size*8 and pos[1] > start_y and pos[1] < start_y+size*8:
                    # TODO: if turn
                    for i in board_gui:
                        for j in i:
                            if j.rect.collidepoint(pos):
                                print(j.board_coord)
                                coord = j.board_coord
                                if board_gui[coord[1]][coord[0]].moveable == False:
                                    sendtext = f'SEL\n{turn}\n{coord[1]}{coord[0]}\n'
                                    bytetext = str.encode(sendtext)
                                    room_socket.sendall(bytetext)

                                    data = room_socket.recv(MAXLEN).split(b'\n')
                                    if data[0] == b'SEL' and int(data[1]) == turn:
                                        moveable = data[2].split()
                                        disable_moveable(board_gui)
                                        for m in moveable:
                                            r = int(m[0])
                                            c = int(m[1])
                                            cur_select = [c, r]
                                            board_gui[r][c].moveable = True
                                else:
                                    sendtext = f'MOV\n{cur_select[1]}{cur_select[0]}{coord[1]}{coord[0]}\n'
                                    bytetext = str.encode(sendtext)
                                    room_socket.sendall(bytetext)

                                    data = room_socket.recv(MAXLEN).split(b'\n')
                                    

            if event.type == pygame_gui.UI_BUTTON_PRESSED:
                pass
                    
            room_manager.process_events(event)
        
        room_manager.update(delta)

        window.blit(room_background, (0, 0))
        display_board(window, board_gui, board)
        room_manager.draw_ui(window)
        pygame.display.flip()



window_state = "login"
while True:
    if window_state == "login":
        window_state = login_screen()
    elif window_state == 'lobby':
        window_state = lobby_screen()
    elif window_state == 'game':
        window_state = game_screen()

