
Copy the code files with extensions ".cc" in scratch folder of installed ns-3.16 directory.

Q1:a) Compilation and Execution Steps:

		Build the script:
			./waf
		Execute the script:
			./waf --run "scratch/first_ques(a) Y"	 (Y is the delay, the values taken are mentioned in the report)
	Files generated:
    first_modified.tr

Q1:b) Compilation and Execution Steps:

		Build the script:
			./waf
		Execute the script:
			./waf --run "scratch/first_ques(b) Y"	 (Y is the delay, the values taken are mentioned in the report)
	Files generated:
    q1.tr-0-0, q1.tr-1-0

Q2:Compilation and Execution Steps:

		Build the script:
			./waf
		Execute the script:
			./waf --run "scratch/second_ques"
	Files generated:
		loss.txt,q2.tr
		

Q3:(a) & (b)
Compilation and Execution Steps:
		Build the script:
			./waf
		Execute the script:
			./waf --run "scratch/third(ab)"	 
	Files generated:
		port1.dat
		port2.dat
		port3.dat
		port4.dat
		port5.dat
		queuesize.dat

Q3:(c)
Compilation and Execution Steps:
	Build the script:
		./waf
	Execute the script:
		./waf --run "scratch/third(c)"
	Files generated:
		Cwnd.dat

Q4:Compilation and Execution Steps:
	Build the script:
		./waf
	Execute the script:
		./waf --run "scratch/fourth_ques Y" (Y specifies the dealy value for link 2.Start from Y= 10ms, gothrough Y = 30ms by increasing Y by 2ms each time.)

	Files generated:
		fourth1.plot
		fourth2.plot
