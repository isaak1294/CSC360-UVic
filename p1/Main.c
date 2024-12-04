/*The goal of this assignment is to make a simple shell interpreter to run on a linux system that supports a specific list of commands.
 * These commands include: ls, cd, bg, and bglist. For bonus marks, we should implement the ability to ouput text to a file, 
 * The ability to navigate through command history, and the ability use bgkill, bgstop, and bgstart commands. */

#include <unistd.h>

void exit();
void runCommands(char** cmd);

//1. Start looping:
int main(void){
    while(1){
        //2. Print the prompt
        char username[] = getlogin();
        char hostname[] = gethostname();
        char cwd[] = getcwd();
        printf("%s@%s:~$ %s", username, hostname, cwd);
        //3. Read the input
        char* input = readInput();
        //4. Tokenize the input
        char** tokens = tokenize(input);
        //5. Run the commands
        runCommands(tokens);
        //6. Free the memory
        free(input);
        free(tokens);
    }
}