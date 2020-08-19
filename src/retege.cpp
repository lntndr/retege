#include "retege.h"
#include <math.h>

retege::retege() {
    baseTime = 5000;
    reason = 3;
    index = 1;
    stripNbr = 6;
    stripDrt = 2;
}

unsigned long retege::updatePrivate(unsigned long target, unsigned long value) {
    unsigned long u = target;
    if ((long)target+value < 0) {
        u = 0;
    } else {
        u = target+value;
    }
    return u;
}

char retege::updatePrivate(char target, char value) {
    return target+value;
}
// Methods
unsigned long retege::evaluateTime(char runningMode) {
    unsigned long eval;
    switch (runningMode) {
        case 0: // LINEAR_EXPOSURE
            eval = baseTime;
            break;
        case 1: // GEOMETRIC_EXPOSURE
            eval = baseTime*(pow((float)2,((float)index)/((float)reason)));
            break;
        case 2: // LINEAR_TEST
            eval = baseTime+(stripNbr*stripDrt);
            break;
        case 3: // GEOMETRIC_TEST
            eval = baseTime+(pow((float)2,((float)stripNbr)/((float)reason)));
            break;
        default:
            eval = 0;
            break;
    }
    return eval;
}
unsigned long retege::getBaseTime() {
    return baseTime;
}
unsigned long retege::getReason() {
    return reason;
}
unsigned long retege::getstripNbr() {
    return stripNbr;
}
unsigned long retege::getstripDrt() {
    return stripDrt;
}
char retege::getIndex() {
    return index;
}
void retege::updateBaseTimeSeconds(bool plus, bool minus) {
    if (plus) baseTime = updatePrivate(baseTime,1000);
    if (minus) baseTime = updatePrivate(baseTime,-1000);
}
void retege::updateBaseTimeTenths(bool plus, bool minus) {
    if (plus) baseTime = updatePrivate(baseTime,100);
    if (minus) baseTime = updatePrivate(baseTime,-100);
}
void retege::updateIndex(bool plus, bool minus) {
    if (plus) index = updatePrivate(index,1);
    if (minus) index = updatePrivate(index,-1);
}
void retege::updateReason(bool plus, bool minus) {
    if (plus) reason = updatePrivate(reason,1);
    if (minus) reason = updatePrivate(reason,-1);
}
void retege::updateStripDuration(bool plus, bool minus) {
    if (plus) stripDrt = updatePrivate(stripDrt,1);
    if (minus) stripDrt = updatePrivate(stripDrt,-1);
}
void retege::updateStripNumber(bool plus, bool minus) {
    if (plus) stripNbr = updatePrivate(stripNbr,1);
    if (minus) stripNbr = updatePrivate(stripNbr,-1);
}