#!/bin/sh

./scripts/build.sh

for i in `seq 1 3`; do
    timeout 2s ./build/aggregation-server > ./build/prod$i.out &
    sleep 1
    timeout 0.5s cat ./test/test$i.in | netcat -t localhost 8080 & 
    sleep 2
    diff ./build/prod$i.out ./test/test$i.out && echo "Test $i OK." || echo "Test $i failed."
done
echo "All tests completed."
