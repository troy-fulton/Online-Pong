import pygame
import random
import sys
import requests
import re
import time

# Set the IP based on the Arduino IP Address for the server
ip_addr = "192.168.137.221"

class Paddle(pygame.Rect):
    def __init__(self, velocity, up_key, down_key, *args, **kwargs):
        self.velocity = velocity
        self.up_key = up_key
        self.down_key = down_key
        super().__init__(*args, **kwargs)

    def move_paddle(self, board_height):
        keys_pressed = pygame.key.get_pressed()

        if keys_pressed[self.up_key]:
            if self.y - self.velocity > 0:
                self.y -= self.velocity

        if keys_pressed[self.down_key]:
            if self.y + self.velocity < board_height - self.height:
                self.y += self.velocity


class Ball(pygame.Rect):
    def __init__(self, velocity, *args, **kwargs):
        self.velocity = velocity
        self.angle = 0
        super().__init__(*args, **kwargs)

    def move_ball(self):
        self.x += self.velocity
        self.y += self.angle


class Pong:
    game_over = 0

    height = 500
    width = 1100

    paddle_width = 10
    paddle_height = 100

    ball_width = 10
    ball_velocity = 10
    ball_angle = 0

    color = (255, 255, 255)

    def __init__(self):
        pygame.init()  # Start the pygame instance.

        # Setup the screen
        self.screen = pygame.display.set_mode((self.width, self.height))
        self.clock = pygame.time.Clock()

        # Create the player objects.

        self.paddles = []
        self.balls = []
        self.paddles.append(Paddle(  # The left paddle
            self.ball_velocity,
            pygame.K_w, #w key 
            pygame.K_s, #s key 
            0,
            self.height / 2 - self.paddle_height / 2,
            self.paddle_width,
            self.paddle_height
        ))

        self.paddles.append(Paddle(  # The right paddle
            self.ball_velocity,
            pygame.K_UP,
            pygame.K_DOWN,
            self.width - self.paddle_width,
            self.height / 2 - self.paddle_height / 2,
            self.paddle_width,
            self.paddle_height
        ))

        self.balls.append(Ball(
            self.ball_velocity,
            self.width / 2 - self.ball_width / 2,
            self.height / 2 - self.ball_width / 2,
            self.ball_width,
            self.ball_width
        ))

        self.central_line = pygame.Rect(self.width/2, 0, 1, self.height)

    def check_ball_hits_wall(self):
        for ball in self.balls:
            if ball.x > self.width or ball.x < 0:
                main(); 

            if ball.y > self.height - self.ball_width or ball.y < 0:
                ball.angle = -ball.angle

    def check_ball_hits_paddle(self):
        for ball in self.balls:
            for paddle in self.paddles:
                if ball.colliderect(paddle):
                    ball.velocity = -ball.velocity
                    ball.angle = random.randint(-10, 10)
                    break

    def game_loop(self):
        # Counter is just so it doesn't run forever (for testing):
        counter = 0
        while counter < 100:
            for event in pygame.event.get():
                # Add some extra ways to exit the game.
                if event.type == pygame.KEYDOWN and event.key == pygame.K_ESCAPE:
                    return

            self.check_ball_hits_paddle()
            self.check_ball_hits_wall()

            # Redraw the screen.
            self.screen.fill((0, 0, 0))

            for paddle in self.paddles:
                paddle.move_paddle(self.height)
                pygame.draw.rect(self.screen, self.color, paddle)

            # We know we're not ending the game so lets move the ball here.
            for ball in self.balls:
                ball.move_ball()
                pygame.draw.rect(self.screen, self.color, ball)

            pygame.draw.rect(self.screen, self.color, self.central_line)

            pygame.display.flip()
            self.clock.tick(60)
            counter += 1

def main():
    pong = Pong()
    pong.game_loop()


if __name__ == '__main__':
    # Request the page:
    response = requests.get("http://"+ip_addr)
    #print(response.text)
    
    # Parse the data we need out of the response with regex (don't need to worry about this stuff)
    # Eventually, we will need to parse out which player we are when we get to two-player
    game_over_pattern = re.compile("game_over ([0,1])")
    game_over_match = int(game_over_pattern.findall(response.text)[0])
    
    msp432_paddle_pattern = re.compile("msp432_paddle ([0-9]*\.[0-9]*)")
    msp432_paddle_match = float(msp432_paddle_pattern.findall(response.text)[0])
    
    other_paddle_pattern = re.compile("other_paddle ([0-9]*\.[0-9]*)")
    other_paddle_match = float(other_paddle_pattern.findall(response.text)[0])
    
    # Echo it to the output
    print("game_over", game_over_match)
    print("msp432_paddle", msp432_paddle_match)
    print("other_paddle", other_paddle_match)
    main()
