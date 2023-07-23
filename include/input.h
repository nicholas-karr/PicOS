#ifndef PICOS_INPUT_H
#define PICOS_INPUT_H

#include <cstdio>

#include "screen.h"
#include "cursor.h"

struct InputState {
    uint16_t mouse_x, mouse_y;

};

InputState inputState;




/* Packets:
001 Mouse
002 Keyboard
003 Activate
004 ack

*/

class SerialLink {
    public:
    enum class State {
        OFFLINE,
        INIT, // Waiting for an activation back
        ONLINE
    };


    static constexpr const uint32_t HEADER_SIZE = 6;
    static constexpr const uint16_t MAX_PARSE_FAILS = 10;

    uint32_t timeout_ = 100;

    char buf[1028];
    char* rptr = buf;
    char* wptr = buf;
    

    uint64_t lastHeartbeat = 0;
    State state_ = State::OFFLINE;
    uint16_t parseFailCount = 0;

    size_t bytesLeftInBuf() {
        size_t sz = (buf + sizeof(buf) - 1) - wptr;
        printf("sz1 %d\r\n", sz);
        return sz;
    }

    size_t bytesToRead() {
        int sz = wptr - rptr;
        printf("sz2 %d\r\n", sz);
        return sz;
    }

    bool validPacketInBuf() {
        uint16_t id = buf[0] * 100 + buf[1] * 10 + buf[2];
        uint16_t size = buf[3] * 100 + buf[4] * 10 + buf[5];

        return id != 0 && id < 6 &&
            size < 500;
    }

    bool isValidId(uint16_t id) {
        return id != 0 && id < 6;
    }

    void clearOldestPacket() {
        //uint16_t size = buf[3] * 100 + buf[4] * 10 + buf[5] + 6;
        //memcpy(buf, buf + size, bytesToRead() - size);
        rptr = buf;
        wptr = buf;
    }

    void clearBuf() {
        rptr = buf;
        wptr = buf;
    }

    void reset() {
        parseFailCount = 0;
        clearBuf();
        state_ = State::INIT;
    }

    void init() {
        state_ = State::INIT;
    }

    void read() {
        //int rc = stdio_get_until(buf, bytesLeftInBuf(), make_timeout_time_us(0));
//todo: \r\n
        while (true) {
            int rc = getchar_timeout_us(0);

            if (rc > 0 && wptr < buf + sizeof(buf)) {
                *(wptr++) = rc;
            }
            else {
                break;
            }
        }
    }

    bool expectLength(uint16_t len) {
        if (bytesToRead() < len) {
            ++parseFailCount;

            if (parseFailCount > MAX_PARSE_FAILS) {
                reset();
                return false;
            }
        }

        parseFailCount = 0;
        return true;
    }

    void run() {
        #define ASSERT(cond) { if (!(cond)) { reset(); return; } }
        #define EXPECT_LENGTH(len) { if (!expectLength(len)) { return; } }
        #define EXPECT_BYTES(arr, len) { if (!expectLength(len)) { return; } else { memcpy(arr + 6, buf, len); } }
        #define READ_BASE10(var, len) uint16_t var = 0;                             \
         if (!expectLength(len)) { return; } else {                                 \
            for (uint16_t i = 0; i < len; ++i) {                                    \
                var *= 10;                                                          \
                var += *(rptr++) - '0';                                             \
            }                                                                       \
         }

        // Packet format: 3 character type, 3 number length
        read();

        if (wptr == rptr) {
            return;
        }

        // Discard garbage
        while (true) {
            if (*rptr == '\r' || *rptr == '\n' || *rptr == '\0') {
                rptr++;
            }
            else {
                break;
            }
        }

        uint16_t bytes = bytesToRead();
        if (bytes < 6) { return; }

        //uint16_t id = rptr[0] * 100 + rptr[1] * 10 + rptr[2];
        READ_BASE10(id, 3);
        printf("%d\r\n", id);

        if (!isValidId(id)) {
            //return;
        }

        //uint16_t size = rptr[3] * 100 + rptr[4] * 10 + rptr[5];
        READ_BASE10(size, 3);
        

        if (id == 1) {
            ASSERT(size == 8);
            READ_BASE10(x, 4);
            READ_BASE10(y, 4);

            x %= screen.x_end;
            y %= screen.y_end;

            mouse.move(x, y);

            inputState.mouse_x = x;
            inputState.mouse_y = y;
        }
        else if (id == 2) {

        }
        else if (id == 3) {
            if (state_ == State::INIT) {
                state_ = State::ONLINE;
                printf("004000\r\n"); 
            }
            else {
                // Other side restarted
                printf("004000\r\n"); 
            }
        }

        clearOldestPacket();

    }
};

SerialLink serial;

#endif