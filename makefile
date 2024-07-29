test_4:
	gcc -g -Wall -Wextra -Wwrite-strings -DP=4 -DN=4096 -oprog4 prog4.c
	./prog4 2>&1 | tee output1
	./checkOutput.pl output1 4
test_8:
	gcc -g -Wall -Wextra -Wwrite-strings -DP=8 -DN=4096 -oprog4 prog4.c
	./prog4 2>&1 | tee output2
	./checkOutput.pl output2 8
test_avg:
	gcc -g -Wall -Wextra -Wwrite-strings -DP=4 -DN=16 -DTEST -oprog4 prog4.c
	./prog4
clean:
	rm output*
	rm prog4