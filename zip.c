#define _POSIX_C_SOURCE 200809L // required for strdup() on cslab

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* Use 16-bit code words */
#define NUM_CODES 65536

/* allocate space for and return a new string s+t */
char *strappend_str(char *s, char *t);

/* allocate space for and return a new string s+c */
char *strappend_char(char *s, char c);

/* look for string s in the dictionary
 * return the code if found
 * return NUM_CODES if not found 
 */
unsigned int find_encoding(char *dictionary[], char *s);

/* write the code for string s to file */
void write_code(int fd, char *dictionary[], char *s);

/* compress in_file_name to out_file_name */
void compress(char *in_file_name, char *out_file_name);

int main(int argc, char **argv)
{

    if (argc != 2)
    {
        printf("Usage: zip file\n");
        exit(1);
    }

    char *in_file_name = argv[1];
    char *out_file_name = strappend_str(in_file_name, ".zip");

    compress(in_file_name, out_file_name);

    /* have to free the memory for out_file_name since strappend_str malloc()'ed it */
    free(out_file_name);

    return 0;
}

/* allocate space for and return a new string s+t */
char *strappend_str(char *s, char *t)
{
    if (s == NULL || t == NULL)
    {
        return NULL;
    }

    // reminder: strlen() doesn't include the \0 in the length
    int new_size = strlen(s) + strlen(t) + 1;
    char *result = (char *)malloc(new_size*sizeof(char));
    strcpy(result, s);
    strcat(result, t);

    return result;
}

/* allocate space for and return a new string s+c */
char *strappend_char(char *s, char c)
{
    if (s == NULL)
    {
        return NULL;
    }

    // reminder: strlen() doesn't include the \0 in the length
    int new_size = strlen(s) + 2;
    char *result = (char *)malloc(new_size*sizeof(char));
    strcpy(result, s);
    result[new_size-2] = c;
    result[new_size-1] = '\0';

    return result;
}

/* look for string s in the dictionary
 * return the code if found
 * return NUM_CODES if not found 
 */
unsigned int find_encoding(char *dictionary[], char *s)
{
    if (dictionary == NULL || s == NULL)
    {
        return NUM_CODES;
    }

    for (unsigned int i=0; i<NUM_CODES; ++i)
    {
        /* code words are added in order, so if we get to a NULL value 
         * we can stop searching */
        if (dictionary[i] == NULL)
        {
            break;
        }

        if (strcmp(dictionary[i], s) == 0)
        {
            return i;
        }
    }
    return NUM_CODES;
}

/* write the code for string s to file */
void write_code(int fd, char *dictionary[], char *s)
{
    if (dictionary == NULL || s == NULL)
    {
        return;
    }

    unsigned int code = find_encoding(dictionary, s);
    // should never call write_code() unless s is in the dictionary 
    if (code == NUM_CODES)
    {
        printf("Algorithm error!");
        exit(1);
    }

    // cast the code to an unsigned short to only use 16 bits per code word in the output file
    unsigned short actual_code = (unsigned short)code;
    if (write(fd, &actual_code, sizeof(unsigned short)) != sizeof(unsigned short))
    {
        perror("write");
        exit(1);
    }
}

/* compress in_file_name to out_file_name */
void compress(char *in_file_name, char *out_file_name)
{
    	char *dictionary[NUM_CODES];
	int fd = open(in_file_name, O_RDONLY);
	if (fd == -1){
		printf("open failed");
	}	

	if (in_file_name[0] == '\0'){
		fd = 0;
	}

	int fd2 = open(out_file_name, O_WRONLY|O_CREAT|O_TRUNC, 0644);

	//Pseudocode Convert
	
	//Initialize Dictionary
	for (int i = 0; i < 256; ++i){
		char h = (char)i;
		dictionary[i] = strappend_char("", h);
	}
	
	
	char c;
	char *currentstring = strappend_char("", c);
	unsigned int position = 256;
	while (read(fd, &c, sizeof(unsigned char)) > 0){
		char *copy = strdup(currentstring);
		free(currentstring);
		char *together = strappend_char(copy, c);
		if (find_encoding(dictionary, together) != NUM_CODES){
			currentstring = strdup(together);
		}
		else{
			write_code(fd2, dictionary, copy);
			if (position < NUM_CODES){
				dictionary[position] = strdup(together);
				++position;
			}
			currentstring = strappend_char("", c);
		}
		free(copy);
		free(together);
	}
	
	close(fd);
	close(fd2);
		
	return;
}
