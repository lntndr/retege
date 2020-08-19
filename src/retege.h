#ifndef RETEGE_H
#define RETEGE_H

class retege {
    private:
        unsigned long baseTime;
        unsigned long reason;
        char index;
        unsigned long stripNbr;
        unsigned long stripDrt;
        unsigned long updatePrivate(unsigned long target, unsigned long value);
        char updatePrivate(char target, char value);

    public:
        // Constructors
        retege();

        // Methods
        unsigned long evaluateTime(char runningMode);

        unsigned long getBaseTime();
        unsigned long getReason();
        unsigned long getstripNbr();
        unsigned long getstripDrt();
        char getIndex();

        void updateBaseTimeSeconds(bool plus, bool minus);
        void updateBaseTimeTenths(bool plus, bool minus);
        void updateIndex(bool plus, bool minus);
        void updateReason(bool plus, bool minus);
        void updateStripDuration(bool plus, bool minus);
        void updateStripNumber(bool plus, bool minus);
};

#endif