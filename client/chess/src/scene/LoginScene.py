from lib.NetworkPygame import NetworkPygame, GameEvent
import pygame
import pygame_gui
from pathlib import Path
from pygame import Rect


class LoginScene():
    def __init__(self, window: pygame.Surface, sock: NetworkPygame, clock: pygame.time.Clock, GAME_EVENT):
        # login screen
        self.manager = pygame_gui.UIManager((1280, 720))
        self.login_button = pygame_gui.elements.UIButton(relative_rect=Rect(680, 300, 200, 120), 
                                                    text='login', manager=self.manager)
        self.id_text_box = pygame_gui.elements.UITextEntryLine(relative_rect=Rect(420, 305, 200, 50), manager=self.manager)
        self.pw_text_box = pygame_gui.elements.UITextEntryLine(relative_rect=Rect(420, 365, 200, 50), manager=self.manager)
        self.alert_label = pygame_gui.elements.UILabel(text='', relative_rect=Rect(420, 430, 200, 50), manager=self.manager)
        self.reg_bt = pygame_gui.elements.UIButton(relative_rect=Rect(730, 435, 100, 40),
                                                   text='register', manager=self.manager)
        image_path = Path(__file__).parent.absolute()/'../../img'
        self.background = pygame.image.load(image_path/'chess logo.png')
        self.chess_logo = pygame_gui.elements.UIImage(relative_rect=Rect(980, 25, 200, 200), 
                                                image_surface=window,
                                                manager=self.manager)
        self.chess_logo.set_image(pygame.image.load(image_path/'logo.jpg'))



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


