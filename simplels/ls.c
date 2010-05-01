/*
 * Simple implementation of ls command-line program.
 * Implemented as part of a programming exercise.
 * Prints the contents of the current directory.
 *
 * Supports the following flags:
 * 	-l produces a detailed listing of the files
 *	-a includes files beginning with ‘.’ (hidden files)
 *	-i prints the index number of the file
 *	-F appends a symbol to the filename to represent the classification of the file:
 *		- / directory
 *		- @ symbolic link
 *		- = socket
 *		- - FIFO
 *		- * executable
 *	-S sorts files by file size
 *	-t sorts files by time last modified
 * 	-X sorts files by file extension

 * author: Ali Scott <ali.scott@gmail.com>
 */

#include "ls.h"
#include "utils.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

// global variable for directory
char* dir;

int main(int argc, char** argv) {
	// command flags default to 0
	int all = FALSE, classify = FALSE, inode = FALSE, details = FALSE;
	// default sort function to alphabetical sort
	void* sort_function = ialphasort;
	char flag;
	// parses command flags - adapted from The C Programming Language p117
	while (--argc > 0 && (*++argv)[0] == '-') {
		while ((flag = *++argv[0])) {
			switch (flag) {
				case 'a':
					all = TRUE; break;
				case 'F':
					classify = TRUE; break;
				case 'i':
					inode = TRUE; break;
				case 'l':
					details = TRUE; break;
				case 'S':
					sort_function = sizesort; break;
				case 't':
					sort_function = mtimesort; break;
				case 'X':
					sort_function = extsort; break;
				default:
					fprintf(stderr, "illegal option %c\n", flag);
					exit(1);
			}
		}
	}
	// set directory to argument or current location if no argument
	dir = argc > 0 ? argv[0] : ".";
	// load entries sorted using specified g man scansort function
	struct dirent** entries;
	int n = scandir(dir, &entries, 0, sort_function);
    if (n < 0) {
		fprintf(stderr, "unable to open '%s'\n", dir);
		exit(1);
	}
	int i;
	// get string lengths for columns
	int fileno_len = maxlen(entries, n, FILENO);
	int link_len = maxlen(entries, n, LINKS);
	int user_len = maxlen(entries, n, USER);
	int group_len = maxlen(entries, n, GROUP);
	int size_len = maxlen(entries, n, SIZE);
	// loop through each entry
	for (i = 0; i < n; i++) {
		// skip files beginning with '.' unless -a flag is used
		if (!all && entries[i]->d_name[0] == '.')
			continue;
		if (inode) {
			printf("%i ", (int) entries[i]->d_fileno);
			if (details)
				printspaces(fileno_len - intlen((int) entries[i]->d_fileno)); 
		}
		struct stat stats = getfilestats(entries[i]->d_name);
		if (details)
			printdetails(stats, link_len, user_len, group_len, size_len);
		printf("%s",  entries[i]->d_name);
		if (classify)
			printf("%c",  getclass(stats));
		if (details && i < n - 1)
			printf("\n");
		else
			printf("  ");
			
	}
	printf("\n");
	exit(0);
}

/*
 * prints the long listing of the file
 * if -l flag is used
 */
void printdetails(struct stat stats, int link_len, int user_len, int group_len, int size_len) {
	char permissions[10];
	printf("%s ", modestr(stats.st_mode, permissions));
	printf("%i ", (int) stats.st_nlink);
	printspaces(link_len - intlen((int) stats.st_nlink)); 
	char* user = getuser(stats);
	printf("%s ", user);
	printspaces(user_len - strlen(user)); 
	char* group = getgroup(stats);
	printf("%s ", group);
	printspaces(group_len - strlen(group)); 
	printspaces(size_len - intlen(stats.st_size));
	printf("%ld ", stats.st_size);
	char time_str[16];
	timestr(stats.st_mtime, time_str);
	printf("%s ", time_str);
}

/*
 * compares two files based on case-insensitive alphabetical ordering of file name
 * similar to alphasort but case-insensitive
 */
int ialphasort(const void* a, const void* b) {
	struct dirent** a2 = (struct dirent** )a;
    struct dirent** b2 = (struct dirent** )b;
    char* a_lower = malloc(strlen((*a2)->d_name) + 1);
    char* b_lower = malloc(strlen((*b2)->d_name) + 1);
    if (a_lower == NULL || b_lower == NULL) {
		fprintf(stderr, "unable to allocate memory\n");
		exit(1);
	}
    strtolower((*a2)->d_name, a_lower);
    strtolower((*b2)->d_name, b_lower);
    int result = strcoll(a_lower, b_lower);
    free(a_lower);
    free(b_lower);
    return result;
}

/*
 * compares two files based on file size
 * if file sizes are the same then uses alphasort
 */
int sizesort(const void* a, const void* b) {
	struct dirent** a2 = (struct dirent**)a;
	struct dirent** b2 = (struct dirent**)b;
	int result = getfilestats((*b2)->d_name).st_size - getfilestats((*a2)->d_name).st_size;
	return result == 0 ? ialphasort(a, b) : result;
}

/*
 * compares two files based on time last modified
 * if times are the same then uses alphasort
 */
int mtimesort(const void* a, const void* b) {
	struct dirent** a2 = (struct dirent**)a;
	struct dirent** b2 = (struct dirent**)b;
	int result = difftime(getfilestats((*b2)->d_name).st_mtime, getfilestats((*a2)->d_name).st_mtime);
	return result == 0 ? ialphasort(a, b) : result;
}

/*
 * compares two files based on alphabetical ordering of file extension
 * if extensions are the same then uses alphasort
 */
int extsort(const void* a, const void* b) {
	struct dirent** a2 = (struct dirent**)a;
	struct dirent** b2 = (struct dirent**)b;
	int result = strcoll(getext((*a2)->d_name), getext((*b2)->d_name));
	return result == 0 ? ialphasort(a, b) : result;
}

/*
 * returns the stats of a file
 * uses lstat for symbolic links
 */
struct stat getstats(char* path) {
	struct stat stats;
	if(stat(path, &stats) || lstat(path, &stats)) {
		fprintf(stderr, "failed to get stats for: %s\n", path);
		exit(1);
	}
	return stats;
}

/*
 * returns the stats of a file without passing full path
 */
struct stat getfilestats(char* name) {
	char* path = malloc(strlen(dir) + strlen(name) + 2);
	if (path == NULL) {
		fprintf(stderr, "unable to allocate memory\n");
		exit(1);
	}
	sprintf(path, "%s/%s", dir, name);
	struct stat result = getstats(path);
	free(path);
	return result;
}

/*
 * returns a symbol representing the classification of a file
 * / - directory
 * @ - symbolic link
 * = - socket
 * | - FIFO
 * * - executable
 */
char getclass(struct stat stats) {
	mode_t mode = stats.st_mode;
	if (S_ISDIR(mode))
		return '/';
	if (S_ISLNK(mode)) 
		return '@';
	if (S_ISSOCK(mode))
		return '=';
	if (S_ISFIFO(mode))
		return '|';
	if (mode & S_IXUSR || mode & S_IXGRP ||mode & S_IXOTH) 
		return '*';
	return '\0';
}

/*
 * returns a string representing the mode of the file
 * the first character represents the type of file
 * 		b - block device
 *		c - character device
 *		d - directory
 *		l - symbolic link
 *		p - FIFO
 *		s - socket
 * the rest represents the permissions of the file
 * where the first three characters represent the user permissions (rwx)
 * the second three represent group permissions
 * the last three represent others' permissions
 */
char* modestr(mode_t mode_bits, char* mode_str) {
	if (S_ISBLK(mode_bits))
		mode_str[0] = 'b';
	else if (S_ISCHR(mode_bits))
		mode_str[0] = 'c';
	else if (S_ISDIR(mode_bits))
		mode_str[0] = 'd';
	else if (S_ISLNK(mode_bits))
		mode_str[0] = 'l';
	else if (S_ISFIFO(mode_bits))
		mode_str[0] = 'p';
	else if (S_ISSOCK(mode_bits))
		mode_str[0] = 's';
	else
		mode_str[0] = '-';	
	mode_str[1] = S_IRUSR & mode_bits ? 'r' : '-';
	mode_str[2] = S_IWUSR & mode_bits ? 'w' : '-';
	mode_str[3] = S_IXUSR & mode_bits ? 'x' : '-';
	mode_str[4] = S_IRGRP & mode_bits ? 'r' : '-';
	mode_str[5] = S_IWGRP & mode_bits ? 'w' : '-';
	mode_str[6] = S_IXGRP & mode_bits ? 'x' : '-';
	mode_str[7] = S_IROTH & mode_bits ? 'r' : '-';
	mode_str[8] = S_IWOTH & mode_bits ? 'w' : '-';
	mode_str[9] = S_IXOTH & mode_bits ? 'x' : '-';
	mode_str[10] = '\0';
	return mode_str;
}

/*
 * creates a time string in the form YYYY-MM-DD hh:mm
 */
char* timestr(time_t time, char* time_str) {
	struct tm* t = gmtime(&time);
	sprintf(time_str, "%i-%02i-%02i %02i:%02i", 1900 + t->tm_year, 1 + t->tm_mon, t->tm_mday, t->tm_hour, t->tm_min);
	return time_str;
}

/*
 * returns the maximum string length of a property of a file
 * this is used for padding when the long listing (-l) flag is used
 */
int maxlen(struct dirent** entries, int n, field_t f) {
	int max = 0;
	struct stat stats;
	int i;
	for (i = 0; i < n; i++) {
		stats = getfilestats(entries[i]->d_name);
		int len = 0;
		switch (f) {
			case FILENO:
				len = intlen(entries[i]->d_fileno); break;
			case SIZE:
				len = intlen(stats.st_size); break;
			case LINKS:
				len = intlen(stats.st_nlink); break;
			case USER:
				len = strlen(getuser(stats)); break;
			case GROUP:
				len = strlen(getgroup(stats)); break;
			default:
				len = 0;
		}
		max = len > max ? len : max;
	}
	return max;	
}

/*
 * returns a file owner's username
 */
char* getuser(struct stat stats) {
	struct passwd* usr = getpwuid(stats.st_uid);
	if (usr == NULL) {
		fprintf(stderr, "could not get user of file\n");
		exit(1);
	}
	return usr->pw_name;
}

/*
 * returns the file's group name
 */
char* getgroup(struct stat stats) {
	struct group* grp = getgrgid(stats.st_gid);
	if (grp == NULL) {
		fprintf(stderr, "could not get group of file\n");
		exit(1);
	}
	return grp->gr_name;
}

/*
 * returns the extension from a filename
 * (everything after the last '.')
 */
char* getext(char* filename) {
	char* ext = strrchr(filename, '.');
	return ext == NULL ? "" : ++ext;
}
