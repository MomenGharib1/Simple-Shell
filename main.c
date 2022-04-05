#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <unistd.h>

char str[500];
int fd;
char *str_arg[500];
char *ptr;
pid_t pid;
char *ptr2 = str;
bool No_wait;


void execute_builtIn();

void execute_command();

void wait_pro();

void do_echo();

void do_export();

void do_cd();

void command_or_builtIn();

void Handle_doll();


// function to parse the string input and splits it to command and arguments
void parse_input() {
    int i = 0;

    char *token = strtok(str, " ");
    while (token != NULL) {
        // checking if the command is export and handle it
        char *ex = strstr(str, "export");
        if (ex) {
            char *token_before = strtok(NULL, "=");
            char *token_after = strtok(NULL, "=");
            token_after = strtok(token_after, "\"");
            //saving the value of the variable in export
            setenv(token_before, token_after, 1);
            token = strtok(NULL, " ");
            if (token == NULL)
                return;
            else
                continue;
        }
        // checking for waiting sign
        if (!strcmp(token, "&")) {
            No_wait = true;
            token = "\0";

            str_arg[i++] = token;

        } else
            str_arg[i++] = token;
        // deleting any spaces in input
        token = strtok(NULL, " ");
    }

    // saving the command and arguments in new array
    str_arg[i] = token;

    char *No_qout;
    char *is_qout;
    for (i = 1; str_arg[i] != NULL; i++) {
        //checking for quotations
        is_qout = strstr(str_arg[i], "\"");
        if (is_qout) {
            // removing quotations
            No_qout = strtok(str_arg[i], "\"");
            str_arg[i] = No_qout;
        } else
            continue;
    }
    // this method handles the $ sign if exists
    Handle_doll();
    //deciding what to do next
    command_or_builtIn();

}


void command_or_builtIn() {
    char *built_in = "cd echo export";
    //checking if the command one of the above
    ptr = strstr(built_in, ptr2);
    if (ptr)
        execute_builtIn();
    else
        execute_command();
}
//if the command is not from he builtIn ones then it comes here to execute it by excevp()
void execute_command() {
    //creating new child
    pid = fork();
    //checking fail
    if (pid == -1) {
        printf("\nChild can't be born :(\n");

    } else if (pid == 0) {
        //processing
        int proc_state = execvp(ptr2, str_arg);
        //handling wrong command
        if (proc_state < 0) {
            printf("\nWrong command!!\n");
            exit(proc_state);
        }
        exit(0);
    } else
        // if id belongs to parent then this method be called to check if parent should wait
        wait_pro();
}

//processing builtIn
void execute_builtIn() {
    char *echo = strstr(ptr2, "echo");
    char *cd = strstr(ptr2, "cd");
    char *export = strstr(ptr2, "export");
    if (echo) {
        do_echo();
    } else if (cd) {
        do_cd();
    } else if (export)
        do_export();
    else {
        if (str[0] == '\0')
            return;
        printf("Wrong command!!\n");
    }
}

void do_cd() {
    char s[100];

    getcwd(s, 100);

    if (str_arg[1] == NULL || strcmp(str_arg[1], "~") == 0) {
        chdir(getenv("HOME"));
        printf("%s\n", getcwd(s, 100));
    } else {
        chdir(str_arg[1]);
        printf("%s\n", getcwd(s, 100));
    }

}

void do_export() {

}

void do_echo() {

    for (int i = 1; str_arg[i] != NULL; i++) {
        printf("%s ", str_arg[i]);
    }

}

void Handle_doll() {
    char *doll;
    for (int i = 1; str_arg[i] != NULL; i++) {
        // checking index where the 4sign
        doll = strstr(str_arg[i], "$");
        if (doll) {
            //removing it
            char *No_doll = strtok(str_arg[i], "$");
            No_doll = getenv(No_doll);
            char *t = strtok(No_doll, " ");
            int j = i;
            //editing the ne argument after deletion of $ sign
            while (t != NULL) {
                str_arg[j++] = t;
                t = strtok(NULL, " ");
            }
        }
    }
}


void wait_pro() {
    //checking if parent should wait or not
    if (!No_wait)
        waitpid(pid, NULL, 0);

    if (No_wait)
        No_wait = false;
}

//function to terminate the process
void proc_exit() {
    char msg[] = "Child process was terminated\n";
    if (fd != 1) {
        write(fd, msg, strlen(msg));
    }

    int wstat;
    pid_t pid2;
    pid2 = wait3(&wstat, WNOHANG, (struct rusage *) NULL);
    if (pid2 == 0) { return; }
    else if (pid2 == -1)
        return;
}

int main() {
    //sending signals when child tends to terminate
    signal(SIGCHLD, proc_exit);
    //opens a new file if not existed
    fd = open("log.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    while (1) {

        int x;
        int i = 0;

        // Read input and save it in array.
        while (1) {
            x = getchar();

            //if user entered "enter" then reading stops
            if (x == '\n')
                break;
            // array to save the input
            str[i] = (char) x;
            i++;
        }

        //checking if the user typed "exit"
        if (!strcmp(str, "exit")) {
            close(fd);
            exit(0);
        }
        parse_input();
        // deleting the array to be ready for the next input
        memset(str, 0, sizeof(str));

    }

}
