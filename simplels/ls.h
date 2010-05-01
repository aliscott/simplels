/*
 * Declarations for the ls program
 * author: 060004694
 */

#include <sys/stat.h>
#include <dirent.h>

/*
 * defines the types for columns of the long listing
 * allows column types to be used in a switch statement
 */
typedef enum field { FILENO, SIZE, LINKS, USER, GROUP } field_t;

/*
 * prints the long listing of the file
 * if -l flag is used
 */
void printdetails(struct stat stats, int link_len, int user_len, int group_len, int size_len);

/*
 * compares two files based on case-insensitive alphabetical ordering of file name
 * similar to alphasort but case-insensitive
 */
int ialphasort(const void* a, const void* b);

/*
 * compares two files based on file size
 * if file sizes are the same then uses alphasort
 */
int sizesort(const void* a, const void* b);

/*
 * compares two files based on time last modified
 * if times are the same then uses alphasort
 */
int mtimesort(const void* a, const void* b);

/*
 * compares two files based on alphabetical ordering of file extension
 * if extensions are the same then uses alphasort
 */
int extsort(const void* a, const void* b);

/*
 * returns the stats of a file
 * uses lstat for symbolic links
 */
struct stat getstats(char* path);

/*
 * returns the stats of a file without passing full path
 */
struct stat getfilestats(char* name);

/*
 * returns a symbol representing the classification of a file
 * / - directory
 * @ - symbolic link
 * = - socket
 * | - FIFO
 * * - executable
 */
char getclass(struct stat stats);

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
char* modestr(mode_t mode_bits, char* mode_str);

/*
 * creates a time string in the form YYYY-MM-DD hh:mm
 */
char* timestr(time_t time, char* time_str);

/*
 * returns the maximum string length of a property of a file
 * this is used for padding when the long listing (-l) flag is used
 */
int maxlen(struct dirent** entries, int n, field_t f);

/*
 * returns a file owner's username
 */
char* getuser(struct stat stats);

/*
 * returns the file's group name
 */
char* getgroup(struct stat stats);

/*
 * returns the extension from a filename
 * (everything after the last '.')
 */
char* getext(char* filename);
