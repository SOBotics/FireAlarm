result=1
while [ $result -ne 0 ]; do
    valgrind ./firealarm
    result=$?
done
