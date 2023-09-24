import pygame
import pygame_gui
from ..lib.NetworkPygame import NetworkPygame, GameEvent
from pygame import Rect

class LobbyScene():
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
        self.rooms_panel_display.layer = 1

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


