/** @file smm.h */
#pragma once

/*******************************************************************************
 *
 * smm.h
 *
 * This is an automatically generated file - please do not edit it directly!
 *
 *******************************************************************************
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace smm {

  void setup();


/* determine current architecture */
#if defined(__AVR_ATmega640__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
	/* arduino mega */
#	define SMM_ARCH_MEGA

#elif defined(__AVR_ATmega48A__) || defined(__AVR_ATmega48PA__) || \
	defined(__AVR_ATmega88A__) || defined(__AVR_ATmega88PA__) || \
	defined(__AVR_ATmega168A__) || defined(__AVR_ATmega168PA__) || \
	defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)
	/* arduino uno (and other things too?) */
#	define SMM_ARCH_UNO

#elif defined(__IMXRT1062__)
#    define SMM_ARCH_TEENSY4

#else
	/* other arduinos */
#	define SMM_ARCH_UNKNOWN

#endif




/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * smm::map
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

/** @brief a stack-allocation only map datatype
 *
 * This class is intended for small (n<50) collections, and the algorithms in it
 * reflect that. They do nothing fancy and they perform precisely zero heap allocation.
 *
 * @tparam Key  The datatype to use for keys
 * @tparam T    The datatype to use for values
 * @tparam sz   The maximum number of elements the map can hold
 */
template <typename Key, typename T, size_t sz>
class map {
	protected:
	size_t m_size;
	Key m_key[sz];
	T m_value[sz];

	int get_index(const Key& key) {
		// for small datasets (like the n~10 ones this object is intended to operate on)
		// a linear search should be good enough and probably better than the red-black
		// trees that the libcxx implementation uses
		// sort+binary search might be worth it at some point?
		for (int i=0; i<m_size; i++) {
			if (m_key[i] == key) {
				return i;
			}
		}
		return -1;
	}

	public:
	map() {
		m_size = 0;
	}

	/** @brief get a reference to an element via a key
	 *
	 * @param key  The key to get an associated value for
	 * @returns  If the key is already present, a reference to the existing value is returned.
	 *           Otherwise, a reference to a new element will be returned.
	 *
	 * @warning  When a map is full, the return value of this function for new keys is undefined!
	 */
	T& operator[](const Key& key) {
		int i = get_index(key);
		if (i < 0) {
			// no matching key, need to create new
			if (m_size >= sz) {
				// container is full, cannot return new reference
				// do NOT use this function if the container is full!!
				// any keys not already in the map will overwrite whatever is in
				// position 0
				return m_value[0];
			}

			i = m_size;
			m_size += 1;

			m_key[i] = key;
			return m_value[i];
		}
		else {
			return m_value[i];
		}
	}

	/** @brief check if a key is present in the map
	 *
	 * @param key  The key to check
	 * @returns  `true` if there is an associated value in the map, and `false` otherwise.
	 */
	bool contains(const Key& key) {
		int i = get_index(key);
		if (i < 0) {
			// no matching key found
			return false;
		}
		else {
			return true;
		}
	}

	/** @brief remove a key-value pair from the map
	 *
	 * @param key  Key identifying the pair to remove
	 * @returns  1 if a pair was removed and 0 otherwise.
	 */
	size_t erase(const Key& key) {
		int i = get_index(key);
		if (i < 0) {
			// no matching key found, ignore
			return 0;
		}
		else {
			size_t count = m_size - i - 1;
			memmove(m_key+i, m_key+i+1, sizeof(Key) * count);
			memmove(m_value+i, m_value+i+1, sizeof(T) * count);
			m_size -= 1;
			return 1;
		}
	}

	/** @brief check if the map is empty
	 *
	 * @returns  `true` if there are no key-value pairs in the map and `false` otherwise.
	 */
	bool empty() {
		return m_size == 0;
	}

	/** @brief get the number of key-value pairs in the map
	 *
	 * @returns  The number of key-value pairs in the map
	 */
	size_t size() {
		return m_size;
	}

	/** @brief get the maximum size of the map
	 *
	 * @returns  The maximum allowed numver of key-value pairs for this map.
	 */
	size_t max_size() {
		return sz;
	}

	/** @brief check if the map is full
	 *
	 * @returns  `true` if the number of key-value pairs is equal to the maximum and `false` otherwise.
	 */
	bool full() {
		return m_size == sz;
	}
};





/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * smm::string
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

/** @brief a stack-only string class
 *
 * @tparam sz  The maximum size, including terminating null character, of the string
 */
template <size_t sz>
class string {
	protected:
	char m_str[sz];

	public:
	/** @brief (constructor)
	 *
	 * Create an empty string. Guaranteed to be null-terminated.
	 */
	string() {
		memset(m_str, 0, sz);
	}

	/** @brief (constructor)
	 *
	 * Set the string to an initial value. If the value is too long, it will be truncated
	 * and null-terminated.
	 *
	 * @param s  The data to copy into the new string object.
	 */
	string(const char *s) {
		memset(m_str, 0, sz);
		operator=(s);
	}

	/** @brief set the string value
	 *
	 * If the value is too long, it will be truncated and null-terminated.
	 *
	 * @param s  The value to set the string to.
	 */
	void operator=(const char* s) {
		strncpy(m_str, s, sz-1);
	}

	/** @brief get a C-style representation of the string
	 *
	 * @returns  A `const char *` pointer to the internal buffer of the string.
	 */
	const char *c_str() {
		return reinterpret_cast<const char*>(m_str);
	}

	/** @brief append a character to the string
	 *
	 * If this would cause the string to become too long to be null-terminated within
	 * the space available, this function will do nothing.
	 *
	 * @param ch  The character to append
	 */
	void push_back(char ch) {
		if (size() + 1 > sz) {
			// not enough space to store an additional character
			return;
		}
		strncat(m_str, &ch, 1);
	}

	/** @brief get the length of the string
	 *
	 * @returns  The total length of the string, including null terminator.
	 */
	size_t size() {
		return strlen(m_str) + 1;
	}

	/** @brief get the maximum size of the string
	 *
	 * @returns  The maximum possible size of the string, including null terminator.
	 */
	size_t max_size() {
		return sz;
	}

	/** @brief compare string equality
	 *
	 * @param lhs  An `smm::string` object
	 * @param rhs  A C-style string
	 * @returns  `true` if the two strings are exactly equal, and `false` otherwise.
	 */
	friend bool operator==(const string& lhs, const char *rhs) {
		return strcmp(lhs.m_str, rhs) == 0;
	}

	/** @brief compare string inequality
	 *
	 * @param lhs  An `smm::string` object
	 * @param rhs  A C-style string
	 * @returns  `true` if the two strings are not equal, and `false` otherwise.
	 */
	friend bool operator!=(const string& lhs, const char *rhs) { return !(lhs == rhs); }

	/** @brief compare string equality
	 *
	 * @param lhs  An `smm::string` object
	 * @param rhs  Another `smm::string` object
	 * @returns  `true` if the two strings contain identical data, and `false` otherwise.
	 */
	friend bool operator==(const string& lhs, const string& rhs) { return lhs == rhs.m_str; }

	/** @brief compare string inequality
	 *
	 * @param lhs  An `smm::string` object
	 * @param rhs  Another `smm::string` object
	 * @returns  `true` if the two strings do not contain identical data, and `false` otherwise.
	 */
	friend bool operator!=(const string& lhs, const string& rhs) { return !(lhs == rhs.m_str); }
};




/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * smm::SerialController
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

#ifndef SMM_SERIAL_KEY_LEN
#define SMM_SERIAL_KEY_LEN 128
#endif

#ifndef SMM_SERIAL_VAL_LEN
#define SMM_SERIAL_VAL_LEN 128
#endif

#ifndef SMM_SERIAL_MAX_CALLBACKS
#define SMM_SERIAL_MAX_CALLBACKS 8
#endif


typedef void (*voidCallback)();
typedef void (*stringCallback)(const char *);
typedef void (*intCallback)(int);
typedef void (*floatCallback)(float);


/** @brief Internal helper structure for storing serial callback functions
 *
 * This structure abstracts the different callbacks so that one interface
 * can call many types of function seamlessly.
 */
struct SerialCallback {
	union {
		voidCallback v;
		stringCallback s;
		intCallback i;
		floatCallback f;
	} callback;
	enum {
		NONE, VOID_T, STRING, INT, FLOAT,
	} type;


	SerialCallback()                  { type=NONE;                  }
	SerialCallback(voidCallback cb)   { type=VOID_T; callback.v=cb; }
	SerialCallback(stringCallback cb) { type=STRING; callback.s=cb; }
	SerialCallback(intCallback cb)    { type=INT;    callback.i=cb; }
	SerialCallback(floatCallback cb)  { type=FLOAT;  callback.f=cb; }

	/** @brief Call the underlying callback
	 *
	 * The value of `value` will be converted from a string into whatever
	 * type is appropriate for the underlying function pointer.
	 *
	 * @param value  Value to pass to the callback function.
	 */
	void operator()(const char *value) {
		switch(type) {
		case VOID_T:
			callback.v();
			break;

		case STRING:
			callback.s(value);
			break;

		case INT:
			callback.i(strtol(value, NULL, 0));
			break;

		case FLOAT:
			callback.f(strtod(value, NULL));
			break;

		default:
			break;
		}
	}
};


#ifndef SMM_NO_SERIAL_CONTROLLER
/** @brief human- and machine-readable serial communication protocol
 *
 * A global instance of this class named `SmmSerial` is included in `SMM_IMPLEMENTATION`,
 * so in most circumstances you should not need to create your own.
 *
 * You should create and register callbacks for your SerialController with the @link SERIAL_CALLBACK SERIAL_CALLBACK @endlink macro.
 *
 * If you don't need this class, you can disable it by defining the macro `SMM_NO_SERIAL_CONTROLLER`
 * before including `smm.h`. This may save you memory because it uses static variables.
 */
class SerialController {
	public:
	typedef string<SMM_SERIAL_KEY_LEN> key_string;
	typedef string<SMM_SERIAL_VAL_LEN> val_string;

	protected:
	static int s_numCallbacks;
	static const char * s_key[SMM_SERIAL_MAX_CALLBACKS];
	static SerialCallback * s_cb[SMM_SERIAL_MAX_CALLBACKS];

	static int FindCallback(const char *k) {
		for (int i=0; i<s_numCallbacks; i++) {
			if (strcmp(k, s_key[i]) == 0) {
				return i;
			}
		}

		return -1;
	}

	key_string m_key;
	unsigned int m_keyOverflow;
	val_string m_val;
	unsigned int m_valOverflow;

	enum {
		WAIT_FOR_PACKET,
		READ_KEY,
		READ_VAL,
	} state;

	void eatPacketChar(char c) {
		switch (c) {
		case '{':
			state = READ_KEY;
			break;
		default:
			// ignore
			break;
		}
	}

	void eatKeyChar(char c) {
		switch(c) {
		case '{':
			// start new packet
			reset();
			state = READ_KEY;
			break;
		case '}':
			// done too early, wait for new packet
			reset();
			break;
		case ':':
			// key finished, read value
			state = READ_VAL;
			break;
		default:
			if (m_key.size() < m_key.max_size()) {
				m_key.push_back(c);
			}
			else {
				m_keyOverflow += 1;
			}
			break;
		}
	}

	void eatValChar(char c) {
		switch(c) {
		case '{':
			// start new packet
			reset();
			state = READ_KEY;
			break;
		case '}':
			// done! 
			ExecuteCallback(m_key.c_str(), m_val.c_str());
			reset();
			break;
		case ':':
			// ??? this should not be here, reset
			reset();
			break;
		default:
			if (m_val.size() < m_val.max_size()) {
				m_val.push_back(c);
			}
			else {
				m_valOverflow += 1;
			}
			break;
		}
	}


	void reset() {
		state = WAIT_FOR_PACKET;
		m_key = "";
		m_val = "";
		m_keyOverflow = 0;
		m_valOverflow = 0;
	}



	public:
	static bool RegisterCallback(const char *name, SerialCallback *cb) {
		int index = FindCallback(name);
		if (index < 0) {
			index = s_numCallbacks;
			s_numCallbacks += 1;
			s_key[index] = name;
			s_cb[index] = cb;
			return true;
		}
		else {
			// already have a callback with that name, return false
			return false;
		}
	}

	static void ExecuteCallback(const char *key, const char *value) {
		int i = FindCallback(key);
		if (i < 0) {
			// do error things
		}
		else {
			(s_cb[i])->operator()(value);
		}
	}

	SerialController() {
		reset();
	}

	void eatCharacter(char c) {
		switch (state) {
		case WAIT_FOR_PACKET:
			eatPacketChar(c);
			break;
		case READ_KEY:
			eatKeyChar(c);
			break;
		case READ_VAL:
			eatValChar(c);
			break;
		default:
			state = WAIT_FOR_PACKET;
			break;
		}
	}

	/** @brief get the number of callbacks currently registered
	 * @returns  The number of callbacks currently registered
	 */
	static size_t num_callbacks() {
		return s_numCallbacks;
	}

	/** @brief initialize serial communication
	 *
	 * @param baudrate  The baudrate to use when communicating.
	 */
	void begin(unsigned long baudrate=115200) {
		Serial.begin(115200);
	}

	/** @brief check for and respond to incoming communication 
	 *
	 * This function should be called fairly frequently.
	 */
	void update() {
		while (Serial.available()) {
			eatCharacter(Serial.read());
		}
	}

	/** @brief send a string-valued pair
	 *
	 * @param key  The string key for the message
	 * @param value  The string value for the message
	 */
	void send(const char *key, char *value) {
		Serial.print("{");
		Serial.print(key);
		Serial.print(":");
		Serial.print(value);
		Serial.println("}");
	}

	/** @brief send an integer-valued pair
	 *
	 * @param key  The string key for the message
	 * @param value  The integer value for the message
	 */
	void send(const char *key, int value) {
		char v[SMM_SERIAL_VAL_LEN];
		snprintf(v, SMM_SERIAL_VAL_LEN, "%d", value);
		send(key, v);
	}

	/** @brief send an float-valued pair
	 *
	 * @param key  The string key for the message
	 * @param value  The float value for the message
	 */
	void send(const char *key, float value) {
		char v[SMM_SERIAL_VAL_LEN];
		snprintf(v, SMM_SERIAL_VAL_LEN, "%f", value);
		send(key, v);
	}
};

#define SERIAL_CAT_(x, y) x##y
#define SERIAL_CAT(x, y) SERIAL_CAT_(x, y)
#define SERIAL_ANONYMOUS(prefix) SERIAL_CAT(prefix, __LINE__)
#define SERIAL_CALLBACK_REGISTER(cb, var, f, name) \
static smm::SerialCallback cb(f); \
static const bool var = SmmSerial.RegisterCallback(name, &cb);

#define SERIAL_CALLBACK_(f, name, arg) \
void f(arg); \
SERIAL_CALLBACK_REGISTER(SERIAL_ANONYMOUS(SERIAL_CALLBACK_CB_), SERIAL_ANONYMOUS(SERIAL_CALLBACK_VAR_), f, name); \
void f(arg)

/** @brief Create a new serial callback
 *
 * The basic usage of this macro is like this:
 * ```
 * SERIAL_CALLBACK("my-callback", const char *str) {
 *     Serial.print("received string '");
 *     Serial.print(str);
 *     Serial.println("'");
 * }
 * ```
 *
 * There are four kinds of callbacks: void, string (shown above), int, and float. To define them,
 * you simply need to provide the appropriate datatype in the `arg` field. Note that for void
 * callbacks, you still need to put a `void` in the arg field, like this:
 *
 * ```
 * SERIAL_CALLBACK("some-void-callback", void) {
 *     // do something
 * }
 * ```
 *
 * This macro is *self-registering*. That is, when you create a serial callback this way, it is now
 * completely set up with `SmmSerial` and will respond to incoming messages on the name you gave; 
 * all you have to do is call `SmmSerial.begin()` in your setup function and `SmmSerial.update()` in 
 * your loop function.
 */
#define SERIAL_CALLBACK(name, arg) SERIAL_CALLBACK_(SERIAL_ANONYMOUS(SERIAL_CALLBACK_), name, arg)

/* end ifndef SMM_NO_SERIAL_CONTROLLER */
#endif




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





/* end namespace smm */
}

/* externs */

extern smm::SerialController SmmSerial;



/*#############################################################################*
 #                                                                             #
 #                               IMPLEMENTATION                                #
 #                                                                             #
 *#############################################################################*/
#if defined(SMM_IMPLEMENTATION)

#ifndef SMM_NO_SERIAL_CONTROLLER
/* static variable declarations */
int smm::SerialController::s_numCallbacks = 0;
const char * smm::SerialController::s_key[SMM_SERIAL_MAX_CALLBACKS];
smm::SerialCallback * smm::SerialController::s_cb[SMM_SERIAL_MAX_CALLBACKS];

/* extern global */
smm::SerialController SmmSerial;
#endif



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




void smm::setup() {

  #ifndef SMM_NO_SWITCH
  smm::SwitchInterruptManager::Setup();
  #endif

}
#endif
