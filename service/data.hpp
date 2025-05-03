struct SPEED {
    int current = 0;
    int avg = 0;
    int max = 0;
};

struct RPM {
    int current = 0;
    int avg = 0;
    int max = 0;
};

struct POWER {
    int current = 0;
    int avg = 0;
    int max = 0;
};

struct THROTTLE {
    int current = 0;
    int avg = 0;
    int max = 0;
};

struct DATA {
    SPEED speed;
    RPM rpm;
    POWER power;
    THROTTLE throttle;
};

inline DATA data;