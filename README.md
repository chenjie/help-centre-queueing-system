# help-centre-queueing-system
UofT Department of Computer Science (DCS) undergraduate help centre queueing system C implementation. This is *NOT* the actual code that is running for the help centre on the second floor of Bahen.

## Getting Started

### Prerequisites

* GCC
* UNIX Shell OR Windows PowerShell

### Download source code and compile
The following instructions are presented using Bash in macOS:
```
# Change to HOME directory
$ cd ~

# Clone this repo and 'cd' into it
$ git clone https://github.com/jellycsc/help-centre-queueing-system.git
$ cd help-centre-queueing-system/

# Let's compile.
$ make
gcc -Wall -g -std=gnu99 -c helpcentre.c
gcc -Wall -g -std=gnu99 -c hcq.c
gcc -Wall -g -std=gnu99 -o helpcentre helpcentre.o hcq.o
```

### Usage
```
Usage: helpcentre config_filename [commands_filename]
```
Note: The sample course_config_file and commands_file are given in the git repo.

### Interactive Mode Example
```
$ ./helpcentre courses.config 
Welcome to the Help Centre Queuing System
Please type a command:
>help
help
quit
add_student name course
print_full_queue
print_currently_serving
print_all_queues
stats_by_course course
add_ta name
remove_ta name
give_up student_name
next ta_name [course]
>print_all_queues
CSC108: 0 in queue
CSC148: 0 in queue
CSC165: 0 in queue
CSC209: 0 in queue
>quit
```

### Batch Mode Example
```
$ ./helpcentre courses.config batch.commands 
Welcome to the Help Centre Queuing System
Please type a command:
>add_ta Stathis
>add_ta Elaine
>add_ta Huiting
>add_ta Demetres
>add_student Michelle CSC108
>add_student Jen CSC108
>add_student Paul CSC108
>add_student Diane CSC108
>add_student Steve CSC108
>sleep
>next Elaine
>next Stathis
>next Huiting CSC108
>sleep
>stats_by_course CSC108
CSC108:Introduction to Programming 
	2: waiting
	3: being helped currently
	0: already helped
	0: gave_up
	6.000000: total time waiting
	0.000000: total time helping
>remove_ta Stathis
>stats_by_course CSC108
CSC108:Introduction to Programming 
	2: waiting
	2: being helped currently
	1: already helped
	0: gave_up
	6.000000: total time waiting
	2.000000: total time helping
>remove_ta Stathis
Error: Invalid TA name.
>quit
```

## Author(s)

| Name                    | GitHub                                     | Email
| ----------------------- | ------------------------------------------ | -------------------------
| Chenjie (Jack) Ni       | [jellycsc](https://github.com/jellycsc)    | nichenjie2013@gmail.com

## Thoughts and future improvements

* Test and run this in the real environment.
* Memory leak can be a big issue in the real environment, where program runs 24/7.

## Contributing to this project

1. Fork it [![GitHub forks](https://img.shields.io/github/forks/jellycsc/help-centre-queueing-system.svg?style=social&label=Fork&maxAge=2592000)](https://github.com/jellycsc/help-centre-queueing-system/fork)
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -m 'Add some feature'`)
4. Push to your feature branch (`git push origin my-new-feature`)
5. Create a new Pull Request

Details are described [here](https://git-scm.com/book/en/v2/GitHub-Contributing-to-a-Project).

## Bug Reporting [![GitHub issues](https://img.shields.io/github/issues/jellycsc/help-centre-queueing-system.svg)](https://github.com/jellycsc/help-centre-queueing-system/issues/)

Please click `issue` button above↑ to report any issues related to this project  
OR you can shoot an email to <nichenjie2013@gmail.com>

## License
This project is licensed under GNU General Public License v3.0 - see [LICENSE](LICENSE) file for more details.
