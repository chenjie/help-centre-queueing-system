#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hcq.h"
#define INPUT_BUFFER_SIZE 256

/*
 * Return a pointer to the struct student with name stu_name
 * or NULL if no student with this name exists in the stu_list
 */
Student *find_student(Student *stu_list, char *student_name) {
    Student *cur_student = stu_list;
    while (cur_student != NULL) {
        if (strcmp(cur_student->name, student_name) == 0) {
            return cur_student;
        }
        cur_student = cur_student->next_overall;
    }
    return NULL;
}



/*   Return a pointer to the ta with name ta_name or NULL
 *   if no such TA exists in ta_list.
 */
Ta *find_ta(Ta *ta_list, char *ta_name) {
    Ta *cur_ta = ta_list;
    while (cur_ta != NULL) {
        if (strcmp(cur_ta->name, ta_name) == 0) {
            return cur_ta;
        }
        cur_ta = cur_ta->next;
    }
    return NULL;
}


/*  Return a pointer to the course with this code in the course list
 *  or NULL if there is no course in the list with this code.
 */
Course *find_course(Course *courses, int num_courses, char *course_code) {
    for (int i = 0; i < num_courses; i++) {
        Course *cur_course = courses + i;
        if (strcmp(cur_course->code, course_code) == 0) {
            return cur_course;
        }
    }
    return NULL;
}


/* Add a student to the queue with student_name and a question about course_code.
 * if a student with this name already has a question in the queue (for any
   course), return 1 and do not create the student.
 * If course_code does not exist in the list, return 2 and do not create
 * the student struct.
 * For the purposes of this assignment, don't check anything about the
 * uniqueness of the name.
 */
int add_student(Student **stu_list_ptr, char *student_name, char *course_code,
    Course *course_array, int num_courses) {
    if (find_student(*stu_list_ptr, student_name) != NULL) {
        return 1;
    }
    if (find_course(course_array, num_courses, course_code) == NULL) {
        return 2;
    }

    // Create new student
    Student *new_student = malloc(sizeof(Student));
    new_student->name = malloc(strlen(student_name) + 1);
    strcpy(new_student->name, student_name);
    new_student->arrival_time = malloc(sizeof(time_t));
    *(new_student->arrival_time) = time(NULL);

    new_student->course = find_course(course_array, num_courses, course_code);
    if (new_student->course->head == NULL) {
        new_student->course->head = new_student;
        new_student->course->tail = new_student;
    } else {
        // Find last course student
        Student *cur_std = new_student->course->head;
        while (cur_std->next_course != NULL) {
            cur_std = cur_std->next_course;
        }
        cur_std->next_course = new_student;
    }

    new_student->next_overall = NULL;
    new_student->next_course = NULL;

    // Append to the end of stu_list
    Student *head = *stu_list_ptr;
    if (head == NULL) {
        *stu_list_ptr = new_student;
    } else {
        Student *cur_student = head;
        while (cur_student->next_overall != NULL) {
            cur_student = cur_student->next_overall;
        }
        cur_student->next_overall = new_student;
        if (cur_student->course == new_student->course) {
            cur_student->next_course = new_student;
        }
    }

    return 0;
}


/* Student student_name has given up waiting and left the help centre
 * before being called by a Ta. Record the appropriate statistics, remove
 * the student from the queues and clean up any no-longer-needed memory.
 *
 * If there is no student by this name in the stu_list, return 1.
 */
int give_up_waiting(Student **stu_list_ptr, char *student_name) {
    Student *target = find_student(*stu_list_ptr, student_name);
    if (target == NULL) {
        return 1;
    }
    // course.bailed
    target->course->bailed ++;
    target->course->wait_time += time(NULL) - *(target->arrival_time);

    // course.head, course.tail, next_course
    Student *cur_course_std = target->course->head;
    if (cur_course_std != NULL) {
        if (strcmp(cur_course_std->name, student_name) == 0) { // head
            target->course->head = cur_course_std->next_course;
            if (cur_course_std->next_course == NULL) { // head and tail
                target->course->tail = NULL;
            }
        } else {
            while (cur_course_std->next_course != NULL) {
                if (strcmp(cur_course_std->next_course->name, student_name) == 0) {
                    if (cur_course_std->next_course == target->course->tail) {
                        // tail deletion
                        target->course->tail = cur_course_std;
                    }
                    cur_course_std->next_course = cur_course_std->next_course->next_course;
                    break;
                }
                cur_course_std = cur_course_std->next_course;
            }
        }
    }

    // next_overall
    // Find the one ahead the target
    Student *cur_student = *stu_list_ptr;
    if (strcmp(cur_student->name, student_name) == 0) { // head
        *stu_list_ptr = cur_student->next_overall;
    } else {
        while (cur_student->next_overall != NULL) { // normal
            if (strcmp(cur_student->next_overall->name, student_name) == 0) {
                cur_student->next_overall = cur_student->next_overall->next_overall;
                break;
            }
            cur_student = cur_student->next_overall;
        }
    }
    free(target->name);
    free(target->arrival_time);
    free(target);

    return 0;
}

/* Create and prepend Ta with ta_name to the head of ta_list.
 * For the purposes of this assignment, assume that ta_name is unique
 * to the help centre and don't check it.
 */
void add_ta(Ta **ta_list_ptr, char *ta_name) {
    // first create the new Ta struct and populate
    Ta *new_ta = malloc(sizeof(Ta));
    if (new_ta == NULL) {
       perror("malloc for TA");
       exit(1);
    }
    new_ta->name = malloc(strlen(ta_name)+1);
    if (new_ta->name  == NULL) {
       perror("malloc for TA name");
       exit(1);
    }
    strcpy(new_ta->name, ta_name);
    new_ta->current_student = NULL;

    // insert into front of list
    new_ta->next = *ta_list_ptr;
    *ta_list_ptr = new_ta;
}

/* The TA ta is done with their current student.
 * Calculate the stats (the times etc.) and then
 * free the memory for the student.
 * If the TA has no current student, do nothing.
 */
void release_current_student(Ta *ta) {
    if (ta->current_student != NULL) {
        ta->current_student->course->helped ++;
        ta->current_student->course->help_time += time(NULL) - *(ta->current_student->arrival_time);

        free(ta->current_student->name);
        free(ta->current_student->arrival_time);
        free(ta->current_student);
        ta->current_student = NULL;
    }
}

/* Remove this Ta from the ta_list and free the associated memory with
 * both the Ta we are removing and the current student (if any).
 * Return 0 on success or 1 if this ta_name is not found in the list
 */
int remove_ta(Ta **ta_list_ptr, char *ta_name) {
    Ta *head = *ta_list_ptr;
    if (head == NULL) {
        return 1;
    } else if (strcmp(head->name, ta_name) == 0) {
        // TA is at the head so special case
        *ta_list_ptr = head->next;
        release_current_student(head);
        // memory for the student has been freed. Now free memory for the TA.
        free(head->name);
        free(head);
        return 0;
    }
    while (head->next != NULL) {
        if (strcmp(head->next->name, ta_name) == 0) {
            Ta *ta_tofree = head->next;
            //  We have found the ta to remove, but before we do that
            //  we need to finish with the student and free the student.
            //  You need to complete this helper function
            release_current_student(ta_tofree);

            head->next = head->next->next;
            // memory for the student has been freed. Now free memory for the TA.
            free(ta_tofree->name);
            free(ta_tofree);
            return 0;
        }
        head = head->next;
    }
    // if we reach here, the ta_name was not in the list
    return 1;
}






/* TA ta_name is finished with the student they are currently helping (if any)
 * and are assigned to the next student in the full queue.
 * If the queue is empty, then TA ta_name simply finishes with the student
 * they are currently helping, records appropriate statistics,
 * and sets current_student for this TA to NULL.
 * If ta_name is not in ta_list, return 1 and do nothing.
 */
int take_next_overall(char *ta_name, Ta *ta_list, Student **stu_list_ptr) {
    Ta *target_ta = find_ta(ta_list, ta_name);
    if (target_ta == NULL) {
        return 1;
    }
    release_current_student(target_ta);

    // Take new student
    if (*stu_list_ptr != NULL) {
        target_ta->current_student = *stu_list_ptr;

        target_ta->current_student->course->head = target_ta->current_student->next_course;
        if (target_ta->current_student->course->tail == target_ta->current_student) {
            target_ta->current_student->course->tail = NULL;
        }
        *stu_list_ptr = (*stu_list_ptr)->next_overall;
        target_ta->current_student->next_overall = NULL;
        target_ta->current_student->next_course = NULL;
        // Add waiting time
        target_ta->current_student->course->wait_time += time(NULL) -
                                *(target_ta->current_student->arrival_time);
        // Change arrival time to current time
        *(target_ta->current_student->arrival_time) = time(NULL);
    }


    return 0;
}



/* TA ta_name is finished with the student they are currently helping (if any)
 * and are assigned to the next student in the course with this course_code.
 * If no student is waiting for this course, then TA ta_name simply finishes
 * with the student they are currently helping, records appropriate statistics,
 * and sets current_student for this TA to NULL.
 * If ta_name is not in ta_list, return 1 and do nothing.
 * If course is invalid return 2, but finish with any current student.
 */
int take_next_course(char *ta_name, Ta *ta_list, Student **stu_list_ptr, char *course_code, Course *courses, int num_courses) {
    Ta *target_ta = find_ta(ta_list, ta_name);
    if (target_ta == NULL) {
        return 1;
    }
    release_current_student(target_ta);

    Course *target_course = find_course(courses, num_courses, course_code);
    if (target_course == NULL) {
        return 2;
    }

    // Take new student
    if (*stu_list_ptr != NULL) {
        target_ta->current_student = target_course->head;

        Student *target_std = target_course->head;
        target_course->head = target_course->head->next_course;
        if (target_course->tail == target_ta->current_student) {
            target_course->tail = NULL;
        }

        // Update next_overall
        Student *cur_student = *stu_list_ptr;
        if (cur_student == target_std) { // head
            *stu_list_ptr = cur_student->next_overall;
        } else {
            while (cur_student->next_overall != NULL) { // normal
                if (cur_student->next_overall == target_std) {
                    cur_student->next_overall = cur_student->next_overall->next_overall;
                    break;
                }
                cur_student = cur_student->next_overall;
            }
        }

        target_ta->current_student->next_overall = NULL;
        target_ta->current_student->next_course = NULL;
        // Add waiting time
        target_ta->current_student->course->wait_time += time(NULL) -
                                *(target_ta->current_student->arrival_time);
        // Change arrival time to current time
        *(target_ta->current_student->arrival_time) = time(NULL);

    }


    return 0;
}


/* For each course (in the same order as in the config file), print
 * the <course code>: <number of students waiting> "in queue\n" followed by
 * one line per student waiting with the format "\t%s\n" (tab name newline)
 * Uncomment and use the printf statements below. Only change the variable
 * names.
 */
void print_all_queues(Student *stu_list, Course *courses, int num_courses) {
    for (int i = 0; i < num_courses; i++) {
        Student *cur_course_std = courses[i].head;
        int counter = 0;
        while (cur_course_std != NULL) {
            counter++;
            cur_course_std = cur_course_std->next_course;
        }
        printf("%s: %d in queue\n", courses[i].code, counter);
        cur_course_std = courses[i].head;
        while (cur_course_std != NULL) {
            printf("\t%s\n", cur_course_std->name);
            cur_course_std = cur_course_std->next_course;
        }
    }


}


/*
 * Print to stdout, a list of each TA, who they are serving at from what course
 * Uncomment and use the printf statements
 */
void print_currently_serving(Ta *ta_list) {
    Ta *cur_ta = ta_list;
    if (cur_ta == NULL) {
        printf("No TAs are in the help centre.\n");
    } else {
        while (cur_ta != NULL) {
            if (cur_ta->current_student == NULL) {
                printf("TA: %s has no student\n", cur_ta->name);
            } else {
                printf("TA: %s is serving %s from %s\n", cur_ta->name,
                    cur_ta->current_student->name, cur_ta->current_student->course->code);
            }
            cur_ta = cur_ta->next;
        }
    }
}


/*  list all students in queue (for testing and debugging)
 *   maybe suggest it is useful for debugging but not included in marking?
 */
void print_full_queue(Student *stu_list) {
    Student *cur_student = stu_list;
    while (cur_student != NULL) {
        printf("%s\n", cur_student->name);
        cur_student = cur_student->next_overall;
    }
}

/* Prints statistics to stdout for course with this course_code
 * See example output from assignment handout for formatting.
 *
 */
int stats_by_course(Student *stu_list, char *course_code, Course *courses, int num_courses, Ta *ta_list) {

    // TODO: students will complete these next pieces but not all of this
    //       function since we want to provide the formatting
    // Course *find_course(Course *courses, int num_courses, char *course_code)

    Course *found = find_course(courses, num_courses, course_code);
    if (found == NULL) {
        return 1;
    }

    Ta *cur_ta = ta_list;
    int students_being_helped = 0;
    while (cur_ta != NULL) {
        if (cur_ta->current_student != NULL) {
            students_being_helped++;
        }
        cur_ta = cur_ta->next;
    }

    struct student *cur_student = stu_list;
    int students_waiting = 0;
    while (cur_student != NULL) {
        students_waiting++;
        cur_student = cur_student->next_overall;
    }

    printf("%s:%s \n", found->code, found->description);
    printf("\t%d: waiting\n", students_waiting);
    printf("\t%d: being helped currently\n", students_being_helped);
    printf("\t%d: already helped\n", found->helped);
    printf("\t%d: gave_up\n", found->bailed);
    printf("\t%f: total time waiting\n", found->wait_time);
    printf("\t%f: total time helping\n", found->help_time);

    return 0;
}


/* Dynamically allocate space for the array course list and populate it
 * according to information in the configuration file config_filename
 * Return the number of courses in the array.
 * If the configuration file can not be opened, call perror() and exit.
 */
int config_course_list(Course **courselist_ptr, char *config_filename) {
    FILE *fp = fopen(config_filename, "r");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }

    char input_buffer[INPUT_BUFFER_SIZE];
    int course_counter = -1;
    int num_of_courses = 0;
    while (fgets(input_buffer, INPUT_BUFFER_SIZE, fp) != NULL) {
        if (course_counter == -1) {
            sscanf(input_buffer, "%d", &num_of_courses);
            *courselist_ptr = malloc(sizeof(Course) * num_of_courses);
        } else {
            // course_code
            strncpy((*courselist_ptr)[course_counter].code, input_buffer, 6);
            (*courselist_ptr)[course_counter].code[6] = '\0';

            // course_description
            int des_len = strlen(input_buffer+7) - 1;
            (*courselist_ptr)[course_counter].description = malloc(des_len + 1);
            strncpy((*courselist_ptr)[course_counter].description, input_buffer+7, des_len);
            (*courselist_ptr)[course_counter].description[des_len] = '\0';

            // Initialize other fields
            (*courselist_ptr)[course_counter].head = NULL;
            (*courselist_ptr)[course_counter].tail = NULL;
            (*courselist_ptr)[course_counter].helped = 0;
            (*courselist_ptr)[course_counter].bailed = 0;
            (*courselist_ptr)[course_counter].wait_time = 0;
            (*courselist_ptr)[course_counter].help_time = 0;
        }
        course_counter++;
    }
    fclose(fp);

    return num_of_courses;
}
