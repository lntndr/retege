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

#ifndef RETEGE_H
#define RETEGE_H
#include <math.h>
#include <Arduino.h>

class retege {
    private:
        unsigned long baseTime;
        unsigned long reason;
        int index;
        unsigned long stripNbr;
        unsigned long stripDrt;
        unsigned long updatePrivate(unsigned long target, int val);
        int updatePrivate(int target, int val);

    public:
        // Constructors
        retege();

        // Methods
        unsigned long evaluateTime(char runningMode);

        unsigned long getBaseTime();
        unsigned long getReason();
        unsigned long getStripNbr();
        unsigned long getStripDrt();
        char getIndex();

        void updateBaseTimeSeconds(bool plus, bool minus);
        void updateBaseTimeTenths(bool plus, bool minus);
        void updateIndex(bool plus, bool minus);
        void updateReason(bool plus, bool minus);
        void updateStripDuration(bool plus, bool minus);
        void updateStripNumber(bool plus, bool minus);

        String getLCD162SetupString(char runningMode);
};

#endif