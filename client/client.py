import pygame
import pygame_gui
import socket

HOST = '127.0.0.1'
PORT = 12345

pygame.init()
window = pygame.display.set_mode((1280,720))
pygame.display.set_caption("Chess")
clock = pygame.time.Clock()

manager = pygame_gui.UIManager((1280, 720))
login_button = pygame_gui.elements.UIButton(relative_rect=pygame.Rect(200, 0, 200, 120), 
                                            text='login', manager=manager,
                                            anchors={'center': 'center'})
id_text_box = pygame_gui.elements.UITextEntryLine(relative_rect=pygame.Rect(-50, -30, 200, 50), manager=manager,
                                                  anchors={'center': 'center'})
pw_text_box = pygame_gui.elements.UITextEntryLine(relative_rect=pygame.Rect(-50, 30, 200, 50), manager=manager,
                                                  anchors={'center': 'center'})
running = True

background = pygame.Surface((1280, 720))
background.fill('#FFFFFF')


client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect((HOST, PORT))

while running:
    delta = clock.tick(60)/1000
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

        if event.type == pygame_gui.UI_BUTTON_PRESSED:
            if event.ui_element == login_button:
                print('sending login information')
                id_text = id_text_box.get_text()
                pw_text = pw_text_box.get_text()
                client_socket.snedall(b'LOG {} {}'.format(id_text, pw_text))
        manager.process_events(event)
    
    manager.update(delta)

    window.blit(background, (0, 0))
    manager.draw_ui(window)

    clock.tick(60)
    pygame.display.flip()

pygame.quit()

client_socket.close()