enum Thread_Flags {
    THREAD_RUNNING = 0x1,
    THREAD_COMPLETED = 0x2,
    THREAD_SHOULD_YIELD = 0x4,
    THREAD_SHOULD_COMPLETE = 0x8,
    THREAD_RUNTIME_ERROR = 0x10
};

const int THREAD_FLAGS_TO_STOP_CYCLING = THREAD_SHOULD_YIELD | THREAD_SHOULD_COMPLETE | THREAD_RUNTIME_ERROR;

struct Lerp_Thread {
    Lerp_Thread();
    ~Lerp_Thread();

    unsigned long thread_id;
    Barrier <Lerp_Call_Record *> *context;
    Barrier <First_Class *> *return_value;  // Only filled in if completed...

    float start_time;
    float end_time;  
    float total_execution_time;

    unsigned long flags;
};

