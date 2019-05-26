/*
Name    : Goutami Padmanabhan
UTA ID  : 1001669338
*/
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

/*** Constants that define parameters of the simulation ***/

#define MAX_SEATS 3        /* Number of seats in the professor's office */
#define professor_LIMIT 10 /* Number of students the professor can help before he needs a break */
#define MAX_STUDENTS 1000  /* Maximum number of students in the simulation */

#define CLASSA 0
#define CLASSB 1

static int students_in_office;   /* Total numbers of students currently in the office */
static int classa_inoffice;      /* Total numbers of students from class A currently in the office */
static int classb_inoffice;      /* Total numbers of students from class B in the office */
static int students_since_break = 0;

typedef struct 
{
  int arrival_time;  // time between the arrival of this student and the previous student
  int question_time; // time the student needs to spend with the professor
  int student_id;
} student_info;

typedef struct 
{
  int student_class;
  int can_i_enter;
  int premission_to_enter;
  int i_have_entered;
  int can_i_leave;
  int permission_to_leave;
  int i_have_left;
} student_prof_comm;

student_prof_comm sp_comm[MAX_STUDENTS];

/* Called at beginning of simulation.  
 */
static int initialize(student_info *si, char *filename) 
{
  students_in_office = 0;
  classa_inoffice = 0;
  classb_inoffice = 0;
  students_since_break = 0;

  int i;
  for(i=0; i < MAX_STUDENTS; i++) 
  {
    sp_comm[i].student_class = -1;
    sp_comm[i].can_i_enter = 0;
    sp_comm[i].premission_to_enter = 0;
    sp_comm[i].i_have_entered = 0;
    sp_comm[i].can_i_leave = 0;
    sp_comm[i].permission_to_leave = 0;
    sp_comm[i].i_have_left = 0;
  }

  /* Read in the data file and initialize the student array */
  FILE *fp;

  if((fp=fopen(filename, "r")) == NULL) 
  {
    printf("Cannot open input file %s for reading.\n", filename);
    exit(1);
  }

  i = 0;
  while ( (fscanf(fp, "%d%d\n", &(si[i].arrival_time), &(si[i].question_time))!=EOF) && i < MAX_STUDENTS ) 
  {
    i++;
  }

 fclose(fp);
 return i;
}

/* Code executed by professor to simulate taking a break 
 * You do not need to add anything here.  
 */
static void take_break() 
{
  printf("The professor is taking a break now.\n");
  sleep(5);
  assert( students_in_office == 0 );
  students_since_break = 0;
}

/* Code for the professor thread. This is fully implemented except for synchronization
 * with the students.  See the comments within the function for details.
 */
void *professorthread(void *junk) 
{
  printf("The professor arrived and is starting his office hours\n");

  int ready_to_break = 0;
  int numSameClassCount = 0;
  int currClassType = CLASSA;
  int switch_class = 0;
  /* Loop while waiting for students to arrive. */
  while (1) 
  {
    // see if i can get someone out of office.. 
    if(students_in_office > 0) 
    {
      int i;
      int student_i_gave_permission = -1;
      for(i = 0; i < MAX_STUDENTS; i++)
      {
        if(sp_comm[i].can_i_leave == 1) 
        {
          sp_comm[i].permission_to_leave = 1;
          student_i_gave_permission = i;
          break;
        }
      }
      // Wait until the student actually leaves.. 
      if(student_i_gave_permission != -1) 
      {
        while(!(sp_comm[i].i_have_left == 1)) 
        {
          //Keep trying again
        }
      }
    }
    //Requirement 3 part 1
    /*Requirement 3: The professor gets tired after answering too many questions. 
      He decides that after helping 10 students he needs to take a break before
      he can help more students. So after the 10th student (counting since the last break)
      enters the professors office no more students are admitted into the office, until
      after the professors's next break*/
    // see if I need to take a break..
    // if there are 10 students who have entered, then professor ready for break
    if(ready_to_break == 0 && students_since_break == professor_LIMIT) 
    {
      ready_to_break = 1;
    }
    // if prof ready for braek and no students in office, take a break
    if(ready_to_break == 1 && students_in_office == 0) 
    {
      take_break();
      ready_to_break = 0;
    }
    /*Requirement 4: In order to be fair to both classes after 5 consecutive students from a
      single class the professor will answer questions from a student from the other class.*/
    if(numSameClassCount >= 5) 
    {
      int i;
      for(i = 0; i < MAX_STUDENTS; i++)
      {
        if(sp_comm[i].can_i_enter == 1 && sp_comm[i].student_class != currClassType) 
        {
          switch_class = 1;
          break;
        }
      }
    }
    //Requirement 3 part 2
    /*Requirement 3: Students that arrive while the professor is taking his break 
      have to wait outside the office*/
    // see if i can let anyone in my office...
    if(ready_to_break == 0) 
    {
      /*Requirement 1: The professors's office has only 3 seats, so no more than 3 students are
        allowed to simultaneously enter the professor’s office. When the office is full and new
        students arrive they have to wait outside the office.*/
      if((switch_class == 0 && students_in_office < MAX_SEATS) 
            || (switch_class == 1 && students_in_office == 0))
      {
        // we have to know which class each student belongs to
        int which_class = -1;
        assert(!(classa_inoffice > 0 && classb_inoffice > 0));

        if(classa_inoffice > 0 && classb_inoffice == 0) 
        {
          which_class = CLASSA;
        } 
        else if (classb_inoffice > 0 && classa_inoffice == 0) 
        {
          which_class = CLASSB;
        } 
        else if (classa_inoffice == 0 && classb_inoffice == 0) 
        {
          if(switch_class == 1) 
          {
            which_class = (currClassType == CLASSA) ? CLASSB : CLASSA;
            switch_class = 0;
          } 
          else 
          {
            which_class = 2;
          }
        }

        int i;
        int student_i_gave_permission = -1;
        for(i = 0; i < MAX_STUDENTS; i++)
        {
          /*Requirement 2: The professor gets confused when helping students from class A and
            class B at the same time. He decides that while students from class A are in his 
            office, no students from class B are allowed to enter, and the other way around.*/
          int class_compatible = 0;
          if(which_class == 2) 
          {
            class_compatible = 1;
          } 
          else 
          {            
            class_compatible = (sp_comm[i].student_class == which_class) ? 1 : 0;
          }
          if(sp_comm[i].can_i_enter == 1 && class_compatible == 1) 
          {
            sp_comm[i].premission_to_enter = 1;
            student_i_gave_permission = i;
            if(sp_comm[i].student_class == currClassType) 
            {
              numSameClassCount++; 
            } 
            else 
            {
              currClassType = sp_comm[i].student_class;
              numSameClassCount = 1;
            }
            break;
          }
        }
        // keep waiting until the students have entered..
        /*global variables are incremented by classa/classb student.. 
          so wait until the updates happen..*/ 
        if(student_i_gave_permission != -1) 
        {
          while(!(sp_comm[student_i_gave_permission].i_have_entered == 1)) 
          {
            // Keep trying again 
          }   
        }
      }
    }
  }
  pthread_exit(NULL);
}
/*Code executed by a class A or B student to get the 
  permission of professor to enter the office.*/
void get_permission_to_enter (student_info *s_info) 
{
  // Ask permission to enter the class.. 
  sp_comm[s_info->student_id].can_i_enter = 1;
  // Wait until permission is received from professor.. 
  while(!(sp_comm[s_info->student_id].premission_to_enter == 1)) 
  {
    // Keep trying again.. 
  }
  sp_comm[s_info->student_id].can_i_enter = -1;
}

/*Code executed by a class A or B student to get the 
  permission of professor to leave the office.*/
void get_permission_to_leave (student_info *s_info) 
{
  // Ask permission to enter the class.. 
  sp_comm[s_info->student_id].can_i_leave = 1;
  // Wait until permission is received from professor.. 
  while(!(sp_comm[s_info->student_id].permission_to_leave == 1)) 
  {
    // Keep trying again.. 
  }
  sp_comm[s_info->student_id].can_i_leave = -1;
}

/* Code executed by a class A student to enter the office.
 * You have to implement this.  Do not delete the assert() statements,
 * but feel free to add your own.
 */
void classa_enter(student_info *s_info) 
{
  get_permission_to_enter(s_info);  

  students_in_office += 1;
  students_since_break += 1;
  classa_inoffice += 1;

  sp_comm[s_info->student_id].i_have_entered = 1;
}

/* Code executed by a class B student to enter the office.
 * You have to implement this.  Do not delete the assert() statements,
 * but feel free to add your own.
 */
void classb_enter(student_info *s_info) 
{
  get_permission_to_enter(s_info);  

  students_in_office += 1;
  students_since_break += 1;
  classb_inoffice += 1;

  sp_comm[s_info->student_id].i_have_entered = 1;
}

/* Code executed by a student to simulate the time he spends in the office asking questions
 * You do not need to add anything here.  
 */
static void ask_questions(int t) 
{
  sleep(t);
}


/* Code executed by a class A student when leaving the office.
 * You need to implement this.  Do not delete the assert() statements,
 * but feel free to add as many of your own as you like.
 */
static void classa_leave(student_info* s_info) 
{
  get_permission_to_leave(s_info);

  students_in_office -= 1;
  classa_inoffice -= 1;

  sp_comm[s_info->student_id].i_have_left = 1;
}

/* Code executed by a class B student when leaving the office.
 * You need to implement this.  Do not delete the assert() statements,
 * but feel free to add as many of your own as you like.
 */
static void classb_leave(student_info *s_info) 
{
  get_permission_to_leave(s_info);

  students_in_office -= 1;
  classb_inoffice -= 1;

  sp_comm[s_info->student_id].i_have_left = 1;
}

/* Main code for class A student threads.  
 * You do not need to change anything here, but you can add
 * debug statements to help you during development/debugging.
 */
void* classa_student(void *si) 
{
  student_info *s_info = (student_info*)si;

  /* enter office */
  classa_enter(s_info);

  printf("Student %d from class A enters the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classb_inoffice == 0 );
  
  /* ask questions  --- do not make changes to the 3 lines below*/
  printf("Student %d from class A starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
  ask_questions(s_info->question_time);
  printf("Student %d from class A finishes asking questions and prepares to leave\n", s_info->student_id);

  /* leave office */
  classa_leave(s_info);  

  printf("Student %d from class A leaves the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

  pthread_exit(NULL);
}

/* Main code for class B student threads.
 * You do not need to change anything here, but you can add
 * debug statements to help you during development/debugging.
 */
void* classb_student(void *si) 
{
  student_info *s_info = (student_info*)si;

  /* enter office */
  classb_enter(s_info);

  printf("Student %d from class B enters the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  assert(classa_inoffice == 0 );

  printf("Student %d from class B starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
  ask_questions(s_info->question_time);
  printf("Student %d from class B finishes asking questions and prepares to leave\n", s_info->student_id);

  /* leave office */
  classb_leave(s_info);        

  printf("Student %d from class B leaves the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

  pthread_exit(NULL);
}

/* Main function sets up simulation and prints report
 * at the end.
 */
int main(int nargs, char **args) 
{
  int i;
  int result;
  int student_type;
  int num_students;
  void *status;
  pthread_t professor_tid;
  pthread_t student_tid[MAX_STUDENTS];
  student_info s_info[MAX_STUDENTS];

  if (nargs != 2) 
  {
    printf("Usage: officehour <name of inputfile>\n");
    return EINVAL;
  }

  num_students = initialize(s_info, args[1]);
  if (num_students > MAX_STUDENTS || num_students <= 0) 
  {
    printf("Error:  Bad number of student threads. "
           "Maybe there was a problem with your input file?\n");
    return 1;
  }

  printf("Starting officehour simulation with %d students ...\n",
    num_students);

  result = pthread_create(&professor_tid, NULL, professorthread, NULL);

  if (result) 
  {
    printf("officehour:  pthread_create failed for professor: %s\n", strerror(result));
    exit(1);
  }

  for (i=0; i < num_students; i++) 
  {

    s_info[i].student_id = i;
    sleep(s_info[i].arrival_time);
                
    student_type = random() % 2;
    sp_comm[i].student_class = student_type;

    if (student_type == CLASSA)
    {
      result = pthread_create(&student_tid[i], NULL, classa_student, (void *)&s_info[i]);
    }
    else // student_type == CLASSB
    {
      result = pthread_create(&student_tid[i], NULL, classb_student, (void *)&s_info[i]);
    }

    if (result) 
    {
      printf("officehour: thread_fork failed for student %d: %s\n", 
            i, strerror(result));
      exit(1);
    }
  }

  /* wait for all student threads to finish */
  for (i = 0; i < num_students; i++) 
  {
    pthread_join(student_tid[i], &status);
  }

  /* tell the professor to finish. */
  pthread_cancel(professor_tid);

  printf("Office hour simulation done.\n");

  return 0;
}
