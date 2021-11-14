# 2021Keiosoftware
2021 Fall semester software engineering class

## Parallel hash insert/search/join operation results:
DATA {
  int key;
  int value;
}
DATA_SIZE = 10000000
BUCKET_SIZE = 1000000
Files name = 'R', 'S' (each 80MB)

### Case 1: pthread_mutex_lock, THREAD_SIZE = 10
Diff: 494826 us (494 ms)
### Case 2: pthread_mutex_lock, THREAD_SIZE = 1000
Diff: 562678 us (562 ms)
### Case 3: CAS_lock, THREAD_SIZE = 10
Diff: 579546 us (579 ms)
### Case 4: CAS_lock, THREAD_SIZE = 1000
Diff: 596872 us (596 ms)
