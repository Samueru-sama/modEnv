#ifndef ENVS_H
#define ENVS_H

#include <stdbool.h>

typedef bool (*ConditionFunc)(void);

typedef struct {
  const char* env;
  const char* value;
  ConditionFunc condition;
} ConditionalEnv;

extern ConditionalEnv conditional_envs[];

char** create_modified_envp(char* const envp[]);
void free_envp(char** envp);

#endif
