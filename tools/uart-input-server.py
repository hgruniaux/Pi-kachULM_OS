from pynput import mouse, keyboard
from enum import IntEnum
from time import sleep
from os import mkfifo
import errno
import time

last_mouse_position = (0, 0)

MIN_MOUSE_MOVE_TIME = 10000000 # in ns
last_mouse_move_time = 0
acc_mouse_move_dx = 0
acc_mouse_move_dy = 0

def clamp(x, min, max):
    if x < min:
        return min
    if x > max:
        return max
    return x

class EventKind(IntEnum):
    MOUSE_MOVE = 0
    MOUSE_CLICK = 1
    MOUSE_SCROLL = 2
    KEY_PRESS = 3
    KEY_RELEASE = 4

def start_event(length: int):
    pipe.write(b'EVT')
    pipe.write(length.to_bytes(length=1, byteorder='little', signed=False))

def encode_int(x: int):
    pipe.write(x.to_bytes(length=4, byteorder='little', signed=True))

def encode_event_kind(kind: EventKind):
    encode_int(kind.value)

def on_mouse_move(x: int, y: int):
    global last_mouse_position
    global last_mouse_move_time
    global acc_mouse_move_dx
    global acc_mouse_move_dy

    dx = x - last_mouse_position[0]
    dy = y - last_mouse_position[1]
    last_mouse_position = (x, y)

    if (time.perf_counter_ns() - last_mouse_move_time) < MIN_MOUSE_MOVE_TIME:
        acc_mouse_move_dx += dx
        acc_mouse_move_dy += dy
        return

    last_mouse_move_time = time.perf_counter_ns()

    dx = clamp(acc_mouse_move_dx + dx, -255, 255)
    dy = clamp(acc_mouse_move_dy + dy, -255, 255)
    acc_mouse_move_dx = 0
    acc_mouse_move_dy = 0

    # Format (3-bytes):
    #   [0-3]: 0001
    #     [4]: dx negative?
    #     [5]: dy negative?
    #   [6-7]: unused
    #  [8-15]: dx magnitude
    # [16-23]: dy magnitude
    # [24-32]: unused

    data = [ 0x01, abs(dx), abs(dy) ]
    if dx < 0:
        data[0] |= (1 << 4)
    if dy < 0:
        data[0] |= (1 << 5)

    pipe.write(bytes(data))
    pipe.flush()

python_button_to_kernel_code = {
    mouse.Button.left: 0,
    mouse.Button.middle: 1,
    mouse.Button.right: 2,
}

def on_mouse_click(x: int, y: int, button: mouse.Button, pressed: bool):
    if button not in python_button_to_kernel_code:
        # Unknown mouse button, ignore the event
        return

    # Format (2-bytes):
    #   [0-3]: 0010
    #     [4]: pressed?
    #   [5-7]: unused
    #  [8-15]: button id
    # [16-32]: unused

    data = [ 0x02, python_button_to_kernel_code[button] ]
    if pressed:
        data[0] |= (1 << 4)

    pipe.write(bytes(data))
    pipe.flush()

def on_mouse_scroll(x: int, y: int, dx: int, dy: int):
    dx = clamp(dx, -255, 255)
    dy = clamp(dy, -255, 255)

    # Format (3-bytes):
    #   [0-3]: 0011
    #     [4]: dx negative?
    #     [5]: dy negative?
    #   [6-7]: unused
    #  [8-15]: dx magnitude
    # [16-23]: dy magnitude

    data = [ 0x03, abs(dx), abs(dy) ]
    if dx < 0:
        data[0] |= (1 << 4)
    if dy < 0:
        data[0] |= (1 << 5)

    pipe.write(bytes(data))
    pipe.flush()

python_key_to_kernel_code = {
        keyboard.KeyCode.from_char('a'): 0x1C,
        keyboard.KeyCode.from_char('b'): 0x32,
        keyboard.KeyCode.from_char('c'): 0x21,
        keyboard.KeyCode.from_char('d'): 0x23,
        keyboard.KeyCode.from_char('e'): 0x24,
        keyboard.KeyCode.from_char('f'): 0x2B,
        keyboard.KeyCode.from_char('g'): 0x34,
        keyboard.KeyCode.from_char('h'): 0x33,
        keyboard.KeyCode.from_char('i'): 0x43,
        keyboard.KeyCode.from_char('j'): 0x3B,
        keyboard.KeyCode.from_char('k'): 0x42,
        keyboard.KeyCode.from_char('l'): 0x4B,
        keyboard.KeyCode.from_char('m'): 0x3A,
        keyboard.KeyCode.from_char('n'): 0x31,
        keyboard.KeyCode.from_char('o'): 0x44,
        keyboard.KeyCode.from_char('p'): 0x4D,
        keyboard.KeyCode.from_char('q'): 0x15,
        keyboard.KeyCode.from_char('r'): 0x2D,
        keyboard.KeyCode.from_char('s'): 0x1B,
        keyboard.KeyCode.from_char('t'): 0x2C,
        keyboard.KeyCode.from_char('u'): 0x3C,
        keyboard.KeyCode.from_char('v'): 0x2A,
        keyboard.KeyCode.from_char('w'): 0x1D,
        keyboard.KeyCode.from_char('x'): 0x22,
        keyboard.KeyCode.from_char('y'): 0x35,
        keyboard.KeyCode.from_char('z'): 0x1A,
        keyboard.KeyCode.from_char('0'): 0x45,
        keyboard.KeyCode.from_char('1'): 0x16,
        keyboard.KeyCode.from_char('2'): 0x1E,
        keyboard.KeyCode.from_char('3'): 0x26,
        keyboard.KeyCode.from_char('4'): 0x25,
        keyboard.KeyCode.from_char('5'): 0x2E,
        keyboard.KeyCode.from_char('6'): 0x36,
        keyboard.KeyCode.from_char('7'): 0x3D,
        keyboard.KeyCode.from_char('8'): 0x3E,
        keyboard.KeyCode.from_char('9'): 0x46,
        keyboard.KeyCode.from_char('`'): 0x0E,
        keyboard.KeyCode.from_char('-'): 0x4E,
        keyboard.KeyCode.from_char('='): 0x55,
        keyboard.KeyCode.from_char('\\'): 0x5D,
        keyboard.Key.backspace: 0x66,
        keyboard.Key.space: 0x29,
        keyboard.Key.tab: 0x0D,
        keyboard.Key.enter: 0x5A,
        keyboard.Key.esc: 0x76,
        keyboard.Key.f1: 0x05,
        keyboard.Key.f2: 0x06,
        keyboard.Key.f3: 0x04,
        keyboard.Key.f4: 0x0C,
        keyboard.Key.f5: 0x03,
        keyboard.Key.f6: 0x0B,
        keyboard.Key.f7: 0x83,
        keyboard.Key.f8: 0x0A,
        keyboard.Key.f9: 0x01,
        keyboard.Key.f10: 0x09,
        keyboard.Key.f11: 0x78,
        keyboard.Key.f12: 0x07,
        keyboard.Key.print_screen: 0xE012,
        keyboard.Key.scroll_lock: 0x7E,
        keyboard.Key.pause: 0xE013,
        keyboard.Key.insert: 0xE070,
        keyboard.Key.home: 0xE06C,
        keyboard.Key.page_up: 0xE07D,
        keyboard.Key.delete: 0xE071,
        keyboard.Key.end: 0xE069,
        keyboard.Key.page_down: 0xE07A,
        keyboard.Key.up: 0xE075,
        keyboard.Key.left: 0xE06B,
        keyboard.Key.down: 0xE072,
        keyboard.Key.right: 0xE074,
        keyboard.Key.ctrl: 0x14,
        keyboard.Key.ctrl_l: 0x14,
        keyboard.Key.ctrl_r: 0xe014,
        keyboard.Key.shift: 0x12,
        keyboard.Key.shift_l: 0x12,
        keyboard.Key.shift_r: 0x59,
        keyboard.Key.alt: 0x11,
        keyboard.Key.alt_l: 0x11,
        keyboard.Key.alt_r: 0xe011,
        keyboard.Key.alt_gr: 0xe011,
        keyboard.Key.cmd: 0xE01F,
        keyboard.Key.cmd_l: 0xE01F,
        keyboard.Key.cmd_r: 0xE027,
        keyboard.Key.menu: 0xE02F,
        keyboard.Key.caps_lock: 0x58,
        keyboard.Key.num_lock: 0x77,
        keyboard.KeyCode.from_char('.'): 0x41,
        keyboard.KeyCode.from_char(','): 0x49,
        keyboard.KeyCode.from_char('/'): 0x4A,
        keyboard.KeyCode.from_char(';'): 0x4C,
        keyboard.KeyCode.from_char('\''): 0x52,
        keyboard.KeyCode.from_char('['): 0x54,
        keyboard.KeyCode.from_char(']'): 0x5B,
    }

is_key_pressed = { key: False for key in python_key_to_kernel_code.keys() }

def on_key_press(key: keyboard.Key):
    if key not in python_key_to_kernel_code:
        # Unknown keyboard key, ignore the event
        return

    if is_key_pressed[key]:
        # Key already pressed... (remanent keys maybe)
        return

    is_key_pressed[key] = True
    code = python_key_to_kernel_code[key]

    # Format (3-bytes):
    #   [0-3]: 0100
    #     [4]: pressed? TRUE
    #   [4-7]: unused
    #  [8-23]: key code, unsigned, little endian

    pipe.write(((1 << 2) | (1 << 4)).to_bytes())
    pipe.write(code.to_bytes(2, byteorder='little', signed=False))
    pipe.flush()

def on_key_release(key: keyboard.Key):
    if key not in python_key_to_kernel_code:
        # Unknown keyboard key, ignore the event
        return

    if not is_key_pressed[key]:
        # Key not yet pressed, can not be released...
        return

    is_key_pressed[key] = False
    code = python_key_to_kernel_code[key]

    # Format (3-bytes):
    #   [0-3]: 0100
    #     [4]: pressed? FALSE
    #   [4-7]: unused
    #  [8-23]: key code, unsigned, little endian

    pipe.write((1 << 2).to_bytes())
    pipe.write(code.to_bytes(2, byteorder='little', signed=False))
    pipe.flush()

FIFO_PATH = '/tmp/uart-input'
try:
    mkfifo(FIFO_PATH)
except OSError as oe:
    print("ERROR")
    if oe.errno != errno.EEXIST:
        raise

with open(FIFO_PATH, 'wb') as pipe:
    print(f"Pipe opened at {FIFO_PATH}.")

    mouse_listener = mouse.Listener(on_move=on_mouse_move, on_click=on_mouse_click, on_scroll=on_mouse_scroll)
    mouse_listener.start()
    print("Mouse listener started.")

    keyboard_listener = keyboard.Listener(on_press=on_key_press, on_release=on_key_release)
    keyboard_listener.start()
    print("Keyboard listener started.")

    while True:
        sleep(1)

