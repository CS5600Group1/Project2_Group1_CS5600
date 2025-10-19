# Project2_Group1_CS5600
Multiple-threaded experiment about climbing the stairs.

## Cooperation

 * **MAIN**:
```C
struct tunnel{
    int step;
    int dir;
    mutex lock;
    queue upstair;
    queue downstair;
    int counter; // starving
};
void tunnelInit(Tunnel tunnel, int step);
void threadUpstair();
void threadDownstair();
void main();
```

* **TEST**
```C
void up29down1();
void up10down10();
void updown_together();
```

* **DOCUMENT**
