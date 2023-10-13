import pygame
import pygame_gui
from pygame import Rect
from pathlib import Path
from pygame_gui.ui_manager import ObjectID, PackageResource
from lib.NetworkPygame import NetworkPygame, GameEvent


class GameConstant:
    SIZE = 60

class ChessSprite():
    chess_sprite_path = Path(__file__ ).parent.absolute()/'../../img/simple'
    chess_sprites = {}
    chess_sprites[21] = pygame.image.load(chess_sprite_path/'rook_b.png')
    chess_sprites[22] = pygame.image.load(chess_sprite_path/'knight_b.png')
    chess_sprites[23] = pygame.image.load(chess_sprite_path/'bishop_b.png')
    chess_sprites[24] = pygame.image.load(chess_sprite_path/'queen_b.png')
    chess_sprites[25] = pygame.image.load(chess_sprite_path/'king_b.png')
    chess_sprites[26] = pygame.image.load(chess_sprite_path/'pawn_b.png')
    chess_sprites[31] = pygame.image.load(chess_sprite_path/'rook_w.png')
    chess_sprites[32] = pygame.image.load(chess_sprite_path/'knight_w.png')
    chess_sprites[33] = pygame.image.load(chess_sprite_path/'bishop_w.png')
    chess_sprites[34] = pygame.image.load(chess_sprite_path/'queen_w.png')
    chess_sprites[35] = pygame.image.load(chess_sprite_path/'king_w.png')
    chess_sprites[36] = pygame.image.load(chess_sprite_path/'pawn_w.png')
    chess_sprites[99] = pygame.image.load(chess_sprite_path/'checker.png')

    for k in chess_sprites:
        chess_sprites[k] = pygame.transform.scale(chess_sprites[k], (GameConstant.SIZE, GameConstant.SIZE))

    def __init__(self, color, x, y, width, height, board_coord):
        super(ChessSprite, self).__init__()
        self.color = color
        self.rect = Rect(x, y, width, height)
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


class PlayerInfoPanel():
    def __init__(self, manager):
        self.manager = manager
        self.panel = pygame_gui.elements.UIPanel(relative_rect=Rect(0, 0, 300, 350),
                                                 manager=self.manager)
        self.username = pygame_gui.elements.UILabel(text='',
                                                    relative_rect=Rect(0, 40, 290, 80),
                                                    manager=self.manager,
                                                    container=self.panel,
                                                    object_id="#username_label")


    def set_position(self, left, top):
        self.panel.set_relative_position((left, top))

    def set_username(self, username):
        self.username.set_text(username)

                                                    
        
class GameScene():
    def __init__(self, window: pygame.Surface, sock: NetworkPygame, clock: pygame.time.Clock, room_id, GAME_EVENT):
        # room
        
        self.size = GameConstant.SIZE

        self.start_x = 400
        self.start_y = 60


        self.board = []
        self.board_gui = []
        for i in range(8):
            row = []
            row_gui = []

            for j in range(8):
                row.append(-1)
                color = '#7A9D54' if (i+j)%2 else '#EEEEEE'
                row_gui.append(ChessSprite(color, self.start_x+self.size*j, self.start_y+self.size*i, self.size, self.size, (j,i)))

            self.board.append(row)
            self.board_gui.append(row_gui)


        # gui elements 
        theme_path = PackageResource('theme', 'game_theme.json')
        self.manager = pygame_gui.UIManager((1280, 720), theme_path)
        self.background = pygame.Surface((1280, 720))
        self.background.fill('#DDDDDD')

        self.start_game_bt_rect = Rect(500, 600, 200, 50)
        
        self.start_game_bt = pygame_gui.elements.UIButton(relative_rect=self.start_game_bt_rect, 
                                                          text='Start Game!',
                                                          manager=self.manager)

        self.p1_time_rect = Rect(0, 0, 200, 100)
        self.p2_time_rect = Rect(0, 0, 200, 100)
        self.p1_time_rect.topleft = (50, 50)
        self.p2_time_rect.topright = (1230, 50)

        self.p1_time_indicator = pygame_gui.elements.UILabel(text='',
                                                             relative_rect=self.p1_time_rect,
                                                             manager=self.manager,
                                                             object_id=ObjectID('#p1_time_label', '@time_label'))
        self.p2_time_indicator = pygame_gui.elements.UILabel(text='',
                                                             relative_rect=self.p2_time_rect,
                                                             manager=self.manager,
                                                             object_id=ObjectID('#p2_time_label', '@time_label'))

        image_path = Path(__file__).parent.absolute()/'../../img'
        self.bl_bunny_img = pygame.image.load(image_path/'bl_bunny.png')
        self.wh_bunny_img = pygame.image.load(image_path/'wh_bunny.png')
        self.bl_bunny_turn_img = pygame.image.load(image_path/'bl_bunny_turn.png')
        self.wh_bunny_turn_img = pygame.image.load(image_path/'wh_bunny_turn.png')

        self.bl_bunny = pygame_gui.elements.UIImage(Rect(1030, 200, 200, 200),
                                                        image_surface=self.bl_bunny_img,
                                                        manager=self.manager)

        self.wh_bunny = pygame_gui.elements.UIImage(Rect(50, 200, 200, 200),
                                                        image_surface=self.wh_bunny_img,
                                                        manager=self.manager)

        self.p1_time = 100.0
        self.p2_time = 100.0

        self.p1_info = PlayerInfoPanel(self.manager)
        self.p2_info = PlayerInfoPanel(self.manager)

        self.p1_info.set_position(20, 350)
        self.p2_info.set_position(960, 350)

        self.finish_window = pygame_gui.elements.UIWindow(rect=Rect(200, 50, 800, 600),
                                                          manager=self.manager)
        self.finish_window.hide()
        self.finish_window.layer = 100
        self.finish_window.enable_close_button = False
        self.finish_label = pygame_gui.elements.UILabel(relative_rect=Rect(30, 30, 500, 100),
                                                        text='asd',
                                                        manager=self.manager,
                                                        container=self.finish_window)
        self.return_to_lobby_bt = pygame_gui.elements.UIButton(relative_rect=Rect(20, 300, 300, 100),
                                                               text='return to lobby',
                                                               manager=self.manager,
                                                               container=self.finish_window)



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
        self.running = True
        self.gaming = False


    def run(self):
        while self.running:
            delta = self.clock.tick(120)/1000
            if self.gaming:
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
        return 'lobby', -1

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
                elif event.ui_element == self.return_to_lobby_bt:
                    self.running = False

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
                        self.gaming = True
                        self.turn = event.turn
                        # set images
                        self.change_bunny_img(self.turn)
                        
                        self.disable_moveable()
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
                    elif event.utype == GameEvent.ROOM_GAME_TIME_CONTINUE:
                        self.p1_time = float(event.p1_time)
                        self.p2_time = float(event.p2_time)
                        self.update_time()
                    elif event.utype == GameEvent.ROOM_GAME_FINISHED:
                        self.finish_label.set_text(f'{event.win_player} has won the game!')
                        self.finish_window.show()

                        self.gaming = False

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

    def change_bunny_img(self, turn):
        if turn%2 == 1:
            self.wh_bunny.set_image(self.wh_bunny_turn_img)
            self.bl_bunny.set_image(self.bl_bunny_img)
        else:
            self.wh_bunny.set_image(self.wh_bunny_img)
            self.bl_bunny.set_image(self.bl_bunny_turn_img)




