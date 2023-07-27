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

lobby_refresh_button = pygame_gui.elements.UIButton(relative_rect=pygame.Rect(0, 0, 50, 50),
                                                    text='refresh', manager=lobby_manager)


client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect((HOST, PORT))

def quit():
    pygame.quit()
    client_socket.close()
    exit()

def fetch_room():
    arr = []
    sendtext = "FET"
    bytetext = sendtext.encode()
    client_socket.sendall(bytetext)
    data = client_socket.recv(1024).split(b'\n')
    for room in data:
        room = room.decode()
        arr.append(room.split('\x01'))
    return arr


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
    rooms = fetch_room()

    while True:
        delta = clock.tick(120)/1000
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                quit()
            
            if event.type == pygame_gui.UI_BUTTON_PRESSED:
                if event.ui_element == lobby_refresh_button:
                    print('refresh')
                    room = fetch_room()
                    
            lobby_manager.process_events(event)
        
        lobby_manager.update(delta)

        window.blit(lobby_background, (0, 0))

        lobby_manager.draw_ui(window)
        pygame.display.flip()


def game_screen():
    pass


window_state = "login"
while True:
    if window_state == "login":
        window_state = login_screen()
    elif window_state == 'lobby':
        window_state = lobby_screen()
    elif window_state == 'game':
        window_state = game_screen()

