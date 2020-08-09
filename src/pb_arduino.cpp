#include "pb_arduino.h"
#include <Print.h>
#include <Stream.h>

// TODO also work for things like the teensy? or better to use other crc
// library then / copy implementation here? use preprocessor to assert avr or
// something?
#include <util/crc16.h>

//#include "Arduino.h"
// TODO preprocessor flags to exclude crc calc?

static bool pb_print_write(pb_ostream_t *stream, const pb_byte_t *buf, size_t count) {
    Print* p = reinterpret_cast<Print*>(stream->state);
    size_t written = p->write(buf, count);
    return written == count;
};

pb_ostream_s as_pb_ostream(Print& p) {
    return {pb_print_write, &p, SIZE_MAX, 0};
};

uint16_t _init_crc = 0xFFFF;
uint16_t curr_crc = _init_crc;
void init_crc() {
    curr_crc = _init_crc;
}

bool crc_good(uint16_t target_crc) {
    // TODO should i change my implementation so this actually is the final
    // check? (i.e. compute crc here on message modified by orig crc or with
    // [modified?] orig crc at end?)
    //return curr_crc == 0;
    
    //Serial.print("curr_crc: ");
    //Serial.println(curr_crc);
    return target_crc == curr_crc;
}

static bool pb_stream_read(pb_istream_t *stream, pb_byte_t *buf, size_t count) {
    Stream* s = reinterpret_cast<Stream*>(stream->state);
    size_t written = s->readBytes(buf, count);

    uint8_t byte_i;
    for (uint8_t i=0; i<written; i++) {
        byte_i = *(buf + i);

        /*
        Serial.print("i=");
        Serial.print(i);
        Serial.print(", byte_i=");
        Serial.println(byte_i, HEX);
        */

        curr_crc = _crc_xmodem_update(curr_crc, byte_i);
    }

    return written == count;
};

pb_istream_s as_pb_istream(Stream& s) {
#ifndef PB_NO_ERRMSG
    return {pb_stream_read, &s, SIZE_MAX, 0};
#else
    return {pb_stream_read, &s, SIZE_MAX};
#endif
};
