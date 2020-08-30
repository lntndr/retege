/*  Copyright 2020 Andrea Lanterna
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "retege.h"

retege::retege() {
    baseTime = 5000;
    reason = 3;
    index = 1;
    stripNbr = 6;
    stripDrt = 2000;
}

unsigned long retege::updatePrivate(unsigned long target, int val) {
    unsigned long u = target;
    if ((long)target+val < 1) {
        u = 1;
    } else {
        u = target+val;
    }
    return u;
}

int retege::updatePrivate(int target, int val) {
    return target+val;
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
            eval = baseTime*(pow((float)2,((float)stripNbr)/((float)reason)));
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
unsigned long retege::getStripNbr() {
    return stripNbr;
}
unsigned long retege::getStripDrt() {
    return stripDrt;
}
char retege::getIndex() {
    return index;
}
void retege::updateBaseTimeSeconds(bool plus, bool minus) {
    if (plus) baseTime = updatePrivate(baseTime,1000);
    if (minus) baseTime = updatePrivate(baseTime,-1000);
    if (baseTime >= 99900) baseTime = 99900;
}
void retege::updateBaseTimeTenths(bool plus, bool minus) {
    if (plus) baseTime = updatePrivate(baseTime,100);
    if (minus) baseTime = updatePrivate(baseTime,-100);
    if (baseTime >= 99900) baseTime = 99900;
}
void retege::updateIndex(bool plus, bool minus) {
    if (plus) index = updatePrivate(index,1);
    if (minus) index = updatePrivate(index,-1);
}
void retege::updateReason(bool plus, bool minus) {
    if (plus) reason = updatePrivate(reason,1);
    if (minus) reason = updatePrivate(reason,-1);
    if (reason >= 24) reason = 24;
}
void retege::updateStripDuration(bool plus, bool minus) {
    if (plus) stripDrt = updatePrivate(stripDrt,100);
    if (minus) stripDrt = updatePrivate(stripDrt,-100);
    if (stripDrt >= 50000) baseTime = 50000;
}
void retege::updateStripNumber(bool plus, bool minus) {
    if (plus) stripNbr = updatePrivate(stripNbr,1);
    if (minus) stripNbr = updatePrivate(stripNbr,-1);
    if (stripNbr >= 24) stripNbr = 24;
}
String retege::getLCD162SetupString(char runningMode) {
    String s = String();
    switch (runningMode) {
        case 0: // LINEAR_EXPOSURE
            s = String((float)baseTime/1000.,1);
            break;
        case 1: // GEOMETRIC_EXPOSURE
            if (index >= 0) {
                s = String((float)baseTime/1000.,1)+
                    "+"+String(index)+"/"+String(reason)+
                    "="+String(float(evaluateTime(runningMode))/1000.,1);
            } else {
                s = String((float)baseTime/1000.,1)+
                    String(index)+"/"+String(reason)+
                    "="+String(float(evaluateTime(runningMode))/1000.,1);
            }
            break;
        case 2: // LINEAR_TEST
            s = String((float)baseTime/1000.,1)+
                "+["+String(stripNbr)+"*"+String((float)stripDrt/1000,1)+"s]";
            break;
        case 3: // GEOMETRIC_TEST
            s = String((float)baseTime/1000.,1)+
                "+["+String(stripNbr)+"*1/"+String(reason)+"]";
            break;
        default:
            break;
    }
    return s;
}