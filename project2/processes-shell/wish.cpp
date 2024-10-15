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

using namespace std;
// NOTE: parallel commands, doesn't wait for all processes before printing
    // ie if one cmd is fine and the other is an error, it prints error for 2nd cmd but runs 1st
void writeError(); 
int runShell(string);
vector<string> processInput(string);
vector<string> extractNoSpaceCmd(string, char);
int parallelCommands(vector<string>);
int forkAndExecute(vector<string>, bool);
const char* checkPath(string, vector<const char*>);

vector<const char*>pathsList = {"/bin"};
const char* ERROR = strdup("ERROR");
const string REDIRECTION_SYMBOL = ">";

int main(int argc, char *argv[]) {
    if (argc == 1) {
        /* INTERACTIVE MODE */  
        string str;
        while(true) {
            cout << "wish> ";
            getline(cin, str);
            
            int ret = runShell(str);
            if (ret == 1)
                writeError();
        }       

    } else if (argc == 2) {
        /* BATCH MODE */
        int fd = open(argv[1], O_RDONLY);
        if (fd == -1) {
            writeError();
            exit(1);
        } else {
            size_t bufferSize = 4096;
	        char buffer[bufferSize];
            stringstream ss;
		    string str;	
            int ret;
            while ((ret = read(fd, buffer, bufferSize)) > 0) {
                // Check for read() errors
                if (ret == -1) {
                    writeError();
                    break;
                }

                ss.write(buffer, ret);
            }
            str = ss.str();

            // Go through str line by line
            string tempStr;
            for (char c : str) {
                if (c == '\n') {
                    if (runShell(tempStr) == 1) {
                        writeError();
                    }
                    tempStr.clear();
                } else
                    tempStr += c;
            }
        }
    } else { // error, more than one file entered
        writeError();
        exit(1);
    }
    exit(0);
}

// Write an error message to STDERR
void writeError() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

// Run shell, calling other functions to fork and execute or run parallel commands
int runShell(string str) {
    bool redirect = false;

    if (str.find(REDIRECTION_SYMBOL) != string::npos)
        redirect = true;
    
    vector<string> line = processInput(str);
    
    if (str.find("&") != string::npos) {
        if (parallelCommands(line) == 1)
            return 1;
    } else {  
        if (forkAndExecute(line, redirect) == 1)
            return 1;   
    }
         
    return 0;
}

// Process string input into vector
vector<string> processInput(string str) {
    stringstream s(str);
    string w;
    vector<string> v;
    
    // Process str into line
    while (s >> w) {
        bool redirectOrParallel = false;

        // Check for no-space redirection to re-parse it with spaces
        if (w.find(REDIRECTION_SYMBOL) != string::npos && w.length() > 1) {
            redirectOrParallel = true;
            vector<string> commands = extractNoSpaceCmd(w, REDIRECTION_SYMBOL[0]);
            v.insert(v.end(), commands.begin(), commands.end()); 
        } 
        // Check for no-space & to re-parse it with spaces
        if (w.find("&") != string::npos && w.length() > 1) {
            redirectOrParallel = true;
            if (w.length() == 1) // Check for a single &
                v.push_back(w);
            else {
                vector<string> commands = extractNoSpaceCmd(w, '&');
                v.insert(v.end(), commands.begin(), commands.end());
            }
        }

        if (!redirectOrParallel)
            v.push_back(w);
    }
    
    return v;
}

// Given string command with redirect or parallel symbols with no whitespace, parse it properly
// and return a vector string of the commands
vector<string> extractNoSpaceCmd(string w, char c) {
    vector<string> cmd;
    string charString{c};
    string spacedRedirection = " " + charString + " ";
    size_t r = w.find_first_of(charString);

    // Loop to split w into substrings based on c character
    while (w.find(c) != string::npos) {
        string arg = w.substr(0, r);
        cmd.push_back(arg);
        cmd.push_back(charString);
        w = w.substr(r+1);     
    }

    // Push the last substring of w into cmd
    cmd.push_back(w);

    return cmd;
}

// Call forkAndExecute for parallel commands in a loop
int parallelCommands(vector<string> line) {
    vector<string> tempLine;
    line.push_back("END"); // To check end of string
    bool redirect = false;

    for (string l : line) {
        if (l == REDIRECTION_SYMBOL)
            redirect = true;
        if (l == "&" || l == "END") {
            if (forkAndExecute(tempLine, redirect) == 1)
                return 1;
            tempLine.clear();
            redirect = false;
        } else tempLine.push_back(l);
    }

    return 0;
}

// Check the command type, then fork and execute it
int forkAndExecute(vector<string> line, bool redirect = false) {
    string redirectionFile = "";

    // Go through line vector to check for redirection command
    if (redirect) {
        // Check that there's a command behind >
        if (line.size() < 3) // > fileName
            return 1;

        for (size_t i = 0; i < line.size(); i++) {
            if (line[i] == REDIRECTION_SYMBOL) {
                // Check for multiple files
                if ((i+1) != (line.size()-1))
                    return 1;

                line.erase(line.begin()+i);
                redirectionFile = line.back();
                line.pop_back();
            }
        }

        // Check that it's a valid redirection command
        if (redirectionFile == "" || redirectionFile == REDIRECTION_SYMBOL) {
                return 1;
        }
    }

    // Check if line is empty before doing anything
    if (line.size() == 0)
        return 0; // no error msg

    size_t arrLength = line.size();
    char* arr[arrLength + 1]; // The +1 is for terminating null char

    for (size_t i = 0; i < arrLength; i++)
        arr[i] = strdup(line[i].c_str());

    // Add terminating null char
    arr[arrLength] = NULL;

    //Checking for built-in commands
    if (line.front() == "cd") {
        if (chdir(arr[1]) == -1)
            return 1;
    } else if (line.front() == "path") {
        if (line.size() <= 1) { // if command is "path" by itself
            pathsList[0] = NULL;
        } else {
            pathsList.clear();
            pathsList.insert(pathsList.end(), arr+1, arr + arrLength);
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

        if (pid == 0) { // child
            // Check for redirection for dup2()
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
        } else if (pid > 0) { // parent
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
    string slash = "/";

    for (const char* p : pathsList) {
        pathStr = p + slash + cmd;
        tempPath = strdup(pathStr.c_str());
        
        if (access(tempPath, X_OK) == 0) {
            path = tempPath;
            break;
        }
    }

    return path;
}