#include "ps2_keyboard.hpp"

/*

  GPIO::set_mode(CLOCK_PIN, GPIO::Mode::INPUT);
  GPIO::set_mode(DATA_PIN, GPIO::Mode::INPUT);

  GPIO::set_event_callback(CLOCK_PIN, [](size_t) {
    static uint64_t message = 0;
    static uint64_t nb_bit = 0;
    static uint64_t parity = 0;

    static uint64_t last_update = 0;

    if (last_update == 0) {
      // Initialise Clock
      last_update = GenericTimer::get_elapsed_time_in_ms();
    }

    if (GenericTimer::get_elapsed_time_in_ms() - last_update > 10) {
      // Too long between 2 interrupts, we reset.
      message = 0;
      nb_bit = 0;
    }

    last_update = GenericTimer::get_elapsed_time_in_ms();

    nb_bit++;
    uint64_t bit_val = (uint64_t)GPIO::read(24);

    parity += bit_val;
    message |= (bit_val << 11);
    message >>= 1;

    if (nb_bit < 11) {
      return;
    }

    nb_bit = 0;

    if (message & 0x1) {
      LOG_WARNING("Invalid start bit {:#x}", message);
    }

    if (!(message & 0x400)) {
      LOG_WARNING("Invalid stop bit {:#x}", message);
    }
    if ((((message & 0x200 >> 8) & 0x1) + (parity & 0x1)) & 0x1) {
      LOG_WARNING("Parity error {:#x} {:#x}", message, parity);
    }

    parity = 0;

    uint8_t key = (message >> 1) & 0xff;
    LOG_INFO("Read: {:#x}", key);
    message = 0;
  });

  GPIO::set_falling_edge_async_detect(CLOCK_PIN, true);

 */
