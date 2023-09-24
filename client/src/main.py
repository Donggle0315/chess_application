import pygame
from constants import *
from lib.NetworkPygame import NetworkPygame
from scene.LoginScene import LoginScene
from scene.LobbyScene import LobbyScene
from scene.GameScene import GameScene

def start_game():
    # pygame init
    pygame.init()
    window = pygame.display.set_mode((1280,720))
    pygame.display.set_caption("Chess")
    clock = pygame.time.Clock()
    GAME_EVENT = pygame.event.custom_type()

    network = NetworkPygame(HOST, PORT, GAME_EVENT)

    login_screen = LoginScene(window, network, clock, GAME_EVENT)

    

    window_state = 'login'
    info = 0
    while True:
        if window_state == 'login':
            window_state, _ = login_screen.run()
        elif window_state == 'lobby':
            lobby_screen = LobbyScene(window, network, clock, GAME_EVENT)
            window_state, info = lobby_screen.run()
        elif window_state == 'game':
            game_screen = GameScene(window, network, clock, info, GAME_EVENT)
            window_state, _ = game_screen.run()
            print(window_state)



if __name__ == '__main__':
    start_game()


