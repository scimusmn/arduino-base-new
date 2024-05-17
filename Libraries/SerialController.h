#ifndef SERIAL_CONTROLLER_H
#define SERIAL_CONTROLLER_H

#include <Arduino.h>
#include <string.h>

#ifndef MAX_KEY_LEN
#define MAX_KEY_LEN 32
#endif

#ifndef MAX_VAL_LEN
#define MAX_VAL_LEN 32
#endif

#define MAX_MSG_LEN (MAX_KEY_LEN + MAX_VAL_LEN + 3)

typedef void (*voidCallback)();
typedef void (*stringCallback)(char*);
typedef void (*intCallback)(int);
typedef void (*floatCallback)(float);

struct StringF {
  int maxSize;
  int size;
  char *str;

  StringF(int maxSize) : maxSize(maxSize), size(0) {
    str = malloc(maxSize * sizeof(char));
  }

  ~StringF() { free(str); }

  void clear() {
    memset(str, 0, maxSize);
    size = 0;
  }

  void append(char *string) {
    int s = snprintf(str, maxSize, "%s%s", str, string);
    size = s >= maxSize ? maxSize-1 : s;
  }

  void append(char c) {
    if (size < maxSize-1) {
      str[size] = c;
      size++;
    }
  }

  int toInt() {
    return atoi(str);
  }

  float toFloat() {
    return atof(str);
  }

  bool equals(StringF string) {
    return strcmp(str, string.str) == 0;
  }

  bool equals(char *string) {
    return strcmp(str, string) == 0;
  }
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * SerialCallback
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

struct SerialCallback {
  StringF key;

  SerialCallback() : key(MAX_KEY_LEN) {}

  typedef enum {
    VOID,
    STRING,
    INT,
    FLOAT
  } ValueType;

  ValueType valueType;

  union {
    voidCallback v;
    stringCallback s;
    intCallback i;
    floatCallback f;
  } callback;

  void respond(StringF value) {
    switch (valueType) {
    case VOID:
      callback.v();
      break;
      
    case STRING:
      callback.s(value.str);
      break;

    case INT:
      callback.i(value.toInt());
      break;

    case FLOAT:
      callback.f(value.toFloat());
      break;

    default:
      // bad valueType, do nothing
      break;
    }
  }
};


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * SerialController
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

#ifndef SERIALCONTROLLER_MAX_CALLBACKS
#define SERIALCONTROLLER_MAX_CALLBACKS 16
#endif

// helper to define repetitious addCallback overloads
#define addCallbackDef(cbType, valType, u)				\
  void addCallback(const char *keyString, cbType callback) {		\
    SerialCallback *cb = callbacks + numCallbacks;			\
    cb->key.str = keyString;						\
    cb->valueType = SerialCallback::ValueType::valType;			\
    cb->callback.u = callback;						\
    numCallbacks++;							\
  }

class SerialController {
private:
  enum {
    WAIT_FOR_START,
    PARSE_KEY,
    PARSE_VALUE,
    N_PARSE_STATES
  } state;


  int numCallbacks;
  SerialCallback callbacks[SERIALCONTROLLER_MAX_CALLBACKS];
  StringF key;
  StringF value;
  bool steleProtocol;

  void waitForSerial(long baudrate) {
    Serial.begin(baudrate);
    while(!Serial);

    // wait for handshake if using steleProtocol
    while(!handshake && steleProtocol) {
      if (Serial.available() && Serial.read() == '{') {
	handshake = true;
	sendMessage("arduino-ready","1");
      }
    }
  }


  void lookupAndRunCallback() {
    if (steleProtocol && key.equals("wake-arduino")) {
      sendMessage("arduino-ready", "1");
      return;
    }

    for (int i=0; i<numCallbacks; i++) {
      Serial.println(callbacks[i].key.str);
      if (key.equals(callbacks[i].key)) {
	callbacks[i].respond(value);
	return;
      }
    }

    if (errorResponse) {
      sendMessage("unknown-command", key.str);
    }
  }

  
public:
  bool handshake;
  bool errorResponse;

  SerialController() : key(MAX_KEY_LEN), value(MAX_VAL_LEN) { state = WAIT_FOR_START; numCallbacks = 0; }

  void setup(long baudrate=115200, bool steleProtocol=true) {
    this->steleProtocol = steleProtocol;
    errorResponse = true;
    waitForSerial(baudrate);
  }

  
  // overloaded callback-adding functions
  addCallbackDef(voidCallback, VOID, v)
  addCallbackDef(stringCallback, STRING, s)
  addCallbackDef(intCallback, INT, i)
  addCallbackDef(floatCallback, FLOAT, f)

  
  // message sending
  void sendMessage(char *messageKey, char *messageValue) {
    char msg[MAX_MSG_LEN];
    int size = snprintf(msg, MAX_MSG_LEN, "{%s:%s}", messageKey, messageValue);
    if (size > MAX_MSG_LEN)
      Serial.println("{message-too-big:1}");
    else
      Serial.println(msg);
  }

  void sendMessage(char *messageKey) {
    sendMessage(messageKey, "1");
  }

  void sendMessage(char *messageKey, int messageValue) {
    char value[MAX_VAL_LEN];
    snprintf(value, MAX_VAL_LEN, "%d", messageValue);
    sendMessage(messageKey, value);
  }


  void sendMessage(char *messageKey, float messageValue) {
    char value[MAX_VAL_LEN];
    snprintf(value, MAX_VAL_LEN, "%f", messageValue);
    sendMessage(messageKey, value);
  }


  // updating
  void update() {
    while (Serial.available() > 0) {
      char c = Serial.read();
      switch (state) {
      case WAIT_FOR_START:
	if (c == '{') {
	  key.clear();
	  value.clear();
	  state = PARSE_KEY;
	}
	break;

      case PARSE_KEY:
	if (c == '{') {
	  state = PARSE_KEY;
	}
	else if (c == ':') {
	  state = PARSE_VALUE;
	}
	else if (c == '}') {
	  // malformed input, look for next token
	  state = WAIT_FOR_START;
	}
	else {
	  key.append(c);
	}
	break;

      case PARSE_VALUE:
	if (c == '}') {
	  lookupAndRunCallback();
	  state = WAIT_FOR_START;
	}
	else {
	  value.append(c);
	}
	break;

      default:
	// something's gone wrong, reset
	state = WAIT_FOR_START;
	break;
      }
    }
  }
};

#endif
