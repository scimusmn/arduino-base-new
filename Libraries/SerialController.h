#pragma once

#ifdef UNIT_TEST
#include "tests/FakeSerial.h"
#else
#include <Arduino.h>
#endif

#include "FixedSizeString.h"
#include "LookupTable.h"

namespace smm {

    typedef void (*voidCallback)();
    typedef void (*stringCallback)(const char*);
    typedef void (*intCallback)(int);
    typedef void (*floatCallback)(float);


    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     *
     * SerialCallback
     *
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     */

    struct SerialCallback {
	SerialCallback() { valueType = NONE; }
	SerialCallback(voidCallback cb)   { valueType = VOID;   callback.v = cb; }
	SerialCallback(stringCallback cb) { valueType = STRING; callback.s = cb; }
	SerialCallback(intCallback cb)    { valueType = INT;    callback.i = cb; }
	SerialCallback(floatCallback cb)  { valueType = FLOAT;  callback.f = cb; }

	typedef enum {
	    NONE,
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

	void operator()(const char *value) {
	    switch (valueType) {
	    case NONE:
		break;
		
	    case VOID:
		callback.v();
		break;
      
	    case STRING:
		callback.s(value);
		break;

	    case INT:
		callback.i(atoi(value));
		break;

	    case FLOAT:
		callback.f(atof(value));
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

    #define NORMAL_PROTOCOL 0
    #define STELE_PROTOCOL 1

    template<int PROTOCOL=NORMAL_PROTOCOL, unsigned int MAX_CALLBACKS=16,
	     size_t MAX_KEY_LEN=32, size_t MAX_VAL_LEN=MAX_KEY_LEN>
    class SerialController {
    public:
	void setup(long baudrate=115200) {}
  
	// overloaded callback-adding functions
	void addCallback(const char *key, voidCallback cb) {
	    m_callbacks.add(key, SerialCallback(cb));
	}
	void addCallback(const char *key, stringCallback cb) {
	    m_callbacks.add(key, SerialCallback(cb));
	}
	void addCallback(const char *key, intCallback cb) {
	    m_callbacks.add(key, SerialCallback(cb));
	}
	void addCallback(const char *key, floatCallback cb) {
	    m_callbacks.add(key, SerialCallback(cb));
	}
  
	// message sending
	void sendMessage(const char *messageKey, const char *messageValue) {}
	void sendMessage(const char *messageKey) {}
	void sendMessage(const char *messageKey, int messageValue) {}
	void sendMessage(const char *messageKey, float messageValue) {}


	// updating
	void update() {
	    while(Serial.available())
		eatCharacter(Serial.read());
	}


	// size info
	size_t maxKeyLength() { return MAX_KEY_LEN; }
	size_t maxValueLength() { return MAX_VAL_LEN; }
	unsigned int maxNumCallbacks() { return MAX_CALLBACKS; }

	unsigned int numCallbacks() { return m_callbacks.size(); }

    private:
	smm::LookupTable<MAX_CALLBACKS, SerialCallback, MAX_KEY_LEN> m_callbacks;
	smm::FixedSizeString<MAX_KEY_LEN> m_key;
	smm::FixedSizeString<MAX_VAL_LEN> m_value;

	enum {
	    WAIT_FOR_START,
	    PARSE_KEY,
	    PARSE_VALUE,
	} m_state;

	
	void eatCharacter(char c) {
	    switch (m_state) {
	    case WAIT_FOR_START:
		if (c == '{')
		    m_state = PARSE_KEY;
		break;

	    case PARSE_KEY:
		if (c == '{' || c == '}')
		    // malformed input!
		    reset();
		else if (c == ':')
		    m_state = PARSE_VALUE;
		else
		    m_key.append(c);
		break;

	    case PARSE_VALUE:
		if (c == '{' || c == ':')
		    // malformed input!
		    reset();
		else if (c == '}')
		    handleMessage();
		else
		    m_value.append(c);
		break;

	    default:
		// something's gone wrong, reset
		reset();
		break;
	    }
	}

	
	void handleMessage() {
	    SerialCallback* cb = m_callbacks[m_key.c_str()];
	    if (cb == nullptr)
		unknownMessage();
	    else
		(*cb)(m_value.c_str());
	    reset();
	}


	void unknownMessage() {
	    Serial.print("{unknown-message:");
	    Serial.print(m_key.c_str());
	    Serial.println("}");
	}
	

	void reset() {
	    m_state = WAIT_FOR_START;
	    m_key.clear();
	    m_value.clear();
	}
    };
}
