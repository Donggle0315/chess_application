import pygame
import pygame_gui
import socket

HOST = '127.0.0.1'
PORT = 12345

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

rooms_panel = pygame_gui.elements.UIPanel(pygame.Rect(30, 30, 900, 600),
                                          manager=lobby_manager)
rooms_bt = []


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

final_create_room_bt.set_relative_position()


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
    arr = []
    sendtext = "FET"
    bytetext = sendtext.encode()
    client_socket.sendall(bytetext)
    data = client_socket.recv(1024).rstrip(b'\x00')

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

def make_rooms_bt(rooms):
    for room in rooms_bt:
        room.kill()
    
    rooms_bt = []

    for room in rooms:
        text = 'Name: {}\n User: {}/{}\n Time: {}\n'.format(room['room_name'], 
                                                            room['cur_user_count'],
                                                            room['max_user_count'],
                                                            room['time'])
        new_bt = pygame_gui.elements.UIButton(relative_rect=(0, 0, 300, 200),
                                              text=text, manager=lobby_manager,
                                              container=rooms_panel)
        new_bt.disable()

        rooms_bt.append(new_bt)


def display_rooms_bt(page):
    cur_idx = (page-1) * 4

    xpos = 50
    ypos = 10
    positions = []
    for i in range(4):
        positions.append((xpos, ypos))
        xpos = (xpos + 400) % 800
        if i % 2 == 1:
            ypos += 200

    for i in rooms_bt:
        i.disable()

    for i in range(cur_idx, len(rooms_bt)):
        cur = rooms_bt[i]
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
                    bytetext = str.encode(sendtext)
                    client_socket.sendall(bytetext)
                    
                    data = client_socket.recv(1024).split(b'\n')
                    head = data[0].decode()
                    print(head)
                    if head == 'SUC':
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
    make_rooms_bt(rooms)
    display_rooms_bt()

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
                    make_rooms_bt(rooms)
                    display_rooms_bt()
                    
                if event.ui_element == create_room_bt:
                    print('create room')
                    create_room_window.show()
                if event.ui_element == final_create_room_bt:
                    print('create room final')
                    rname = room_name_text_box.text
                    rmax_user = max_user_menu.selected_option
                    rtime = time_menu.selected_option

                    sendtext = f'CRE\n{rname}\n{rmax_user}\n{rtime}\n'
                    bytetext = str.encode(sendtext)
                    client_socket.sendall(bytetext)
                    
                    data = client_socket.recv(1024).split(b'\n')
                    if data[0] == b'ADD':
                        room_socket.connect((data[1], data[2]))
                        return 'game'
                    else:
                        print('wrong data')
                    


            lobby_manager.process_events(event)
        
        lobby_manager.update(delta)

        window.blit(lobby_background, (0, 0))

        lobby_manager.draw_ui(window)
        pygame.display.flip()


def game_screen():
    while True:
        delta = clock.tick(120)/1000
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                quit()
            
            if event.type == pygame_gui.UI_BUTTON_PRESSED:
                pass
                    
            room_manager.process_events(event)
        
        room_manager.update(delta)

        window.blit(room_background, (0, 0))

        room_manager.draw_ui(window)
        pygame.display.flip()



window_state = "lobby"
while True:
    if window_state == "login":
        window_state = login_screen()
    elif window_state == 'lobby':
        window_state = lobby_screen()
    elif window_state == 'game':
        window_state = game_screen()

