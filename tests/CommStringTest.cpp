/* This code is part of the CPISync project developed at Boston University.  Please see the README for use and references. */
/*
 * File:   CommStringTest.cpp
 * Author: kaets
 *
 * Created on May 18, 2018, 10:52:33 AM
 */

#include "CommStringTest.h"
#include "CommString.h"
#include "Auxiliary.h"

CPPUNIT_TEST_SUITE_REGISTRATION(CommStringTest);

CommStringTest::CommStringTest() {
}

CommStringTest::~CommStringTest() {
}

void CommStringTest::setUp() {
}

void CommStringTest::tearDown() {
}

inline int randBetween(int lower, int upper) {
    int length = (rand() % (upper + 1));
    if(length < lower) length = lower;
    return length;
}

inline byte randByte() {
    return (byte) (rand() % 256);
}
inline string randString(int lower, int upper) {
    stringstream str;

    // pick a length in between lower and upper, inclusive
    int length = randBetween(lower, upper);

    for(int jj = 0; jj < length; jj++)
        str << randByte(); // generate a random character and add to stringstream

    return str.str();
}

void CommStringTest::testGetString() {
    const int TIMES = 50;
    for(int ii = 0; ii < TIMES; ii++) {
        const int LOWER = 0;
        const int UPPER = 10;

        // b64 case
        string init = randString(LOWER, UPPER);
        CommString cs(base64_encode(init, init.length()), true);
        CPPUNIT_ASSERT_EQUAL(init, cs.getString());

        // non-b64 case
        CommString ds(init, false);
        string s = ds.getString();
        CPPUNIT_ASSERT_EQUAL(init, ds.getString());
    }
}

void CommStringTest::testGetName() {
    CommString cs;
    const string name = "CommString";
    CPPUNIT_ASSERT_EQUAL(name, cs.getName());
}

void CommStringTest::testComm(){
    const int TIMES = 50;
    for(int ii = 0; ii < TIMES; ii++) {
        CommString cs;
        const int LOWER = 0;
        const int UPPER = 10;

        cs.commConnect(); // should do nothing
        cs.commListen(); // " "

        string toSend = randString(LOWER, UPPER);
        cs.commSend(toSend.data(), toSend.length());

        CPPUNIT_ASSERT_EQUAL((long) toSend.length(), cs.getXmitBytes());

        long byteNumRecv = randBetween(LOWER, toSend.length());

        CPPUNIT_ASSERT_EQUAL(toSend.substr(0, byteNumRecv), cs.commRecv(byteNumRecv));
        CPPUNIT_ASSERT_EQUAL(byteNumRecv, cs.getRecvBytes());

        cs.commClose();
    }

}