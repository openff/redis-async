g++ -std=c++11 -o async-redis async-redis.cpp -g -L/usr/local/lib -lhiredis -lev
sync 10w->25s   async-redis:10w ->  1.4s

async:
test write cmd count:100000
count:100000  steady_clock time:1477.4
system time: 1477.55

sync:
10w 25s
count:100000  steady_clock time:25479.6
system time: 25479.8
