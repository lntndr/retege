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