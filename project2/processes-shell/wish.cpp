#include <iostream>
#include <string>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <vector>
#include <sstream>
#include <algorithm> // for find() function

using namespace std;
// TODO better way for redirection: vector<string> redirectionFile
    // to check for multiple files or no files, use .size()
// NEXT parallel commands
// NEXT allow no-whitespace with & and <
// NEXT batch mode
void writeError(); 
int interactiveMode(string);
const char* checkPath(string, vector<const char*>);

vector<const char*>pathsList = {"/bin/"};
const char* ERROR = strdup("ERROR");

int main(int argc, char *argv[]) {
    if (argc == 1) {
        /* INTERACTIVE MODE */  
        string str;
        while(true) {
            cout << "wish> ";
            getline(cin, str);
            
            int ret = interactiveMode(str);
            if (ret == 1)
                writeError();
        }       

    } else if (argc == 2) {
        /* BATCH MODE */

    } else { // error, more than one file entered
        writeError();
        exit(1);
    }
    exit(0);
}

// Write an error message to stderr
void writeError() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

// Run shell and let user input commands, return 1 if any error occurs
int interactiveMode(string str) {
    vector<string> line;
    bool redirect = false;
    stringstream s(str);
    string w;

    while (s >> w) {
        // Check for redirection
        if (w == ">") {
            redirect = true;
            line.push_back(">");
        } else line.push_back(w);
    }

    // If redirect, set file name and remove from line
    string redirectionFile;
    if (redirect) {
        redirectionFile = line.back();
        if (redirectionFile == "" || redirectionFile == ">")
            return 1;
        else {
            line.erase(line.end() - 2, line.end()); // remove file name and >
            // Check for multiple files
            auto it = find(line.begin(), line.end(), ">");
            if (it != line.end())
                return 1;
        }
    }

    char* arr[line.size() + 1];

    for (size_t i = 0; i < line.size(); i++)
        arr[i] = strdup(line[i].c_str());

    // Add terminating null char
    arr[line.size()] = NULL;

    //Checking for built-in commands
    if (line.front() == "cd") {
        if (chdir(arr[1]) == -1)
            return 1;
    } else if (line.front() == "path") {
        if (line.size() <= 1) {
            pathsList[0] = NULL;
        } else {
            for (size_t i = 0; i < sizeof(arr) + 1; i++) {
                pathsList[i] = arr[i+1];
            }
        }
    } else if (line.front() == "exit") {
        if (line.size() == 1)
            exit(0);
        else return 1; // error, cannot give args to exit
    } else {
        // Check command path
        const char* path;
        if (pathsList[0] != NULL) {
            path = checkPath(line.front(), pathsList);
            if (path == ERROR)
                return 1;
        } else return 1; // Will not recognize non-built-in commands if pathsList is empty

        pid_t pid = fork();

        if (pid == 0) {
            // child

            // Check for ">" redirection
            if (redirect) {
                int fd = open(redirectionFile.c_str(), O_WRONLY + O_TRUNC + O_CREAT);
                if (fd == -1)
                    return 1;
                if (dup2(fd, STDOUT_FILENO) == -1)
                    return 1;
            }

            int ret_exec = execv(path, arr);

            if (ret_exec == -1) // execv error
                return 1;
        } else if (pid > 0) {
            // parent
            wait(NULL);
        } else {
            // error
            return 1;
        }
    }
         
    return 0;
}

// Use access() to check if the command path(s) is valid, else return ERROR
const char* checkPath(string cmd, vector<const char*> pathsList) {
    const char* path = ERROR;
    const char* tempPath;
    string pathStr;

    for (const char* p : pathsList) {
        pathStr = p + cmd;
        tempPath = strdup(pathStr.c_str());
        
        if (access(tempPath, X_OK) == 0) {
            path = tempPath;
            break;
        }
    }

    return path;
}