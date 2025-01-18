/* @INCLUDE */
#include <avr/io.h>
#include <avr/interrupt.h>

/* @HEADER */
#ifndef SMM_NO_SWITCH
class Switch;

#ifdef SMM_ARCH_TEENSY4
#define READ_PIN digitalReadFast
#else
#define READ_PIN digitalRead
#endif

#ifndef SMM_SWITCHES_POLL_RATE
// default: poll every 1ms
#define SMM_SWITCHES_POLL_RATE 1000
#endif

class SwitchInterruptManager {
  public:
  static void Setup();
  static bool SetupDone;
  static void SetPollRate(unsigned long us);
  static void AddSwitch(Switch *b);
  static void Poll();
  private:
  static Switch * list;
  #ifdef SMM_ARCH_TEENSY4
  static IntervalTimer timer;
  #endif
};


/** @brief base switch class
 *
 * This is a pure virtual class, so you *must* implement a child class before using it. Typical usage
 * looks like this:
 *
 * ```
 * class CounterButton : public smm::Switch {
 *     public:
 *     int count;
 *     CounterButton() : smm::Switch(10), count(0) {}
 *     void onLow() {
 *         // press
 *         count += 1;
 *     }
 *     void onHigh() {
 *         // release, ignore
 *     }
 * } counter;
 * ```
 *
 * Thanks to the auto-registration of the base class's constructor, the `counter` object now is bound
 * to a PCINT interrupt and will immediately respond to changes on Arduino pin 10, with no need
 * to poll.
 *
 * Note that it is good practice to keep your `onLow` and `onHigh` functions as short and simple as possible,
 * as they may interrupt *any* part of your code. If you have performace- or timing-critical sections of code,
 * you can disable interrupts with the `cli()` function and re-enable them with the `sei()` function,
 * but your switches will not be able to respond during these sections of code. (Of course, a polled switch
 * wouldn't be able to respond during said sections either...)
 *
 * As with the SerialController class, the auto-registration system for smm::Switch uses static member variables.
 * This means that simply including this code will result in increased memory usage.
 * If you are not using it and wish to conserve RAM, you can disable it by defining `SMM_NO_SWITCH`
 * before including `smm.h`.
 */
class Switch {
  protected:
  friend class SwitchInterruptManager;
  volatile uint8_t pin;
  volatile uint16_t state;
  volatile Switch *next;

  void addSwitch(Switch *b);
  void update();

  public:
  /** @brief (constructor)
   *
   * @param pin  The Arduino pin to monitor
   * @param debounceTime  The time in milliseconds to debounce after a state change
   */
  Switch(int pin, bool pullUp, bool defaultState);

  /** @brief pure virtual function, called when going LOW */
  virtual void onLow() = 0;
  /** @brief pure virtual function, called when going HIGH */
  virtual void onHigh() = 0;
};
#endif



/* @IMPLEMENTATION */
#ifndef SMM_NO_SWITCH
/* smm::Switch implementation */

// SwitchInterruptManager static members
bool smm::SwitchInterruptManager::SetupDone = false;
smm::Switch * smm::SwitchInterruptManager::list = nullptr;
#ifdef SMM_ARCH_TEENSY4
static IntervalTimer smm::SwitchInterruptManager::timer;
#endif


#define DEBUG(msg) Serial.println(msg); for(int i=0; i<100; i++) {}


void smm::SwitchInterruptManager::Setup() {
  if (SetupDone) { return; }
  Serial.begin(9600);
  delay(200);
  DEBUG("a");
  SetupDone = true;
  #ifdef SMM_ARCH_TEENSY4
    timer.begin(Poll, SMM_SWITCHES_POLL_RATE);
  #else
    // TODO: other architectures
    Serial.println(
      "\n\n\n\n\n\n\n\n\n\n!! WARNING !!\n"
      "This architecture is not currently supported by smm::Switch!"
    );
  #endif
  DEBUG("b");
}
void smm::SwitchInterruptManager::SetPollRate(unsigned long us) {
  #ifdef SMM_ARCH_TEENSY4
    timer.update(us);
  #else
    // TODO: other architectures
    Serial.println(
      "\n\n\n\n\n\n\n\n\n\n!! WARNING !!\n"
      "This architecture is not currently supported by smm::Switch!"
    );
  #endif
}
void smm::SwitchInterruptManager::AddSwitch(Switch *b) {
  // Setup();
  if (list == nullptr) {
    list = b;
  } else {
    list->addSwitch(b);
  }
}
void smm::SwitchInterruptManager::Poll() {
  if (list != nullptr) {
    list->update();
  }
}


void smm::Switch::addSwitch(Switch *b) {
  if (next != nullptr) {
    next->addSwitch(b);
  }
  else {
    next = b;
  }
}
void smm::Switch::update() {
  state = (state << 1) | READ_PIN(pin) | 0xe000;
  if (state == 0xf000) {
    // going low
    onLow();
  } else if (state == 0xefff) {
    // going high
    onHigh();
  }
  if (next != nullptr) {
    next->update();
  }
}
smm::Switch::Switch(int pin, bool pullUp=true, bool defaultState=false) : pin(pin) {
  next = nullptr;
  if (defaultState) {
    state = 0xffff;
  } else {
    state = 0;
  }
  if (pullUp) {
    pinMode(pin, INPUT_PULLUP);
  }
  else {
    pinMode(pin, INPUT);
  }
  SwitchInterruptManager::AddSwitch(this);
}
#endif


/* @SETUP */
  #ifndef SMM_NO_SWITCH
  smm::SwitchInterruptManager::Setup();
  #endif
/* @END */
