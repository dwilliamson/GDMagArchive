#ifndef INC_DM_NAMES_H

typedef struct st_Name Name;

extern void   dmNamesInit(void);
extern Name * dmStringID(char *str);
extern char * dmStringFromID(Name *id);

#endif